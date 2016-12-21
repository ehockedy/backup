#BASED ON PROVIDED SURCE CODE
import csv
import cv2
import numpy as np
import math
import os

import random

########### Load Data Set - Example
path_to_data = "HAPT-data-set-DU"
trainx = "Train/x_train.txt"
trainy = "Train/y_train.txt"
testx = "Test/x_test.txt"
testy = "Test/y_test.txt"
# load full data set (unsplit)


def run(n, r):
        readerDataTrain=csv.reader(open(os.path.join(path_to_data, trainx),"rt", encoding='ascii'),delimiter=',')
        readerDataTest=csv.reader(open(os.path.join(path_to_data, testx),"rt", encoding='ascii'),delimiter=',')
        readerClassTrain=csv.reader(open(os.path.join(path_to_data, trainy),"rt", encoding='ascii'),delimiter=',')
        readerClassTest=csv.reader(open(os.path.join(path_to_data, testy),"rt", encoding='ascii'),delimiter=',')

        dataList = []
        for row in readerDataTrain:
                if len(row) == 0:
                        print(row)
                dataList.append(row)
        for row in readerDataTest: 
                if len(row) == 0:
                        print(row)
                dataList.append(row)

        random.seed(r)
        random.shuffle(dataList)
        
        classList = []
        for row in readerClassTrain:
                if len(row) == 0:
                        print(row)
                classList.append(row)
        for row in readerClassTest: 
                if len(row) == 0:
                        print(row)
                classList.append(row)

        random.seed(r)
        random.shuffle(classList)
                
        ########### Write Data Set - Example
        # write first N% of the entries to first file

        N = n

        fileA = open("dataTrain.txt", "wt", encoding='ascii')
        writerA = csv.writer(fileA, delimiter=',')
        writerA.writerows(dataList[0:int(math.floor(len(dataList)* (N/100.0)))])
        fileA.close()
        #print("dataTrain.txt done")
        # write the remaining (100-N)% of the entries of the second file

        fileB = open("dataTest.txt", "wt", encoding='ascii')
        writerB = csv.writer(fileB, delimiter=',')
        writerB.writerows(dataList[int(math.floor(len(dataList)* (N/100.0))):len(dataList)])
        fileB.close()
        #print("dataTest.txt done")


        ####

        fileC = open("classTrain.txt", "wt", encoding='ascii')
        writerC = csv.writer(fileC, delimiter=',')
        writerC.writerows(classList[0:int(math.floor(len(classList)* (N/100.0)))])
        fileC.close()
        #print("classTrain.txt done")
        fileD = open("classTest.txt", "wt", encoding='ascii')
        writerD = csv.writer(fileD, delimiter=',')
        writerD.writerows(classList[int(math.floor(len(classList)* (N/100.0))):len(classList)])
        fileD.close()
        #print("classTest.txt done")

        #print(len(dataList), len(classList))

#####################################################################
