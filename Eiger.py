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

from eiger import database, ClusterAnalysis, PCA, LinearRegression

def run(args):
    """
    Interface into Eiger model generation/polling/serialization/printing/etc.
    """
    print "Loading training data..."
    training_DC = database.DataCollection(args['training_datacollection'], 
                                          db=args['db'], 
                                          user=args['user'], 
                                          passwd=args['passwd'],
                                          host=args['host'])
    metric_ids = training_DC.metricIndexByType('deterministic', 
                                               'nondeterministic')
    metric_names = [training_DC.metrics[id][0] for id in metric_ids]
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
    rotation_matrix = training_pca.nonzeroComponents()
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
    kmeans = ClusterAnalysis.KMeans(rotated_training_profile, k=args['clusters'])
    clusters = kmeans.kmeans()

    # reserve a vector for each model created per cluster
    models = [0] * len(clusters)

    print "Modeling..."
    with tempfile.NamedTemporaryFile(delete=False) as modelfile:
        modelfile.write("%s\n%s\n" % (len(metric_names), '\n'.join(metric_names)))
        modelfile.write("[%s](%s)\n" % 
                (len(kmeans.means), ','.join([str(mean) for mean in kmeans.means.tolist()])))
        modelfile.write("[%s](%s)\n" % 
                (len(kmeans.stdevs), ','.join([str(stdev) for stdev in kmeans.stdevs.tolist()])))
        modelfile.write("[%s,%s]" % rotation_matrix.shape)
        modelfile.write("(%s)\n" % 
                        ','.join(["(%s)" % 
                            ','.join([str(elem) for elem in row]) 
                            for row in rotation_matrix.tolist()]))
        for i,cluster in enumerate(clusters):
            cluster_profile = rotated_training_profile[cluster,:]
            cluster_performance = training_performance[cluster,:]
            regression = LinearRegression.LinearRegression(cluster_profile,
                                                           cluster_performance)
            pool = regression.powerLadder(cluster_profile.shape)
            (models[i], r_squared, attempts) = regression.select(pool, 
                                                    threshold=args['threshold'])
            
            # dump model to file
            modelfile.write('Model %s\n' % i)
            modelfile.write("[%s](%s)\n" % (rotation_matrix.shape[1],
                                            ','.join([str(center) for center in
                                                kmeans.centers[i].tolist()])))
            modelfile.write(repr(models[i]))
            modelfile.write('\n') # need a trailing newline
            print "Model: " + str(models[i])

            print "Finished modeling cluster %s: r squared = %s" % (i,r_squared)
       
    # if we want to save the model file, copy it now
    if args['output'] == True:
        shutil.move(modelfile.name, training_DC.name + '.model')
    else:
        os.remove(modelfile.name)


    if(args['test_fit']):
        print "Testing fit..."
        _runExperiment(kmeans, models, rotation_matrix, training_DC, args,
                       metric_names)
    if(args['experiment_datacollection']):
        print "Running experiment on data collection %s..." % \
              (args['experiment_datacollection'],)
        experiment_DC = database.DataCollection(args['experiment_datacollection'], 
                                                db=args['db'], 
                                                user=args['user'], 
                                                passwd=args['passwd'],
                                                host=args['host'])
        _runExperiment(kmeans, models, rotation_matrix, experiment_DC, args, 
                       metric_names)
    print "Done!"

def _runExperiment(kmeans, models, rotation_matrix, 
                   experiment_DC, args, metric_names):
    expr_metric_ids = experiment_DC.metricIndexByType('deterministic', 
                                                      'nondeterministic')
    expr_metric_names = [experiment_DC.metrics[id][0] for id in expr_metric_ids]
    if expr_metric_names != metric_names:
        print ("Training datacollection and experiment datacollection "
               "do not have matching metrics. Aborting...")
        return
    for idx,metric in enumerate(experiment_DC.metrics):
        if(metric[0] == args['performance_metric']):
            performance_metric_id = idx
    performance = experiment_DC.profile[:,performance_metric_id]
    profile = experiment_DC.profile[:,expr_metric_ids]
    rotated_profile = np.dot(profile, rotation_matrix)
    
    cluster_membership = kmeans.closestCluster(rotated_profile)
    clusters = [[i for i,cluster in enumerate(cluster_membership) 
                 if cluster == cluster_id] for cluster_id in range(kmeans.k)]

    prediction = np.empty_like(performance)
    for i,cluster in enumerate(clusters):
        if len(cluster) == 0:
            continue
        prediction[cluster,:] = abs(models[i].poll(rotated_profile[cluster,:]))
   
    if(args['plot_performance_line']):
        _figureline(performance, prediction)
    if(args['plot_performance_scatter']):
        _scatter(performance, prediction)
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

def _figureline(actual, prediction):
    import os.path
    plt.figure()
    xx = range(0, len(prediction))
    rects0 = []
    rects1 = []
    rects0 = plt.plot(actual,'r'+args['plot_line_marker_data'])
    rects1 = plt.plot(prediction,'b'+args['plot_line_marker_pred'])

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

def _scatter(actual, prediction):
    import os.path
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
    
    
    return vars(parser.parse_args())

if __name__ == "__main__":
    
    args = main()
    run(args)
    pass
