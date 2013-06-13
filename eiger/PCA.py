#
# \file PCA.py
# \author Andrew Kerr <arkerr@gatech.edu>
# \date June 27, 2011
#
# \brief implements Principal Component Analysis for Eiger performance modeling framework
#
# What is Eiger? http://www.youtube.com/watch?v=PEYiSeDr1UE
#

from scipy import linalg
import numpy as np
import math, cmath

####################################################################################################
#
#
class PCA:
	def __init__(self, X, scale=True, rotate=True):
		"""
		Constructs a PCA object from a matrix in R^{N-by-M}, where N is the number of data points, and 
			M is the number of metrics
		X - input matrix from PCA analysis
		"""
		
		self.X = X
		if(self.X.size == 0):
			self.components = np.eye(0)
			self.loadings = np.zeros((0,))
			return
		(rows,cols) = np.shape(self.X)
		try:
			np.seterr(all='ignore')
			self.Z = self.pretreat(self.X,scale)
			(U, S, VT) = linalg.svd(self.Z,full_matrices=False)
			self.loadings = S**2	#singular values are the square root of the eigenvalues
			self.components = VT.T
			if(rotate is True):
				vmax = VARIMAX(self.components)
				(R, self.components) = vmax.compute()
		except ValueError:
			self.components = np.eye(cols)
			self.loadings = np.zeros((cols,))
		finally:
			np.seterr()

	def pc(self):
		return self.components
	
	def reduced(self, targetVariance = None, targetComponents = None):
		"""
		Eliminates dimensions in PCA space depending on certain parameters.
		Principal components will be added to the final set until one of the conditions
		are met.
		targetVariance - percent of total variance to capture 
		targetComponents - number of components to capture
		Returns (components, loadings, componentCount, captured variance)
		"""
		(rows, cols) = np.shape(self.X)
		maxComponents = cols if targetComponents == None else targetComponents
		maxVariance = 1.1 if targetVariance == None else targetVariance
		
		variances = self.loadings
		explainedVar = variances / sum(variances) if sum(variances) > 0 else variances # percent each component contributes to total variance
		actualVariance = count = 0
		for var in explainedVar:
			actualVariance += var
			count += 1
			if actualVariance >= maxVariance or count >= maxComponents:
				break
		return (self.components[:,0:count], self.loadings[0:count], count, actualVariance)

	def project(self, M):
		return np.dot(self.components.T, M)
	
	def zeroMean(self, X):
		"""
		Returns a matrix whose columns are zero mean
		"""
		means = np.mean(X, axis=0)		
		return (X - means, means)
		
	def pretreat(self, X, scale):	
		Y, means = self.zeroMean(X)
		if scale is True:
			std = np.std(Y,axis=0,ddof=1)
			#for dimensions with zero variance, we just want to keep whatever 
			#values we have after division, so just divide by 1
			std[std==0.0] = 1.0
			Y /= std
		return Y

	def covariance(self, Y):
		n = Y.shape[0]
		return np.matrix(Y.T) * np.matrix(Y) / n


def PlotScree(s, log=False, title=""):
	"""
	Makes scree plot of eigenvalues.
	s - vector of singular values (eigenvalues) in descending magnitude
	log - if true, makes y axis log scale
	"""
	import matplotlib.pyplot as plt
	ax = plt.subplot(111)
	ax.plot(range(1,len(s)+1), s, 'ro-', linewidth=2)
	ylabel = 'Eigenvalue'
	if(log == True):
		ax.set_yscale('log')
		ylabel = ylabel + ' (log scale)'
	plt.xlabel('Principal Component Number')
	plt.ylabel(ylabel)
	plt.title(title)
	plt.show()



def PlotPCsPerMetric(pcs, metrics, title=""):
	"""
	Plot out how each metric contributes to the principal components.
	pcs - NxP matrix of principal components
	metrics - N metrics
	"""
	import matplotlib.pyplot as plt
	plt.subplot(111)
	(rows,cols) = np.shape(pcs)
	colors = ['%s' % (x * 1.0 / cols,) for x in range(cols)]
	w = 0.4

	rects = range(cols)
	xx = range(rows)
	for i in range(cols):
		rects[i] = plt.bar([i*w+x*(cols+1)*w for x in xx], [pcs[x,i] for x in xx], width=w, color=colors[i], label="PC " + str(i))
	plt.legend(loc='upper right')
	plt.xticks([x * ((cols+1) * w) + cols*w/2.0 for x in range(rows)], [x.name for x in metrics], rotation='90')
	plt.title(title)
	plt.show()



	"""
	xx = range(0,cols)
	rects = range(rows)
#	print rects
	for i in range(rows):
		rects[i] = plt.bar([i*(cols*w + w) + w*x for x in xx], [pcs[i,x] for x in xx], width=w, color = colors)
	plt.title('Principal component loadings per metric')
#		plt.legend([rects[i] for i in range(rows)], loc='upper right')
	plt.show()
	"""

def PlotMetricsPerPC(pcs, metrics, title=""):
	"""
	Plot out how each PC is constructed out of each metric.
	pcs - NxP matrix of principal components
	metrics - N metrics
	"""
	import matplotlib.pyplot as plt
	plt.subplot(111)
	(rows,cols) = np.shape(pcs)
	colors = ['%s' % (x * 1.0 / rows,) for x in range(rows)]
	w = 0.4

	rects = range(rows) #rows = N metrics
	xx = range(cols)    #cols = P principal components
	for i in range(rows):
		rects[i] = plt.bar([i*w+x*(rows+1)*w for x in xx], [pcs[i,x] for x in xx], width=w, color=colors[i], label=metrics[i].name)
	plt.legend(loc='upper right')
	plt.xticks([rows*w/2.0 + x*((rows+1)*w) for x in xx], ["PC " + str(x) for x in xx], rotation='90')
	plt.title(title)
	plt.show()




#
#
class VARIMAX:

	def __init__(self, X, maxIterations = 100, Epsilon = 1.0e-5):
		self.maxIterations = maxIterations
		self.Epsilon = Epsilon
		self.X = np.matrix(X)
		pass
	
	def compute(self):
		X = self.X.copy()
		n, k = X.shape
		converged = False
		iterations = 0
		variance = sum(sum(np.multiply(X, X), 1).T)[0,0]
		R = np.matrix(np.eye(k))
		while (not converged) and (iterations < self.maxIterations):
#			print "Iteration", iterations
			for j in range(0, k-1):
				for l in range(j+1, k):
					phi = self._phi(X, j, l)
#					print "(%s, %s) - phi = %s" % (j, l, phi)
					self._rotateInPlace(X, phi, j, l)
					self._rotateInPlace(R, phi, j, l)
			newVariance = sum(sum(np.multiply(X, X), 1).T)[0,0]
			iterations += 1
			converged = ((newVariance-variance)/newVariance < self.Epsilon)
			variance = newVariance
		return R, np.array(X)

	def _phi(self, A, k, l):
		D, M = A.shape
		u = sum(np.power(A[:,k] + A[:,l] * (1.0j), 4))/D
		v = sum(np.power(A[:,k] + A[:,l] * (1.0j), 2))/D
		p = u - v*v
		phi = cmath.phase(p[0,0]) / 4.0
		return phi

	def _rotateInPlace(self, A, phi, k, l):
		c, s = math.cos(phi), math.sin(phi)
		U = np.hstack((A[:,k], A[:,l]))
		R = np.matrix([[c, -s],[s, c]])
		U = U * R
		A[:,k] = U[:,0]
		A[:,l] = U[:,1]
		pass

	def sorted(self):
		"""
		Returns the collection of PCAs and corresponding eigenvalues, sorted in descending order of eigenvalue
		"""
		return 0

####################################################################################################
#

def exampleInit(m, n):
	A = np.zeros((m, n))
	A[0] = np.array( [ 3.0 * math.cos(math.pi * 2 * j / n) for j in range(0, n) ] )
	A[1] = np.array( [ 1.0 * math.sin(math.pi * 2 * j / n) for j in range(0, n) ] )
	r = 31.0
	c = math.cos(math.pi*r/180.0)
	s = math.sin(math.pi*r/180.0)
	R = np.matrix([ [c, s], [-s, c] ])
	A = R * A + np.random.randn(m, n) * 0.1
	return A

def simplePCATest():

	from pylab import plot, show, figure, title, bar
	
	# construct some test matrix
	m = 2
	n = 55
	X = exampleInit(m, n)
	
	pca = PCA(X)
	V = pca.pc()
	print "PCA: V = ", V
	
	vmax = VARIMAX(pca.pc())
	R,rotatedV = vmax.compute()
	print "VARIMAX = "
	print R
	
	PX = pca.project(X)
	print "varimax * V * X = ", PX
	print "rotated principal components = ", rotatedV

	figure()
	plot(X[0,:], X[1,:], '.')
	plot([0, V[0,0]], [0, V[1,0]], '--')
	plot([0, V[0,1]], [0, V[1,1]], '--')
	title('Original Data')

	figure()
	plot(PX[0,:], PX[1,:], '.')
	title('Projected')
	show()

def generateTestData(N, p, pcas):
	def func(X):
		return X[0] 

def PCATest():
	from pylab import plot, show, figure, title, bar
	
	N = 20
	p = 4
	pcas = 2
	X = generateTestData(N, p, pcas)
	pca = PCA(X)
	vmax = VARIMAX(pca.pc())
	
	
	

if __name__ == "__main__":
	simplePCATest()
	
