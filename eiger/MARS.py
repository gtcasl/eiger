from __future__ import division
import numpy as np
import copy
import operator

class MARS:
    """
    Multivariate Adaptive Regression Splines
    
    A smarter way to do regression. Non-parametric modeling that learns the model
    functions from the data itself.
    """
    def __init__(self, X, Y, maxM, max_interactions, modelID=None, db=None):
        """
        Train the MARS model to up to maxM functions and an order
        of interactions up to max_interactions.
        """
        if modelID is not None:
            self.fromDatabase(modelID, db)
            return
        self.X = X
        self.Y = Y

        def helper1(i,j):
            return "3 %s %s" % (j, self.X[i,j])
        def helper2(i,j):
            return "4 %s %s" % (j, self.X[i,j])

        constant_function = "0"

        M = [[0,],]

        """
        Each element in lookup maps a function number to the actual lambda
        function object. This is used when actual experiments are run, but
        not during model creation.
        """
        lookup = [constant_function,]
        for i in range(self.X.shape[0]):
            for j in range(self.X.shape[1]):
                lookup.append(helper1(i,j))
                lookup.append(helper2(i,j))

        self.E = np.array([[self._decode(f)(x) for f in lookup] for x in self.X]) # each function evaluated for each row in self.X

        (betas, trainingError, oldU) = self.fit(M)

        while len(M) < maxM and trainingError > 0:
            bestErr = 0
            for i in range(1,len(lookup),2):
                for j in range(len(M)):
                    if len(M[j]) >= max_interactions:
                        continue
                    if i in M[j]: # restriction that each input can appear only once in a product
                        continue

                    M.append([g for g in M[j] if g != 0] + [i,])
                    M.append([g for g in M[j] if g != 0] + [i+1,])
                    (b,err,tempU) = self.fit(M,oldU)
                    candidates = [M.pop(),M.pop()]
                    if((trainingError - err) > bestErr and (trainingError - err) > 0):
                        bestErr = trainingError - err
                        winners = candidates
                        besttempU = tempU
            M.append(winners[1])
            M.append(winners[0])
            oldU = besttempU
            trainingError = trainingError - bestErr

        (betas, trainingError, oldU) = self.fit(M)
        (self.gcv, model, betas) = self.find_min_gcv(M,betas,trainingError)

        """ we want to decode the models so we don't have to keep lookup around """
        self.functions = [[lookup[y] for y in x] for x in model]
        self.weights = [x for x in betas.flat]

    def find_min_gcv(self, model, betas, tError):
        """
        Recursive call to find the minimum gcv be eliminating one term at
        a time from the model.
        """

        c = 3 # this is the number specified in the book for non-additive models
        r = len(model) # number of basis functions in the model. different than lambda??
        N = self.X.shape[0]
        K = (r - 1) / 2
        m_lambda = r + c * K
        gcv = tError * (1.0 / (N * (1 - m_lambda/N)**2))
        
        if(len(model) == 1):
            return (gcv, model, betas)
    
        candidateModel = []
        candidateBetas = []
        candidateErr = float('inf')
        for i in range(0,len(model)):
            candidate = []
            for j in range(0,i):
                candidate.append(model[j])
            for j in range(i+1,len(model)):
                candidate.append(model[j])

            (b,err,oldU) = self.fit(candidate)
            if(err - tError < candidateErr):
                candidateModel = candidate
                candidateBetas = b
                candidateErr = err
        
    
        (gcv_smaller, model_smaller, betas_smaller) = self.find_min_gcv(candidateModel, candidateBetas, candidateErr)
        if(gcv < gcv_smaller):
            return (gcv, model, betas)
        return (gcv_smaller, model_smaller, betas_smaller)

    def fit(self, model, oldU=None):
        """
        Performs model estimation minimizing residual sum-of-squares.

        Least-squares evaluation of supplied model. Returns betas and 
        training error. Model is list of lists of function indexes.

        Only builds the newest row if the old U matrix is provided,
        significantly enhancing performance.
        """
        if(oldU==None):
            U = np.ones((self.E.shape[0],len(model)))
            for i,h in enumerate(model):
                for f in h:
                    U[:,i] *= self.E[:,f]
        else:
            U = np.ones((self.E.shape[0],1))
            for f in model[-1]:
                U[:,0] *= self.E[:,f]
            U = np.hstack((oldU,U))

        (betas, residues, rank, s) = np.linalg.lstsq(U,self.Y)
        try:
            SSerr = residues[0,0]
        except IndexError:
            yhat = np.dot(U, betas)
            SSerr = np.sum((self.Y - yhat)**2)

        return (betas, SSerr, U)

    def evaluate(self,T):
        return np.array([reduce(operator.mul, [self._decode(f)(T) for f in h]) for h in self.functions])

    def poll(self,T):
        """
        Determine the y values of the given input vectors
        """
        U = self.evaluate(T)
        return np.dot(U, self.weights)

    def commit(self, db, datacollectionID, trainingComponents, machineComponents):
        """
        Commits this model to a database.
        """
        cursor = db.cursor()
        cursor.execute("""INSERT IGNORE INTO eiger_model(dataCollectionID, trainingPCs, machinePCs, regression) VALUES("%s","%s","%s","%s")""" % (datacollectionID, trainingComponents, machineComponents, 'mars'))
        self.modelID = db.insert_id()

        for (function, b) in zip(self.functions, self.weights):
            cursor.execute("""INSERT IGNORE INTO eiger_model_predictor(modelID, beta) VALUES("%s","%s")""" % (self.modelID, b))
            predID = db.insert_id()
            for basis in function:
                cursor.execute("""INSERT IGNORE INTO eiger_model_function(function) VALUES("%s")""" % (basis,))
                cursor.execute("""SELECT ID FROM eiger_model_function WHERE function LIKE '%s'""" % (basis,))
                funcID = cursor.fetchone()[0]
                cursor.execute("""INSERT IGNORE INTO eiger_model_map(predictorID,functionID) VALUES(%s,%s)""" % (predID, funcID))

        db.commit()

    def fromDatabase(self, modelID, db):
        """
        Retrieves a MARS model from database
        """
        self.functions = []
        self.weights = []
        cursor = db.cursor()
        cursor.execute("""SELECT ID,beta FROM eiger_model_predictor WHERE modelID = %s""" % (modelID,))
        rows = cursor.fetchall()
        for row in rows:
            cursor.execute("""SELECT t2.function FROM eiger_model_map as t1 JOIN eiger_model_function AS t2 ON t1.functionID = t2.ID where t1.predictorID = %s""" % (row[0],))
            self.functions.append(list(cursor.fetchall()[0]))
            self.weights.append(float(row[1]))

    def toFile(self, fid):
        """
        Writes out this performance model to the given file descriptor.

        Function encoding is as follows:
            0 - f(x) = 1
            3 j c - f(x) = (x[j] - c) if x[j] > c else 0.0
            4 j c - f(x) = (c - x[j]) if x[j] < c else 0.0
        """
        fid.write("[%s]" % (len(self.functions),))
        fid.write("(")
        for x in self.weights:
            fid.write("%s," % (x,))
        fid.write(")\n")
        for function in self.functions:
            fid.write("%s\n" % (' '.join(function)))

    def _decode(self, F):
        """
        Given an encoded string F, return a lambda expression evaluating that function
        """
        func = F.split()
        if(func[0] == '0'):
            return lambda x: 1
        elif(func[0] == '3'):
            j = int(func[1])
            c = float(func[2])
            return lambda x: x[j] - c if x[j] > c else 0.0
        elif(func[0] == '4'):
            j = int(func[1])
            c = float(func[2])
            return lambda x: c - x[j] if x[j] < c else 0.0
        else:
            raise ValueError('Invalid function encoding')


if __name__ == "__main__":
    print "Testing MARS..."

    f = np.array([[1,],[4,],[7,]])
    g = np.array([[8,],[2,],[5,]])

    np.random.seed(12345)
    trials = 60
    f = np.random.rand(trials,6)
    f = np.atleast_2d(f)
    """
    g = []
    for x in f:
        g.append( ((3-2) * np.random.random_sample() + 2) * x + .5*np.random.random_sample())
    g = np.array(g)
    """
    def func(i,j):
        res = []
        for x in i:
            res.append( ((3-2) * np.random.rand() + 2) * f[x[0],0] + \
                        ((5-4) * np.random.rand() + 4) * f[x[0],1] + \
                        ((9-7) * np.random.rand() + 7) * f[x[0],2] + \
                        ((8-7) * np.random.rand() + 7) * f[x[0],3] + \
                        ((9-8) * np.random.rand() + 8) * f[x[0],4] + \
                        ((2-1) * np.random.rand() + 1) * f[x[0],5] + \
                        8 * np.random.rand())
        return np.array(res)
    g = np.fromfunction(func,(trials,1))
    m = MARS(f,g,max_interactions=2,maxM=80)

    print "DONE!"

    dovals = False
    doxy = True

    if(dovals or doxy):
        import matplotlib.pyplot as plt

        test = f
#       test = np.array([[x/100.0 for x in range(0,100)]]).T
        res = m.poll(test)
        
        if(dovals):
            xx = [tt[0] for tt in test]
            yy = [rr[0] for rr in res]
            plt.scatter([ff[0] for ff in f], [gg[0] for gg in g])
            plt.scatter(xx, yy, c='red')
            plt.show()
        
        if(doxy):
            plt.scatter([gg for gg in g], [rr for rr in res])
            plt.show()

    pass
