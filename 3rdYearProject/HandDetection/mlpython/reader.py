#BASED ON PROVIDED SOURCE CODE
import csv
import cv2
import os
import numpy as np

########### Define classes

classes = {} # define mapping of classes
inv_classes = {v: k for k, v in classes.items()}

########### Load Data Set
path_to_data = "HAPT-data-set-DU"
trainx = "Train/x_train.txt"
trainy = "Train/y_train.txt"
testx = "Test/x_test.txt"
testy = "Test/y_test.txt"

# Training data - as currently split
def readTrainingAttributes():
        attribute_list = []
        label_list = []
        #missing_list = []
        reader=csv.reader(open("../trainingData1.txt","rt", encoding='ascii'),delimiter=',') #os.path.join(path_to_data, trainx)
        for entry in reader:
                entryList = []
                #missing = []
                label_list.append(entry[0])
                for i in range(1,26):
                        if entry[i] != '?':
                                entryList.append(float(entry[i]))
                                #missing.append(1)
                        else:
                                entryList.append(0)
                                #missing.append(1)
                #missing_list.append(missing)
                attribute_list.append(entryList)
        #training_attributes=np.array(attribute_list)#.astype(np.float32)
        #training_labels=np.array(label_list)
        return attribute_list, label_list
#att, lab = readTrainingAttributes()
#print(att[0], lab[0])
"""def readTrainingLabels():
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
"""
