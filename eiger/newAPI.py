import numpy as np
import MySQLdb as mdb

def getDataCollections(*args, **kwargs):
	db = mdb.connect(*args, **kwargs)
	cursor = db.cursor()
	cursor.execute("""SELECT name, description from datacollections""")
	return [dc for dc in cursor.fetchall()]


class DataCollection:

	def __init__(self, name, *args, **kwargs):
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
		self.metrics = ([], {})
		for (idx, (name, desc, mtype)) in enumerate(cursor.fetchall()):
			if (name,desc,mtype) not in self.metrics[0]:
				self.metrics[0].append((name,desc,mtype))
			self.metrics[1][name] = idx
		
		# get all data now
		n_trials = len(self._trial_id_map)
		n_mets = len(self.metrics[1])
		self.profile = np.ndarray((n_trials,n_mets))
		cursor.execute(""" \
				SELECT t.ID,mets.name,dm.metric \
					FROM deterministic_metrics as dm \
					JOIN datasets as ds \
					ON dm.datasetID = ds.ID \
					JOIN metrics as mets \
					ON dm.metricID = mets.ID \
					JOIN trials as t \
					ON t.datasetID = ds.ID \
					WHERE t.dataCollectionID = %s \
				UNION \
				SELECT tr.ID,mets.name,ndm.metric \
					FROM nondeterministic_metrics as ndm \
					JOIN metrics as mets \
					ON ndm.metricID = mets.ID \
					JOIN executions as ex \
					ON ndm.executionID = ex.ID \
					JOIN trials as tr \
					ON ex.trialID = tr.ID \
					WHERE tr.dataCollectionID = %s \
				UNION \
				SELECT t.ID,mets.name,mm.metric \
					FROM machine_metrics as mm \
					JOIN machines as mach \
					ON mm.machineID = mach.ID \
					JOIN metrics as mets \
					ON mm.metricID = mets.ID \
					JOIN trials as t \
					ON t.machineID = mach.ID \
					WHERE t.dataCollectionID = %s""" % 
					(self._my_id, self._my_id, self._my_id))
		for (trial, name, value) in cursor.fetchall():
			self.profile[self._trial_id_map[trial], \
						 self.metrics[1][name]] = value
		cursor.close()

	def _loadObject(self, cursor, identifier):
		command = """SELECT trials.id,tbl.name,tbl.description \
				FROM %ss as tbl \
				JOIN trials ON trials.%sID = tbl.ID \
				WHERE trials.dataCollectionID = %s""" % \
				(identifier, identifier, self._my_id,)
		cursor.execute(command)
		destination = ([], {})
		for (trial,name,desc) in cursor.fetchall():
			if (name,desc) not in destination[0]:
				destination[0].append((name,desc))
			destination[1].setdefault(name, [])
			destination[1][name].append(self._trial_id_map[trial])
		return destination

if __name__ == "__main__":
	d = DataCollection('Nbuild', 
			host='localhost',user='root',passwd='root',db='minimd')
	pass

