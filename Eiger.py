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
import json
import sys

from sklearn.cluster import KMeans
from eiger import database, PCA, LinearRegression

def trainModel(args):
    print "Training the model..."
    training_DC = database.DataCollection(args.training_dc, args.database)
    try:
        performance_metric_id = [m[0] for m in training_DC.metrics].index(args.target)
    except ValueError:
        print "Unable to find target metric '%s', " \
        "please specify a valid one: " % (args.target,)
        for (my_name,my_desc,my_type) in training_DC.metrics:
            print "\t%s" % (my_name,)
        return
    training_performance = training_DC.profile[:,performance_metric_id]
    metric_names = [m[0] for m in training_DC.metrics if m[0] != args.target]
    if args.predictor_metrics != None:
        metric_names = filter(lambda x: x in args.predictor_metrics, metric_names)
    metric_ids = [[m[0] for m in training_DC.metrics].index(n) for n in metric_names]
    if not metric_ids:
        print "Unable to make model for empty data collection. Aborting..."
        return
    training_profile = training_DC.profile[:,metric_ids]

    #pca
    training_pca = PCA.PCA(training_profile)
    nonzero_components = training_pca.nonzeroComponents()
    rotation_matrix = training_pca.components[:,nonzero_components]
    rotated_training_profile = np.dot(training_profile, rotation_matrix)

    #kmeans
    n_clusters = args.clusters
    kmeans = KMeans(n_clusters)
    means = np.mean(rotated_training_profile, axis=0)
    stdevs = np.std(rotated_training_profile - means, axis=0, ddof=1)
    stdevs[stdevs==0.0] = 1.0
    clusters = kmeans.fit_predict((rotated_training_profile - means)/stdevs)

    # reserve a vector for each model created per cluster
    models = [0] * len(clusters)

    print "Modeling..."
    # for printing the json file
    json_root = {}
    with tempfile.NamedTemporaryFile(delete=False) as modelfile:
        # For printing the original model file encoding 
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
        # for printing the json file
        json_root["metric_names"] = [name for name in metric_names]
        json_root["means"] = [mean for mean in means.tolist()]
        json_root["std_devs"] = [stdev for stdev in stdevs.tolist()]
        json_root["rotation_matrix"] = [[elem for elem in row] for row in rotation_matrix.tolist()]
        json_root["clusters"] = []

        for i in range(n_clusters):
            cluster_profile = rotated_training_profile[clusters==i,:]
            cluster_performance = training_performance[clusters==i]
            regression = LinearRegression.LinearRegression(cluster_profile,
                                                           cluster_performance)
            pool = [LinearRegression.identityFunction()]
            for col in range(cluster_profile.shape[1]):
                if('inv_quadratic' in args.regressor_functions):
                    pool.append(LinearRegression.powerFunction(col, -2))
                if('inv_linear' in args.regressor_functions):
                    pool.append(LinearRegression.powerFunction(col, -1))
                if('inv_sqrt' in args.regressor_functions):
                    pool.append(LinearRegression.powerFunction(col, -.5))
                if('sqrt' in args.regressor_functions):
                    pool.append(LinearRegression.powerFunction(col, .5))
                if('linear' in args.regressor_functions):
                    pool.append(LinearRegression.powerFunction(col, 1))
                if('quadratic' in args.regressor_functions):
                    pool.append(LinearRegression.powerFunction(col, 2))
                if('log' in args.regressor_functions):
                    pool.append(LinearRegression.logFunction(col))
                if('cross' in args.regressor_functions):
                    for xcol in range(col, cluster_profile.shape[1]):
                        pool.append(LinearRegression.crossFunction(col, xcol))
                if('div' in args.regressor_functions):
                    for xcol in range(col, cluster_profile.shape[1]):
                        pool.append(LinearRegression.divFunction(col,xcol))
                        pool.append(LinearRegression.divFunction(xcol,col))
            (models[i], r_squared, r_squared_adj) = regression.select(pool, 
                    threshold=args.threshold,
                    folds=args.nfolds)
            
            # dump model to original file encoding
            modelfile.write('Model %s\n' % i)
            modelfile.write("[%s](%s)\n" % (rotation_matrix.shape[1],
                                            ','.join([str(center) for center in
                                                kmeans.cluster_centers_[i].tolist()])))
            modelfile.write(repr(models[i]))
            modelfile.write('\n') # need a trailing newline

            # dump model for json encoding
            json_cluster = {}
            json_cluster["center"] = [center for center in kmeans.cluster_centers_[i].tolist()]
            # get models in json format
            json_cluster["regressors"] = models[i].toJSONObject()
            json_root["clusters"].append(json_cluster)

            print "Index\tMetric Name"
            print '\n'.join("%s\t%s" % metric for metric in enumerate(metric_names))
            print "PCA matrix:"
            print rotation_matrix 
            print "Model:\n" + str(models[i])

            print "Finished modeling cluster %s:" % (i,)
            print "r squared = %s" % (r_squared,)
            print "adjusted r squared = %s" % (r_squared_adj,)
       
    # if we want to save the model file, copy it now
    outfilename = training_DC.name + '.model' if args.output == None else args.output
    if args.json == True:
        with open(outfilename, 'w') as outfile:
            json.dump(json_root, outfile, indent=4)
    else:
        shutil.copy(modelfile.name, outfilename)
    if args.test_fit:
        args.experiment_dc = args.training_dc
        args.model = outfilename
        testModel(args)


def dumpCSV(args):
    training_DC = database.DataCollection(args.training_dc, args.database)
    names = [met[0] for met in training_DC.metrics]
    if args.metrics != None:
        names = args.metrics
    header = ','.join(names)
    idxs = training_DC.metricIndexByName(names)
    profile = training_DC.profile[:,idxs]
    output = sys.stdout if args.output == None else args.output
    np.savetxt(output, profile, delimiter=',', 
            header=header, comments='')

def testModel(args):
    print "Testing the model fit..."
    test_DC = database.DataCollection(args.experiment_dc, args.database)

    # Read in the model file (bespoke version)
    lines = iter(open(args.model,'r').read().splitlines())
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
    _runExperiment(kmeans, means, stdevs, models, rotation_matrix, test_DC,
            args, metric_names)

def plotModel(args):
    print "Plotting model..."
    # Read in the model file (bespoke version)
    lines = iter(open(args.model,'r').read().splitlines())
    n_params = int(lines.next())
    metric_names = [lines.next() for i in range(n_params)]
    means = _stringToArray(lines.next())
    stdevs = _stringToArray(lines.next())
    rotation_matrix = _stringToArray(lines.next())
    if args.plot_pcs_per_metric:
        PCA.PlotPCsPerMetric(rotation_matrix, metric_names, 
                             title="PCs Per Metric")
    if args.plot_metrics_per_pc:
        PCA.PlotMetricsPerPC(rotation_matrix, metric_names, 
                             title="Metrics Per PC")

def _stringToArray(string):
    """
    Parse string of form [len](number,number,number,...) to a numpy array.
    """
    length = string[:string.find('(')]
    values = string[string.find('('):]
    arr = np.array(literal_eval(values))
    return np.reshape(arr, literal_eval(length))

def _runExperiment(kmeans, means, stdevs, models, rotation_matrix, 
                   experiment_DC, args, metric_names):
    unordered_metric_ids = experiment_DC.metricIndexByType('deterministic', 
                                                           'nondeterministic')
    unordered_metric_names = [experiment_DC.metrics[mid][0] for mid in unordered_metric_ids]
    # make sure all metric_names are in experiment_DC.metrics[:][0]
    have_metrics = [x in unordered_metric_names for x in metric_names]
    if not all(have_metrics):
        print("Experiment DC does not have matching metrics. Aborting...")
        return
    # set the correct ordering
    expr_metric_ids = [unordered_metric_ids[unordered_metric_names.index(name)] 
                       for name in metric_names]
        
    for idx,metric in enumerate(experiment_DC.metrics):
        if(metric[0] == args.target):
            performance_metric_id = idx
    performance = experiment_DC.profile[:,performance_metric_id]
    profile = experiment_DC.profile[:,expr_metric_ids]
    rotated_profile = np.dot(profile, rotation_matrix)
    means = np.mean(rotated_profile, axis=0)
    stdevs = np.std(rotated_profile - means, axis=0, ddof=1)
    stdevs = np.nan_to_num(stdevs)
    stdevs[stdevs==0.0] = 1.0
    
    clusters = kmeans.predict((rotated_profile - means)/stdevs)

    prediction = np.empty_like(performance)
    for i in range(len(kmeans.cluster_centers_)):
        prediction[clusters==i] = abs(models[i].poll(rotated_profile[clusters==i]))

    if args.show_prediction:
        print "Actual\t\tPredicted"
        print '\n'.join("%s\t%s" % x for x in zip(performance,prediction))

    mse = sum([(a-p)**2 for a,p in 
               zip(performance, prediction)]) / len(performance)
    rmse = math.sqrt(mse)
    mape = 100 * sum([abs((a-p)/a) for a,p in 
                      zip(performance,prediction)]) / len(performance)

    print "Number of experiment trials: %s" % len(performance)
    print "Mean Average Percent Error: %s" % mape
    print "Mean Squared Error: %s" % mse
    print "Root Mean Squared Error: %s" % rmse

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = \
            'Command line interface into Eiger performance modeling framework \
            for all model generation, polling, and serialization tasks.',
            argument_default=None,
            fromfile_prefix_chars='@')

    subparsers = parser.add_subparsers(title='subcommands')
    train_parser = subparsers.add_parser('train',
            help='train a model with data from the database',
            description='Train a model with data from the database')
    train_parser.set_defaults(func=trainModel)
    dump_parser = subparsers.add_parser('dump',
            help='dump data collection to CSV',
            description='Dump data collection as CSV')
    dump_parser.set_defaults(func=dumpCSV)
    test_parser = subparsers.add_parser('test',
            help='test how well a model predicts a data collection',
            description='Test how well a model predicts a data collection')
    test_parser.set_defaults(func=testModel)
    plot_parser = subparsers.add_parser('plot',
            help='plot the behavior of a model',
            description='Plot the behavior of a model')
    plot_parser.set_defaults(func=plotModel)
    #convert_parser = subparsers.add_parser('convert',
    #        help='transform a model into a different file format',
    #        description='Transform a model into a different file format')

    """TRAINING ARGUMENTS"""
    train_parser.add_argument('database', type=str, help='Name of the database file')
    train_parser.add_argument('training_dc', type=str,
            help='Name of the training data collection')
    train_parser.add_argument('target', type=str,
            help='Name of the target metric to predict')
    train_parser.add_argument('--test-fit', action='store_true', default=False,
            help='If set will test the model fit against the training data.')
    train_parser.add_argument('--show-prediction', action='store_true',
            default=False,
            help='If set, send the actual and predicted values to stdout.')
    train_parser.add_argument('--predictor-metrics', nargs='*',
            help='Only use these metrics when building a model.')
    train_parser.add_argument('--output', type=str, 
            help='Filename to output file to, otherwise use "<training_dc>.model"')
    train_parser.add_argument('--clusters', '-k', type=int, default=1,
            help='Number of clusters for kmeans')
    train_parser.add_argument('--threshold', type=float,
            help='Cutoff threshold of increase in adjusted R-squared value when'
            ' adding new predictors to the model')
    train_parser.add_argument('--nfolds', type=int,
            help='Number of folds to use in k-fold cross validation.')
    train_parser.add_argument('--regressor-functions', nargs='*',
            default=['inv_quadratic', 'inv_linear', 'inv_sqrt', 'sqrt',
                'linear', 'quadratic', 'log', 'cross', 'div'],
            help='Regressor functions to use. Options are linear, quadratic, '
            'sqrt, inv_linear, inv_quadratic, inv_sqrt, log, cross, and div. '
            'Defaults to all.')
    train_parser.add_argument('--json', action='store_true', default=False,
            help='Output model in JSON format, rather than bespoke')

    """DUMP CSV ARGUMENTS"""
    dump_parser.add_argument('database', type=str, help='Name of the database file')
    dump_parser.add_argument('training_dc', type=str,
            help='Name of the data collection to dump')
    dump_parser.add_argument('--metrics', nargs='*',
            help='Only dump these metrics.')
    dump_parser.add_argument('--output', type=str, help='Name of file to dump CSV to')

    """TEST ARGUMENTS"""
    test_parser.add_argument('database', type=str, help='Name of the database file')
    test_parser.add_argument('experiment_dc', type=str,
            help='Name of the data collection to experiment on')
    test_parser.add_argument('model', type=str,
            help='Name of the model to use')
    test_parser.add_argument('target', type=str,
            help='Name of the target metric to predict')
    test_parser.add_argument('--show-prediction', action='store_true',
            default=False,
            help='If set, send the actual and predicted values to stdout.')

    """PLOT ARGUMENTS"""
    plot_parser.add_argument('model', type=str,
            help='Name of the model to use')
    plot_parser.add_argument('--plot-pcs-per-metric', action='store_true',
            default=False,
            help='If set, plots the breakdown of principal components per metric.')
    plot_parser.add_argument('--plot-metrics-per-pc',
            action='store_true',
            default=False,
            help='If set, plots the breakdown of metrics per principal component.')

    args = parser.parse_args()
    args.func(args)
    print "Done."

