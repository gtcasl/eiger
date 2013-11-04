# \file NearestNeighbors.py
# \author Eric Anger <eanger@gatech.edu>
# \date October 8, 2012
#
# \brief selects a model to fit the datapoints using k-Nearest Neighbors
#
#

import numpy as np
import math

#
class NearestNeighbors:
    #
    def __init__(self, X, Y, k):
        """
        Constructs a model selector given a set of data points and their outputs.
        
        X is m-by-n, where m is number of data points and n is number of metrics
        Y is m-by-1
        """
        self.X = X
        self.Y = Y
        self.M = X.shape[0]
        self.N = X.shape[1]
        self.k = k
        
        assert(self.M == Y.shape[0])
    
#
    def poll(self, t):
        distances = {}
        for x,y in zip(self.X,self.Y):
            distance = math.sqrt(np.sum(np.square(np.subtract(t,x))))
            distances[distance] = y
        if(0 in distances.keys()):
            return distances[0][0,0]
        else:
            return sum([distances[x][0,0]/x for x in sorted(distances.keys())[:self.k]])/self.k #closest k trials

