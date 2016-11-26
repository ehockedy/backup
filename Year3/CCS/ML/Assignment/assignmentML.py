#The 561 attributes are all laid out in features.txt
#Each 561-long vector describes an action
#Each number in y-train is a classifier for a vector (i.e. nth number is the classified class of nth vector)
               
import reader
import cv2
import numpy as np

trainingAttributes = reader.readTrainingAttributes()
trainingLabels = reader.readTrainingLabels()
testingAttributes = reader.readTestingAttributes()
testingLabels = reader.readTestingLabels()

#Create the decision tree object
dtree = cv2.ml.DTrees_create()
dtree.setCVFolds(1);       # the number of cross-validation folds/iterations - fix at 1
dtree.setMaxCategories(25); # max number of categories (use sub-optimal algorithm for larger numbers)
dtree.setMaxDepth(20);       # max tree depth
dtree.setMinSampleCount(25); # min sample count
dtree.setPriors(np.float32([1,1,1,1]));  # the array of priors, the bigger weight, the more attention to the assoc. class
                                     #  (i.e. a case will be judjed to be maligant with bigger chance))
dtree.setRegressionAccuracy(0);      # regression accuracy: N/A here
dtree.setTruncatePrunedTree(True);   # throw away the pruned tree branches
dtree.setUse1SERule(True);      # use 1SE rule => smaller tree
dtree.setUseSurrogates(False);  # compute surrogate split, no missing data

#print(len(testingAttributes[0]))
#print(testingLabels[0])

varTypes = np.array([cv2.ml.VAR_NUMERICAL]*561 + [cv2.ml.VAR_CATEGORICAL], np.float64)

#Train the decision tree
#ROW_SAMPLE means the training samples are the matrix rows (COL_SAMPLE is the alternative)
#dtree.train(trainingAttributes, cv2.ml.ROW_SAMPLE, trainingLabels)
dtree.train(cv2.ml.TrainData_create(trainingAttributes, cv2.ml.ROW_SAMPLE, trainingLabels.astype(int), varType = varTypes))

#Still not quite sure what predict returns
a, b = dtree.predict(testingAttributes[0], cv2.ml.ROW_SAMPLE); #This is the prediction for a singe data input
#a is the python thing to say it returns correctly
#b is the output, not sure why he just compares the first item in the list in the example code
#Probably best to just round the floats. Do what is done on the example code
#Decision trees, decision forests, boosted trees, etc are all separate methods
print(b)

#a[:,0] means a list of the 0th (so first) attribute from all the lists (:th)
#a[0] == a[0,:]

