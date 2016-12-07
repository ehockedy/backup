#####################################################################

# Example : load HAPT data set only
# basic illustrative python script

# For use with test / training datasets : HAPT-data-set-DU

# Author : Toby Breckon, toby.breckon@durham.ac.uk

# Copyright (c) 2014 / 2016 School of Engineering & Computing Science,
#                    Durham University, UK
# License : LGPL - http://www.gnu.org/licenses/lgpl.html

#####################################################################

import csv
import cv2
import os
import numpy as np

########### Define classes

classes = {} # define mapping of classes
inv_classes = {v: k for k, v in classes.items()}

########### Load Data Set

newDataSplit = True

if newDataSplit == True:
        path_to_data = "" #"HAPT-data-set-DU"
        trainx = "dataTrain.txt" #"Train/x_train.txt"
        trainy = "classTrain.txt" #"Train/y_train.txt"
        testx = "dataTest.txt" #"Test/x_test.txt"
        testy = "classTest.txt" #"Test/y_test.txt
else:
        path_to_data = "HAPT-data-set-DU"
        trainx = "Train/x_train.txt"
        trainy = "Train/y_train.txt"
        testx = "Test/x_test.txt"
        testy = "Test/y_test.txt"

# Training data - as currently split
def readTrainingAttributes():
        attribute_list = []
        reader=csv.reader(open(os.path.join(path_to_data, trainx),"rt", encoding='ascii'),delimiter=' ')
        for row in reader:
                if len(row) > 0:
                 # attributes in columns 0-561
                        attribute_list.append(list(row[i] for i in (range(0,561))))
        training_attributes=np.array(attribute_list).astype(np.float32)
        return training_attributes

def readTrainingLabels():
        label_list = []
        reader=csv.reader(open(os.path.join(path_to_data, trainy),"rt", encoding='ascii'),delimiter=' ')
        for row in reader:
                if len(row) > 0:
                        label_list.append(row[0])
        training_labels=np.array(label_list).astype(np.integer)#changed for SVM
        return training_labels


# Testing data - as currently split
def readTestingAttributes():
        attribute_list = []
        reader=csv.reader(open(os.path.join(path_to_data, testx),"rt", encoding='ascii'),delimiter=' ')
        for row in reader:
                if len(row) > 0:
                        # attributes in columns 0-561
                        attribute_list.append(list(row[i] for i in (range(0,561))))
        testing_attributes=np.array(attribute_list).astype(np.float32)
        return testing_attributes

def readTestingLabels():        
        label_list = []
        reader=csv.reader(open(os.path.join(path_to_data, testy),"rt", encoding='ascii'),delimiter=' ')
        for row in reader:
                if len(row) > 0:
                        # attributes in column 1
                        label_list.append(row[0])
        testing_labels=np.array(label_list).astype(np.integer)##CHANGED FOR SVM
        return testing_labels
        
###########  test output for sanity

"""print(training_attributes)
print(len(training_attributes))
print(training_labels)
print(len(training_labels))

print(testing_attributes)
print(len(testing_attributes))
print(testing_labels)
print(len(testing_labels))"""

#####################################################################

