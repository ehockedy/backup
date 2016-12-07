#The 561 attributes are all laid out in features.txt
#Each 561-long vector describes an action
#Each number in y-train is a classifier for a vector (i.e. nth number is the classified class of nth vector)
"""
- Could use data from testing as training data to increase accuracy
- Cross validation:
    - Split all data into training and testing
    - Train and run to find accuracy
    - Split data in a different way
    - Train and find accuracy again
    - Continue like this for i iterations
    - Overall accuracy is the average accuracy after run each time
Done:
- Tree
- Forest
- SVM
- NN (not working)
"""

 
import reader
import cv2
import numpy as np

trainingAttributes = reader.readTrainingAttributes()
trainingLabels = reader.readTrainingLabels()
testingAttributes = reader.readTestingAttributes()
testingLabels = reader.readTestingLabels()

params = {
        "cvfolds":1,
        "maxCategories":25,
        "maxDepth": 20,
        "minSampleCount":25,
        "setPriors":[1,1,1,1],
        "regressionAccuracy":0,
        "truncatePruneTree":True,
        "use1SERule":True,
        "useSurrogates":False
    }

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


varTypes = np.array([cv2.ml.VAR_NUMERICAL]*561 + [cv2.ml.VAR_CATEGORICAL], np.float64)

#Train the decision tree
#ROW_SAMPLE means the training samples are the matrix rows (COL_SAMPLE is the alternative)
#dtree.train(trainingAttributes, cv2.ml.ROW_SAMPLE, trainingLabels)
dtree.train(cv2.ml.TrainData_create(trainingAttributes, cv2.ml.ROW_SAMPLE, trainingLabels.astype(int), varType = varTypes))

#Still not quite sure what predict returns
correct = 0
a, b = dtree.predict(testingAttributes[0], cv2.ml.ROW_SAMPLE); #This is the prediction for a singe data input
for i in range(0, len(testingAttributes)):
    a, _ = dtree.predict(testingAttributes[i], cv2.ml.ROW_SAMPLE)
    if round(a) == testingLabels[i]:
        correct += 1
print(correct, "of", len(testingAttributes), "=", "%.2f" % (correct/len(testingAttributes)) + "%")
    
#a is the python thing to say it returns correctly
#b is the output, not sure why he just compares the first item in the list in the example code
#Probably best to just round the floats. Do what is done on the example code
#Decision trees, decision forests, boosted trees, etc are all separate methods
print(len(b))
print(a)
#a[:,0] means a list of the 0th (so first) attribute from all the lists (:th)
#a[0] == a[0,:]

