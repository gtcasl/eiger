#!/usr/bin/python
#
# \file Eiger.py
# \author Eric Anger <eanger@gatech.edu>
# \date July 6, 2012
#
# \brief Command line interface into Eiger modeling framework
#
# \changes Added more plot functionality; Benjamin Allan, SNL 5/2013
#

import argparse
import matplotlib.pyplot as plt
import sys
import numpy as np
import math
import MySQLdb

from eiger import API, ClusterAnalysis, PCA, PerformanceModel

class Eiger:
    """
    Interface into Eiger model generation/polling/serialization/printing/etc.
    """
    def __init__(self, args):
        """
        args is dictionary containing all the appropriate arguments.
        """
        self.args = args

    def connect(self):
        """ Connecting to database """
        args = self.args
        params = {}
        params['username'] = args['database_user']
        params['password'] = args['database_password']
        params['dbname'] = args['database_name']
        params['dblocation'] = args['database_location']
        print "Connecting to database..."
        self.d = API.Connect(**params)

    def _fitModels(self):
        args = self.args
        self.models = []
        fullTrainingProfile = self.trainingData.profile
        fullTrainingPerformance = self.trainingData.performance
        fullUsedTrials = self.trainingData.usedTrials
        for cluster in self.clusters:

            self.trainingData.profile = fullTrainingProfile[cluster,:]
            self.trainingData.performance = fullTrainingPerformance[cluster,:]
            self.trainingData.usedTrials = [fullUsedTrials[i] for i in cluster]

            params = {}
            params['applicationVariance'] = args['application_pc_variance']
            params['maxApplicationComponents'] = args['max_application_pcs']
            params['machineVariance'] = args['machine_pc_variance']
            params['maxMachineComponents'] = args['max_machine_pcs']
            params['rotateApplication'] = args['rotate_application_pcs']
            params['rotateMachine'] = args['rotate_machine_pcs']
            params['maxM'] = args['mars_max_functions']
            params['max_interactions'] = args['mars_max_interactions']
            params['regressionType'] = args['regression_type']
            params['threshold'] = args['threshold']
            params['k'] = args['nearest_neighbors']

            model = PerformanceModel.PerformanceModel()
            model.train(self.trainingData, **params)
            self.models.append(model)
        self.trainingData.profile = fullTrainingProfile
        self.trainingData.performance = fullTrainingPerformance
        self.fullUsedTrials = self.trainingData.usedTrials

    def trainModels(self):
        """ Training new performance model(s) """

        print "Training model(s)..."
        args = self.args
        self.trainingData = API.DataCollection()
        params = {}
        params['datasets'] = args['training_dataset_ids']
        params['metricset'] = args['metric_ids']
        params['trialset'] = args['training_trial_ids']
        params['appset'] = args['training_application_ids']
        params['machineset'] = args['training_machine_ids']
        self.trainingData.fromDatabase(args['training_datacollection_id'], self.d, **params)

        params = {}
        params['maxIterations'] = args['max_cluster_iterations']
        params['k'] = args['clusters']
        self.kmeans = ClusterAnalysis.KMeans(self.trainingData.profile,**params)
        self.clusters = self.kmeans.kmeans()
        self._fitModels()

    def experiment(self):
        """ Run experiment against trained model(s) """
        if('experiment_datacollection_id' not in self.args or \
           self.args['experiment_datacollection_id'] is None):
            return

        print "Running experiments..."
        args = self.args
        params = {}
        params['trialset'] = args['experiment_trial_ids']
        params['appset'] = args['experiment_application_ids']
        params['machineset'] = args['experiment_machine_ids']
        params['metricset'] = args['metric_ids']
        self.experimentData = API.DataCollection()
        self.experimentData.fromDatabase(args['experiment_datacollection_id'], self.d, **params)
        try:
            closest = self.kmeans.closestCluster(self.experimentData.profile)
        except AttributeError:
            # kmeans doesn't exist, so no clusters
            closest = [0 for x in self.experimentData.usedTrials]
        self.prediction = []
        self.actual = np.matrix([[]]).T
        for i,model in enumerate(self.models):
            trials = [self.experimentData.usedTrials[trial] for trial in range(len(self.experimentData.usedTrials)) \
                     if closest[trial]==i]
            if(len(trials) == 0):
                continue
            params = {}
            params['trialset'] = trials
            params['metricset'] = args['metric_ids']
            clusterExperimentData = API.DataCollection()
            clusterExperimentData.fromDatabase(args['experiment_datacollection_id'], self.d, **params)

            experiment = clusterExperimentData.project(model.applicationPCs, model.machinePCs)
            self.prediction += model.predict(experiment).tolist()
            self.actual = np.vstack((self.actual,clusterExperimentData.performance))
        
        if(args['show_prediction_statistics']):
            act = [a[0,0] for a in self.actual]
            mse = sum([(a-p)**2 for a,p in zip(act, self.prediction)]) / len(act)
            rmse = math.sqrt(mse)
            mape = 100 * sum([abs((a-p)/a) for a,p in zip(act,self.prediction)]) / len(act)

            print "Number of experiment trials: %s" % len(act)
            print "Mean Average Percent Error: %s" % mape
            print "Mean Squared Error: %s" % mse
            print "Root Mean Squared Error: %s" % rmse

    def _figure(self):
        plt.figure()
        import os.path
        xx = range(0, len(self.prediction))
        rects0 = []
        rects1 = []
        if(len(self.prediction) == 1):
            rects0 = plt.bar([0,1],[self.prediction[0], 0], .4, color='blue')
            rects0 = [rects0[0]]
            rects1 = plt.bar([0.4,1.4], [self.experimentData.performance[0], 0], .4, color='black')
            rects1 = [rects1[1]]
        else:
            foo = [f for f in self.prediction]
            baz = [f[0] for f in self.actual]
            rects0 = plt.bar([x for x in xx], foo, 0.4, color='blue')
            rects1 = plt.bar([x + 0.4 for x in xx], baz, 0.4, color='black')
        plt.xticks([x + 0.4 for x in xx], [app.name for app in self.experimentData.getApplications()], rotation='90')
        plt.legend((rects0[0], rects1[0]), ('Predicted', 'Actual'), loc='upper left')
        if args['plot_identifier'] != 'NoName':
            plt.title(args['plot_identifier']+ " Eiger model (blue) and training (red)")
        else:
            plt.title('Performance of applications versus prediction')  
        if args['plot_dump'] == True:
            pfname=os.path.join(args['plot_dir'],args['plot_identifier']+'_eiger_bar.pdf')
            plt.savefig(pfname,format="pdf")
        else:
            plt.show()

    def _figureline(self):
        import os.path
        plt.figure()
        xx = range(0, len(self.prediction))
        rects0 = []
        rects1 = []
        if(len(self.prediction) == 1):
            rects0 = plt.bar([0,1],[self.prediction[0], 0], .4, color='blue')
            rects0 = [rects0[0]]
            rects1 = plt.bar([0.4,1.4], [self.experimentData.performance[0], 0], .4, color='black')
            rects1 = [rects1[1]]
        else:
            foo = [f for f in self.prediction]
            baz = [f[0,0] for f in self.actual]
            rects0 = plt.plot(baz,'r'+args['plot_line_marker_data'])
            rects1 = plt.plot(foo,'b'+args['plot_line_marker_pred'])

        if args['plot_identifier'] != 'NoName':
            plt.title(args['plot_identifier']+ " Eiger model (blue,"+args['plot_line_marker_pred']+") and training (red,"+args['plot_line_marker_data']+")")
        else:
            plt.title('Performance of applications versus prediction')  
        if args['plot_performance_log'] == True:
            plt.yscale('log')
        plt.xlabel('sample number')
        if args['plot_dump'] == True:
            pfname=os.path.join(args['plot_dir'],args['plot_identifier']+'_eiger_train.pdf')
            plt.savefig(pfname,format="pdf")
        else:
            plt.show()

    def _scatter(self):
        import os.path
        args = self.args
        plt.figure()
        plt.plot([x[0,0] for x in self.actual], [y for y in self.prediction], 'b'+args['plot_scatter_marker'])
        xmin=min(x[0,0] for x in self.actual)
        xmax=max(x[0,0] for x in self.actual)
        ymin=min(y for y in self.prediction)
        ymax=max(y for y in self.prediction)
        diagxmin=min(math.fabs(x[0,0]) for x in self.actual)
        diagymin=min(math.fabs(y) for y in self.prediction)
        diagpmin=min(diagxmin,diagymin)
        pmin=min(xmin,ymin)
        pmax=max(xmax,ymax)
        plt.plot([diagpmin,pmax],[diagpmin,pmax],'k-')
        if args['plot_identifier'] != 'NoName':
            plt.title(args['plot_identifier'])
        plt.xlabel('Walltime of experiment')
        plt.ylabel('Walltime of eiger fit')
        if args['plot_performance_log'] == True:
            plt.yscale('log')
            plt.xscale('log')
        if args['plot_scatter_free'] != True:
            plt.axes().set_aspect('equal')
        if args['plot_dump'] == True:
            pfname=os.path.join(args['plot_dir'],args['plot_identifier']+'_eiger_scatter.pdf')
            plt.savefig(pfname,format="pdf")
        else:
            plt.show()

    def _plotClustering(self):
        profilePCA = PCA.PCA(self.trainingData.profile)
        flattened = self.trainingData.profile * profilePCA.components[:,:2]
        plt.figure()
        colors = ['red','blue','green','orange','purple','brown','pink','grey','yellow','lime']
        for c,cluster in enumerate(self.clusters):
            plt.scatter([flattened[i,0] for i in cluster], [flattened[i,1] for i in cluster], marker='o', c = colors[c], s=40, label='Cluster ' + str(c))
        plt.xlabel('PCA 0')
        plt.ylabel('PCA 1')
        plt.title('Clustering')
        plt.legend(loc='best')
        plt.show()
        

    def plot(self):
        """ Visualize model details or performance """
        print "Visualizing..."
        args = self.args
        for i,model in enumerate(self.models):
            if(args['plot_machine_scree']):
                PCA.PlotScree(model.machinePCA.loadings, log=False, title="Machine PCA Scree Plot, Cluster %" + str(i))
            if(args['plot_application_scree']):
                PCA.PlotScree(model.applicationPCA.loadings, log=False)
            if(args['plot_machine_pcs_per_metric']):
                PCA.PlotPCsPerMetric(model.machinePCs, self.trainingData.machineMetrics, title="Machine PCs Per Metric, Cluster " + str(i))
            if(args['plot_application_pcs_per_metric']):
                PCA.PlotPCsPerMetric(model.applicationPCs, self.trainingData.applicationMetrics, title="Application PCs Per Metric, Cluster " + str(i))
            if(args['plot_machine_metrics_per_pc']):
                PCA.PlotMetricsPerPC(model.machinePCs, self.trainingData.machineMetrics, title="Machine Metrics Per PC, Cluster " + str(i))
            if(args['plot_application_metrics_per_pc']):
                PCA.PlotMetricsPerPC(model.applicationPCs, self.trainingData.applicationMetrics, title="Application Metrics Per PC, Cluster " + str(i))

        if(args['plot_performance_bar']):
            self._figure()
        if(args['plot_performance_line']):
            self._figureline()
        if(args['plot_performance_scatter']):
            self._scatter()
        if(args['plot_clustering']):
            self._plotClustering()

    def run(self):

        try:
            self.connect()

            # train new model
            self.trainModels()

            # run tests against trained model
            self.experiment()


            

            # visualize
            self.plot()
        except KeyError as e:
            print "ERROR: Missing parameter %s" % e
            sys.exit(1)


        ###############################

        # TODO: reading and writing models to db
        """
        if 'mID' in args:
            performance = PerformanceModel.PerformanceModel(modelID=args['mID'], db=d)
        if('commit' in args and args['commit']):
            performance.commit(d)
            print "Model ID: %s" % (performance.model.modelID)
        """
        # dump model to file
        if(self.args['output'] is not None):
            fid = open(self.args['output'], 'w')
            for i,model in enumerate(self.models):
                fid.write('Model %s\n' % i)
                model.toFile(fid)
            fid.close()

class Main:
    """ Main program to parse arguments from command line"""
    
    def main(self):
        parser = argparse.ArgumentParser(description = \
                'Command line interface into Eiger performance modeling framework \
                 for all model generation, polling, and serialization tasks.',
                 argument_default=None,
                 fromfile_prefix_chars='@')

        """
        CONNECTION ARGUMENTS
        """
        parser.add_argument('--database-name',
                            type=str,
                            help='Name of the database to connect to')
        parser.add_argument('--database-user',
                            type=str,
                            help='Username to connect to the database')
        parser.add_argument('--database-password',
                            type=str,
                            default='',
                            help='Password to connect to the database')
        parser.add_argument('--database-location',
                            type=str,
                            default='localhost',
                            help='IP address or hostname of database-hosting computer')

        """
        TRAINING DATA ARGUMENTS
        """
        parser.add_argument('--training-datacollection-id', '-t',
                            type=int,
                            help='ID of training data collection')
        parser.add_argument('--metric-ids',
                            type=int,
                            nargs='+',
                            help='If set, given metric IDs are the only ones included in modeling')
        parser.add_argument('--training-dataset-ids',
                            type=str,
                            nargs='+',
                            help='If set, given dataset IDs are the only ones included in model training')
        parser.add_argument('--training-trial-ids',
                            type=int,
                            nargs='+',
                            help='If set, given trial IDs are the only ones included in model training')
        parser.add_argument('--training-application-ids',
                            type=int,
                            nargs='+',
                            help='If set, given application IDs are the only ones included in model training')
        parser.add_argument('--training-machine-ids',
                            type=int,
                            nargs='+',
                            help='If set, given machine IDs are the only ones included in model training')


        """
        EXPERIMENT DATA ARGUMENTS
        """
        parser.add_argument('--experiment-datacollection-id', '-e',
                            type=int,
                            help='ID of experiment data collection')
        parser.add_argument('--application-dataset-ids',
                            type=str,
                            nargs='+',
                            help='If set, given dataset IDs are the only ones included in experimentation')
        parser.add_argument('--experiment-trial-ids',
                            type=int,
                            nargs='+',
                            help='If set, given trial IDs are the only ones included in experimentation')
        parser.add_argument('--experiment-application-ids',
                            type=int,
                            nargs='+',
                            help='If set, given application IDs are the only ones included in experiment')
        parser.add_argument('--experiment-machine-ids',
                            type=int,
                            nargs='+',
                            help='If set, given machine IDs are the only ones included in experiment')
        parser.add_argument('--show-prediction-statistics',
                            action='store_true',
                            default=False,
                            help='If set several statistics will be printed out describing the experimental prediction.')

        """
        PLOTTING ARGUMENTS
        """
        parser.add_argument('--plot-performance-bar',
                            action='store_true',
                            default=False,
                            help="If set, plots the exeriment data collection's actual and predicted performance.")
        parser.add_argument('--plot-performance-line',
                            action='store_true',
                            default=False,
                            help="If set, plots the exeriment data collection's actual and predicted performance.")
        parser.add_argument('--plot-performance-scatter',
                            action='store_true',
                            default=False,
                            help='If set, plots the experiment data collection actual vs predicted performance.')
        parser.add_argument('--plot-performance-log',
                            action='store_true',
                            default=False,
                            help='If set, scatter and line plots use log scale.')
        parser.add_argument('--plot-scatter-free',
                            action='store_true',
                            default=False,
                            help='If set, let scatter identity line wander from 45 degrees')
        parser.add_argument('--plot-scatter-marker',
                            choices=['.' , ',' , '+' , 'x' , 'o'],
                            default='.',
                            help='Change the scatter plot marker type')
        parser.add_argument('--plot-line-marker-pred',
                            choices=['.' , ',' , '+' , 'x' , 'o'],
                            default='x',
                            help='Change the marker type for prediction lines')
        parser.add_argument('--plot-line-marker-data',
                            choices=['.' , ',' , '+' , 'x' , 'o'],
                            default='+',
                            help='Change the marker type for training data lines')
        parser.add_argument('--plot-machine-scree',
                            action='store_true',
                            default=False,
                            help='If set, plots the scree graph from the machine PCA')
        parser.add_argument('--plot-application-scree',
                            action='store_true',
                            default=False,
                            help='If set, plots the scree graph from the application PCA')
        parser.add_argument('--plot-machine-pcs-per-metric',
                            action='store_true',
                            default=False,
                            help='If set, plots the breakdown of principal components per metric from machine PCA.')
        parser.add_argument('--plot-application-pcs-per-metric',
                            action='store_true',
                            default=False,
                            help='If set, plots the breakdown of principal components per metric from application PCA.')
        parser.add_argument('--plot-machine-metrics-per-pc',
                            action='store_true',
                            default=False,
                            help='If set, plots the breakdown of metrics per principal component from machine PCA.')
        parser.add_argument('--plot-application-metrics-per-pc',
                            action='store_true',
                            default=False,
                            help='If set, plots the breakdown of metrics per principal component from application PCA.')
        parser.add_argument('--plot-clustering',
                            action='store_true',
                            default=False,
                            help='If set, attempts to visualize clustering')
        parser.add_argument('--plot-identifier',
                            type=str,
                            default='NoName',
                            help='Name of the result for plot titles')

        parser.add_argument('--plot-dump',
                            action='store_true',
                            default=False,
                            help='If set, direct performance plots to files rather than screen.')
        parser.add_argument('--plot-dir',
                            default=".",
                            help='If set, direct plot files to directory other than cwd.')


        """
        MODEL CONSTRUCTION ARGUMENTS
        """
        parser.add_argument('--model-id',
                            type=int,
                            help='Model ID to be retrieved from the database')
        parser.add_argument('--output', '-o',
                            type=str,
                            help='Filename where this model should be saved to')
        parser.add_argument('--commit',
                            action='store_true',
                            default=False,
                            help='If set, commits this model to the database')
        parser.add_argument('--application-pc-variance',
                            type=float,
                            help='Portion of total variance to capture in application PCA out of 1')
        parser.add_argument('--max-application-pcs',
                            type=int,
                            help='Maximum number of components to retain from application PCA')
        parser.add_argument('--rotate-application-pcs',
                            action='store_true',
                            default=False,
                            help='If set, VARIMAX rotation is performed on application principal components')
        parser.add_argument('--machine-pc-variance',
                            type=float,
                            help='Percent of total variance to capture in machine PCA')
        parser.add_argument('--max-machine-pcs',
                            type=int,
                            help='Maximum number of components to retain from machine PCA')
        parser.add_argument('--rotate-machine-pcs',
                            action='store_true',
                            default=False,
                            help='If set, VARIMAX rotation is performed on machine principal components')
        parser.add_argument('--clusters', '-k',
                            type=int,
                            default=1,
                            help='Number of clusters for kmeans')
        parser.add_argument('--max-cluster-iterations',
                            type=int,
                            help='Maximum number of iterations to use when computing clusters')
        parser.add_argument('--regression-type', '-r',
                            type=str,
                            default='linear',
                            help='Type of regression: either "linear" or "mars"')
        parser.add_argument('--mars-max-functions',
                            type=int,
                            help='Maximum number of functions allowed in MARS regression')
        parser.add_argument('--mars-max-interactions', '-I',
                            type=int,
                            help='Maximum order of interactions allowed in MARS regression')
        parser.add_argument('--threshold',
                            type=float,
                            help='Cutoff threshold of increase in adjusted R-squared value when adding new predictors to the model')
        parser.add_argument('--nearest-neighbors',
                            type=int,
                            help='number of nearest neighbors for nearest regression')
        parser.add_argument('--input-filename',
                            type=str,
                            default='',
                            help='Name of file to read parameters from')
        
        
        
        args = vars(parser.parse_args())
        if args['input_filename'] != '':
            return vars(parser.parse_args(['@' + args['input_filename']]))
        return args

if __name__ == "__main__":
    
    args = Main().main()
    e = Eiger(args)
    e.run()
    pass

