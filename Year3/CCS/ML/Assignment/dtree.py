#BASED ON PROVIDED SURCE CODE
import reader
import cv2
import numpy as np

#Read in the data using functions specified in the reader file
def readData():
    trainingAttributes = reader.readTrainingAttributes()
    trainingLabels = reader.readTrainingLabels()
    testingAttributes = reader.readTestingAttributes()
    testingLabels = reader.readTestingLabels()
    return (trainingAttributes, trainingLabels, testingAttributes, testingLabels)


def runDTree(maxD, maxC, minS, trainingAttributes, trainingLabels, testingAttributes, testingLabels):
    dtree = cv2.ml.DTrees_create()
    dtree.setCVFolds(1);       #the number of cross-validation folds/iterations - fix at 1
    dtree.setMaxCategories(maxC); #max number of categories (use sub-optimal algorithm for larger numbers)
    dtree.setMaxDepth(maxD);       #max tree depth
    dtree.setMinSampleCount(minS); #min sample count
    dtree.setPriors(np.float32([1, 1, 1, 1, 1 , 1, 1, 1, 1, 1, 1, 1])); #TRY CHANGING THIS # the array of priors, the bigger weight, the more attention to the assoc. class
                                         #  (i.e. a case will be judjed to be maligant with bigger chance))
    dtree.setRegressionAccuracy(0);      # regression accuracy: N/A here
    dtree.setTruncatePrunedTree(True);   # throw away the pruned tree branches
    dtree.setUse1SERule(True);      # use 1SE rule => smaller tree
    dtree.setUseSurrogates(False);  # compute surrogate split, no missing data


    varTypes = np.array([cv2.ml.VAR_NUMERICAL]*561 + [cv2.ml.VAR_CATEGORICAL], np.float64)

    #Train the decision tree
    #ROW_SAMPLE means the training samples are the matrix rows (COL_SAMPLE is the alternative)
    dtree.train(cv2.ml.TrainData_create(trainingAttributes, cv2.ml.ROW_SAMPLE, trainingLabels.astype(int), varType = varTypes))

    correct = 0 #Hold the number of correct values
    for i in range(0, len(testingAttributes)-1):
        _, res = dtree.predict(testingAttributes[i], cv2.ml.ROW_SAMPLE)
        if res[0] == testingLabels[i]:
            correct += 1
    return correct, len(testingAttributes)
