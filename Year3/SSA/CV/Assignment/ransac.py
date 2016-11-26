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

nums = ["00007", "00111", "00194", "00255"]
ORIGINAL = []
EDITED = []

MULTIPLIER = 0.65

radii = [14, 11, 8, 5, 2, 12, 4, 3, 2, 10, 1]

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
    for col in range(0,imgIN.shape[1]):
            for row in range((int)(imgIN.shape[0]*MULTIPLIER),imgIN.shape[0]): #//2 because dont care about top half
                if imgIN[row,col][0] == 255 and imgIN[row,col][1] == 255 and imgIN[row,col][2] == 255:
                    whitePoints.append((row,col))
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
                    #print(whiteCounter, pointCounter)
                else:
                    pointCounter+=1
            #imgIN[y,x] = [0,0,255]
        """for y in range(ymin, ymax):
            x = getX(y, gradient, c)
            if(x >= width):
                x = width-1
            if(imgIN[y,x][0] == 255 and imgIN[y,x][1] == 255 and imgIN[y,x][2] == 255):
                whiteCounter += 1
                pointCounter +=1
            elif(imgIN[y,x][0] == 0 and imgIN[y,x][1] == 0 and imgIN[y,x][2] == 255): # if the point is part of an edge, but not already been counted
                pointCounter+=1
            imgIN[y,x] = [0,0,255]"""
        #print(((whiteCounter/pointCounter)*100), percentInLine, lengthLine)
        if (whiteCounter/pointCounter) >= percentInLine and pointCounter >= 100:
            percentInLine = (whiteCounter/pointCounter)
            lengthLine = pointCounter
            bestPointA = (ymin,xmin)
            bestPointB = (ymax,xmax)
            bestGradient = gradient
            bestC = c
            print(bestPointA, bestPointB)
            #print("Y")
    for x in range(min(bestPointA[1], bestPointB[1]), max(bestPointA[1], bestPointB[1])):
            y = getY(x, bestGradient, bestC) #int(gradient*x + c)
            if(y < height) and (y >= height*MULTIPLIER):
                if(imgIN[y,x][0] == 255 and imgIN[y,x][1] == 255 and imgIN[y,x][2] == 255):
                    imgIN[y,x] = [0,255,0]
                else:
                    imgIN[y,x] = [0,0,255]
    print(percentInLine, lengthLine, bestPointA, bestPointB)
    return imgIN

def changeHue(imgIN, imgOUT, lowH, highH, lowS, highS, lowV, highV):
    for col in range(0,imgIN.shape[1]):
        for row in range((int)(imgIN.shape[0]*MULTIPLIER),imgIN.shape[0]): #//2 because dont care about top half
            currH = imgIN[row,col][0]
            currS = imgIN[row,col][1]
            currV = imgIN[row,col][2]
            if(currH < lowH or currH > highH or currS < lowS or currS > highS or currV < lowV or currV > highV):
                if fill == "WHITE":
                    imgOUT[row,col][2] = 255
                    imgOUT[row,col][1] = 0
                else:
                    imgOUT[row,col][2] = 0
    return imgOUT

def checkAround(img, radii, fill): #Only acts on a single output image
    maxRadius = max(radii)
    for col in range(maxRadius,img.shape[1]-maxRadius): #+/- radius because of overflow
        for row in range((int)(img.shape[0]*MULTIPLIER),img.shape[0]-maxRadius): #//2 because dont care about top half
            for radius in radii:
                left = img[row, col - radius]
                up = img[row - radius, col]
                right = img[row, col + radius]
                down = img[row + radius, col]
                if ((up[2] == 255 and up[1] == 0) and (down[2] == 255 and down[1] == 0)) or ((left[2] == 255 and left[1] == 0) and (right[2] == 255 and right[1] == 0)):
                    img[row,col][2] = 255
                    img[row,col][1] = 0
    return img


def isRoad(imgsIN, imgsOUT, radius, fillVal):
    for i in range(0, len(imgsOUT)):
        img = imgsOUT[i]
        for col in range(0,img.shape[1]):
            for row in range((int)(img.shape[0]*MULTIPLIER),img.shape[0]): #//2 because dont care about top half
                for x in range(-radius, radius):
                    for y in range(-radius, radius):
                        if row + x < 482 and col + y < 640:
                            pix = img[row+x, col+y]
                            if fillVal == "WHITE":
                                if (pix[2] == 255 and pix[1] == 0): #if there is a blocked out pixel in the surrounding location # or (pix[0] == 30 and pix[1] == 255 and pix[2] == 255):
                                    x = radius
                                    y = radius
                                    img[row, col][2] = 254 #block that pixel, but not so affects next check
                                    img[row, col][1] = 0
                                elif(x == radius-1 and y == radius-1): #if have checked all pixels, and no surrounding pixels are blocked out
                                    img[row, col][0] = 30
                                    img[row, col][1] = 255
                                    img[row, col][2] = 255
                            else:
                                if (pix[2] == 0): #if there is a blocked out pixel in the surrounding location # or (pix[0] == 30 and pix[1] == 255 and pix[2] == 255):
                                    x = radius
                                    y = radius
                                    img[row, col][2] = 1
                                elif(x == radius-1 and y == radius-1): #if have checked all pixels, and no surrounding pixels are blocked out
                                    img[row, col][0] = 30
                                    img[row, col][1] = 255
                                    img[row, col][2] = 255
                
                                
if True:
    
        #cv2.namedWindow(nums[i], cv2.WINDOW_NORMAL)
        #ORIGINAL[i] = cv2.cvtColor(ORIGINAL[i], cv2.COLOR_HSV2BGR)
        #cv2.imshow(nums[i], ORIGINAL[i])
        


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
    while(keep_processing):
        for i in range(0,len(ORIGINAL)):
            ORIGINAL[i] = cv2.cvtColor(ORIGINAL[i], cv2.COLOR_BGR2HSV)
            EDITED[i] = copy.copy(ORIGINAL[i])
            #EDITED[i] = cv2.cvtColor(EDITED[i], cv2.COLOR_BGR2HSV)
            
        """for i in range(0, len(ORIGINAL)):
            ORIGINAL[i] = cv2.cvtColor(ORIGINAL[i], cv2.COLOR_BGR2HSV)
            EDITED[i] = cv2.cvtColor(EDITED[i], cv2.COLOR_BGR2HSV)"""
        
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

        """for img in range(0,len(ORIGINAL)):
            EDITED[img] = ORIGINAL[img]"""

        if update == "UPDATE":
            #lower_hue = lower_hue2
            #upper_hue = upper_hue2
            #changeHue(ORIGINAL, EDITED, lower_hue, upper_hue, lower_sat, upper_sat, lower_val, upper_val)
            if around == "A":
                checkAround(ORIGINAL, EDITED, radii, "WHITE")
            print("DONE")
            for img in range(0,len(ORIGINAL)):
                EDITED[img] = changeHue(ORIGINAL[img], EDITED[img], lower_hue, upper_hue, lower_sat, upper_sat, lower_val, upper_val)
                EDITED[img] = checkAround(EDITED[img], radii, "WHITE")
                EDITED[img] = checkAround(EDITED[img], radii, "WHITE")
                EDITED[img] = checkAround(EDITED[img], radii, "WHITE")
                EDITED[img] = cv2.cvtColor(EDITED[img], cv2.COLOR_HSV2BGR)
                ORIGINAL[img] = cv2.cvtColor(ORIGINAL[img], cv2.COLOR_HSV2BGR)
                cv2.namedWindow(nums[img] + "2", cv2.WINDOW_NORMAL)
                cv2.moveWindow(nums[img]+"2", 10+img*330, 100)
                cv2.imshow(nums[img] + "2", EDITED[img])
                smoothed = cv2.GaussianBlur(EDITED[img],(smoothing_neighbourhood,smoothing_neighbourhood),0);
                EDITED[img] = cv2.Canny(smoothed, lower_threshold, upper_threshold, apertureSize=sobel_size); #CANNY CHANGES IT TO A 1 CHANNEL IMAGE
                EDITED[img] = cv2.cvtColor(EDITED[img], cv2.COLOR_GRAY2BGR)
                EDITED[img] = ransac(EDITED[img], 1,1000)
                cv2.namedWindow(nums[img], cv2.WINDOW_NORMAL)
                cv2.moveWindow(nums[img], 10+img*330, 400)
                cv2.imshow(nums[img], EDITED[img])
                print("Done")
        
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
        
        """if(update == "UPDATE"):# and (lower_hue2 != lower_hue or upper_hue2 != upper_hue)):
            for img in range(0,len(ORIGINAL)):
                EDITED[img] = ORIGINAL[img]
            lower_hue = lower_hue2
            upper_hue = upper_hue2
            changeHue(ORIGINAL, EDITED, lower_hue, upper_hue)
        elif(update == "RESET"):
            for img in range(0,len(ORIGINAL)):
                EDITED[img] = ORIGINAL[img]
 
        smoothing_neighbourhood = max(3, smoothing_neighbourhood);
        if not(smoothing_neighbourhood % 2):
            smoothing_neighbourhood = smoothing_neighbourhood + 1;

        sobel_size = max(3, sobel_size);
        if not(sobel_size % 2):
            sobel_size = sobel_size + 1;

        for img in range(0,len(ORIGINAL)):
            EDITED[img] = cv2.cvtColor(EDITED[img], cv2.COLOR_HSV2BGR)
            ORIGINAL[img] = cv2.cvtColor(ORIGINAL[img], cv2.COLOR_HSV2BGR)
            if update == "UPDATE":
                #EDITED[img] = ORIGINAL[img]
                smoothed = cv2.GaussianBlur(EDITED[img],(smoothing_neighbourhood,smoothing_neighbourhood),0);
                EDITED[img] = cv2.Canny(smoothed, lower_threshold, upper_threshold, apertureSize=sobel_size); #CANNY CHANGES IT TO A 1 CHANNEL IMAGE
                EDITED[img] = cv2.cvtColor(EDITED[img], cv2.COLOR_GRAY2BGR)
            cv2.namedWindow(nums[img], cv2.WINDOW_NORMAL)
            cv2.imshow(nums[img], EDITED[img])
            #EDITED[img] = ORIGINAL[img]"""


        
            
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
        else:
            update = "FALSE"

        """for img in range(0,len(ORIGINAL)):
            EDITED[img] = ORIGINAL[img]"""

#else:
#    print("No image loaded from file.")


cv2.destroyAllWindows()


