# \file LinearRegression.py
# \author Andrew Kerr <arkerr@gatech.edu>
# \date July 14, 2011
#
# \brief selects a model to fit the datapoints using ordinary least squares regression
#
# What is Eiger? http://www.youtube.com/watch?v=PEYiSeDr1UE
#

import numpy as np
import random
import copy
import math
from sklearn.cross_validation import KFold

class Function:
    """ 
    Containter for managing different representations of model functions 
    """
    def __init__(self, func, encoded, readible=None):
        self.encoded = encoded
        self.func = func
        self.readible = encoded if readible==None else readible
    def __str__(self):
        return self.readible
    def __repr__(self):
        return self.encoded
    def __call__(self, vec):
        try:
            return self.func(vec)
        except OverflowError:
            return 0.0

class Model:
    """
    An instance that includes both a vector of functions and a vector of weights.
    All functions are encoded the decoded upon evaluation.
    """

    #
    def __init__(self, functions, weights=None):
        self.functions = functions
        self.weights = weights

    #
    def poll(self, T):
        """
        Given a model and weights, returns a data set evaluted by the model
        """
        U = np.array([[function(row) for function in self.functions] for row in T])
        return np.dot(U, self.weights)

    def __repr__(self):
        """
        Writes out this performance model to the given file descriptor.
        """
        weight_repr = repr(self.weights)[1:-1]
        func_repr = repr(self.functions)[1:-1].replace(', ', '\n')
        return "[%r](%s)\n%s" % (len(self.functions), weight_repr, func_repr)

    def __str__(self):
        result = ''.join(["%.3e * %s + " % pred for pred in zip(self.weights, self.functions)])
        return result[:-3] #get rid of trailing ' + ' string

#
class LinearRegression:
    """
    Object to construct a model for a particular set of data by iterating over a collection of
    possible models, estimating weights and total squared error for each model, then choosing
    the one that minimizes error. Geared toward flexibility over performance.
    
    A pool consists of a p-element array P, where each element p_i is a tuple of functions F.
    F maps a row of X onto a real number. Note, the pool may have greater or fewer dimensions as X.
    """
    
    #
    def __init__(self, X, Y):
        """
        Constructs a model selector given a set of data points and their outputs.
        
        X is m-by-n, where m is number of data points and n is number of metrics
        Y is m-by-1
        """
        self.X = X
        self.Y = Y
        self.M = X.shape[0]
        self.N = X.shape[1]
        
        assert(self.M == Y.shape[0])
    
#
    def select(self, pool, threshold=None, folds=None):
        """
        Selects a model based on a pool of models for each metric. Pool is an N-tuple, where each
        element p_i is a tuple of at least one element.
        threshold is the adjusted rsquared value required to add a candidate function to the final model pool
        
        returns (model(functions, weights), rsquared, trials)
        """

        if threshold == None:
            threshold = 0
        if folds == None:
            all_indices = np.indices((self.M,)).tolist()[0]
            kfold = ((all_indices, all_indices),)
        else:
            kfold = KFold(self.M, n_folds=folds, shuffle=True)

        rsquared = float('-inf')
        i = 0
        for train_index, test_index in kfold:
            print "Training fold %s" % i
            i = i + 1
            candidate, candidate_rsquared = self.search_regression(threshold,
                                                                   train_index,
                                                                   test_index,
                                                                   pool)
            print "Candidate R^2: %s" % candidate_rsquared
            if(candidate_rsquared > rsquared):
                model = candidate
                rsquared = candidate_rsquared
        return (model, rsquared)

    def search_regression(self, threshold, train_index, test_index, pool):
        """
        Performs ordinary least-squares linear regression using the data set X and the
        indicated models.
        
        returns (model functions, beta, rsquared)
        """

        original_pool = copy.copy(pool)
        train_lookup = np.matrix([[f(x.flat) for f in pool] 
                                  for x in self.X[train_index]])
        test_lookup  = np.matrix([[f(x.flat) for f in pool] 
                                  for x in self.X[test_index]])

        M = []
        Beta = []
        done = False
        model_r2_adj = float('-inf')
        model_r2 = 0
#       print "START LEN: ", str(len(model.functions))
        while(not done):
            max_adj = float('-inf')
            for func in pool:
                candidate = Model(M+[func,])
                (m,rsquared) = self._evaluateModel(candidate,
                                                   train_index,
                                                   train_lookup,
                                                   test_index,
                                                   test_lookup,
                                                   original_pool)
                n = len(test_index)
                k = len(candidate.functions)
                rsquared_adjusted = 1 - (1-rsquared) * (n - 1) / (n - k - 1)
                if(rsquared_adjusted > max_adj):
                    max_adj = rsquared_adjusted
                    new_function = func
                    new_r2 = rsquared
                    new_beta = m.weights

            if(max_adj != float('-inf') and (max_adj - model_r2_adj) > threshold):
                M.append(new_function)
                pool.remove(new_function)
                model_r2_adj = max_adj
                model_r2 = new_r2
                Beta = new_beta
            else:
                done = True

        return (Model(M, Beta), model_r2)
    
    def _evaluateModel(self, model, train_index, train_lookup, test_index, 
                       test_lookup, pool):
        """
        Evaluates the given model.
        """
        # lookup: n_trials x n_functions
        train_data = np.take(train_lookup, 
                             [pool.index(i) for i in model.functions], 
                             axis=1, mode='clip')
        test_data  = np.take(test_lookup, 
                             [pool.index(i) for i in model.functions], 
                             axis=1, mode='clip')
        train_res = self.Y[train_index,:]
        test_res = self.Y[test_index,:]
        (b, residues, rank, s) = np.linalg.lstsq(train_data, train_res)
        yhat = np.dot(test_data, b)
        ybar = np.average(test_res)

        SSerr = np.sum(np.square(np.subtract(test_res,yhat)))
        SStot = np.sum(np.square(np.subtract(test_res,ybar)))

        rsquared = 1 - SSerr/SStot

        return (Model(model.functions, [x for x in b.flat]), rsquared)

def stringToFunction(encoded_string):
    """
    Convert an encoded function string to a Function object.

    Please note that if you want to add or change a function, you need to add
    it to this list, making sure to preserve the order with respect to the
    first parameter of the encoding.
    """
    function_generators = [identityFunction,
                           powerFunction,
                           crossFunction,
                           sqrtFunction,
                           logFunction]
    encoding = encoded_string.split()
    return function_generators[int(encoding[0])](*encoding[1:])

def identityFunction():
    return Function(lambda x: 1.0, '0', '1')

def powerFunction(i,n):
    fn = lambda x: math.pow(abs(x[int(i)]),float(n)) if x[int(i)] != 0.0 else 1.0
    return Function(fn, '1 %s %s' % (i,n), 'x[%s]^%s' % (i,n))

def crossFunction(i,j):
    fn = lambda x: x[int(i)] * x[int(j)]
    return Function(fn, '2 %s %s' % (i,j), 'x[%s] * x[%s]' % (i,j))

def sqrtFunction(i):
    fn = lambda x: math.sqrt(abs(x[int(i)]))
    return Function(fn, '3 %s' % i, 'sqrt(|x[%s]|)' % i)

def logFunction(i):
    fn = lambda x: math.log(abs(x[int(i)]), 2) if x[int(i)] != 0.0 else 1.0
    return Function(fn, '4 %s' % i, 'log(|x[%s]|)' % i)

def powerLadderPool(Xshape):
    pool = [identityFunction()]
    for i in range(Xshape[1]):
        pool.append(powerFunction(i, -2))
        pool.append(powerFunction(i, -1))
        pool.append(powerFunction(i, -.5))
        pool.append(logFunction(i))
        pool.append(powerFunction(i, .5))
        pool.append(powerFunction(i, 1))
        pool.append(powerFunction(i, 2))
        for j in range(i,Xshape[1]):
            pool.append(crossFunction(i,j))

    return pool
    
#################################################################################################
#
#
if __name__ == "__main__":

    def testNumpy():
        X = np.matrix([[1,2,3],[4,5,6]])
        print "X = ", X
        
        for row in X:
            print "row = ", row
        
        print "Trying to iterate over elements"
        
        for row in X:
            print "row"
            for r in row.flat:
                print r

    #
    def testTwoMetrics():
        U = lambda x,y: 0.25*x*x - 0.75 * y
        
        X = None
        for y in range(-3, 4):
            Xt = np.matrix([ [x,y] for x in range(-3,4)])
            X = np.vstack([X, Xt]) if X != None else Xt
        
        Y = np.matrix([ [U(r[0,0], r[0,1]),] for r in X])
        
        selector = LinearRegression(X, Y)
        pool = selector.polynomialPool([3 for r in range(0, 2)])        
        model, indices, squaredError, trials = selector.select(pool)
        
        print "Beta = ", model.beta
        print "Indices = ", indices
    
    #
    def testQuadraticModel():
        """
        Simple demonstration with a quadratic model and one metric
        """
        from pylab import plot, show
    
        U = lambda x: 2.5*x*x*x
    
        X = np.matrix([ [y,] for y in range(0, 16)])
        Y = np.matrix([ [U(r[0,0]) + 80 * random.random() - 40.0 for r in row] for row in X])

        selector = LinearRegression(X, Y)
        model, indices, squaredError, trials = selector.select(selector.polynomialPool([3 for r in range(0, 1)]))
    
        Z = model.evaluate(X)

        xx = X.T[0]
        yy = Y.T[0]
        zz = Z.T[0]
    
        print "Beta = ", model.beta
        print xx
        print yy
        print zz
    
        plot(xx, zz, 'x', color='b')
        plot(xx, yy, 'o', color='b')
        show()

    testQuadraticModel()

