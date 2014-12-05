# \file API.py
# \author Andrew Kerr <arkerr@gatech.edu>
# \date 
# \brief models a data collection and associated classes
#
# What is Eiger? http://www.youtube.com/watch?v=PEYiSeDr1UE
#

import copy
import numpy as np
import sqlite3


def Connect(db):
    return sqlite3.connect(db)


def sortedDictionary(_dict):
    return [_dict[key] for key in sorted(_dict.keys())]


class Metric:
    """
    Represents a uniquely named metric in a data collection
    """
    
    def __init__(self, ID=0, datatype='other', name='', description='', db=None):
        self._ID = ID

        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT type, name, description 
                FROM metrics WHERE ID=?""", (ID,))
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
        return "(ID: %s, datatype: %s, name: %s, description: %s)" % \
                (self._ID, self.datatype, self.name, self.description)

    def commit(self,db):
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO metrics(type,name,description) """
                       """VALUES(?,?,?)""", 
                       (self.datatype, self.name, self.description))
        self._ID = db.insert_id()


class NonDeterministicMetric:
    """
    Represents an instance of a nondeterministic metric associated with a metric
    """

    def __init__(self, executionID=0, metricID=0,  metric=0):
        self.executionID = executionID 
        self.metricID = metricID
        self.metric = metric

    def __str__(self):
        return "(executionID: %s, metricID: %s) - Value: %s" % \
                (self.executionID, self.metricID, self.metric)

    def commit(self, db):
        cursor = db.cursor()
        cursor.execute("""INSERT INTO nondeterministic_metrics"""
                       """(executionID, metricID, metric) VALUES(?,?,?)""", 
                       (self.executionID, self.metricID, self.metric))
    

class DeterministicMetric:
    """
    Represents an instance of a deterministic metric associated with a dataset
    """

    def __init__(self, datasetID=0, metricID=0, metric=0):
        self.datasetID = datasetID
        self.metricID = metricID
        self.metric = metric

    def __str__(self):
        return "(datasetID: %s, metricID: %s) - Value: %s" % \
                (self.datasetID, self.metricID, self.metric)

    def commit(self, db):
        cursor = db.cursor()
        cursor.execute("""INSERT INTO deterministic_metrics"""
                       """(datasetID, metricID, metric) VALUES(?,?,?)""", 
                       (self.datasetID, self.metricID, self.metric))


class MachineMetric:
    """
    Represents an instance of a metric associated with a machine
    """

    def __init__(self, machineID=0, metricID=0, metric=0):
        self.machineID = machineID
        self.metricID = metricID
        self.metric = metric

    def __str__(self):
        return "(machineID: %s, metricID: %s) - Value: %s" % \
                (self.machineID, self.metricID, self.metric)

    def commit(self, db):
        cursor = db.cursor()
        cursor.execute("""SELECT COUNT(*) FROM machine_metrics """
                       """WHERE machineID=? AND metricID=?""",
                       (self.machineID, self.metricID))
        row = cursor.fetchone()
        if(row[0] == 0):
            cursor.execute("""INSERT INTO machine_metrics"""
                           """(machineID, metricID, metric) VALUES(?,?,?)""",
                           (self.machineID, self.metricID, self.metric))


class Execution:
    """
    Describes a run of instrumentation that makes up one trial
    """
    def __init__(self, ID=0, trialID=0, machineID=0, db=None):
        self._ID, self.trialID, self.machineID = (ID, trialID, machineID)

        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT trialID, machineID FROM executions """
                           """WHERE ID=?""", (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.trialID = row[0]
            self.machineID = row[1]

    def __str__(self):
        return "(ID: %s, trialID: %s, machineID: %s)" % \
                (self._ID, self.trialID, self.machineID)

    def commit(self, db):
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO executions(trialID, machineID) """
                       """VALUE(?,?)""",
                       (self.trialID, self.machineID))
        self._ID = db.insert_id()


class Trial:
    """
            metricID = data[0]
            metric = data[1]
    Stores data produced during a particular run of an application on 
    particular hardware.
    """
    def __init__(self, ID=0, machineID=0, applicationID=0, datasetID=0, 
                 dataCollectionID=0, db=None):
        self._ID = ID
        self.machineID = machineID
        self.applicationID = applicationID
        self.datasetID = datasetID
        self.dataCollectionID = dataCollectionID
        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT dataCollectionID, machineID, """
                           """applicationID, datasetID, """
                           """FROM trials WHERE ID=?""", (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.dataCollectionID = row[0]
            self.machineID = row[1]
            self.applicationID = row[2]
            self.datasetID = row[3]

    def __str__(self):
        return "(ID: %s, dataCollectionID: %s, machineID: %s, applicationID: %s)" % \
                (self._ID, self.dataCollectionID, self.machineID, 
                 self.applicationID)
    
    def commit(self, db):
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO trials(dataCollectionID, """
                       """machineID, applicationID, datasetID) """
                       """VALUE(?,?,?,?,?)""", 
                       (self.dataCollectionID, self.machineID, 
                        self.applicationID, self.datasetID))
        self._ID = db.insert_id()


class Machine:
    """
    Describes a particular execution environment
    """
    def __init__(self, ID=0, name="", description="", db=None):
        self._ID = ID
        self.name = name
        self.description = description
        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT name, description FROM machines """
                           """WHERE ID=?""", (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.name = row[0]
            self.description = row[1]
    
    def __str__(self):
        return "(ID: %s, Name: %s, Description: %s)" % \
                (self._ID, self.name, self.description)

    def commit(self, db):
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO machines(name, description) """
                       """VALUES(?,?)""",
                       (self.name, self.description))
        self._ID = db.insert_id()


class Application:
    """
    Describes an application being executed
    """
    def __init__(self, ID=0, name="", description="", db = None):
        self._ID = ID
        self.name = name
        self.description = description
        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT name, description FROM applications """
                           """WHERE ID=?""", (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.name = row[0]
            self.description = row[1]
    
    def __str__(self):
        return "(ID: %s, Name: %s, Description: %s)" % \
                (self._ID, self.name, self.description)

    def commit(self,db):
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO applications (name, description) """
                       """VALUES(?,?)""",
                       (self.name, self.description))
        self._ID = db.insert_id()
        

class Dataset:
    """
    Describes the input data on which an application is run
    """
    def __init__(self, ID=0, applicationID=0, name='', description='', 
                 created='', url='', db=None):
        self._ID = ID
        self.applicationID = applicationID
        self.name = name
        self.description = description
        self.created = created
        self.url = url
        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT applicationID, name, description, """
                           """created, url FROM datasets WHERE ID=?""", 
                           (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.applicationID = row[0]
            self.name = row[1]
            self.description = row[2]
            self.created = row[3]
            self.url = row[4]
    
    def __str__(self):
        return "(ID: %s, Application ID: %s, Name: %s, Description: %s, Created: %s, URL: %s)" % \
                (self._ID, self.applicationID, self.name, self.description, 
                 self.created, self.url)
    
    def commit(self, db):
        cursor = db.cursor()
        if(self.created == ''):
            createtime = 'NOW()'
        else:
            createtime = self.created
        cursor.execute("""INSERT IGNORE INTO datasets """
                       """(applicationID, name, description, created, url) """
                       """VALUES(?,?,?,?,?)""", 
                       (self.applicationID, self.name, self.description, 
                        createtime, self.url))

        self._ID = db.insert_id()
        cursor.execute("""SELECT created from datasets WHERE ID=?""", (self._ID,))
        row = cursor.fetchone()
        self.created = row[0]


class DataCollection:
    """
    DataCollection represents the contents of many trial runs.
    """
    def __init__(self, ID=0, name='', description='', created='', db=None):
        self._ID = ID
        self.name = name
        self.description = description
        self.created = created
        
        if(self._ID != 0):
            cursor = db.cursor()
            cursor.execute("""SELECT name, description, created """
                           """FROM datacollections WHERE ID=?""", (self._ID,))
            row = cursor.fetchone()
            cursor.close()
            self.name = row[0]
            self.description = row[1]
            self.created = row[2]
    
    def __str__(self):
        return "(ID: %s, Name: %s, Description: %s, Created: %s)" % \
                (self._ID, self.name, self.description, self.created)

    def commit(self, db):
        """
        Commits just this DataCollection to the eiger database.
        """
        cursor = db.cursor()
        if(self.created == ''):
            createtime = 'NOW()'
        else:
            createtime = self.created
        cursor.execute("""INSERT IGNORE INTO datacollections"""
                       """(name, description, created) VALUES(?,?,?)""", 
                       (self.name, self.description, createtime))

        self._ID = db.insert_id()
        cursor.execute("""SELECT created from datacollections WHERE ID=?""", 
                       (self._ID,))
        row = cursor.fetchone()
        self.created = row[0]
    

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

