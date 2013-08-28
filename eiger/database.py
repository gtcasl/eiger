import numpy as np
import MySQLdb as mdb

def getDataCollections(*args, **kwargs):
    db = mdb.connect(*args, **kwargs)
    cursor = db.cursor()
    cursor.execute("""SELECT name, description from datacollections""")
    return [dc for dc in cursor.fetchall()]


class DataCollection:

    def __init__(self, name=None, *args, **kwargs):

        self._trial_id_map = {}
        self.apps = [] #[(name, desc, [row idxs])]
        self.machines = [] #[(name, desc, [row idxs])]
        self.datasets = [] #[(name, desc, [row idxs])]
        self.metrics = [] #[(name, desc, type)]
        self.profile = np.ndarray((0,0)) #[num trials, num metrics]

        if name:
            self._load(name, *args, **kwargs)

    def metricIndexByType(self, *args):
        """ Return column index corresponding to each metric type listed in args """
        return [idx for idx, x in enumerate(self.metrics) \
                if x[2] in args]

    def _load(self, name, *args, **kwargs):
        """ load a data collection from db """
        db = mdb.connect(*args, **kwargs)
        cursor = db.cursor()

        cursor.execute("""SELECT ID FROM datacollections \
                WHERE name="%s" """ % (name,))
        self._my_id = cursor.fetchone()[0]

        # create consistent mapping of trialID to profile index
        cursor.execute("""SELECT ID from trials where \
                dataCollectionID = %s""" % (self._my_id,))
        self._trial_id_map = {trialid[0]: index for index,trialid \
                in enumerate(cursor.fetchall())}

        self.apps = self._loadObject(cursor, "application")
        self.machines = self._loadObject(cursor, "machine")
        self.datasets = self._loadObject(cursor, "dataset")

        # create consistent mapping of metricID to profile index
        cursor.execute(""" \
                SELECT DISTINCT mets.name,mets.description,mets.type \
                    FROM nondeterministic_metrics as ndm \
                    JOIN metrics as mets \
                    ON mets.ID = ndm.metricId \
                    JOIN executions as ex \
                    ON ndm.executionID = ex.ID \
                    JOIN trials as tr \
                    ON ex.trialID = tr.ID \
                    WHERE tr.dataCollectionID = %s \
                UNION \
                SELECT DISTINCT mets.name,mets.description,mets.type \
                    FROM deterministic_metrics as dm \
                    JOIN metrics as mets \
                    ON mets.ID = dm.metricID \
                    JOIN datasets as ds \
                    ON dm.datasetID = ds.ID \
                    JOIN trials as tr \
                    ON ds.ID = tr.datasetID \
                    WHERE tr.dataCollectionID = %s \
                UNION \
                SELECT DISTINCT mets.name,mets.description,mets.type \
                    FROM machine_metrics as mm \
                    JOIN metrics as mets \
                    ON mets.ID = mm.metricID \
                    JOIN machines as mach \
                    ON mm.machineID = mach.ID \
                    JOIN trials as tr \
                    ON mach.ID = tr.machineID \
                    WHERE tr.dataCollectionID = %s""" \
                    % (self._my_id, self._my_id, self._my_id))
        self.metrics = [x for x in cursor.fetchall()]
        
        # get all data now
        n_trials = len(self._trial_id_map)
        n_mets = len(self.metrics)
        self.profile = np.ndarray((n_trials,n_mets))
        for idx, (name, desc, mtype) in enumerate(self.metrics):
            cursor.execute(""" \
                    SELECT t.ID,dm.metric \
                        FROM deterministic_metrics as dm \
                        JOIN datasets as ds \
                        ON dm.datasetID = ds.ID \
                        JOIN metrics as mets \
                        ON dm.metricID = mets.ID \
                        JOIN trials as t \
                        ON t.datasetID = ds.ID \
                        WHERE t.dataCollectionID = %s \
                        && mets.name = "%s" \
                    UNION \
                    SELECT tr.ID,ndm.metric \
                        FROM nondeterministic_metrics as ndm \
                        JOIN metrics as mets \
                        ON ndm.metricID = mets.ID \
                        JOIN executions as ex \
                        ON ndm.executionID = ex.ID \
                        JOIN trials as tr \
                        ON ex.trialID = tr.ID \
                        WHERE tr.dataCollectionID = %s \
                        && mets.name = "%s" \
                    UNION \
                    SELECT t.ID,mm.metric \
                        FROM machine_metrics as mm \
                        JOIN machines as mach \
                        ON mm.machineID = mach.ID \
                        JOIN metrics as mets \
                        ON mm.metricID = mets.ID \
                        JOIN trials as t \
                        ON t.machineID = mach.ID \
                        WHERE t.dataCollectionID = %s \
                        && mets.name = "%s" """ % \
                    (self._my_id, name, self._my_id, name, self._my_id, name))
            for (trial, value) in cursor.fetchall():
                self.profile[self._trial_id_map[trial], idx] = value
        cursor.close()

    def _loadObject(self, cursor, identifier):
        destination = []
        command = """SELECT DISTINCT tbl.name, tbl.description \
                FROM %ss as tbl \
                JOIN trials ON trials.%sID = tbl.ID \
                WHERE trials.dataCollectionID = %s""" % \
                (identifier, identifier, self._my_id,)
        cursor.execute(command)
        for (name, desc) in cursor.fetchall():
            command = """SELECT trials.ID \
                    FROM %ss as tbl \
                    JOIN trials ON trials.%sID = tbl.ID \
                    WHERE trials.dataCollectionID = %s
                    && tbl.name = "%s" && tbl.description = "%s" """ % \
                    (identifier, identifier, self._my_id, name, desc)
            cursor.execute(command)
            destination.append((name, desc, 
                                [self._trial_id_map[trialID[0]] \
                                 for trialID in cursor.fetchall()]))
        return destination

if __name__ == "__main__":
    d = DataCollection('Nbuild', 
            host='localhost',user='root',passwd='root',db='minimd')
    pass

