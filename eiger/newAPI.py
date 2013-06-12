class DataCollection:

	def __init__(self, name, db):
		""" load a data collection from db """
		self.profile = np.ndarray((0,0))

		cursor.execute("""SELECT name,description FROM applications \
				JOIN trials ON trials.applicatioinID = applications.ID \
				WHERE trials.dataCollectionID = %s""" (dcid,))
		self.apps = {app: np.ndarray(0,) for app in cursor.fetchall()}
		
		cursor.execute("""SELECT name,description FROM machines  \
				JOIN trials ON trials.machineID  = machines.ID \
				WHERE trials.dataCollectionID = %s""" (dcid,))
		self.machines = {machine: np.ndarray(0,) \
				for machine in cursor.fetchall()}

		cursor.execute("""SELECT name,description FROM datasets  \
				JOIN trials ON trials.datasetID  = datasets.ID \
				WHERE trials.dataCollectionID = %s""" (dcid,))
		self.datasets = {dataset: np.ndarray(0,) \
				for dataset in cursor.fetchall()}

		# gets all metrics, not just ones in this data collection.
		# TODO: eliminate unused metrics
		cursor.execute("""SELECT name,description,type FROM metrics""")
		self.metrics = {metric: np.ndarray(0,) \
				for metric in cursor.fetchall()}
		
		# create consistent mapping of trialID to profile index
		cursor.execute("""SELECT ID from trials where \
				dataCollectionID = %s""" % (dcid,))
		self._trial_id_map = {trialid[0]: index for index,trialid \
				in enumerate(cursor.fetchall()}

		# create consistent mapping of metricID to profile index
		cursor.execute("""SELECT ndm.metricID \
				FROM nondeterministic_metrics as ndm \
				JOIN executions as ex \
				ON ndm.executionID = ex.ID \
				JOIN trials as tr \
				ON ex.trialID = tr.ID \
				where tr.dataCollectionID = %s""" % (dcid,))
		self._nondet_metric_id_map = {metid[0]: index for index,metid \
				in enumerate(cursor.fetchall()}
		
		# get all data now
		cursor.execute("""SELECT ds.name,mets.name,dm.metric \
				FROM deterministic_metrics as dm \
				JOIN datasets as ds \
				ON dm.datasetID = ds.ID \
				JOIN trials as t \
				ON t.datasetID = ds.ID \
				where t.dataCollectionID = %s""" % (dcid,))
		self._det_metrics = {dset: {met: val} for (dset,met,val) \
				in cursor.fetchall()}
		
		cursor.execute("""SELECT ma.name,mets.name,mm.metric \
				FROM machine_metrics as mm \
				JOIN machines as ma \
				ON mm.machineID = ma.ID \
				JOIN trials as t \
				ON t.machineID = ma.ID \
				where t.dataCollectionID = %s""" % (dcid,))
		self._machine_metrics = {machine: {met: val} for (machine,met,val) \
				in cursor.fetchall()}
	
		n_trials = len(self._trial_id_map)
		n_nondetmets = len(self._nondet_metric_id_map)
		self._nondet_metrics = np.ndarray((n_trials,n_nondetmets))
		cursor.execute("""SELECT tr.ID,ndm.metricID,ndm.metric \
				FROM nondeterministic_metrics as ndm \
				JOIN executions as ex \
				ON ndm.executionID = ex.ID \
				JOIN trials as tr \
				ON ex.trialID = tr.ID \
				where tr.dataCollectionID = %s""" % (dcid,))
		for (trial,metric,value) in cursor.fetchall():
			self._nondet_metrics[ \
					self._trial_id_map[trial], \
					self._nondet_metric_id_map[metric]] = value

	@property
	def profile(self):
		""" expand out compressed form of profile into a real matrix """
		pass

