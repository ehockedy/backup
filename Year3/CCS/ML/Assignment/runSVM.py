import reader
import cv2
import numpy as np
import selectlines
import svm

selectlines.run(80.0, 1) #%that is training, seed set
data = svm.readData() #reads in the data from the training and testing files
svm.runSVM(1.0,1.0,5.0, data[0], data[1], data[2], data[3]) #performs svm. first 3 params are C, gamma, and degree

#ADD OTHER METHOD, THEN EXPERIMENT AND DO CROSS VALIDATION
