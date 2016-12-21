#BASED ON PROVIDED SURCE CODE
import reader
import cv2
import numpy as np
import selectlines
import svm

best = 0
bestc = 0
bestg = 0
bestd = 0
bestp = 0
total = 0

cs = [0.2] #best = 10
gs = [0.06] #best = 0.01
ds = [9] 
ps = [90.0] 
kmeansIterations = 4

for p in ps:
    print("P =", p)   
    for c in cs:
        print("C =",c)
        for g in gs:
            print("  G =",g)
            for d in ds:
                print("    D =",d)
                count = 0
                for i in range(0, kmeansIterations):
                    selectlines.run(p, i) #%that is training, seed set
                    data = svm.readData() #reads in the data from the training and testing files
                    correct, total = svm.runSVM(c, g, d, data[0], data[1], data[2], data[3]) #performs svm. first 3 params are C, gamma, and degree
                    percent = (correct/total)*100.0
                    count+=percent
                average = count/kmeansIterations
                print("      Average=", average)
                if average > best:
                    best = average
                    bestc = c
                    bestg = g
                    bestd = d
                    bestp = p
                    print("New best:", best)

print("Best:", best, "c:", bestc, "g:", bestg, "d:", bestd, "p:", bestp)

