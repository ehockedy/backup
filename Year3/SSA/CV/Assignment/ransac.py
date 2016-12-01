"""
RANSAC things:
- Convert colour space because some images are very dark
- First do a canny edge detection to get thin obvious lines
- Then for RANSAC on Canny image, find any points that are the end of a line by going through all pixels and storing any that only have 1 adjacent white pixel
- Choose 2 random end pixels and do RANSAC i.e. find equation of lie between them, the for each x value between them, find the y value. If (x,y) is white then is obviously part of line between them. A line is there if a high proportion of white points lie in between the two crandomly chosen points
- Repeat this many times
- After repeats done, of the found lines, choose the 2 biggest as they are most likely to be the road sides. Could also check the angle i.e. won't be horizontal and unlikely to be vertical.
- Could also use colour value of found line to determine if line is road - it should be gray
- Could also take into account position in image i.e. road should start from bottom of image
- I think you conver to HSV because it is illumination invariant i.e. the h value is the same for different brightnesses

Colour space preprocessing things:
- Could find most common hue value as this should be the road, then have the lower and upper hue values a threshold either side of this
- Identify if a pixel is part of the road by checking if there are any blocked out bits by searching around the pixel, since the road shouldn'h have that many blocked out pixels, whereas side of road has lots
- May need to lower lower_sat as some pics do not work (e.g. 212, 174)
- checkAround works very well

- IMPLEMENT RANSAC THEN SEE HOW IT WORKS WITH CURRENT PREPROCESSING

Observations:
-bgr to hsv, s and v to 150, back to rgb work well
"""

#img.shape = (rows, cols, chans)
#img[row,col] = [b,g,r]
#img[row,col] = [h,s,v], saturation = intensity of colour, value = brightness 0 = bright

import cv2
import copy
import random

#windowName = "100%";
windowParam = "PARAMETERS";
#windowName3 = "10%";
#img = cv2.imread("road-images2016-DURHAM/vlcsnap-00317.png");
#imgB = cv2.imread("road-images2016-DURHAM/vlcsnap-00314.png")
#nums = ["00194"]#
nums = ["00390", "00413", "00131", "00292"]
ORIGINAL = []
EDITED = []

MULTIPLIER = 0.65

#radii = [14, 11, 8, 5, 2, 12, 4, 3, 2, 10, 1]
#radii = [5, 2, 4, 3, 2, 1]
#radii = [10, 7, 4, 8, 3, 1]
radii = [10, 7, 4, 8, 3, 5, 2, 1]
radii2 = [3,2,1,5]

for i in nums:
    name = "road-images2016-DURHAM/vlcsnap-" + i + ".png"
    ori = cv2.imread(name)
    edi = cv2.imread(name)
    ori = cv2.cvtColor(ori, cv2.COLOR_BGR2HSV)
    for col in range(0, ori.shape[1]):
            for row in range(0, (int)(ori.shape[0]*MULTIPLIER)):
                ori[row, col][1] = 0
                ori[row, col][2] = 255
    ori = cv2.cvtColor(ori, cv2.COLOR_HSV2BGR)
    ORIGINAL.append(ori)
    EDITED.append(edi)

def nothing(x):
    pass

fill = "WHITE"

def getX(y, m, c):
    return int((y-c)/m)

def getY(x, m, c):
    return int(m*x + c)

def getGradient(p1, p2):
    return (p1[0] - p2[0])/(p1[1]-p2[1])

def getC(x, y, g):
    return y-g*x

def ransac(imgIN, threshold, iterations):
    whitePoints = []
    width = imgIN.shape[1]
    height = imgIN.shape[0]
    percentInLine = 0
    lengthLine = 0
    bestPointA = (0,0)
    bestPointB = (height-1, width-1)
    bestGradient = 0
    bestC = 0
    percentInLine2 = 0
    lengthLine2 = 0
    bestPointA2 = (0,0)
    bestPointB2 = (height-1, width-1)
    bestGradient2 = 0
    bestC2 = 0
    for col in range(0,imgIN.shape[1]):
            for row in range((int)(imgIN.shape[0]*MULTIPLIER),imgIN.shape[0]): #//2 because dont care about top half
                if imgIN[row,col][0] == 255 and imgIN[row,col][1] == 255 and imgIN[row,col][2] == 255:
                    whitePoints.append((row,col))
    #print(len(whitePoints))
    for it in range(0,iterations):
        whiteCounter = 0
        pointCounter = 0
        points = random.sample(whitePoints, 2)
        while(points[0][0] == points[1][0] or points[0][1] == points[1][1]):
            points = random.sample(whitePoints, 2)
        pA = points[0]
        pB = points[1]
        gradient = getGradient(pA, pB) #(pA[0] - pB[0])/(pA[1]-pB[1]) #NEED TO CHANGE SOMETHING AS x IN CV GOES FROM TOP TO BOTTOM
        c = getC(pA[1], pA[0], gradient) #pA[0]-gradient*pA[1]
        xmax = getX(height-1, gradient, c) #int((imgIN.shape[0]-1 - c)/gradient) #when y value is maximum in the image i.e. 482
        if(xmax >= width): #If x max goes beyond the side boundary, make it the maximum possible x value for the image
            xmax = width - 1
        if(xmax < 0):
            xmax = 0
        xmin = getX(height*MULTIPLIER, gradient, c) #int((imgIN.shape[0]*MULTIPLIER - c)/gradient)
        if(xmin >= width): 
            xmin = width - 1
        if(xmin < 0):
            xmin = 0
            
        ymax = getY(width-1, gradient, c) #int((imgIN.shape[0]-1 - c)/gradient) #when y value is maximum in the image i.e. 482
        if(ymax >= height): #If x max goes beyond the side boundary, make it the maximum possible x value for the image
            ymax = height - 1
        if(ymax < 0):
            ymax = 0
        ymin = getY(0, gradient, c) 
        if(ymin >= height): 
            ymin = height - 1
        if(ymin < 0):
            ymin = height*MULTIPLIER
            
        if(xmax < xmin):
            xmin, xmax = xmax, xmin
        if(ymax < ymin):
            ymin, ymax = ymax, ymin
        for x in range(xmin, xmax):
            y = getY(x, gradient, c) #int(gradient*x + c)
            if(y < height) and (y >= height*MULTIPLIER):
                if(imgIN[y,x][0] == 255 and imgIN[y,x][1] == 255 and imgIN[y,x][2] == 255): #If the point is white i.e. part of an edge
                    whiteCounter+=1
                    pointCounter+=1
                else:
                    pointCounter+=1
        if gradient <= 0 and gradient > -1 and ((pointCounter > 50 and (whiteCounter/pointCounter) >= percentInLine) or (pointCounter > 25 and (whiteCounter/pointCounter) >= 0.8)):
            percentInLine = (whiteCounter/pointCounter)
            lengthLine = pointCounter
            bestPointA = (ymin,xmin)
            bestPointB = (ymax,xmax)
            bestGradient = gradient
            bestC = c
        elif gradient > 0 and gradient < 1 and ((pointCounter > 50 and (whiteCounter/pointCounter) >= percentInLine2) or (pointCounter > 25 and (whiteCounter/pointCounter) >= 0.8)):
            percentInLine2 = (whiteCounter/pointCounter)
            lengthLine2 = pointCounter
            bestPointA2 = (ymin,xmin)
            bestPointB2 = (ymax,xmax)
            bestGradient2 = gradient
            bestC2 = c
    for x in range(min(bestPointA[1], bestPointB[1]), max(bestPointA[1], bestPointB[1])):
            y = getY(x, bestGradient, bestC) #int(gradient*x + c)
            if(y < height) and (y >= height*MULTIPLIER):
                if(imgIN[y,x][0] == 255 and imgIN[y,x][1] == 255 and imgIN[y,x][2] == 255):
                    imgIN[y,x] = [0,255,0]
                else:
                    imgIN[y,x] = [0,0,255]
    for x in range(min(bestPointA2[1], bestPointB2[1]), max(bestPointA2[1], bestPointB2[1])):
            y = getY(x, bestGradient2, bestC2) #int(gradient*x + c)
            if(y < height) and (y >= height*MULTIPLIER):
                if(imgIN[y,x][0] == 255 and imgIN[y,x][1] == 255 and imgIN[y,x][2] == 255):
                    imgIN[y,x] = [0,255,0]
                else:
                    imgIN[y,x] = [255,0,0]
    #print("best gradient =", bestGradient, " best gradient2 =", bestGradient2, "line1 len:", lengthLine, "line2 len:", lengthLine2)#percentInLine, lengthLine, bestPointA, bestPointB)
    print("Done ransac")
    return imgIN

def mostCommon(img, attribute): #Attribute: 0 = hue, 1 = sat, 2 = val
    colours = {}
    best = 0
    step = 1
    for col in range(0,img.shape[1], step):
        for row in range((int)(img.shape[0]*MULTIPLIER),img.shape[0], step):
            if colours.get(img[row,col][attribute]):
                colours[img[row,col][attribute]]+=1
            else:
                colours[img[row,col][attribute]] = 1
            if colours[img[row,col][attribute]] > best:
                best = img[row,col][attribute]
    return best

def changeHue(imgIN, imgOUT, lowH, highH, lowS, highS, lowV, highV):
    xcutmin = imgIN.shape[1]*0.3
    xcutmax = imgIN.shape[1]*0.85
    ycutmax = imgIN.shape[0]*0.64
    for col in range(0,imgIN.shape[1]):
        for row in range((int)(imgIN.shape[0]*MULTIPLIER),imgIN.shape[0]): #//2 because dont care about top half
            if True:#not ((col > xcutmin and col < xcutmax) and row > ycutmax):
                currH = imgIN[row,col][0]
                currS = imgIN[row,col][1]
                currV = imgIN[row,col][2]
                if(currH < lowH or currH > highH or currS < lowS or currS > highS or currV < lowV or currV > highV):
                    if fill == "WHITE":
                        imgOUT[row,col][2] = 255
                        imgOUT[row,col][1] = 0
                    else:
                        imgOUT[row,col][2] = 0
    print("Done changeHue")
    return imgOUT

def checkAround(img, radii, fill): #Only acts on a single output image
    maxRadius = max(radii)
    for radius in radii:
        for col in range(maxRadius,img.shape[1]-maxRadius): #+/- radius because of overflow
            for row in range((int)(img.shape[0]*MULTIPLIER),img.shape[0]-maxRadius): #//2 because dont care about top half
            #for radius in radii:
                if not(img[row,col][2] == 255 and img[row,col][1] == 0): #if not already white
                    left = img[row, col - radius]
                    up = img[row - radius, col]
                    right = img[row, col + radius]
                    down = img[row + radius, col]
                    if ((up[2] == 255 and up[1] == 0) and (down[2] == 255 and down[1] == 0)) or ((left[2] == 255 and left[1] == 0) and (right[2] == 255 and right[1] == 0)):
                        img[row,col][2] = 255
                        img[row,col][1] = 0
    print("Done checkAround")    
    return img

def antiCheckAround(img, radii): #Only acts on a single output image
    maxRadius = max(radii)
    for radius in radii:
        for col in range(maxRadius,img.shape[1]-maxRadius): #+/- radius because of overflow
            for row in range((int)(img.shape[0]*MULTIPLIER),img.shape[0]-maxRadius): #//2 because dont care about top half
            #for radius in radii:
                if (img[row,col][2] == 255 and img[row,col][1] == 0): #if already white
                    left = img[row, col - radius]
                    up = img[row - radius, col]
                    right = img[row, col + radius]
                    down = img[row + radius, col]
                    if ((up[2] != 255 and up[1] != 0) and (down[2] != 255 and down[1] != 0)):
                        img[row,col][0] = (int(up[0])+int(down[0]))//2
                        img[row,col][1] = (int(up[1])+int(down[1]))//2
                        img[row,col][2] = (int(up[2])+int(down[2]))//2
                    elif ((left[2] != 255 and left[1] != 0) and (right[2] != 255 and right[1] != 0)):
                        img[row,col][0] = (int(left[0])+int(right[0]))//2
                        img[row,col][1] = (int(left[1])+int(right[1]))//2
                        img[row,col][2] = (int(left[2])+int(right[2]))//2
    print("Done antiCheckAround")
    return img

def averageOutX(img):
    for row in range((int)(img.shape[0]*MULTIPLIER),img.shape[0]):
        for col in range(2,img.shape[1]-2):
            #print((img[row,col-1][0] + img[row,col+1][0])//2)
            img[row,col][0] = (int(img[row,col-1][0]) + int(img[row,col+1][0]) + int(img[row,col-2][0]) + int(img[row,col+2][0]))//4
            img[row,col][1] = (int(img[row,col-1][1]) + int(img[row,col+1][1]) + int(img[row,col-2][1]) + int(img[row,col+2][1]))//4
            img[row,col][2] = (int(img[row,col-1][2]) + int(img[row,col+1][2]) + int(img[row,col-2][2]) + int(img[row,col+2][2]))//4
    return img

def averageOutY(img):
    for col in range(0,img.shape[1]):
        for row in range((int)(img.shape[0]*MULTIPLIER),img.shape[0]-2):
            #print((img[row,col-1][0] + img[row,col+1][0])//2)
            img[row,col][0] = (int(img[row-1,col][0]) + int(img[row+1,col][0]))//2
            img[row,col][1] = (int(img[row-1,col][1]) + int(img[row+1,col][1]))//2
            img[row,col][2] = (int(img[row-1,col][2]) + int(img[row+1,col][2]))//2
    return img
                
                                
if True:
    cv2.namedWindow(windowParam, cv2.WINDOW_NORMAL);
    lower_threshold = 70;
    cv2.createTrackbar("Lower Canny threshold", windowParam, lower_threshold, 255, nothing);
    upper_threshold = 145;
    cv2.createTrackbar("Upper Canny threshold", windowParam, upper_threshold, 255, nothing);
    smoothing_neighbourhood = 3;
    cv2.createTrackbar("Gaussian smoothing", windowParam, smoothing_neighbourhood, 15, nothing);
    sobel_size = 3; # greater than 7 seems to crash
    cv2.createTrackbar("Sobel size", windowParam, sobel_size, 7, nothing);
    lower_hue = 95; #h value goes from 0 to 179 (i.e. 180 degrees). Blue is 240/360 = 120
    upper_hue = 135;
    cv2.createTrackbar("Lower hue", windowParam, lower_hue, 179, nothing);
    cv2.createTrackbar("Upper hue", windowParam, upper_hue, 179, nothing);
    lower_sat = 10; #colour intensity
    upper_sat = 240;
    cv2.createTrackbar("Lower sat", windowParam, lower_sat, 255, nothing);
    cv2.createTrackbar("Upper sat", windowParam, upper_sat, 255, nothing);
    lower_val = 10; #colour brightness
    upper_val = 240;
    cv2.createTrackbar("Lower val", windowParam, lower_val, 255, nothing);
    cv2.createTrackbar("Upper val", windowParam, upper_val, 255, nothing);

    keep_processing = True
    update = "UPDATE."
    around = "B"
    q = False
    w = False
    e = False
    r = False
    while(keep_processing):
        for i in range(0,len(ORIGINAL)):
            ORIGINAL[i] = cv2.cvtColor(ORIGINAL[i], cv2.COLOR_BGR2HSV)
            EDITED[i] = copy.copy(ORIGINAL[i])
        
        lower_threshold = cv2.getTrackbarPos("Lower Canny threshold", windowParam);
        upper_threshold = cv2.getTrackbarPos("Upper Canny threshold", windowParam);
        smoothing_neighbourhood = cv2.getTrackbarPos("Gaussian smoothing", windowParam);
        sobel_size = cv2.getTrackbarPos("Sobel size", windowParam);
        lower_hue = cv2.getTrackbarPos("Lower hue", windowParam);
        upper_hue = cv2.getTrackbarPos("Upper hue", windowParam);
        lower_sat = cv2.getTrackbarPos("Lower sat", windowParam);
        upper_sat = cv2.getTrackbarPos("Upper sat", windowParam);
        lower_val = cv2.getTrackbarPos("Lower val", windowParam);
        upper_val = cv2.getTrackbarPos("Upper val", windowParam);

        if update == "UPDATE":
            if around == "A":
                checkAround(ORIGINAL, EDITED, radii, "WHITE")
            for img in range(0,len(ORIGINAL)):
                print("BEGIN", nums[img])
                attr = 0
                mc = mostCommon(ORIGINAL[img], attr)
                mc1 = mostCommon(ORIGINAL[img], 1)
                mc2 = mostCommon(ORIGINAL[img], 2)
                #print("mc:",mc, mc1, mc2)
                lower_hue = mc - 20
                upper_hue = mc + 20
                if lower_hue < 0:
                    lower_hue = 0
                if upper_hue > 255:
                    upper_hue = 255
                if q == True:
                    EDITED[img] = changeHue(ORIGINAL[img], EDITED[img], lower_hue, upper_hue, lower_sat, upper_sat, lower_val, upper_val)
                if w == True:
                    EDITED[img] = changeHue(ORIGINAL[img], EDITED[img], lower_hue, upper_hue, lower_sat, upper_sat, lower_val, upper_val)
                    EDITED[img] = checkAround(EDITED[img], radii, "WHITE")
                if e == True:
                    EDITED[img] = changeHue(ORIGINAL[img], EDITED[img], lower_hue, upper_hue, lower_sat, upper_sat, lower_val, upper_val)
                    EDITED[img] = checkAround(EDITED[img], radii, "WHITE")
                    EDITED[img] = antiCheckAround(EDITED[img], radii2)
                EDITED[img] = cv2.cvtColor(EDITED[img], cv2.COLOR_HSV2BGR)
                ORIGINAL[img] = cv2.cvtColor(ORIGINAL[img], cv2.COLOR_HSV2BGR)
                cv2.namedWindow(nums[img] + "2", cv2.WINDOW_NORMAL)
                cv2.moveWindow(nums[img]+"2", 10+img*330, 100)
                cv2.imshow(nums[img] + "2", EDITED[img])
                smoothed = cv2.GaussianBlur(EDITED[img],(smoothing_neighbourhood,smoothing_neighbourhood),0);
                EDITED[img] = cv2.Canny(smoothed, lower_threshold, upper_threshold, apertureSize=sobel_size); #CANNY CHANGES IT TO A 1 CHANNEL IMAGE
                EDITED[img] = cv2.cvtColor(EDITED[img], cv2.COLOR_GRAY2BGR)
                if r == True:
                    EDITED[img] = ransac(EDITED[img], 1,2000)
                cv2.namedWindow(nums[img], cv2.WINDOW_NORMAL)
                cv2.moveWindow(nums[img], 10+img*330, 400)
                cv2.imshow(nums[img], EDITED[img])
            print("PROCESSING FINISHED\n")
        
        else:
            for img in range(0,len(ORIGINAL)):
                EDITED[img] = cv2.cvtColor(EDITED[img], cv2.COLOR_HSV2BGR)
                ORIGINAL[img] = cv2.cvtColor(ORIGINAL[img], cv2.COLOR_HSV2BGR)
                #
                cv2.namedWindow(nums[img] + "2", cv2.WINDOW_NORMAL)
                cv2.moveWindow(nums[img]+"2", 10+img*330, 100)
                cv2.resizeWindow(nums[img]+"2", 320, 250);
                cv2.imshow(nums[img] + "2", ORIGINAL[img])
                cv2.namedWindow(nums[img], cv2.WINDOW_NORMAL)
                cv2.moveWindow(nums[img], 10+img*330, 400)
                cv2.resizeWindow(nums[img], 320, 250);
                cv2.imshow(nums[img], EDITED[img])
                #
                #cv2.namedWindow(nums[img], cv2.WINDOW_NORMAL)
                #cv2.imshow(nums[img], ORIGINAL[img])    
            
        key = cv2.waitKey(0);
        if (key == ord('x')):
            keep_processing = False;
        elif(key == ord('z')):
            update = "UPDATE"
            fill = "WHITE"
            around = "B"
        elif(key == ord('v')):
            update = "UPDATE"
            fill = "BLACK"
        elif(key == ord('a')):
            update = "UPDATE"
            around = "A"
        elif(key == ord('c')):
            update = "RESET"
            q = False
            w = False
            e = False
        elif(key == ord('q')):
            update = "UPDATE"
            q = True
            w = False
            e = False
        elif(key == ord('w')):
            update = "UPDATE"
            w = True
            q = False
            e = False
        elif(key == ord('e')):
            update = "UPDATE"
            e = True
            w = False
            q = False
        elif(key == ord('r')):
            update = "UPDATE"
            r = not (r and r)
            print("RANSAC =", r)
        else:
            update = "FALSE"


#else:
#    print("No image loaded from file.")


cv2.destroyAllWindows()


