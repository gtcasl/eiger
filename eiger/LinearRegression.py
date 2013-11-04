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
        return self.func(vec)

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
    def select(self, pool, threshold=None):
        """
        Selects a model based on a pool of models for each metric. Pool is an N-tuple, where each
        element p_i is a tuple of at least one element.
        threshold is the adjusted rsquared value required to add a candidate function to the final model pool
        
        returns (model(functions, weights), rsquared, trials)
        """

        if threshold == None:
            threshold = 0
        
        # lookup: n_trials x n_functions
        lookup = np.matrix([[f(x.flat) for f in pool] for x in self.X])
        model, rsquared = self.search_regression(threshold,lookup,pool)
        return (model, rsquared)

    def search_regression(self, threshold, lookup, pool):
        """
        Performs ordinary least-squares linear regression using the data set X and the
        indicated models.
        
        returns (model functions, beta, rsquared)
        """

        original_pool = copy.copy(pool)

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
                (m,rsquared) = self._evaluateModel(candidate,lookup,original_pool)
                n = len(self.X)
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
    
    def _evaluateModel(self, model, lookup, pool):
        """
        Evaluates the given model.
        """
#       U = lookup[:,[pool.index([i]) for i in model.functions]]
        U = np.take(lookup, [pool.index(i) for i in model.functions], axis=1, mode='clip')
        (b, residues, rank, s) = np.linalg.lstsq(U,self.Y)
        (n,k) = np.shape(U)
        yhat = np.dot(U, b)
        ybar = np.average(self.Y)

        """ 
        we already collect sum squared error from least squares, but if the rank is
        wrong or whatever, we'll just recalculate it
        """
        try:
            SSerr = residues[0,0]
        except IndexError:
            SSerr = np.sum(np.square(np.subtract(self.Y,yhat)))
        SStot = np.sum(np.square(np.subtract(self.Y,ybar)))

        rsquared = 1 - SSerr/SStot

        return (Model(model.functions, [x for x in b.flat]), rsquared)

def powerLadderPool(Xshape):
    def powerHelper(i,n):
        fn = lambda x: math.pow(abs(x[int(i)]),float(n)) if x[int(i)] != 0.0 else 1.0
        return Function(fn, "1 %s %s" % (i,n), 'x[%s]^%s' % (i,n))
    def logHelper(i):
        fn = lambda x: math.log(abs(x[int(i)]), 2) if x[int(i)] != 0.0 else 1.0
        return Function(fn, "4 %s" % i, 'log(|x[%s]|)' % i)
    def crossHelper(i, j):
        fn = lambda x: x[int(i)] * x[int(j)]
        return Function(fn, "2 %s %s" % (i,j), 'x[%s] * x[%s]' % (i,j))

    pool = [Function(lambda x: 1.0, "0", "1")]
    for i in range(Xshape[1]):
        pool.append(powerHelper(i, -2))
        pool.append(powerHelper(i, -1))
        pool.append(powerHelper(i, -.5))
        pool.append(logHelper(i))
        pool.append(powerHelper(i, .5))
        pool.append(powerHelper(i, 1))
        pool.append(powerHelper(i, 2))
        for j in range(i,Xshape[1]):
            pool.append(crossHelper(i,j))

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
        from mpl_toolkits.mplot3d import Axes3D
        import numpy as np
        import matplotlib.pyplot as plt
        
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
        from pylab import plot, xlabel, ylabel, show
    
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
        pass

    testQuadraticModel()

