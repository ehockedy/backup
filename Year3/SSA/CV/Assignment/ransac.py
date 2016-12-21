"""
TO RUN: just run the srcipt, it will show the RANSAC images one at a time, going from image 1 up to image 525
"""


import cv2
import copy
import random
import math
import numpy

def nothing(x):
    pass


def getX(y, m, c):
    return int((y-c)/m)

def getY(x, m, c):
    return int(m*x + c)

def getGradient(p1, p2):
    return (p1[0] - p2[0])/(p1[1]-p2[1])

def getC(x, y, g):
    return y-g*x

def distPoints(a,b):
    return math.sqrt((a[0]-b[0])**2 + (a[1]-b[1])**2)


#This finds 2 points and uses high school maths to find the equation of the line, gradient, etc.
#It then extends the line and finds the white points a long it.
#The higher proportion of white points to the length, the better (more likely to be a rode edge) the line is
#It returns the best 2 points
def ransac(imgIN, iterations, sideOfRoad):
    whitePoints = []
    width = imgIN.shape[1] #640
    height = imgIN.shape[0] #482
    percentInLine = 0
    lengthLine = 0
    bestPointA = (0,0)
    bestPointB = (height-1, width-1)
    bestGradient = 0
    bestC = 0
    ming = -2 #The minimum value the gradient can be
    maxg = 0 #Maximum value the gradient can be
    if sideOfRoad == "RIGHT": #The side of the road that is being checked
        ming = 0 #If its the right hand side, we want positive gradients only
        maxg = 2
    for col in range(0,imgIN.shape[1]):
        for row in range((int)(imgIN.shape[0]*MULTIPLIER),imgIN.shape[0]):
            if imgIN[row,col][0] == 255 and imgIN[row,col][1] == 255 and imgIN[row,col][2] == 255:
                whitePoints.append((row,col)) #FInd the white points in the canny image
    for it in range(0,iterations): #Iterate a number of times
        whiteCounter = 0
        pointCounter = 0
        points = random.sample(whitePoints, 2) #Choose 2 white points
        while(points[0][0] == points[1][0] or points[0][1] == points[1][1] or getGradient(points[0], points[1]) < ming or getGradient(points[0], points[1]) > maxg): #So dont have points with same x or y value, and makse sure the gradient is correct depending on which side of the page it is
            points = random.sample(whitePoints, 2)
        pA = points[0]
        pB = points[1]
        gradient = getGradient(pA, pB) 
        c = getC(pA[1], pA[0], gradient)
        xmax = getX(height-1, gradient, c) #Get the x value when y is the top of the part of the image we are looking at
        cont = False
        if(xmax >= width): #If x max goes beyond the side boundary, make it the maximum possible x value for the image
            xmax = width - 1
            cont = True #Only want lines that start from the bottom of the image
        if(xmax < 0):
            xmax = 0
            cont = True
        xmin = getX(height*MULTIPLIER, gradient, c) #Get the x value for the y value at the bottom of the image
        if(xmin >= width): 
            xmin = width - 1
            cont = True
        if(xmin < 0):
            xmin = 0
            cont = True
        if cont == True and sideOfRoad == "LEFT": #Dont want to have left side line starting from beyond the x = 0 as the road almost always stats from x > 0
            continue            
        if(xmax < xmin):
            xmin, xmax = xmax, xmin
        for x in range(xmin, xmax): #Go through each x value
            y = getY(x, gradient, c) #find the corresponding y value
            if(y < height) and (y >= height*MULTIPLIER): #Check its within the image
                if(imgIN[y,x][0] == 255 and imgIN[y,x][1] == 255 and imgIN[y,x][2] == 255): #If the point is white i.e. part of an edge
                    whiteCounter+=1 #Increase the number of white points by 1
                    pointCounter+=1
                else:
                    pointCounter+=1
        # 1st condition: right line must intersect with x=0 above 0.8*the height of the image (this stops any small corner lines)
        #The length of the line must be > 100
        #If the line is suitable, and better than the current best
        if pointCounter > 0 and (not (getY(xmin, gradient, c) > height*0.825 and sideOfRoad == "RIGHT")) and gradient < maxg and gradient > ming and ((pointCounter >= 100 and (whiteCounter/pointCounter) >= percentInLine) or (pointCounter > 50 and pointCounter < 100 and (whiteCounter/pointCounter) >= 0.9)):
            percentInLine = (whiteCounter/pointCounter) #Update the best line 
            lengthLine = pointCounter
            bestPointA = (xmin,getY(xmin, gradient, c))
            bestPointB = (xmax,getY(xmax, gradient, c))
            bestGradient = gradient
            bestC = c
    #print("Done ransac")
    return bestPointA, bestPointB #Return the 2 end points of the best line

def mostCommonHue(img): #Attribute: 0 = hue, 1 = sat, 2 = val
    colours = {}
    best = 0
    bestVal = 0
    step = 5
    total = 0
    for col in range(0,img.shape[1], step): #Check only every 5 pixels to recuce computation
        for row in range((int)(img.shape[0]*MULTIPLIER),img.shape[0], step):
            if img[row,col][0] > 0:
                total+=1 #Count the number of pixels
                if colours.get(img[row,col][0]):
                    colours[img[row,col][0]]+=1 #If this pixel value alrady seen, increase its frequency
                else:
                    colours[img[row,col][0]] = 1
                if colours[img[row,col][0]] > bestVal: #If not seen, put a new entry in the dictionary
                    best = img[row,col][0]
                    bestVal = colours[img[row,col][0]]
    return best, bestVal/total #Return the most common hue value, and the proportion of it.

def changeHue(imgIN, imgOUT, lowH, highH, lowS, highS, lowV, highV):
    for col in range(0,imgIN.shape[1]):
        for row in range((int)(imgIN.shape[0]*MULTIPLIER),imgIN.shape[0]): 
            currH = imgIN[row,col][0]
            currS = imgIN[row,col][1]
            currV = imgIN[row,col][2]
            if(currH < lowH or currH > highH or currS < lowS or currS > highS or currV < lowV or currV > highV):                
                #make it white - obviously not part of the road
                imgOUT[row,col][2] = 255 
                imgOUT[row,col][1] = 0
    #print("Done changeHue")
    return imgOUT

def fillGaps(img, radii, fill): #Checks the surrounding pixels. If they are white and this pixel is not, make it white to "fill in" and gaps so that Canny does not detec this as a small edge.
    maxRadius = max(radii)
    for radius in radii: #radii is the different distances away that get tested. I found that going betwen smaller and larger values worked. I chose to not just go for all numbers within a certain range for speed
        for col in range(maxRadius,img.shape[1]-maxRadius): #+/- radius because of overflow
            for row in range((int)(img.shape[0]*MULTIPLIER),img.shape[0]-maxRadius): 
            #for radius in radii:
                if not(img[row,col][2] == 255 and img[row,col][1] == 0): #if not already white
                    left = img[row, col - radius]
                    up = img[row - radius, col]
                    right = img[row, col + radius]
                    down = img[row + radius, col]
                    #Check ony in the 2 directions (horizontal and vertical)
                    #Unlikely to fill in parts of the road as there are not many white pixels, and the sides of the road are greater than the maximum search distance.
                    if ((up[2] == 255 and up[1] == 0) and (down[2] == 255 and down[1] == 0)) or ((left[2] == 255 and left[1] == 0) and (right[2] == 255 and right[1] == 0)):
                        img[row,col][2] = 255
                        img[row,col][1] = 0
    #print("Done checkAround")    
    return img

#Checks the surrounding pixels of a white puxel and if the surrounding are not white, this pixel becomes not-white.
def removeIslands(img, radii): #Only acts on a single output image
    maxRadius = max(radii)
    for radius in radii:
        for col in range(maxRadius,img.shape[1]-maxRadius): #+/- radius because of overflow
            for row in range((int)(img.shape[0]*MULTIPLIER),img.shape[0]-maxRadius): #//2 because dont care about top half
                if (img[row,col][2] == 255 and img[row,col][1] == 0): #if already white
                    left = img[row, col - radius]
                    up = img[row - radius, col]
                    right = img[row, col + radius]
                    down = img[row + radius, col]
                    #Check in only the horizontal and vertical direction because if were to do any 2, it would find the pixels on the edge of the road due to the nature of it going diagonally
                    if ((up[2] != 255 and up[1] != 0) and (down[2] != 255 and down[1] != 0)): #Check in the vertical direction
                        img[row,col][0] = (int(up[0])+int(down[0]))//2
                        img[row,col][1] = (int(up[1])+int(down[1]))//2
                        img[row,col][2] = (int(up[2])+int(down[2]))//2
                    elif ((left[2] != 255 and left[1] != 0) and (right[2] != 255 and right[1] != 0)): #Check in horizontal direction
                        img[row,col][0] = (int(left[0])+int(right[0]))//2
                        img[row,col][1] = (int(left[1])+int(right[1]))//2
                        img[row,col][2] = (int(left[2])+int(right[2]))//2
    #print("Done antiCheckAround")
    return img


def preProcessing(img, lower_hue, upper_hue, lower_sat, upper_sat, lower_val, upper_val, radii, radii2):
    img = cv2.cvtColor(img, cv2.COLOR_BGR2HSV) #Convert to HSV
    imgEdit = copy.copy(img) #Set the edited value
    #print("BEGIN")
    imgEdit = changeHue(img, imgEdit, lower_hue, upper_hue, lower_sat, upper_sat, lower_val, upper_val)
    imgEdit = removeIslands(imgEdit, radii2) ######COMMENT THIS OUT TO SPEED UP#######
    imgEdit = fillGaps(imgEdit, radii, "WHITE") ######COMMENT THIS OUT TO SPEED UP#######
    imgEdit = removeIslands(imgEdit, radii2) ######COMMENT THIS OUT TO SPEED UP#######

    #RETURN TO BGR 
    #img = cv2.cvtColor(img, cv2.COLOR_HSV2BGR)
    imgEdit = cv2.cvtColor(imgEdit, cv2.COLOR_HSV2BGR)
    
    #print("Pre-processing finished")
    return imgEdit

def ransacPrep(imgEdit, smoothing_neighbourhood, lower_threshold, upper_threshold, sobel_size):
    #PREPARE FOR RANSAC
    smoothed = cv2.GaussianBlur(imgEdit,(smoothing_neighbourhood,smoothing_neighbourhood),0);
    imgEdit = cv2.Canny(smoothed, lower_threshold, upper_threshold, apertureSize=sobel_size); #CANNY CHANGES IT TO A 1 CHANNEL IMAGE
    imgEdit = cv2.cvtColor(imgEdit, cv2.COLOR_GRAY2BGR)
    #print("Done Canny")
    return imgEdit

def printRansac(img, imgEdit, iterations):
    leftA, leftB = ransac(imgEdit, iterations, "LEFT") #LINE ON LEFT SIDE OF ROAD
    rightA, rightB = ransac(imgEdit, iterations, "RIGHT") #LINE ON RIGHT SIDE OF ROAD
    cv2.line(img, leftA,leftB, [0,0,255], 2)
    cv2.line(img, rightA,rightB, [0,0,255], 2)
    return img
    

if True:
    radii = [10, 7, 4, 8, 3, 5, 2, 1]
    radii2 = [5,4,3,2,1]
    i = 1
    fill = ""

    MULTIPLIER = 0.65
    
    #Canny threhold
    lower_threshold = 200;
    upper_threshold = 250;

    #RANSAC prep
    smoothing_neighbourhood = 3;
    sobel_size = 3 # greater than 7 seems to crash

    #Hue
    lower_hue = 100 #The minimum allowed hue value
    upper_hue = 150 #The maximum allowed hue value

    #Saturation
    lower_sat = 10 #colour intensity
    upper_sat = 240

    #Value
    lower_val = 10 #colour brightness
    upper_val = 240

    keep_processing = True
    while(i <= 525):
    #for i in [171, 36]:
        if i < 10:
            fill = "00"
        elif i < 100:
            fill = "0"
        else:
            fill = ""
        name = "road-images2016-DURHAM/vlcsnap-00" + fill + str(i) + ".png"
        #print(name)
        
        
        finalImg = cv2.imread(name)
        img = copy.copy(finalImg)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2HSV) #Convert to HSV
        
        for col in range(0, img.shape[1]): #Make the first 65% white
            for row in range(0, (int)(img.shape[0]*MULTIPLIER)):
                img[row, col][1] = 0
                img[row, col][2] = 255

        mc, percent = mostCommonHue(img)
        variance = 30 #The range above and below the average hue value that is allowd
        if percent < 0.1: #If the most common is not really that frequent
            variance = 40
        elif percent < 0.075: #If it is infrequent
            variance = 60
        elif percent < 0.05: #If it is very infrequent
            variance = 70
        lower_hue = max(mc - variance, 0)  #The minimum allowed hue value
        upper_hue = min(mc + variance, 255)
        
        img = cv2.cvtColor(img, cv2.COLOR_HSV2BGR) #Convert back to BGR
        preProcessed = preProcessing(img, lower_hue, upper_hue, lower_sat, upper_sat, lower_val, upper_val, radii, radii2)
        
        canny = ransacPrep(preProcessed, smoothing_neighbourhood, lower_threshold, upper_threshold, sobel_size)
        
        finalImg = printRansac(finalImg, canny, 3000)

        print(name, "detected 2 edges/lines")
        
        cv2.namedWindow("Output", cv2.WINDOW_NORMAL)
        cv2.imshow("Output", finalImg)
        
        #cv2.imwrite("outputImages/vlcsnap-00" + fill + str(i) + ".png", finalImg)

        i+=1
        key = cv2.waitKey(1);
        if key == ord('w'):
            i = (i-1)%525
            if i==0:
                i = 525
        elif key == ord('e'):
            i = (i+1)%525 
            if i == 0:
                i = 1
        elif key == ord('r'):
            i = random.randint(1,525)

        if (key == ord('x')):
            keep_processing = False;

cv2.destroyAllWindows()


