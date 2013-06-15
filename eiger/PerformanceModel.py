#
# \file PerformanceModel.py
# \author Andrew Kerr <arkerr@gatech.edu>
# \date June 27, 2011
#
# \brief given a cluster of training data and several outputs, computes a polynomial performance
#   model
#  
import scipy.linalg as linalg
import numpy as np
import math
import MySQLdb
import tempfile
import subprocess
import os

import PCA
import MARS
import LinearRegression
import NearestNeighbors

class PerformanceModel:
    """
    Object for selecting a performance model from a DataCollection
    """
    #
    def __init__(self, modelID=None, db=None):
        """
        Creates new PerformanceModel and populates it from the database if one is given
        """
        if(modelID != None):
            self.fromDatabase(modelID, db)

    def train(self, training, regressionType=None, applicationVariance=None, machineVariance=None, maxApplicationComponents=None, maxMachineComponents=None, rotateApplication=None, rotateMachine=None, maxM=40, max_interactions=2,threshold=None, k=None):
        """
        Trains the PerformanceModel instance given a profile of training data and results
        """

        self.training = training

        # a list describing, in order, the metrics describing the input to the performance model
        self.inputMetrics = self.training.applicationMetrics + self.training.machineMetrics
        
        # perform principal component analysis on training data and machine profile

        self.applicationPCA = PCA.PCA(self.training.profile, rotate=rotateApplication,scale=True)
        self.machinePCA = PCA.PCA(self.training.machine, rotate=rotateMachine)
        
        self.machinePCs, self.machineStdDev, self.machineComponents, self.machineVariance = self.machinePCA.reduced(machineVariance,maxMachineComponents)
        self.applicationPCs, self.applicationStdDev, self.applicationComponents, self.applicationVariance = self.applicationPCA.reduced(applicationVariance, maxApplicationComponents)
        
        
        # project training data and machine parameters onto PCs
        self.projectedTrainingProfile = self.training.project(self.applicationPCs,self.machinePCs)
        
        # perform performance modeling only if requested
        if(regressionType is None):
            return
        elif(regressionType.lower() == "linear"):
            """ LinearRegression version"""
            self.regression = LinearRegression.LinearRegression(self.projectedTrainingProfile, self.training.performance)
#           modelPool = self.regression.quadraticPlusCrossTerms(self.projectedTrainingProfile.shape, self.machineComponents)
            modelPool = self.regression.powerLadder(self.projectedTrainingProfile.shape)
#           modelPool = self.regression.linearPool(self.projectedTrainingProfile.shape)
            self.model, self.rsquared, self.trials = self.regression.select(modelPool, threshold=threshold)
        elif(regressionType.lower() == "mars"):
            """ MARS version """
            self.model = MARS.MARS(np.array(self.projectedTrainingProfile),np.array(self.training.performance), max_interactions=max_interactions, maxM=maxM)
        elif(regressionType.lower() == "nearest"):
            """ k-NearestNeighbors version """
            self.model = NearestNeighbors.NearestNeighbors(self.projectedTrainingProfile, self.training.performance, k)
        else:
            raise ValueError("Invalid regression type")
        self.regressionType = regressionType.lower()
        
    #
    def predict(self, experiment):
        """
        Given experimental metrics, evalutes and returns predicted performance.
        """
        return np.array([self.model.poll(x) for x in np.array(experiment)])

    def commit(self, db):
        """
        Commit this performance model to the database. Includes the result of PCA.
        """

        cursor = db.cursor()
        try:
            cursor.execute("""INSERT IGNORE INTO eiger_principal_component_analysis(dataCollectionID, machinePC, machineWeight, trainingPC, trainingWeight) VALUES("%s", "%s", "%s", "%s", "%s")""" % (self.training._ID, self.machinePCA.components.tolist(), self.machinePCA.loadings.tolist(), self.applicationPCA.components.tolist(), self.applicationPCA.loadings.tolist()))
            self.PCAID = db.insert_id()

            self.model.commit(db,self.training._ID,self.applicationComponents, self.machineComponents)

        except MySQLdb.Error, e:
            print "Error %d: %s" % (e.args[0], e.args[1])
            raise

    def toFile(self, fid):
        """
        Serializes the performance model to the given file descriptor
        fid - file descriptor to write to.

        File format is as follows:
            [<# rows in PCA matrix>,<# cols in PCA matrix]((<val00>,<val01>,...),(<val10>,<val11>...),...)
            [<#functions in model>](<beta1,beta2,...)
            <encoded functions in model, separated by newline>
            <name of each input metric for the model, separated by newline>
        """
        
        n1,p1 = self.applicationPCs.shape if self.applicationPCs is not None else (0,0)
        n2,p2 = self.machinePCs.shape if self.machinePCs is not None else (0,0)
        fid.write("[%s,%s]" % (n1+n2,p1+p2))
        fid.write("(")
        for i in range(n1+n2):
            fid.write("(")
            for j in range(p1+p2):
                if(i<n1 and j<p1):
                    val = self.applicationPCs[i,j]
                elif(i>n1 and j>p1):
                    val = self.machinePCs[i-n1,j-p1]
                else:
                    val = 0.0
                fid.write("%s," % (val,))
            fid.write("),")
        fid.write(")\n")

        self.model.toFile(fid)

        for x in self.inputMetrics:
            fid.write("%s\n" % (x.name,))

    def fromDatabase(self, modelID, db):
        """
        Retrieves model identified by modelID from a database.
        """
        self.modelID = modelID
        cursor = db.cursor()
        cursor.execute("""SELECT dataCollectionID, trainingPCs, machinePCs FROM eiger_model WHERE ID=%s""" % (modelID,))
        row = cursor.fetchone()
        self.dataCollectionID = row[0]
        self.applicationPCCount = row[1]
        self.machinePCCount = row[2]

        cursor.execute("""SELECT trainingPC, machinePC from eiger_principal_component_analysis WHERE dataCollectionID = %s""" % (self.dataCollectionID,))
        row = cursor.fetchone()
        self.applicationPCs = np.array(eval(row[0]))[:,:self.applicationPCCount] if row[0] != '[]' else None
        self.machinePCs = np.array(eval(row[1]))[:,:self.machinePCCount] if row[1] != '[]' else None

        cursor.execute("""SELECT regression from eiger_model where ID = %s""" % (modelID,))
        row = cursor.fetchone()
        self.regressionType = row[0]

        if(self.regressionType == "linear"):
            self.model = LinearRegression.Model(None,None,modelID=modelID,db=db)
        elif(self.regressionType == "mars"):
            self.model = MARS.MARS(None,None,None,None,modelID=modelID,db=db)
        else:
            raise ValueError("invalid regression type from database")




####################################################################################################
#
if __name__ == "__main__":
    pass
####################################################################################################
