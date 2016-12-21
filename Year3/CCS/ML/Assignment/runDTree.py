#BASED ON PROVIDED SURCE CODE
"""
TO RUN THE SCRIPT (same for runSVM):
This script lets you specify multiple parameters that you want to change
Fill in the lists to decide on the values you want, the list can hold as many values, but must have at least one.
The order of the program will be:
- selectlines.run(pc, seed) #LOCATED IN THE selectlines FILE - This takes in all the data and randomizes the
  data based on the seed, then splits it by putting pc% into the training data and the rest into the testing data.
- x = y.readData() #LOCATED IN THE RELEVANT MACHNE LEARNING TECHNIQUE file - This reads int he 4 data sets,
  testing/training attributes/classes and stores them in a tuple that is used in the next function
- correct, total = runX(params...) this carries out the training and testing of the specified machine learning technique.
  correct is the number of correctly classified features (true positives), total is the total size of the testing set.
- Once it has been run k times, the average accuracy is calculated and this is taken to be the overall accuracy.
- It then prints the best and the parameters used, as well as printing out that curret parameter value as it goes through each one

-Note that I have modified the selectlines and reader files. They still basically work the same
-selectlines find the files in the path \HAPT-data-set-DU\Train or \HAPT-data-set-DU\Test and writes them to text files in the current dirctory with the specified split between training and testing
-reader then reads in these recently written files and writes to 4 new files (dataTrain.txt, dataTest.txt, classTrain.txt, classTest.txt)
-By splitting up the original data files and new text files, it means the original files are unedited.
"""
import reader
import cv2
import numpy as np
import selectlines
import dtree as dt


#Variables to hold the best values
best = 0
bestd = 0
bestc = 0
bests = 0
bestp = 0
total = 0

ps = [90.0] #Training/testing percentage split
C = [2] #Max categories
D = [25] #Max tree depth
S = [1] #Minimum sample count
kmeansIterations = 4

#Initialise data
selectlines.run(ps[0], 0)#%that is training, seed set
data = dt.readData()

for p in ps: 
    print("P =", p)   
    for maxD in D:
        print(" maxD =",maxD)
        for maxC in C:
            print("  maxC =",maxC)
            for minS in S:
                print("    minS =",minS)
                count = 0
                for i in range(0, kmeansIterations):
                    selectlines.run(p, i)#%that is training, seed set
                    data = dt.readData() #reads in the data from the training and testing files   
                    correct, total = dt.runDTree(maxD, maxC, minS, data[0], data[1], data[2], data[3]) #Run the decision tree. This involves training and then testing all the featuer vectors
                    percent = (correct/total)*100.0 #The percentage of the test data that is correctly classified
                    count+=percent
                average = count/kmeansIterations
                print("      Average=", average)
                if average > best:
                    best = average
                    bestd = maxD
                    bestc = maxC
                    bests = minS
                    bestp = p
                    print("New best:", average)

print("Best:", best, "maxD:", bestd, "maxC:", bestc, "minS:", bests, "p:", bestp)
