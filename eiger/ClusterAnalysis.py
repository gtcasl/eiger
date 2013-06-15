#
# \file ClusterAnalysis.py
# \author Andrew Kerr <arkerr@gatech.edu>
# \date June 27, 2011
#
# \brief given an input matrix and an orthornomal basis, identifies clusters of applications
#
import numpy as np

##
#
# Iterative implementation of Lloyd's Algorithm solving J. MacQueen's formulation of k-means
# see: "Some methods for classification and analysis of multivariate observations"
#
class KMeans:

    ##
    #
    def __init__(self, data, k = 10, maxIterations = 50):
        """
        Constructs a K-Means cluster analysis pass for an N-by-P matrix, where N is the number of
        observations of dimension P
        """
        self.data = data
        self.k = k
        self.N = self.data.shape[0]
        self.maxIterations = maxIterations
        self._initialize()
    
    ##
    #
    def _initialize(self):
        self.clusters = [i % self.k for i in range(0, self.N)]
        self.counts = [0 for i in range(0, self.k)]
        self.centers = self.data[0:self.k, :]
    
    ##
    #
    def _evalCenters(self):
        self.centers = np.matrix(np.zeros((self.k, self.data.shape[1])))
        for i in range(0, self.N):
            self.centers[self.clusters[i], :] += self.data[i, :]
        for i in range(0, self.k):
            self.centers[i, :] /= float(self.counts[i])
    
    ##
    #
    def _assignClusters(self):
        newClusters = [0 for i in range(0, self.N)]
        newCounts = [0 for i in range(0, self.k)]
        converged = True
        for i in range(0, self.N):
            bestDistance = 0
            bestCluster = 0
            for c in range(0, self.k):
                diff = self.data[i, :] - self.centers[c, :]
                distance = np.dot(diff, diff.T)
                if not c or distance < bestDistance:
                    bestCluster = c
                    bestDistance = distance
            if bestCluster != self.clusters[i]:
                converged = False
            newClusters[i] = bestCluster
            newCounts[bestCluster] += 1
            
        for c in range(0, self.k):
            if newCounts[c] == 0:
                print "Cluster %s has no elements" % (c,)
                print "  it had %s elements before" % (self.counts[c], )
            
        self.clusters = newClusters
        self.counts = newCounts
        
        return converged
    
    ##
    #
    def collect(self):
        """
        Returns an array of clusters, where each cluster is a sorted array containing the index of
        member elements.
        """
        clusters = [[] for i in range(0, self.k)]
        for i in range(0, self.N):
            clusters[self.clusters[i]].append(i)
        return clusters
    
    def plot(self, iteration):
        """
        """
        figure()
        clusters = self.collect()
        color = ['#0000a0', '#00a000', '#a00000', '#00a0f0', '#a0f000', '#f000a0', '#f000f0', '#00f0f0', '#f0f000']
        p = 0
        for c in clusters:
#           print c
            xx = [self.data[i, :] for i in c]
            plot([x[0, 0] for x in xx], [x[0, 1] for x in xx], 'o', color = color[p])
            p += 1
        
        plot([x[0, 0] for x in self.centers], [x[0, 1] for x in self.centers], '+', color='#000000')
        title('K = %s' % (iteration,))
#       print self.centers
        
    
    def kmeans(self):
        """
        Performs k-means cluster analysis on the input data set return a jagged array of arrays
        """
        converged = False
        iterations = 0
        while not converged:
            converged = self._assignClusters()
            iterations += 1
            if iterations > self.maxIterations:
                converted = True
            if not converged:
                self._evalCenters()
        return self.collect()

    def closestCluster(self, experiment):
        """
        Finds which cluster is closest to each row in experiment
        """
        winners = []
        for e in experiment:
            mindist = float('inf')
            for c,cluster in enumerate(self.centers):
                dist = 0.0
                for i in range(0,self.data.shape[1]):
                    dist += (e[0,i] - cluster[0,i])**2
                if(dist < mindist):
                    mindist = dist
                    winner = c
            winners.append(winner)

        return winners
            


###################################################################################################
#
#
if __name__ == "__main__":
# test KMeans demo

    import matplotlib.patches as mpatches
    from pylab import plot, show, title, figure

    data = np.matrix(np.random.rand(32, 2))
    experiment = np.matrix(np.random.rand(4,2))
    for k in range(4, 9, 2):
        maxIterations = 10
        kmeans = KMeans(data, k, maxIterations)
        res = kmeans.kmeans()
        print res
        closest = kmeans.closestCluster(experiment)
        print "closest"
        print closest
        kmeans.plot(k)
        show()
    pass
    """
    import scipy.cluster.hierarchy as h
    import numpy as np
    import scipy.spatial.distance as d
    import matplotlib.pyplot as pp

    n = 32
    thresh = .15
    np.random.seed(18237)
    data = np.matrix(np.random.rand(n,2))
    dist = d.pdist(data,'euclidean')
    res = h.linkage(dist)

    clust = h.fcluster(res,t=thresh,criterion='distance')
    print
    for x,y in 


    fig = pp.figure()
    ax = fig.add_subplot(111)

    num_clusters = max(clust)
    colors = [str(x) for x in clust/float(num_clusters)]
    """

    """
    for i in range(0,n,1):
        ax.scatter(data[i,0],data[i,1],s=40, c=colors[i])

    h.dendrogram(res, color_threshold=thresh)
    pp.show()
    pass

    """

