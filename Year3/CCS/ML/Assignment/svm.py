import reader
import cv2
import numpy as np
import selectlines
use_svm_autotrain = False;

#selectlines.run(80.0, 2)

def readData():
    trainingAttributes = reader.readTrainingAttributes()
    print("Read training attributes")
    trainingLabels = reader.readTrainingLabels()
    print("Read training labels")
    testingAttributes = reader.readTestingAttributes()
    print("Read testing attributes")
    testingLabels = reader.readTestingLabels()
    print("Read testing labels")
    return (trainingAttributes, trainingLabels, testingAttributes, testingLabels)

def runSVM(C, G, D, trainingAttributes, trainingLabels, testingAttributes, testingLabels): #1.0, 1.0, 5.0
    
    varTypes = np.array([cv2.ml.VAR_NUMERICAL]*561 + [cv2.ml.VAR_CATEGORICAL], np.float64)


    svm = cv2.ml.SVM_create()
    svm.setKernel(cv2.ml.SVM_LINEAR);
    #svm.train(cv2.ml.TrainData_create(trainingAttributes, cv2.ml.ROW_SAMPLE, trainingLabels.astype(int), varType = varTypes))

    #Small C makes the cost of misclassificaiton low ("soft margin"), thus allowing more of them for the sake of wider "cushion".
    #Large C makes the cost of misclassification high ('hard margin"), thus forcing the algorithm to explain the input data stricter and potentially overfit.
    svm.setC(C); # penalty constant on margin optimization

    svm.setType(cv2.ml.SVM_C_SVC); # multiple class (2 or more) classification

    svm.setGamma(G); # used for SVM_RBF kernel only, otherwise has no effect
    svm.setDegree(D);  # used for SVM_POLY kernel only, otherwise has no effect

    # set the relative weights importance of each class for use with penalty term
    svm.setClassWeights(np.float32([1,1,1,1,1,1,1,1,1,1,1,1]));

    if (use_svm_autotrain) :
        # use automatic grid search across the parameter space of kernel specified above
        # (ignoring kernel parameters set previously)
        # if it is available : see https://github.com/opencv/opencv/issues/7224
        svm.trainAuto(cv2.ml.TrainData_create(training_attributes, cv2.ml.ROW_SAMPLE, training_class_labels.astype(int)), kFold=10);
    else :
        # use kernel specified above with kernel parameters set previously
        svm.train(trainingAttributes, cv2.ml.ROW_SAMPLE, trainingLabels);
        #svm.train(cv2.ml.TrainData_create(trainingAttributes, cv2.ml.ROW_SAMPLE, trainingLabels.astype(int), varType = varTypes))

    print("training done")
    correct = 0
    for i in range(0, len(testingAttributes)):
        # (to get around some kind of OpenCV python interface bug, vertically stack the
        #  example with a second row of zeros of the same size and type which is ignored).

        sample = np.vstack((testingAttributes[i,:], np.zeros(len(testingAttributes[i,:])).astype(np.float32)));

        a, result = svm.predict(sample, cv2.ml.ROW_SAMPLE)

        if result[0] == testingLabels[i]:
            correct += 1
    print(correct, "of", len(testingAttributes), "=", "%.2f" % (correct/len(testingAttributes)*100) + "%")

#data = readData()
#runSVM(1.0,1.0,5.0, data[0], data[1], data[2], data[3])

