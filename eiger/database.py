import numpy as np
import sqlite3

def getModels(database_name, source_name=None):
    """Get list of all models in the given database file."""

    conn = sqlite3.connect(database_name)
    cmd = 'SELECT models.ID, models.description, models.created, model_sources.name FROM models JOIN model_sources ON model_sources.ID = models.source_id'
    if source_name != None:
        cmd = cmd + ' WHERE model_sources.name = "' + str(source_name) + '"'
    cursor = conn.execute(cmd)
    # The format of the results is [ID, description, created, source_name]
    return cursor.fetchall()

def addModelFromFile(database_name, model_file, source_name, description=''):
    """Add a model file to DB"""

    conn = sqlite3.connect(database_name)
    cursor = conn.execute('SELECT ID from model_sources where name = "' + str(source_name) + '"')
    source_id = cursor.fetchone()
    if source_id is None:
        conn.execute('INSERT INTO model_sources(name) VALUES(' + str(source_name) + ')')
        cursor = conn.execute('SELECT ID from model_sources where name = "' + str(source_name) + '"')
        source_id = cursor.fetchone()
    with open(model_file, 'rb') as input_file:
        ablob = input_file.read()
        cmd = 'INSERT INTO models(description,source_id,data) VALUES("' + str(description) + '",' + str(source_id[0]) + ',?)'
        conn.execute(cmd, [sqlite3.Binary(ablob),])
        conn.commit()

def dumpModelToFile(database_name, model_file, ID):
    """Dump model from DB to file"""

    conn = sqlite3.connect(database_name)
    row = conn.execute('SELECT data FROM models WHERE ID = ' + str(ID)).fetchone()
    with open(model_file, 'wb') as output_file:
        output_file.write(row[0])

def getDataCollections(database_name):
    """Get list of data collections in the given database file."""

    conn = sqlite3.connect(database_name)
    cursor = conn.execute('SELECT name, description from datacollections')
    return cursor.fetchall()


class DataCollection:
    """Container object for all the data in a single collection.
    
    Members:
        name -> Name of this data collection.
        apps -> All applications used in trials of this data collection.
                Of the form [(name, desc, [row idxs])].
        machines -> All machines trials were executed on.
                    Of the form [(name, desc, [row idxs])].
        datasets -> All input datasets to applications of this data collection.
                    Of the form [(name, desc, [row idxs])].
        metrics -> The set up metrics describing all trials, datasets, and
                   machines.  Of the form [(name, desc, type)].
        profile -> Two dimensional array of all data. Each row is a trial and 
                   each column corresponds to a metric in the metrics vector 
                   with the same index.
    """

    def __init__(self, name='', database_name=None):
        """Read this object from database if supplied."""
        self.name = name
        self._trial_id_map = {}
        self.apps = [] #[(name, desc, [row idxs])]
        self.machines = [] #[(name, desc, [row idxs])]
        self.datasets = [] #[(name, desc, [row idxs])]
        self.metrics = [] #[(name, desc, type)]
        self.profile = np.empty((0,0)) #[num trials, num metrics]

        if database_name is not None:
            self._load(database_name)

    def metricIndexByType(self, *args):
        """ Return column index corresponding to each metric type listed in args """
        return [idx for idx, x in enumerate(self.metrics) \
                if x[2] in args]

    def metricIndexByName(self, names):
        """ Return column index corresponding to each metric name"""
        return [idx for idx, x in enumerate(self.metrics) \
                if x[0] in names]

    def _load(self, database_name):
        """Load this data collection from the given database."""
        db = sqlite3.connect(database_name)
        cursor = db.cursor()

        cursor.execute('SELECT ID FROM datacollections WHERE name=? ',
                       (self.name,))
        my_id = cursor.fetchone()[0]

        # create consistent mapping of trialID to profile index
        cursor.execute('SELECT ID from trials where dataCollectionID=?', 
                       (my_id,))
        self._trial_id_map = {trialid[0]: index for index,trialid \
                in enumerate(cursor.fetchall())}

        self.apps = self._loadObject(db, my_id, "application")
        self.machines = self._loadObject(db, my_id, "machine")
        self.datasets = self._loadObject(db, my_id, "dataset")

        # create consistent mapping of metricID to profile index
        cursor.execute('SELECT DISTINCT mets.name,mets.description,mets.type '
                       'FROM nondeterministic_metrics as ndm '
                       'JOIN metrics as mets '
                       'ON mets.ID = ndm.metricId '
                       'JOIN trials as tr '
                       'ON ndm.trialID = tr.ID '
                       'WHERE tr.dataCollectionID = ? '
                       'UNION '
                       'SELECT DISTINCT mets.name,mets.description,mets.type '
                       'FROM deterministic_metrics as dm '
                       'JOIN metrics as mets '
                       'ON mets.ID = dm.metricID '
                       'JOIN datasets as ds '
                       'ON dm.datasetID = ds.ID '
                       'JOIN trials as tr '
                       'ON ds.ID = tr.datasetID '
                       'WHERE tr.dataCollectionID = ? '
                       'UNION '
                       'SELECT DISTINCT mets.name,mets.description,mets.type '
                       'FROM machine_metrics as mm '
                       'JOIN metrics as mets '
                       'ON mets.ID = mm.metricID '
                       'JOIN machines as mach '
                       'ON mm.machineID = mach.ID '
                       'JOIN trials as tr '
                       'ON mach.ID = tr.machineID '
                       'WHERE tr.dataCollectionID = ?',
                       (my_id, my_id, my_id))
        self.metrics = [x for x in cursor.fetchall()]
        
        # get all data now
        n_trials = len(self._trial_id_map)
        n_mets = len(self.metrics)
        self.profile = np.empty((n_trials,n_mets))
        for idx, (name, desc, mtype) in enumerate(self.metrics):
            cursor.execute('SELECT t.ID,dm.metric '
                           'FROM deterministic_metrics as dm '
                           'JOIN datasets as ds '
                           'ON dm.datasetID = ds.ID '
                           'JOIN metrics as mets '
                           'ON dm.metricID = mets.ID '
                           'JOIN trials as t '
                           'ON t.datasetID = ds.ID '
                           'WHERE t.dataCollectionID = ? '
                           'AND mets.name = ? '
                           'UNION '
                           'SELECT tr.ID,ndm.metric '
                           'FROM nondeterministic_metrics as ndm '
                           'JOIN metrics as mets '
                           'ON ndm.metricID = mets.ID '
                           'JOIN trials as tr '
                           'ON ndm.trialID = tr.ID '
                           'WHERE tr.dataCollectionID = ? '
                           'AND mets.name = ? '
                           'UNION '
                           'SELECT t.ID,mm.metric '
                           'FROM machine_metrics as mm '
                           'JOIN machines as mach '
                           'ON mm.machineID = mach.ID '
                           'JOIN metrics as mets '
                           'ON mm.metricID = mets.ID '
                           'JOIN trials as t '
                           'ON t.machineID = mach.ID '
                           'WHERE t.dataCollectionID = ? '
                           'AND mets.name = ?',
                           (my_id, name, my_id, name, my_id, name))
            for (trial, value) in cursor.fetchall():
                self.profile[self._trial_id_map[trial], idx] = value
        cursor.close()

    def _loadObject(self, db, my_id, identifier):
        """Load the top level objects.
        
        Load top level objects (machine, application, dataset) for this
        data collection from a database.
        """
        cursor = db.cursor()
        destination = []
        command = 'SELECT DISTINCT tbl.name, tbl.description ' \
                  'FROM {0}s as tbl ' \
                  'JOIN trials ON trials.{0}ID=tbl.ID ' \
                  'WHERE trials.dataCollectionID=?'.format(identifier)
        cursor.execute(command, (my_id,))
        for (name, desc) in cursor.fetchall():
            command = 'SELECT trials.ID ' \
                      'FROM {0}s as tbl JOIN trials ON trials.{0}ID=tbl.ID ' \
                      'WHERE trials.dataCollectionID=? ' \
                      'AND tbl.name=? AND tbl.description=?'.format(identifier)
            cursor.execute(command, (my_id, name, desc))
            destination.append((name, desc, 
                                [self._trial_id_map[trialID[0]] \
                                 for trialID in cursor.fetchall()]))
        return destination


if __name__ == "__main__":
    d = DataCollection(name='hpccg', database_name='database/byfl.db')

