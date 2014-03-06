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
import numpy as np
import math
import tempfile
import shutil
import os
from ast import literal_eval

from sklearn.cluster import KMeans
from eiger import database, PCA, LinearRegression

def run(args):
    """
    Interface into Eiger model generation/polling/serialization/printing/etc.
    """
    if args['input'] == None:
        print "Loading training data..."
        training_DC = database.DataCollection(args['training_datacollection'], 
                                              db=args['db'], 
                                              user=args['user'], 
                                              passwd=args['passwd'],
                                              host=args['host'])
        if(args['predictor_metrics'] is not None):
            metric_ids = training_DC.metricIndexByName(args['predictor_metrics'])
        else:
            metric_ids = training_DC.metricIndexByType('deterministic', 
                                                       'nondeterministic')
        metric_names = [training_DC.metrics[mid][0] for mid in metric_ids]
        try:
            training_profile = training_DC.profile[:,metric_ids]
        except IndexError:
            print "Unable to make model for empty data collection. Aborting..."
            return
        for idx,metric in enumerate(training_DC.metrics):
            if(metric[0] == args['performance_metric']):
                performance_metric_id = idx
        training_performance = training_DC.profile[:,performance_metric_id]

        #pca
        training_pca = PCA.PCA(training_profile)
        nonzero_components = training_pca.nonzeroComponents()
        ncols = np.shape(training_profile)[1]
        rotation_matrix = np.eye(ncols)[:,nonzero_components]
        rotated_training_profile = np.dot(training_profile, rotation_matrix)

        print "Visualizing PCA..."
        if(args['plot_scree']):
            print training_pca.loadings
            PCA.PlotScree(training_pca.loadings, log=False, 
                              title="PCA Scree Plot")
        if(args['plot_pcs_per_metric']):
            PCA.PlotPCsPerMetric(rotation_matrix, metric_names, 
                                 title="PCs Per Metric")
        if(args['plot_metrics_per_pc']):
            PCA.PlotMetricsPerPC(rotation_matrix, metric_names, 
                                 title="Metrics Per PC")
        #kmeans
        n_clusters = args['clusters']
        kmeans = KMeans(n_clusters)
        means = np.mean(rotated_training_profile, axis=0)
        stdevs = np.std(rotated_training_profile - means, axis=0, ddof=1)
        stdevs[stdevs==0.0] = 1.0
        clusters = kmeans.fit_predict((rotated_training_profile - means)/stdevs)

        # reserve a vector for each model created per cluster
        models = [0] * len(clusters)

        print "Modeling..."
        with tempfile.NamedTemporaryFile(delete=False) as modelfile:
            modelfile.write("%s\n%s\n" % (len(metric_names), '\n'.join(metric_names)))
            modelfile.write("[%s](%s)\n" % 
                    (len(means), ','.join([str(mean) for mean in means.tolist()])))
            modelfile.write("[%s](%s)\n" % 
                    (len(stdevs), ','.join([str(stdev) for stdev in stdevs.tolist()])))
            modelfile.write("[%s,%s]" % rotation_matrix.shape)
            modelfile.write("(%s)\n" % 
                            ','.join(["(%s)" % 
                                ','.join([str(elem) for elem in row]) 
                                for row in rotation_matrix.tolist()]))
            for i in range(n_clusters):
                cluster_profile = rotated_training_profile[clusters==i,:]
                cluster_performance = training_performance[clusters==i,:]
                regression = LinearRegression.LinearRegression(cluster_profile,
                                                               cluster_performance)
                pool = LinearRegression.powerLadderPool(cluster_profile.shape)
                pool = [LinearRegression.identityFunction()]
                for col in range(cluster_profile.shape[1]):
                    if('inv_quadratic' in args['regressor_functions']):
                        pool.append(LinearRegression.powerFunction(col, -2))
                    if('inv_linear' in args['regressor_functions']):
                        pool.append(LinearRegression.powerFunction(col, -1))
                    if('inv_sqrt' in args['regressor_functions']):
                        pool.append(LinearRegression.powerFunction(col, -.5))
                    if('sqrt' in args['regressor_functions']):
                        pool.append(LinearRegression.powerFunction(col, .5))
                    if('linear' in args['regressor_functions']):
                        pool.append(LinearRegression.powerFunction(col, 1))
                    if('quadratic' in args['regressor_functions']):
                        pool.append(LinearRegression.powerFunction(col, 2))
                    if('log' in args['regressor_functions']):
                        pool.append(LinearRegression.logFunction(col))
                    if('cross' in args['regressor_functions']):
                        for xcol in range(col, cluster_profile.shape[1]):
                            pool.append(LinearRegression.crossFunction(col, xcol))
                (models[i], r_squared) = regression.select(pool, 
                                                        threshold=args['threshold'],
                                                        folds=args['nfolds'])
                
                # dump model to file
                modelfile.write('Model %s\n' % i)
                modelfile.write("[%s](%s)\n" % (rotation_matrix.shape[1],
                                                ','.join([str(center) for center in
                                                    kmeans.cluster_centers_[i].tolist()])))
                modelfile.write(repr(models[i]))
                modelfile.write('\n') # need a trailing newline
                print "Index\tMetric Name"
                print '\n'.join("%s\t%s" % (i, metric_names[i]) for i in nonzero_components)
                print "Model:\n" + str(models[i])

                print "Finished modeling cluster %s: r squared = %s" % (i,r_squared)
           
        # if we want to save the model file, copy it now
        if args['output'] == True:
            shutil.move(modelfile.name, training_DC.name + '.model')
        else:
            os.remove(modelfile.name)
    else:
        lines = iter(open(args['input'],'r').read().splitlines())
        n_params = int(lines.next())
        metric_names = [lines.next() for i in range(n_params)]
        means = _stringToArray(lines.next())
        stdevs = _stringToArray(lines.next())
        rotation_matrix = _stringToArray(lines.next())
        models = []
        centroids = []
        try:
            while True:
                name = lines.next() # kill a line
                centroids.append(_stringToArray(lines.next()))
                weights = _stringToArray(lines.next())
                functions = [LinearRegression.stringToFunction(lines.next()) 
                             for i in range(weights.shape[0])]
                models.append(LinearRegression.Model(functions, weights))
        except StopIteration:
            pass
        kmeans = KMeans(len(centroids))
        kmeans.cluster_centers_ = np.array(centroids)

    if(args['experiment_datacollection'] or args['test_fit']):
        DC = args['experiment_datacollection'] if \
            args['experiment_datacollection'] else args['training_datacollection']
        print "Running experiment on data collection %s..." % \
              (DC,)
        experiment_DC = database.DataCollection(DC, 
                                                db=args['db'], 
                                                user=args['user'], 
                                                passwd=args['passwd'],
                                                host=args['host'])
        _runExperiment(kmeans, means, stdevs, models, rotation_matrix,
                       experiment_DC, args, metric_names)
    print "Done!"

def _stringToArray(string):
    """
    Parse string of form [len](number,number,number,...) to a numpy array.
    """
    just_values = string[string.find('('):]
    return np.array(literal_eval(just_values))

def _runExperiment(kmeans, means, stdevs, models, rotation_matrix, 
                   experiment_DC, args, metric_names):
    unordered_metric_ids = experiment_DC.metricIndexByType('deterministic', 
                                                           'nondeterministic')
    unordered_metric_names = [experiment_DC.metrics[mid][0] for mid in unordered_metric_ids]
    if set(unordered_metric_names) != set(metric_names):
        print ("Training datacollection and experiment datacollection "
               "do not have matching metrics. Aborting...")
        return
    # set the correct ordering
    expr_metric_names = [unordered_metric_names.index(name) 
                         for name in metric_names]
    expr_metric_ids = [unordered_metric_ids[unordered_metric_names.index(name)] 
                       for name in metric_names]
        
    for idx,metric in enumerate(experiment_DC.metrics):
        if(metric[0] == args['performance_metric']):
            performance_metric_id = idx
    performance = experiment_DC.profile[:,performance_metric_id]
    profile = experiment_DC.profile[:,expr_metric_ids]
    rotated_profile = np.dot(profile, rotation_matrix)
    means = np.mean(rotated_profile, axis=0)
    stdevs = np.std(rotated_profile - means, axis=0, ddof=1)
    stdevs[stdevs==0.0] = 1.0
    
    clusters = kmeans.predict((rotated_profile - means)/stdevs)

    prediction = np.empty_like(performance)
    for i in range(len(kmeans.cluster_centers_)):
        prediction[clusters==i,:] = abs(models[i].poll(rotated_profile[clusters==i,:]))

    if(args['show_prediction']):
        print "Actual\t\tPredicted"
        print '\n'.join("%s\t%s" % x for x in zip(performance,prediction))
    if(args['plot_performance_line']):
        _figureline(performance, prediction, args)
    if(args['plot_performance_scatter']):
        _scatter(performance, prediction, args)
    if(args['show_prediction_statistics']):
        mse = sum([(a-p)**2 for a,p in 
                   zip(performance, prediction)]) / len(performance)
        rmse = math.sqrt(mse)
        mape = 100 * sum([abs((a-p)/a) for a,p in 
                          zip(performance,prediction)]) / len(performance)

        print "Number of experiment trials: %s" % len(performance)
        print "Mean Average Percent Error: %s" % mape
        print "Mean Squared Error: %s" % mse
        print "Root Mean Squared Error: %s" % rmse

def _figureline(actual, prediction, args):
    plt.figure()
    plt.plot(actual,'r'+args['plot_line_marker_data'])
    plt.plot(prediction,'b'+args['plot_line_marker_pred'])

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

def _scatter(actual, prediction, args):
    plt.figure()
    plt.plot(actual, prediction, 'b'+args['plot_scatter_marker'])
    xmin=min(actual)
    xmax=max(actual)
    ymin=min(prediction)
    ymax=max(prediction)
    diagxmin=min(math.fabs(x) for x in actual)
    diagymin=min(math.fabs(y) for y in prediction)
    diagpmin=min(diagxmin,diagymin)
    pmin=min(xmin,ymin)
    pmax=max(xmax,ymax)
    plt.plot([diagpmin,pmax],[diagpmin,pmax],'k-')
    if args['plot_identifier'] != 'NoName':
        plt.title(args['plot_identifier'])
    plt.xlabel('Observed')
    plt.ylabel('Modeled')
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

def main():
    """ Main program to parse arguments from command line"""
    parser = argparse.ArgumentParser(description = \
            'Command line interface into Eiger performance modeling framework \
             for all model generation, polling, and serialization tasks.',
             argument_default=None,
             fromfile_prefix_chars='@')

    """
    CONNECTION ARGUMENTS
    """
    parser.add_argument('--db',
                        type=str,
                        help='Name of the database to connect to')
    parser.add_argument('--user',
                        type=str,
                        help='Username to connect to the database')
    parser.add_argument('--passwd',
                        type=str,
                        default='',
                        help='Password to connect to the database')
    parser.add_argument('--host',
                        type=str,
                        default='localhost',
                        help='IP address or hostname of database-hosting computer')

    """
    TRAINING DATA ARGUMENTS
    """
    parser.add_argument('--training-datacollection', '-t',
                        type=str,
                        help='Name of training data collection')
    parser.add_argument('--performance-metric',
                        type=str,
                        help='Name of the performance metric to predict')
    parser.add_argument('--show-prediction-statistics',
                        action='store_true',
                        default=False,
                        help='If set several statistics will be printed out describing the experimental prediction.')
    parser.add_argument('--test-fit',
                        action='store_true',
                        default=False,
                        help='If set will test the model fit against the training data.')
    parser.add_argument('--show-prediction',
                        action='store_true',
                        default=False,
                        help='If set, send the actual and predicted values to stdout.')
    parser.add_argument('--predictor-metrics',
                        nargs='*',
                        help='List of metrics to use when building a model.')

    """
    EXPERIMENT DATA ARGUMENTS
    """
    parser.add_argument('--experiment-datacollection', '-e',
                        type=str,
                        help='Name of experiment data collection')

    """
    PLOTTING ARGUMENTS
    """
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
    parser.add_argument('--plot-scree',
                        action='store_true',
                        default=False,
                        help='If set, plots the scree graph from principal component analysis')
    parser.add_argument('--plot-pcs-per-metric',
                        action='store_true',
                        default=False,
                        help='If set, plots the breakdown of principal components per metric.')
    parser.add_argument('--plot-metrics-per-pc',
                        action='store_true',
                        default=False,
                        help='If set, plots the breakdown of metrics per principal component.')
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
    parser.add_argument('--input', '-i',
                        type=str,
                        default=None,
                        help='File name of model file to be read and used for running experiments.')
    parser.add_argument('--output', '-o',
                        action='store_true',
                        default=False,
                        help='Filename where this model should be saved to')
    parser.add_argument('--clusters', '-k',
                        type=int,
                        default=1,
                        help='Number of clusters for kmeans')
    parser.add_argument('--threshold',
                        type=float,
                        help='Cutoff threshold of increase in adjusted R-squared value when adding new predictors to the model')
    parser.add_argument('--nfolds',
                        type=int,
                        help='Number of folds to use in k-fold cross validation.')
    parser.add_argument('--regressor-functions',
                        nargs='*',
                        default=['inv_quadratic', 'inv_linear', 'inv_sqrt', 
                                 'sqrt', 'linear', 'quadratic', 'log', 'cross'],
                        help='Regressor functions to use. Options are linear, quadratic, sqrt, inv_linear, inv_quadratic, inv_sqrt, log, and cross. Defaults to all.')
    
    
    return vars(parser.parse_args())

if __name__ == "__main__":
    
    in_args = main()
    run(in_args)

