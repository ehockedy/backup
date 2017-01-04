#BASED ON PROVIDED SURCE CODE
import reader
import cv2
import numpy as np
import random

#Read in the data using functions specified in the reader file
def readData(n):
    att, lab = reader.readTrainingAttributes()
    random.seed(n)
    random.shuffle(att)
    random.seed(n)
    random.shuffle(lab)
    length = int(len(att)*0.9)
    trainAtt = np.array(att[0:length]).astype(np.float32)
    trainLab = np.array(lab[0:length]).astype(np.float32)
    testAtt = np.array(att[length:]).astype(np.float32)
    testLab = np.array(lab[length:]).astype(np.float32)
    return (trainAtt, trainLab, testAtt, testLab)


def runDTree(maxD, maxC, minS, trainAtt, trainLab, testAtt, testLab):
    dtree = cv2.ml.DTrees_create()
    dtree.setCVFolds(1);       #the number of cross-validation folds/iterations - fix at 1
    dtree.setMaxCategories(maxC); #max number of categories (use sub-optimal algorithm for larger numbers)
    dtree.setMaxDepth(maxD);       #max tree depth
    dtree.setMinSampleCount(minS); #min sample count
    #dtree.setPriors(np.float32([1, 1, 1, 1, 1 , 1, 1, 1, 1, 1, 1, 1])); #TRY CHANGING THIS # the array of priors, the bigger weight, the more attention to the assoc. class
                                         #  (i.e. a case will be judjed to be maligant with bigger chance))
    dtree.setRegressionAccuracy(0);      # regression accuracy: N/A here
    dtree.setTruncatePrunedTree(True);   # throw away the pruned tree branches
    dtree.setUse1SERule(True);      # use 1SE rule => smaller tree
    dtree.setUseSurrogates(False);  # compute surrogate split, no missing data


    varTypes = np.array([cv2.ml.VAR_NUMERICAL]*25 + [cv2.ml.VAR_CATEGORICAL], np.float64)

    #Train the decision tree
    #ROW_SAMPLE means the training samples are the matrix rows (COL_SAMPLE is the alternative)
    #dtree.train(cv2.ml.TrainData_create(trainAtt, cv2.ml.ROW_SAMPLE, trainLab.astype(int)))
    #cv2.ml.TrainData_create(
    #dtree.train()
    dtree.train(trainAtt, cv2.ml.ROW_SAMPLE, trainLab)#, 0, 0, varTypes, missingDataMask=miss)
    correct = 0 #Hold the number of correct values
    for i in range(0, len(testAtt)):
        _, res = dtree.predict(testAtt[i], cv2.ml.ROW_SAMPLE)
        #print(res[0], testLab[i])
        if res[0] == testLab[i]:
            correct += 1
    return correct, len(testAtt)
count = 0
for i in range(0, 10):
    trAtt, trLab, teAtt, teLen = readData(i)
    #print(trLab)
    c, l = runDTree(25, 2, 1, trAtt, trLab, teAtt, teLen)
    count += (c/l)*100
    print(str((c/l)*100) + "%")
print(count/10)
