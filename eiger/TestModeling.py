#
#
#

import PCA
import ClusterAnalysis
import LinearRegression
import PerformanceModel

import numpy as np

#################################################################################################
#
#
def LoadSynthetic():
	"""
	Constructs a simple matrix of data in which two pincipal components drive four metrics.
	"""
	F = lambda x: (x[0], 2*x[1], 1.25*x[0], 0.24*x[0] + 1.0*x[1])

	# objective is to obscure these points with function F, then use PCA to recover
	# them.
	points = [
		(0, 0),
		(0.5, 0),
		(0, 0.5),
		(1,1),
		(1,2),
		(3,1),
		(3,3),
		(-2, 2),
		(-1, -1),
		(2, 2),
		(3, 0.5),
		(3, 0.75),
	]

	output = np.matrix([F(p) for p in points])
	metrics = ["A", "B", "C", "D"]
	results = []
	return (output, results, metrics, points)	

#################################################################################################
#
#
def TestSynthetic():
	cube, results, metricNames, points = LoadSynthetic()
	
	pca = PCA.PCA(cube)
	(V, S, components) = pca.PC(0.85)	
	C = cube * V
	
	print "S = ", S
	print "Components = ", components
	
	from pylab import bar, show, xticks, figure, plot, title
	
	figure()
	plot([p[0] for p in points], [p[1] for p in points], 'o', color='black')
	title('Original Data in PC Space')
	
	figure()
	PCA.PlotPCA(V, S, components)
	xticks([x+0.5 for x in range(0, len(metricNames))], metricNames, rotation=0)
	title('Principal Component Factor Loadings')
	
	figure()
	plot(C[:,0].flat, C[:,1].flat, 'o')
	title('Data projected onto PCs')
	show()

#################################################################################################
#
#
def TestModelingFramework():
	import data
	
	datacube = data.load()
	
	print "Loaded data"
	
	print "Machine PCA on ", datacube.profileMachine
	machine = PCA.PCA(datacube.profileMachine)
	(MV, MS, MComponents) = machine.PC(0.95)
	projectedMachineCube = datacube.profileMachine * MV
	
	print "Application profile PCA"
	pca = PCA.PCA(datacube.profileCube)
	(V, S, components) = pca.PC(0.85)
		
	projectedCube = np.hstack((datacube.profileCube * V, datacube.selectMachine(projectedMachineCube)))
	#projectedCube = datacube.profileCube * V
	
	print projectedCube
	
	print "Machine components = ", MComponents
	print "Machine data = ", projectedMachineCube
	print "Data Components = ", components
	print "Data cube shape = ", projectedCube.shape
	
	if True:
		selector = LinearRegression.LinearRegression(projectedCube, datacube.profilePerformance)
		pool = selector.polynomialPool([3 for i in range(0, projectedCube.shape[1])])
		functions, betas, indices, squaredError, trials = selector.select(pool)
		model = PerformanceModel.Model(functions, betas)
	
		if model == None:
			print "Singular"
		else:
			print "Success! Indices = ", indices, ", squared error = ", squaredError
			
			R = datacube.experimentalMachine * MV
			replicatedMachine = np.zeros((datacube.experimentalCube.shape[0], R.shape[1]))
			for r in range(datacube.experimentalCube.shape[0]):
				replicatedMachine[r,:] = R[0,:]
			
			prediction = model.evaluate(np.hstack((datacube.experimentalCube * V, replicatedMachine)))
			print "prediction.shape = ", prediction.shape
			print "experimental.shape = ", datacube.experimentalPerformance.shape
			
			from pylab import bar, plot, show, xticks, legend
			
			xx = range(0, prediction.shape[0])
			bar([x for x in xx], prediction.flat, 0.4, color='black')
			bar([x + 0.4 for x in xx], datacube.experimentalPerformance.flat, 0.4)
			xticks([x + 0.4 for x in xx], datacube.applications, rotation='90')
			legend(('Predicted', '8800GTX'), loc='upper right')
			show()

#################################################################################################
#
#
if __name__ == "__main__":
	TestModelingFramework()

#################################################################################################
#
#

