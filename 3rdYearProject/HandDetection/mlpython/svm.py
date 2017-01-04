#BASED ON PROVIDED SURCE CODE
import reader
import cv2
import numpy as np
import selectlines
import random
use_svm_autotrain = False;

def readData(n):
    att, lab = reader.readTrainingAttributes()
    random.seed(n)
    random.shuffle(att)
    random.seed(n)
    random.shuffle(lab)
    length = int(len(att)*0.85)
    trainAtt = np.array(att[0:length]).astype(np.float32)
    trainLab = np.array(lab[0:length]).astype(np.integer)
    testAtt = np.array(att[length:]).astype(np.float32)
    testLab = np.array(lab[length:]).astype(np.integer)
    return (trainAtt, trainLab, testAtt, testLab)

def runSVM(C, G, D, trainAtt, trainLab, testAtt, testLab): 
    
    #varTypes = np.array([cv2.ml.VAR_NUMERICAL]*25 + [cv2.ml.VAR_CATEGORICAL], np.float64)


    svm = cv2.ml.SVM_create()
    svm.setKernel(cv2.ml.SVM_POLY);

    #Small C makes the cost of misclassificaiton low ("soft margin"), thus allowing more of them for the sake of wider "cushion".
    #Large C makes the cost of misclassification high ('hard margin"), thus forcing the algorithm to explain the input data stricter and potentially overfit.
    svm.setC(C); # penalty constant on margin optimization

    svm.setType(cv2.ml.SVM_C_SVC); # multiple class (2 or more) classification

    svm.setGamma(G); # used for SVM_RBF kernel only, otherwise has no effect
    svm.setDegree(D);  # used for SVM_POLY kernel only, otherwise has no effect

    # set the relative weights importance of each class for use with penalty term
    #svm.setClassWeights(np.float32([1,1,1,1,1,1,1,1,1,1,1,1]));

    if (use_svm_autotrain) :
        # use automatic grid search across the parameter space of kernel specified above
        # (ignoring kernel parameters set previously)
        # if it is available : see https://github.com/opencv/opencv/issues/7224
        svm.trainAuto(cv2.ml.TrainData_create(training_attributes, cv2.ml.ROW_SAMPLE, training_class_labels.astype(int)), kFold=10);
    else :
        # use kernel specified above with kernel parameters set previously
        svm.train(trainAtt, cv2.ml.ROW_SAMPLE, trainLab);
        #svm.train(cv2.ml.TrainData_create(trainingAttributes, cv2.ml.ROW_SAMPLE, trainingLabels.astype(int), varType = varTypes))

    correct = 0
    for i in range(0, len(testAtt)):
        # (to get around some kind of OpenCV python interface bug, vertically stack the
        #  example with a second row of zeros of the same size and type which is ignored).

        sample = np.vstack((testAtt[i,:], np.zeros(len(testAtt[i,:])).astype(np.float32)));

        a, result = svm.predict(sample, cv2.ml.ROW_SAMPLE)

        if result[0] == testLab[i]:
            correct += 1
    return correct, len(testAtt)

best = 0
bk, bg, bd = 0, 0, 0 #best: k=anything, g=0.035, d=8, gives about 93% with 4 classes
count = 0
for i in range(0, 5):
    trAtt, trLab, teAtt, teLen = readData(i)
    #print(trLab)
    for k in range(50000, 51000, 500):
        k = k/10000
        for g in range(30, 40, 5):
            g = g/1000
            for d in range(7, 9, 1):
                c, l = runSVM(k, g, d, trAtt, trLab, teAtt, teLen)
                count += (c/l)*100
                if (c/l)*100 > best:
                    best = (c/l)*100
                    bk, bg, bd = k, g, d
                print(k, g, d, str((c/l)*100) + "%")
print(bk, bg, bd, best)
#print(count/10)
