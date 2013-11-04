# \file API.py
# \author Andrew Kerr <arkerr@gatech.edu>
# \date 
# \brief models a data collection and associated classes
#
# What is Eiger? http://www.youtube.com/watch?v=PEYiSeDr1UE
#

import copy
import numpy as np
import MySQLdb

####################################################################################################

def Connect(dblocation, dbname, username, password):
    return MySQLdb.connect(host=dblocation, user=username, passwd=password, db=dbname)

def sortedDictionary(_dict):
    return [_dict[key] for key in sorted(_dict.keys())]

class Metric:
    """
    Represents a uniquely named metric in a data collection
    """
    
    def __init__(self, ID = 0, datatype = 'other', name = '', description = '', db = None):
        self._ID = ID

        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT type, name, description 
                FROM metrics WHERE ID=%s""" % (ID,))
            row = cursor.fetchone()
            cursor.close()
            self.datatype = row[0]
            self.name = row[1]
            self.description = row[2]
        else:
            self.datatype = datatype
            self.name = name
            self.description = description
    
    def __str__(self):
        return "(ID: %s, datatype: %s, name: %s, description: %s)" % (self._ID, self.datatype, self.name, self.description)

    def commit(self,db):
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO metrics(type,name,description) VALUES("%s","%s","%s")""" % (self.datatype, self.name, self.description))
        self._ID = db.insert_id()

class NonDeterministicMetric:
    """
    Represents an instance of a nondeterministic metric associated with a metric
    """

    def __init__(self, executionID = 0, metricID = 0,  metric = 0):
        self.executionID = executionID 
        self.metricID = metricID
        self.metric = metric

    def __str__(self):
        return "(executionID: %s, metricID: %s) - Value: %s" % (self.executionID, self.metricID, self.metric)

    def commit(self,db):
        cursor = db.cursor()
        cursor.execute("""INSERT INTO nondeterministic_metrics(executionID, metricID, metric) VALUES(%s,%s,%s)""" % \
            (self.executionID, self.metricID, self.metric))
    
class DeterministicMetric:
    """
    Represents an instance of a deterministic metric associated with a dataset
    """

    def __init__(self, datasetID = 0, metricID = 0,  metric = 0):
        self.datasetID = datasetID
        self.metricID = metricID
        self.metric = metric

    def __str__(self):
        return "(datasetID: %s, metricID: %s) - Value: %s" % (self.datasetID, self.metricID, self.metric)

    def commit(self,db):
        cursor = db.cursor()
        cursor.execute("""INSERT INTO deterministic_metrics(datasetID, metricID, metric) VALUES(%s,%s,%s)""" % \
            (self.datasetID, self.metricID, self.metric))

class MachineMetric:
    """
    Represents an instance of a metric associated with a machine
    """

    def __init__(self, machineID = 0, metricID = 0, metric = 0):
        self.machineID = machineID
        self.metricID = metricID
        self.metric = metric

    def __str__(self):
        return "(machineID: %s, metricID: %s) - Value: %s" % (self.machineID, self.metricID, self.metric)

    def commit(self,db):
        cursor = db.cursor()
        cursor.execute("""SELECT COUNT(*) FROM machine_metrics WHERE machineID=%s AND metricID=%s""" % (self.machineID, self.metricID))
        row = cursor.fetchone()
        if(row[0] == 0):
            cursor.execute("""INSERT INTO machine_metrics(machineID, metricID, metric) VALUES(%s,%s,%s)""" % \
                (self.machineID, self.metricID, self.metric))

class Execution:
    """
    Describes a run of instrumentation that makes up one trial
    """
    def __init__(self, ID = 0, trialID = 0, machineID = 0, db = None):
        self._ID, self.trialID, self.machineID = (ID, trialID, machineID)

        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT trialID, machineID FROM executions where ID=%s""" % (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.trialID = row[0]
            self.machineID = row[1]

    def __str__(self):
        return "(ID: %s, trialID: %s, machineID: %s)" % (self._ID, self.trialID, self.machineID)

    def commit(self, db):
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO executions(trialID, machineID) VALUE(%s,%s)""" % (self.trialID, self.machineID))
        self._ID = db.insert_id()

class Trial:
    """
            metricID = data[0]
            metric = data[1]
    Stores data produced during a particular run of an application on particular hardware.
    """
    def __init__(self, ID = 0, machineID = 0, applicationID = 0, datasetID = 0, propertiesID = 0, dataCollectionID = 0, db = None):
        self._ID, self.machineID, self.applicationID, self.datasetID, self.propertiesID, self.dataCollectionID = \
            (ID, machineID, applicationID, datasetID, propertiesID, dataCollectionID)
        self.nondeterministicProfile = ([],[])
        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT dataCollectionID, machineID, applicationID, datasetID, propertiesID
                FROM trials WHERE ID=%s""" % (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.dataCollectionID = row[0]
            self.machineID = row[1]
            self.applicationID = row[2]
            self.datasetID = row[3]
            self.propertiesID = row[4]

    def __str__(self):
        return "(ID: %s, dataCollectionID: %s, machineID: %s, applicationID: %s)" % (self._ID, self.dataCollectionID, self.machineID, self.applicationID)
    
    def commit(self,db):
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO trials(dataCollectionID,machineID,applicationID,datasetID,propertiesID) 
            VALUE(%s,%s,%s,%s,%s)""" % (self.dataCollectionID,self.machineID,self.applicationID,self.datasetID,self.propertiesID))
        self._ID = db.insert_id()

class Properties:
    """
    Describes any extra properties pertaining to a particular trial
    """
    def __init__(self, ID = 0, name = "", trialID = 0, propertyID = 0, db = None):
        self._ID = ID
        self.name = name
        self.trialID = trialID
        self.propertyID = propertyID
        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT trialID, propertyName, property FROM properties where ID=%s""" % (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.trialID = row[0]
            self.name = row[1]
            self.propertyID = row[2]

    def __str__(self):
        return "(ID: %s, Name: %s, Description: %s)" % (self._ID, self.name, self.description)

    def commit(self,db):
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO properties(trialID, propertyName, property) 
            VALUE(%s,"%s",%s)""" % (self.trialID,self.name,self.propertyID))
        self._ID = db.insert_id()

class Machine:
    """
    Describes a particular execution environment
    """
    def __init__(self, ID = 0, name = "", description = "", metrics = {}, db = None):
        self._ID = ID
        self.name = name
        self.description = description
        self.metrics = metrics
        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT name, description
                FROM machines WHERE ID=%s""" % (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.name = row[0]
            self.description = row[1]
    
    def metric(self):
        return [self.metrics[metricID] for metricID in sorted(self.metrics.keys())]
    
    def __str__(self):
        return "(ID: %s, Name: %s, Description: %s) - Value: %s" % (self._ID, self.name, self.description, self.metric())

    def commit(self,db):
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO machines(name,description) VALUES("%s","%s")""" % (self.name, self.description))
        self._ID = db.insert_id()

class Application:
    """
    Describes an application being executed
    """
    def __init__(self, ID = 0, name = "", description = "", datasets = [], db = None):
        self._ID = ID
        self.name = name
        self.description = description
        self.datasets = datasets
        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT name, description
                FROM applications WHERE ID=%s""" % (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.name = row[0]
            self.description = row[1]
    
    def __str__(self):
        return "(ID: %s, Name: %s, Description: %s)" % (self._ID, self.name, self.description)

    def commit(self,db):
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO applications (name,description) VALUES("%s","%s")""" % (self.name, self.description))
        self._ID = db.insert_id()
        
class Dataset:
    """
    Describes the input data on which an application is run
    """
    def __init__(self, ID = 0, applicationID = 0, name = '', description = '', created = '', url = '', db = None):
        self._ID = ID
        self.applicationID = applicationID
        self.name = name
        self.description = description
        self.created = created
        self.url = url
        self.deterministicProfile = ([],[])
        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT applicationID, name, description, created, url
                FROM datasets WHERE ID=%s""" % (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.applicationID = row[0]
            self.name = row[1]
            self.description = row[2]
            self.created = row[3]
            self.url = row[4]
    
    def __str__(self):
        return "(ID: %s, Application ID: %s, Name: %s, Description: %s, Created: %s, URL: %s)" % \
                (self._ID, self.applicationID, self.name, self.description, self.created, self.url)
    
    def commit(self,db):
        cursor = db.cursor()
        if(self.created == ''):
            createtime = 'NOW()'
        else:
            createtime = self.created
        cursor.execute("""INSERT IGNORE INTO datasets (applicationID,name,description,created,url) 
            VALUES(%s,"%s","%s",%s,"%s")""" % (self.applicationID, self.name, self.description, createtime, self.url))

        self._ID = db.insert_id()
        cursor.execute("""SELECT created from datasets WHERE ID=%s""" % (self._ID,))
        row = cursor.fetchone()
        self.created = row[0]

class DataCollection:
    """
    DataCollection represents the contents of a run consisting of
    
    profile - an N-by-M matrix (N: trials, M: metrics) of recorded metrics
    machine - a U-by-V matrix (U: each machine, V: each machine metric) describing the machines
    performance - an N-by-1 vector of performance results
    applications - an array of application names
    trials - an N-by-3 matrix selecting (machine, application name, identifier)
        machine - indicates a particular row in self.machine
        application name - indicates a particular row in self.applications
        identifier - indicates a particular row in self.identifier
    identifier - an array uniquely identifying a particular trial
    """
    
    #
    #
    def __init__(self, ID = 0, name = '', description = '', created = '', db = None):
        self._ID = ID
        self.name = name
        self.description = description
        self.created = created
        
        self.machines = {}
        self.applications = {}
        self.trials = {}
        self.datasets = {}
        self.metrics = {}
        if(self._ID != 0):
            self.fromDatabase(ID, db)
    
    def __str__(self):
        return "(ID: %s, Name: %s, Description: %s, Created: %s)" % (self._ID, self.name, self.description, self.created)

    #
    #
    def commit(self, db):
        """
        Commits just this DataCollection to the eiger database.
        """
        cursor = db.cursor()
        if(self.created == ''):
            createtime = 'NOW()'
        else:
            createtime = self.created
        cursor.execute("""INSERT IGNORE INTO datacollections(name,description,created) 
            VALUES("%s","%s",%s)""" % (self.name, self.description, createtime))

        self._ID = db.insert_id()
        cursor.execute("""SELECT created from datacollections WHERE ID=%s""" % (self._ID,))
        row = cursor.fetchone()
        self.created = row[0]
    
    #
    #
    def getProfile(self):
        """
        Returns (profile, metrics, usedTrials)
        
        profile - an N-by-M matrix (N: trials, M: metrics) of recorded metrics
        metrics - an M-list of metric descriptors
        usedTrials - an N list of trials taken from full trial pool to make this profile
        """
        profile = []
        metrics = []
        usedTrials = []
        for trialID in sorted(self.trials.keys()):
            trial = self.trials[trialID]
            dataset = self.datasets[trial.datasetID]

            tempMetrics = [self.metrics[metricID] for metricID in dataset.deterministicProfile[0]] + \
                          [self.metrics[metricID] for metricID in trial.nondeterministicProfile[0] if self.metrics[metricID].datatype != 'result']

            if(len(tempMetrics) > len(metrics)):
                metrics = tempMetrics
                profile = []
                usedTrials = []

            if(len(tempMetrics) == len(metrics) and all(t._ID == m._ID for t,m in zip(tempMetrics, metrics))):
                trialProfile = dataset.deterministicProfile[1] + \
                               [metric for metricID,metric in zip(trial.nondeterministicProfile[0], trial.nondeterministicProfile[1]) if self.metrics[metricID].datatype != 'result']   
        
                profile.append(trialProfile)
                usedTrials.append(trialID)
            
        return (np.matrix(profile), metrics, usedTrials)
    
    #
    #
    def getMachine(self):
        """
        Returns (machine, metrics)
        
        machine - a U-by-V matrix (U: each machine, V: each machine metric) describing the machines
        metrics - a V-list of metric descriptors
        """
        
        profile = []
        metrics = None
        for machineID in sorted(self.machines.keys()):
            machine = self.machines[machineID]
            profile.append(machine.metric())
            if metrics == None:
                metrics =  [self.metrics[metricID] for metricID in sorted(machine.metrics.keys())]
        profile = np.matrix(profile)
        return (profile, metrics)
    
    #
    #
    def selectMachine(self, projection = None):
        """
        Replicates rows of the machine enough times to concatenate with profile matrix
        
        projection - 
        If specified, projects the machine matrix via V-by-P matrix returned by PCA. 
        Uses self.trials[:,0] to construct N-by-P matrix containing machine parameters
        """
        
        profile = []
        for trialID in self.usedTrials:
            trial = self.trials[trialID]
            dataset = self.datasets[trial.datasetID]
            trialProfile = sortedDictionary(self.machines[trial.machineID].metrics)
            profile.append(trialProfile)
        
        profile = np.matrix(profile)
        if projection != None:
            profile = profile * projection
        return profile
    
    #
    #
    def getPerformance(self):
        """
        Returns (performance, metrics)
        
        performance - an N-by-V matrix of performance results
        metrics - 
        """
        profile = []
        metrics = None
        for trialID in self.usedTrials:
            trial = self.trials[trialID]
            
            dataset = self.datasets[trial.datasetID]
            trialProfile = [metric for metricID, metric in zip(trial.nondeterministicProfile[0], trial.nondeterministicProfile[1])
                    if self.metrics[metricID].datatype == 'result' ]
            if metrics == None:
                metrics = [self.metrics[metricID] for metricID in trial.nondeterministicProfile[0]
                    if self.metrics[metricID].datatype == 'result' ]
            profile.append(trialProfile)
        
        profile = np.matrix(profile)
        return (profile, metrics)
    
    #
    #
    def getApplications(self):
        """
        Returns
        
        applications - an array of application names sorted by trials
        """
        applications = []
        for trialID in self.usedTrials:
            applications.append(self.applications[self.trials[trialID].applicationID])
        return applications
    
    #
    #   
    def fromDatabase(self, ID, db, datasets=None, metricset=None, trialset=None, machineset=None, appset=None):
        """
        Selects a DataCollection instance from a database
        """

        cursor = db.cursor()
        cursor.execute("""SELECT name, description, created 
            FROM datacollections WHERE ID=%s""" % (ID,))
        row = cursor.fetchone()
        self._ID = ID
        self.name = row[0]
        self.description = row[1]
        self.created = row[2]

        command = "SELECT t1.metricID, t1.metric, \
                   t3.ID, t3.machineID, t3.applicationID, t3.datasetID, t3.propertiesID \
                   FROM nondeterministic_metrics AS t1 \
                   JOIN executions AS t2 \
                   ON t1.executionID = t2.ID \
                   JOIN trials AS t3 \
                   ON t3.ID = t2.trialID \
                   WHERE t3.dataCollectionID = %s" % (self._ID)

        if(datasets != None):
            command = command + " AND t3.datasetID IN("
            for x in datasets:
                command = command + "%s," % (x,)
            command = command[0:len(command)-1]
            command = command + ")"

        if(trialset != None):
            command = command + " AND t3.ID IN("
            for x in trialset:
                command = command + "%s," % (x,)
            command = command[0:len(command)-1]
            command = command + ")"

        if(machineset != None):
            command = command + " AND t3.machineID IN("
            for x in machineset:
                command = command + "%s," % (x,)
            command = command[0:len(command)-1]
            command = command + ")"

        if(appset != None):
            command = command + " AND t3.applicationID IN("
            for x in appset:
                command = command + "%s," % (x,)
            command = command[0:len(command)-1]
            command = command + ")"

        if(metricset != None):
            command = command + " AND t1.metricID IN("
            for x in metricset:
                command = command + "%s," % (x,)
            command = command[:len(command)-1] + ")"

        cursor.execute(command)
        trials = cursor.fetchall()
        cursor.close()
        for row in trials:
            metricID = row[0]
            metric = row[1]
            trialID = row[2]
            machineID = row[3]
            applicationID = row[4]
            datasetID = row[5]
            propertiesID = row[6]
            
            if trialID not in self.trials:
                self.trials[trialID] = Trial(trialID, machineID, applicationID, datasetID, propertiesID,db=db)
            
            if datasetID not in self.datasets:
                self.datasets[datasetID] = self._loadDataset(db, datasetID, metricset)

            if applicationID not in self.applications:
                self.applications[applicationID] = self._loadApplication(db, applicationID)
            
            if metricID not in self.trials[trialID].nondeterministicProfile[0]:
                self.trials[trialID].nondeterministicProfile[0].append(metricID)
                self.trials[trialID].nondeterministicProfile[1].append(metric)

            if metricID not in self.metrics:
                self.metrics[metricID] = self._loadMetric(db, metricID)

            if machineID not in self.machines:
                self.machines[machineID] = copy.deepcopy(self._loadMachine(db, machineID))


    
        self.profile, self.applicationMetrics, self.usedTrials = self.getProfile()
        self.machine, self.machineMetrics = self.getMachine()
        self.performance, self.performanceMetrics = self.getPerformance()
        
        return self

    def project(self, applicationPCs, machinePCs):
        """
        Construct the concatenated, projected profile of application and machine metrics on their respective PC space
        Returns matrix of projected metrics onto PC space
        """
        projectedApplication = self.profile * applicationPCs
        projectedMachine = self.selectMachine(machinePCs)   
        return np.hstack((projectedApplication, projectedMachine))
    
    #
    #
    def _loadDataset(self, db, datasetID, metricset=None):
        cursor = db.cursor()
        cursor.execute("""SELECT ID, applicationID, name, description, created, url 
            FROM datasets WHERE ID=%s""" % (datasetID,))
        row = cursor.fetchone()
        dataset = Dataset(row[0], row[1], row[2], row[3], row[4], db=db)
        
        cmd = "SELECT metricID, metric FROM deterministic_metrics WHERE datasetID = %s" % (datasetID,)
        if(metricset != None):
            cmd = cmd + " AND metricID IN("
            for x in metricset:
                cmd = cmd + "%s," % (x,)
            cmd = cmd[:len(cmd)-1] + ")"
        cmd = cmd + " ORDER BY metricID"
        cursor.execute(cmd)
        data = cursor.fetchone()
        while data:
            metricID = data[0]
            metric = data[1]
            if metricID not in dataset.deterministicProfile[0]:
                dataset.deterministicProfile[0].append(metricID)
                dataset.deterministicProfile[1].append(metric)
            data = cursor.fetchone()
        for metricID in dataset.deterministicProfile[0]:
            if metricID not in self.metrics:
                self.metrics[metricID] = self._loadMetric(db, metricID)
        cursor.close()
        return dataset
    
    #
    #
    def _loadApplication(self, db, applicationID):
        cursor = db.cursor()
        cursor.execute("""SELECT ID, name, description 
            FROM applications WHERE ID=%s""" % (applicationID, ))
        row = cursor.fetchone()
        cursor.close()
        return Application(row[0], row[1], row[2], db=db)
    
    #
    #
    def _loadMetric(self, db, metricID):
        cursor = db.cursor()
        cursor.execute("""SELECT ID, type, name, description 
            FROM metrics WHERE ID=%s""" % (metricID,))
        row = cursor.fetchone()
        cursor.close()
        return Metric(row[0], row[1], row[2], row[3], db=db)
    
    #
    #
    def _loadMachine(self, db, machineID):
        cursor = db.cursor()
        cursor.execute("""SELECT ID, name, description FROM machines WHERE ID=%s""" % (machineID,))
        row = cursor.fetchone()
        machine = Machine(row[0], row[1], row[2], db=db)
        cursor.execute("""SELECT machine_metrics.metric, machine_metrics.metricID FROM machine_metrics
            LEFT JOIN metrics ON machine_metrics.metricID = metrics.ID 
            WHERE machine_metrics.machineID = %s""" % (machineID,))
        rows = cursor.fetchall()
        for row in rows:
            metricID = row[1]
            machine.metrics[metricID] = row[0]
            if metricID not in self.metrics:
                    self.metrics[metricID] = self._loadMetric(db, metricID)

        cursor.close()
        return machine

    def _printMachines(self):
        print "Printing machines:"
        for machineID in sorted(self.machines.keys()):
            print self.machines[machineID]
        print ""




####################################################################################################

if __name__ == "__main__":
    def TestDataCollection():
        collection = DataCollection()
        collection.fromDatabase(1, Connect(db='foo'))
        
        profile,metrics = collection.getMachine()
        print "machine profile:"
        print profile
        print "machine metrics:"
        for metric in metrics:
            print metric
            
        profile,metrics = collection.getProfile()
        print "profile:"
        print profile
        print "metrics:"
        for metric in metrics:
            print metric
    
        profile,metrics = collection.getPerformance()
        print "performance profile:"
        print profile
        print "performance metrics:"
        for metric in metrics:
            print metric
    
        profile = collection.selectMachine()
        print "select machine profile:"
        print profile
    
    def Test2():
        d = Connect(db='foo')


    TestDataCollection()
    #Test2()

#
