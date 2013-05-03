#
# \file TestPerformanceModel.py
# \author Andrew Kerr <arkerr@gatech.edu>
# \date July 20, 2011
#
# \brief demonstrates the PerformanceModel class
#

import PCA
import PerformanceModel
import API
import math
import numpy as np

if __name__ == "__main__":
	
	def TestPerformanceModel(trainingset, \
							 experimentset, \
							 regressionType, \
							 tvar=None, \
							 maxt=None, \
							 mvar=None, \
							 maxm=None, 
							 dofigure=False, \
							 simple=True, \
							 dbname="eiger_development", \
							 dbuser="eric", \
							 dbpassword="", \
							 dsets=None, \
							 mID=None, \
							 metricset=None, \
							 doscatter=False, \
							 plotmachinescree=False, \
							 plottrainingscree=False, \
							 plotmachinePCsPerMetric=False, \
							 plottrainingPCsPerMetric=False, \
							 plotmachineMetricsPerPC=False, \
							 plottrainingMetricsPerPC=False, \
							 dumpmodel=None, \
							 maxM=None, \
							 max_interactions=None, \
							 commit=False):
		"""
		inputs:
			trainingset - ID of training datacollection
			experimentset - ID of experiment datacollection
			regressionType - string describing type of regression. either 'linear' or 'mars',case insensitive
			tvar - percent of total variance to capture in profile PCA
			maxt - maximum number of profile principal components to retain from PCA
			mvar - percent of total variance to capture in machine PCA
			maxm - maximum number of machine principal components to retain from PCA
			dofigure - if true, plots a bar graph of expected vs predicted performance of all trials in experiment datacollection
			simple - boolean, if false perform more complex stargazer-style regression search
			dbname - name of the database to connect to
			dbuse - username to connect to database
			dbpassword - password to connect to database
			dsets - subsets of datasets to perform the model construction on
			mID - ID of the model to retrieve from the database instead of constructing new
			metricset - set of metric IDs to include in the model generation. if None uses all metrics.
				NOTE: metricset MUST include the result metric ID if not None!
			doscatter - if true, plot the actual performance vs the predicted. Good plots should all be along line y = x
			plotmachinescree - if true, draw scree plot from machine PCA
			plottrainingscree - if true, draw scree plot from training PCA
			plotmachinePCsPerMetric - if true, plot machine metrics broken down by PCs they load on
			plottrainingPCsPerMetric - if true, plot training metrics broken down by PCs they load on
			plotmachineMetricsPerPC - if true, plot machine PCs broken down by which metrics load them
			plottrainingMetricsPerPC - if true, plot training PCs broken down by which metrics load them
			dumpmodel - dumps model to the given filename if not None
			maxM - maximum number of functions in MARS models
			max_interactions - max degree of functions in MARS models
			commit - if true, commit this performancemodel to the database
		"""
		
		d = API.Connect(user=dbuser, password=dbpassword, db=dbname)

		experimentData = API.DataCollection()
		
		if(mID != None):
			experimentData.fromDatabase(experimentset, d, datasets=dsets)
			performance = PerformanceModel.PerformanceModel(modelID=mID, db=d)
		else:
			trainingData = API.DataCollection()
			trainingData.fromDatabase(trainingset, d, datasets=dsets, metricset=metricset)
			performance = PerformanceModel.PerformanceModel()
			performance.train(trainingData,regressionType,trainingVariance=tvar,maxTrainingComponents=maxt,machineVariance=mvar,maxMachineComponents=maxm, rotateTraining=True, simple_regression=simple, maxM=maxM, max_interactions=max_interactions)

			if(commit):
				performance.commit(d)
				print "Model ID: %s" % (performance.model.modelID)
			if(trainingset != experimentset):
				experimentData.fromDatabase(experimentset, d)
			else:
				experimentData = trainingData

		"""
		EXTRA PLOTTING FUNCTIONS
		"""
		if(plotmachinescree):
			PCA.PlotScree(performance.trainingMachinePCA.loadings, log=True, title="Machine PCA Scree Plot")
		if(plottrainingscree):
			PCA.PlotScree(performance.trainingPCA.loadings, log=True, title="Training PCA Scree Plot")
		if(plotmachinePCsPerMetric):
			PCA.PlotPCsPerMetric(performance.machinePCs, trainingData.machineMetrics, title="Machine PCs Per Metric")
		if(plottrainingPCsPerMetric):
			PCA.PlotPCsPerMetric(performance.trainingPCs, trainingData.profileMetrics, title="Training PCs Per Metric")
		if(plotmachineMetricsPerPC):
			PCA.PlotMetricsPerPC(performance.machinePCs, trainingData.machineMetrics, title="Machine Metrics Per PC")
		if(plottrainingMetricsPerPC):
			PCA.PlotMetricsPerPC(performance.trainingPCs, trainingData.profileMetrics, title="Training Metrics Per PC")

		experiment = experimentData.project(performance.trainingPCs, performance.machinePCs)
		prediction = performance.predict(experiment)

		if(dumpmodel != None):
			fid = open(dumpmodel, 'w')
			performance.toFile(fid)
			fid.close()

		print "--- DONE ---"

		percentError = [math.fabs(p - a) / a * 100 for p,a in zip(prediction.flat, experimentData.performance)]
		print "Average Percent Error: %f" % (np.average(percentError),)
		print "Min: %f  Max: %f  StdDev: %f" % (min(percentError),max(percentError), np.std(percentError))
		if(False):
			import matplotlib.pyplot as plt
			plt.hist(percentError, 100, normed=1, range=(0, 100))
			plt.xlabel('Percent Error')
			plt.ylabel('Percent of Kernels')
			plt.show()

		if(dofigure):
			from pylab import bar, plot, show, xlabel, ylabel, xticks, legend, figure, title
			figure()
			xx = range(0, prediction.shape[0])
			rects0 = []
			rects1 = []
			if(prediction.shape[0] == 1):
				rects0 = bar([0,1],[prediction[0], 0], .4, color='blue')
				rects0 = [rects0[0]]
				rects1 = bar([0.4,1.4], [experimentData.performance[0], 0], .4, color='black')
				rects1 = [rects1[1]]
			else:
			 	foo = [f for f in prediction]
			 	baz = [f[0] for f in experimentData.performance]
				rects0 = bar([x for x in xx], foo, 0.4, color='blue')
				rects1 = bar([x + 0.4 for x in xx], baz, 0.4, color='black')
			xticks([x + 0.4 for x in xx], [app.name for app in experimentData.getApplications()], rotation='90')

			legend((rects0[0], rects1[0]), ('Predicted', 'Actual'), loc='upper left')
			title('Performance of applications versus prediction')	
			show()
		if(doscatter):
			from pylab import figure, plot, scatter, show, xlabel, ylabel, xticks, legend, title
			figure()
			scatter([x for x in experimentData.performance],[y for y in prediction])
			xlabel('Performance of experiment datacollection')
			ylabel('Predicted performance of experiment datacollection')
			show()
		return True
	
	params = {
		"trainingset" : 1, 
		"experimentset": 1, 
		"tvar" : None, 
		"maxt" : None, 
		"mvar" : None, 
		"maxm" : None, 
		"dofigure" : True, 
		"simple" : True, 
		"dbname" : "ben", 
		"dbuser" : "eric", 
		"dbpassword" : "", 
		"dsets" : None, 
		"mID" : 4, 
		"metricset" : None, 
		"doscatter" : True, 
		"plotmachinescree" : False, 
		"plottrainingscree" : False,  
		"plotmachinePCsPerMetric" : False, 
		"plottrainingPCsPerMetric" : False, 
		"plotmachineMetricsPerPC" : False, 
		"plottrainingMetricsPerPC" : False,
		"dumpmodel" : None,
		"regressionType" : 'Linear',
		"maxM" : 3,
		"max_interactions" : 2,
		"commit" : False
	}
	TestPerformanceModel(**params)
#	TestPerformanceModel(1,1,maxt=3,dofigure=False, simple=True, dbname="ben", dsets=None, mID=None)
#	TestPerformanceModel(1,1,dofigure=False, simple=True, dbname="ben2", metricset=None, doscatter=True)
#	TestPerformanceModel(1,1,maxt=3,dofigure=False, simple=False, dbname="eiger", dsets=[x for x in range(1,26)])
#

