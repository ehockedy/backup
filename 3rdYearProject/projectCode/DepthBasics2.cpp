/*
Code for identfying position and then tracking location of a users hands using depth images obtained from a Microsoft Kinect v2.
Adapted from example code included in the Kinect SDK.
*/
#pragma once 
//#if 0
#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include "header.h"
#include "DepthBasics2.h"
#include <fstream>

#define PRINT( s ){wostringstream os_; os_ << s; OutputDebugStringW( os_.str().c_str() );} //Allows output to debug terminal below

using namespace cv;
using namespace std;
using namespace shark;


float getDist(cv::Point p, pixel p2)
{
	double mult = ((double)p2.depth / 255.0)*0.5 + 0.5;
	float xDist = p.x - p2.xpos;
	float yDist = p.y - p2.ypos;
	float dist = sqrt(xDist*xDist + yDist*yDist)*mult;
	float rounded = round(dist*1000.0) / 1000.0;
	return rounded;
}

/// <summary>
/// Constructor
/// </summary>
CDepthBasics::CDepthBasics() :
	imgD(Mat(Size(cDepthWidth, cDepthHeight), CV_8UC4, Scalar(0, 0, 0))),
	imgG(Mat(Size(cDepthWidth, cDepthHeight), CV_8UC1)),
	//imgD(Mat(Size(480, 360), CV_8UC4, Scalar(0, 0, 0))),
	//imgG(Mat(Size(480, 360), CV_8UC4, Scalar(0, 0, 0))),
	//run(0),
	minDepth(500),
	maxDepth(1200),
    m_pKinectSensor(NULL),
    m_pDepthFrameReader(NULL)
    //m_pDepthRGBX(NULL)
{
    // create heap storage for depth pixel data in RGBX format
    //m_pDepthRGBX = new RGBQUAD[cDepthWidth * cDepthHeight]; //RGBQUAD has 4 values
	depthArr = new BYTE[cDepthHeight * cDepthWidth]; //Stores the depth pixel values. Use this as it only stores 1 value for each pixel so faster processing.
	newPoints.first = pixel{ 0,0,0 }; //The first of the closest 2 pixels
	newPoints.second = pixel{ 0,0,0 }; //The second of the closest 2 pixels
	prevPoints.first = pixel{ 0,0,0 }; //The previous first closest pixel
	prevPoints.second = pixel{ 0,0,0 }; //The previous second closest pixel
	refreshFrame = 0;
	vid = VideoWriter("handsOutput1.avi", CV_FOURCC('M', 'P', '4', '2'), 15.0, Size(cDepthWidth, cDepthHeight), 1);
	doVid = false;
	//remove("output.txt");
	/*output.open("data.csv", ios::app);//fstream::out);
	
	ifstream ifs("trainedRF.model");
	boost::archive::polymorphic_text_iarchive ia(ifs);  //re train
	model.read(ia);
	ifs.close();

	train = true;*/
}
  

/// <summary>
/// Destructor
/// </summary>
CDepthBasics::~CDepthBasics()
{
	//output.close();
    /*if (m_pDepthRGBX)
    {
        delete [] m_pDepthRGBX;
        m_pDepthRGBX = NULL;
    }*/
	if (depthArr)
	{
		delete[] depthArr;
		depthArr = NULL;
	}

    // done with depth frame reader
    SafeRelease(m_pDepthFrameReader);

    // close the Kinect Sensor
    if (m_pKinectSensor)
    {
        m_pKinectSensor->Close();
    }
	
    SafeRelease(m_pKinectSensor);
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
void CDepthBasics::SetUp(bool hands, int k, int s, int check, bool draw, int step, int depth)
{
	maxDepth = depth;
	twoHands = hands;
	ksize = k;
	sigma = s;
	checkRadius = check;
	//namedWindow("Hand output", WINDOW_AUTOSIZE);
	InitializeDefaultSensor();
	Update(); //Fills in depthArr
	//PrepareImage();
	DWORD time = GetTickCount();
	while(GetTickCount() - time < 2000)//imgD.at<Vec3b>(Point(50,50))[0] == 205 && imgD.at<Vec3b>(Point(100, 50))[0] == 205 && imgD.at<Vec3b>(Point(50, 100))[0] == 205 && imgD.at<Vec3b>(Point(100, 100))[0] == 205)
	{
		Update();
		
	}
	FindHands(true, step, twoHands);
}

/// <summary>
/// Main processing function
/// </summary>
void CDepthBasics::Update()
{
    if (!m_pDepthFrameReader)
    {
        return;
    }

    IDepthFrame* pDepthFrame = NULL;

    HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

    if (SUCCEEDED(hr))
    {
        INT64 nTime = 0;
        IFrameDescription* pFrameDescription = NULL;
        int nWidth = 0;
        int nHeight = 0;
        USHORT nDepthMinReliableDistance = 0;
        USHORT nDepthMaxDistance = 3000;
        UINT nBufferSize = 0;
        UINT16 *pBuffer = NULL;

        hr = pDepthFrame->get_RelativeTime(&nTime);

        if (SUCCEEDED(hr))
        {
            hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
        }

        if (SUCCEEDED(hr))
        {
            hr = pFrameDescription->get_Width(&nWidth);
        }

        if (SUCCEEDED(hr))
        {
            hr = pFrameDescription->get_Height(&nHeight);
        }

        if (SUCCEEDED(hr))
        {
            hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
        }

        if (SUCCEEDED(hr))
        {
			nDepthMaxDistance = maxDepth;
        }

        if (SUCCEEDED(hr))
        {
            hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);            
        }

        if (SUCCEEDED(hr))
        {
            ProcessDepth(nTime, pBuffer, nWidth, nHeight, nDepthMinReliableDistance, nDepthMaxDistance);
        }

        SafeRelease(pFrameDescription);
    }

    SafeRelease(pDepthFrame);
	PrepareImage();
}

/// <summary>
/// Initializes the default Kinect sensor
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT CDepthBasics::InitializeDefaultSensor()
{
    HRESULT hr; //Data type that represents error conditions and warning conditions 

    hr = GetDefaultKinectSensor(&m_pKinectSensor); //Get the kinect sensor
    if (FAILED(hr))
    {
		cout << "CANT FIND KINECT";
        return hr;
    }

    if (m_pKinectSensor)
    {
        // Initialize the Kinect and get the depth reader
        IDepthFrameSource* pDepthFrameSource = NULL;

        hr = m_pKinectSensor->Open();

        if (SUCCEEDED(hr))
        {
            hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource); //Gets the source of the depth frames
        }

        if (SUCCEEDED(hr))
        {
            hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader); //Create a new reader for the depth frame source
        }

        SafeRelease(pDepthFrameSource); //Releases the pointer pDepthFrameSource and sets it to NULL
    }

    if (!m_pKinectSensor || FAILED(hr))
    {
		cout << "\nNo ready Kinect found!\n\n" << endl;
        return E_FAIL;
    }

    return hr;
}

/// <summary>
/// Handle new depth data
/// <param name="nTime">timestamp of frame</param>
/// <param name="pBuffer">pointer to frame data</param>
/// <param name="nWidth">width (in pixels) of input image data</param>
/// <param name="nHeight">height (in pixels) of input image data</param>
/// <param name="nMinDepth">minimum reliable depth</param>
/// <param name="nMaxDepth">maximum reliable depth</param>
/// </summary>
void CDepthBasics::ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nWidth, int nHeight, USHORT nMinDepth, USHORT nMaxDepth)
{
    // Make sure we've received valid data
    if (depthArr && pBuffer && (nWidth == cDepthWidth) && (nHeight == cDepthHeight))
    {
        //RGBQUAD* pRGBX = m_pDepthRGBX;
		BYTE* pDepth = depthArr;

        // end pixel is start + width*height - 1
        const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);

        while (pBuffer < pBufferEnd)
        {
            USHORT depth = *pBuffer;

            // Note: Using conditionals in this loop could degrade performance.
            // Consider using a lookup table instead when writing production code.

			if (depth > nMaxDepth || depth < nMinDepth)
			{
				depth = 1; //Make black, but not so much that causes division errors later.
			}
			else
			{
				double depth2 = (255.0 - ((((float)depth - nMinDepth) / (float)(nMaxDepth - nMinDepth)) * 254.0)); //Make closest pixels white, and make the deeper ones darker
				//double depth3 = 255 / pow(2, 1+255/(255-depth2));
				depth = (int)depth2; 
			}

			if (depth == 0) //Just a check
			{
				depth = 1;
			}

			/*BYTE intensity = static_cast<BYTE>(depth);

            pRGBX->rgbRed   = intensity;
            pRGBX->rgbGreen = intensity;
            pRGBX->rgbBlue  = intensity;*/

			BYTE intensity = static_cast<BYTE>(depth);
			*pDepth = intensity;
			++pDepth;

            //++pRGBX;
            ++pBuffer;
        }
    }

	//Draw();
}



void CDepthBasics::PrepareImage()
{
	imgD = Mat(Size(cDepthWidth, cDepthHeight), CV_8UC1, depthArr); //Must be CV_8UC1 because 1 colour channel
	//imgG = Mat(Size(cDepthWidth, cDepthHeight), CV_8UC1); //The matrix to be gaussian blurred
	if (ksize % 2 == 0) //sinze kernel size must be odd
	{
		ksize += 1;
	}
	GaussianBlur(imgD, imgG, Size(ksize, ksize), sigma); //65,65,150 are the current default blur parameters
	//GaussianBlur(imgG, imgG, Size(1, 45), 100); //65,65,150 are the current default blur parameters
	//Mat outline = imgD.clone();
	cvtColor(imgD, imgD, COLOR_GRAY2RGB);
	cvtColor(imgG, imgG, COLOR_GRAY2RGB);
}

void CDepthBasics::FindHands(bool refreshFrame, int step, bool twohands)
{
	if (refreshFrame)
	{
		if (twohands)
		{
			newPoints = initTwoHands(&imgG, step); //Search the whole image for the closest 2 points as opposed to locally for each
		}
		else
		{
			newPoints.first = initOneHand(&imgG, step);
		}
	}
	else
	{
		newPoints.first = findClosestInRange(&imgG, prevPoints.first, checkRadius, 1, step); //Search locally for the closest pixel around the first current closest
		if (twohands) {
			newPoints.second = findClosestInRange(&imgG, prevPoints.second, checkRadius, 2, step); //Search around the second
		}
	}
	prevPoints = newPoints;
}

pair<vector<Point>, Point> CDepthBasics::ProcessHand(pixel pix, bool draw)
{
	Mat hand1 = getHandArea(imgD, pix);
	//float proportion = getHandProportion(hand1, pix.depth);
	//cout << proportion << endl;
	vector<vector<Point> > contours = getContours(hand1, pix, &imgD);
	int contourIndex = getContourIndex(contours);
	vector<Point> convexHull = getHull(contours[contourIndex]);
	Point best = getMaxPoint(&imgD, convexHull, pix);
	if (draw)
	{
		drawHull(&imgD, convexHull); //Draw the hull points
		//drawBoxes(&imgD, pix); //Draw the boxes on the output image
		drawContours(imgD, contours, contourIndex, Scalar(255, 255, 0), 2); //Draw area around hand
		circle(imgD, best, 3, Scalar(0, 0, 255), -1); //Draw fingertip
	}
	return pair<vector<Point>, Point>(convexHull, best);
}

void CDepthBasics::DrawClassification(int c)
{
	putText(imgD, to_string(c), Point(30, 50), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 255), 3);
}

Mat CDepthBasics::Draw(int divisor) //divisor is how much the image is scaled down by
{
	Mat smallImg(cDepthHeight/divisor, cDepthWidth/ divisor, CV_8UC1);
	resize(imgD, smallImg, smallImg.size());
	//imshow("Hand output", smallImg);
	return smallImg;
}

bool CDepthBasics::inRange(int radius, pixel pix1, pixel pix2)
{
	bool isIn = false;
	double a = (pix1.xpos - pix2.xpos);
	double b = (pix1.ypos - pix2.ypos);
	if (sqrt(a*a + b*b) < radius)
	{
		isIn = true;
	}

	return isIn;
}

pixel CDepthBasics::initOneHand(Mat *img, int s)
{
	pixel closest; //Holds the values of the current second closest
	closest.depth = 0;
	int step = s; //The jump between pixels
	//find closest pixel
	for (int c = 0; c < img->cols; c = c + step)
	{
		for (int r = 0; r < img->rows; r = r + step)
		{
			if (img->at<Vec3b>(Point(c, r))[0] > closest.depth)
			{
				closest.xpos = c;
				closest.ypos = r;
				closest.depth = img->at<Vec3b>(Point(c, r))[0];
			}
		}
	}
	return closest;
}

pair<pixel, pixel> CDepthBasics::initTwoHands(Mat *img, int s) //Find the closest 2 pixels, that are at least a certain distance away from each other
{
	pixel closest; //Holds the values of the current closest
	closest.depth = 0;
	pixel closest2; //Holds the values of the current second closest
	closest2.depth = 0;
	int step = s; //The jump between pixels
	pixel testPix; //Holds the pixel currently being looked at

	//find closest pixel
	for (int c = 0; c < img->cols; c = c + step)
	{
		for (int r = 0; r < img->rows; r = r + step)
		{
			testPix.xpos = c;
			testPix.ypos = r;
			testPix.depth = img->at<Vec3b>(Point(c, r))[0]; //Point(c, r) because if without it does row then column (matrix style) as opposed to coordinate style.
			if (testPix.depth > closest.depth)
			{
				closest.xpos = c;
				closest.ypos = r;
				closest.depth = testPix.depth;
			}
		}
	}

	//find second closest pixel, that is out of range of the first one
	for (int c = 0; c < img->cols; c = c + step)
	{
		for (int r = 0; r < img->rows; r = r + step)
		{
			testPix.xpos = c;
			testPix.ypos = r;
			testPix.depth = img->at<Vec3b>(Point(c, r))[0];
			if (testPix.depth > closest2.depth && !inRange(75, closest, testPix)) //75 is an arbitrary value
			{
				closest2.xpos = c;
				closest2.ypos = r;
				closest2.depth = testPix.depth;
			}
		}
	}
	pair<pixel, pixel> points(closest, closest2);
	return points; //Return the closest 2 pixels
}

pixel CDepthBasics::findClosestInRange(Mat *img, pixel currentPix, int radius, int point, int s) //Find the closest pixel within a certain pixel range
{
	pixel closest;
	closest.xpos = currentPix.xpos;
	closest.ypos = currentPix.ypos;
	closest.depth = 1;
	pixel testPix;  //Holds the pixel currently being looked at
	int step = s;
	double multiplier;

	if (point == 1) //The search area differs based on whch point this is. 
	{
		multiplier = ((double)prevPoints.first.depth / 255.0)*0.5 + 0.5;
	}
	else if (point == 2)
	{
		multiplier = ((double)prevPoints.second.depth / 255.0)*0.5 + 0.5;
	}

	radius = radius*multiplier; //Scale the radius

	int cmin = max(0, currentPix.xpos - radius); //Make sure don't go outside of image
	int cmax = min(img->cols - 1, currentPix.xpos + radius);

	int rmin = max(0, currentPix.ypos - radius);
	int rmax = min(img->rows - 1, currentPix.ypos + radius);

	for (int c = cmin; c <= cmax; c = c + step) //Go through columns
	{
		for (int r = rmin; r <= rmax; r = r + step) //Go through rows
		{
			testPix.xpos = c;
			testPix.ypos = r;
			testPix.depth = img->at<Vec3b>(Point(c, r))[0];
			if (testPix.depth > closest.depth && ( //Find the point with the closest depth
				(point == 1 && !inRange(50, testPix, prevPoints.second)) //Check that if this is the first point, the test pixel is not the same as the previous second point 
				|| (point == 2 && !inRange(50, testPix, prevPoints.first)))
				&& inRange(radius, testPix, currentPix)) //Check that if this is the second point, the test pixel is not the same as the previous first point 
			{
				closest.xpos = c;
				closest.ypos = r;
				closest.depth = testPix.depth;
			}
		}
	}

	return closest;
}

Mat CDepthBasics::getHandArea(Mat img, pixel point)
{
	double mult = ((double)point.depth / 255.0)*0.5 + 0.5; //The multiplier used to make the box/checking areas around the centre pixel change size as the hand moves deeper

	int sideDist = (int)(120 * mult);
	int upDist = (int)(150 * mult);
	int downDist = (int)(70 * mult);

	int xcoor1 = max(point.xpos - sideDist, 0);
	int ycoor1 = max(point.ypos - upDist, 0);
	int xcoor2 = min(point.xpos + sideDist, img.cols-1);
	int ycoor2 = min(point.ypos + downDist, img.rows-1);

	Mat smallMat;
	img(Rect(Point(xcoor1, ycoor1), Point(xcoor2, ycoor2))).copyTo(smallMat);
	return smallMat;
}

float CDepthBasics::getHandProportion(Mat img, float minDepth)
{
	int step = 2;
	float threshold = minDepth * 0.8;
	int closeCounter = 0; //The number of pixels that are above the threshold
	int totalCounter = 0;
	for (int c = 0; c < img.cols; c = c + step) {
		for (int r = 0; r < img.rows; r = r + step) {
			if (((int)(img.at<Vec3b>(Point(c, r))[0])) >= threshold) { //If pixel is within depth range
				closeCounter++;
			}
			totalCounter++;
		}
	}
	return (float)(closeCounter) / (float)(totalCounter); //return thr proportion of pixels that are a similar intensity than the detected hand
}

void CDepthBasics::drawBoxes(Mat *img, pixel point)
{
	double multiplier = ((double)point.depth / 255.0)*0.5 + 0.5; //The multiplier used to make the box/checking areas around the centre pixel change size as the hand moves deeper

	//Specify the dimensions of the boxes drawn
	int centralPointSize = 2;
	int checkBoxSize = (int)(60 * multiplier);
	int mainBoxLeft = (int)(120 * multiplier);
	int mainBoxRight = (int)(120 * multiplier);
	int mainBoxUp = (int)(150 * multiplier);
	int mainBoxDown = (int)(70 * multiplier);

	int x = point.xpos;
	int y = point.ypos;

	rectangle(*img, Point(x - centralPointSize, y - centralPointSize), Point(x + centralPointSize, y + centralPointSize), Scalar(255, 0, 255), centralPointSize*2); //Draw the centre of the hand
	rectangle(*img, Point(x - mainBoxLeft, y - mainBoxUp), Point(x + mainBoxRight, y + mainBoxDown), Scalar(0, 255, 0), 3); //Draw a box around the area checked between frames
	rectangle(*img, Point(x - checkBoxSize, y - checkBoxSize), Point(x + checkBoxSize, y + checkBoxSize), Scalar(255, 0, 0), 3); //This area is the image that will be used to identify the hand

}

void CDepthBasics::drawPixels(Mat *img, pixel point, int size)
{
	int x = point.xpos;
	int y = point.ypos;
	for (int k = -size; k < size; k++) //colour that pixel
	{
		for (int l = -2; l < 3; l++)
		{
			int a = x + k;
			int b = y + l;
			if (a >= 0 && b >= 0 && a < img->cols && b < img->rows)
			{
				//imgToDrawOn->at<Vec3b>(a, b)[0] = 0;
				img->at<Vec3b>(Point(a, b))[1] = 0;
				//imgToDrawOn->at<Vec3b>(a, b)[2] = 255;
			}
		}
	}
}

vector<vector<Point> > CDepthBasics::getContours(Mat img, pixel p, Mat *imgDraw)
{
	//Used to get contours for finding convex hull points. Source: http://docs.opencv.org/2.4/doc/tutorials/imgproc/shapedescriptors/hull/hull.html 
	double mult = ((double)p.depth / 255.0)*0.5 + 0.5;
	Mat outImg = img.clone();
	cvtColor(outImg, outImg, COLOR_RGB2GRAY);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	int thresh = p.depth - 40;//the depth away from the central pixel that is allowed to be included in the hand region
	threshold(outImg, outImg, thresh, 255, THRESH_BINARY);

	/// Find contours
	int xoffset = max(0, p.xpos - (int)(120 * mult));
	int yoffset = max(0, p.ypos - (int)(150 * mult));
	findContours(outImg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(xoffset, yoffset));

	return contours;
}

int CDepthBasics::getContourIndex(vector<vector<Point> > contours)
{
	//Find the biggest group of contours - this will be the one that contains the outside shape of the hand
	int maxi = 0;
	int maxiSize = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > maxiSize)
		{
			maxi = i;
			maxiSize = contours[i].size();
		}
	}
	return maxi;
}

vector<Point> CDepthBasics::getHull(vector<Point> contours)
{
	//get set of convex hull points from the contours
	vector<Point> hull(contours.size());
	if (contours.size() > 0)
	{
		convexHull(Mat(contours), hull, false);
	}
	return hull;
}

void CDepthBasics::drawHull(Mat *img, vector<Point> hull)
{
	circle(*img, hull[0], 3, Scalar(0, 255, 0), -1);
	for (int i = 1; i< hull.size(); i++)
	{
		circle(*img, hull[i], 3, Scalar(255, 0, 255));// , 1, 8, vector<Vec4i>(), 0, Point());
	}
}


Point CDepthBasics::getMaxPoint(Mat *img, vector<Point> hull, pixel centre)
{
	Point best;
	double dist = 0;
	for (int i = 0; i < hull.size(); i++)
	{
		if (hull[i].y < centre.ypos && getDist(hull[i], centre) > dist)
		{
			dist = getDist(hull[i], centre);
			best = hull[i];
		}
	}
	return best;
}

pixel CDepthBasics::getHand1()
{
	return newPoints.first;
}

pixel CDepthBasics::getHand2()
{
	return newPoints.second;
}
//#endif