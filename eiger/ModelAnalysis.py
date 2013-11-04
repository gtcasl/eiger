#
# \file ModelAnalysis.py
# \author Eric Anger <eanger@gatech.edu>
# \date August 30, 2012
#
# \brief A few tools to help with the analysis of performance models for validity
#
import numpy.linalg as la
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import math

def getResiduals(y,yhat):
    """ Calculates residuals """
    return [yi - yhati for yi,yhati in zip(y,yhat)]

def studentizResiduals(X, residuals, model):
    """
    Calculates the studentized residuals of the given regression
    """
    n = len(residuals)
    m = len(model.functions)

    hat = X*la.inv(X.T*X)*X.T
    sighatsquared = [1 / float(n - m - 1) * sum([rj**2 for j,rj in enumerate(residuals) if j!=i]) for i in range(len(residuals))]
    sresiduals = [ri / (math.sqrt(si) * math.sqrt(1-min(1,0, hat[i,i]))) for i,ri,si in zip(range(len(residuals)), residuals, sighatsquared)]

    print "n: %s" % n
    print "percent of |studentized residual| > 2 : %s" % str(len([x for x in sresiduals if abs(x) > 2])/float(len(sresiduals))*100)
    for i,x in enumerate(sresiduals):
        if(abs(x))>3:
            print "res[%s] : %s" % (i,x)

    return sresiduals

def plotStudentizedResiduals(sresiduals):
    """ 
    Plots histogram of studentized residuals superimposed with the
    'ideal' normal distribution.
    """
    nn,bins,patches = plt.hist(sresiduals, normed=True, bins=50, alpha=0.75)
    plt.plot(bins, mlab.normpdf(bins,0,1),'r--',linewidth=1)
    plt.show()

def plotResidualsVsPredicted(residuals,yhat):
    plt.plot(residuals,yhat,'ro')
    plt.xlabel('residuals')
    plt.ylabel('predicted values (yhat)')
    plt.show()

def plotMetricsVsResiduals(X,residuals,metricDescriptors):
    for i in range(X.shape[1]):
        pmin = min(X[:,i].flat)
        pmax = max(X[:,i].flat)
        plt.plot([pmin,pmax],[0,0],'k-',linewidth=1)
        plt.plot(X[:,i],residuals,'ro')
        plt.xlabel(metricDescriptors[i])
        plt.ylabel('residuals')
        plt.show()

