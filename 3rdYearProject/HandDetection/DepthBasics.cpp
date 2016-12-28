/*
Code for identfying position and then tracking location of a users hands using depth images obtained from a Microsoft Kinect v2.
Adapted from example code included in the Kinect SDK.
*/

#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include "DepthBasics.h"

#include <opencv2/opencv.hpp>

#define PRINT( s ){wostringstream os_; os_ << s; OutputDebugStringW( os_.str().c_str() );} //Allows output to debug terminal below

using namespace cv;
using namespace std;

/// <summary>
/// Entry point for the application
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="hPrevInstance">always 0</param>
/// <param name="lpCmdLine">command line arguments</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
/// <returns>status</returns>
int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    CDepthBasics application;
    application.Run(hInstance, nShowCmd);
}

/// <summary>
/// Constructor
/// </summary>
CDepthBasics::CDepthBasics() :
	imgD(Mat(Size(cDepthWidth, cDepthHeight), CV_8UC4, Scalar(0, 0, 0))),
	imgG(Mat(Size(cDepthWidth, cDepthHeight), CV_8UC1)),
	//imgD(Mat(Size(480, 360), CV_8UC4, Scalar(0, 0, 0))),
	//imgG(Mat(Size(480, 360), CV_8UC4, Scalar(0, 0, 0))),
	run(0),
	minDepth(500),
	maxDepth(1000),
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
}
  

/// <summary>
/// Destructor
/// </summary>
CDepthBasics::~CDepthBasics()
{
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
int CDepthBasics::Run(HINSTANCE hInstance, int nCmdShow)
{
	namedWindow("window1", WINDOW_AUTOSIZE);
	InitializeDefaultSensor();
	
    // Main message loop
    while (run == 0)
    {
        Update();
    }

	return 0;
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
        USHORT nDepthMaxDistance = 1000;
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
			nDepthMaxDistance = 1000;
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
        PRINT("\nNo ready Kinect found!\n\n");
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
				float depth2 = (255.0 - ((((float)depth - nMinDepth) / (float)(nMaxDepth - nMinDepth)) * 254.0)); //Make closest pixels white, and make the deeper ones darker
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

	Draw();
}

void CDepthBasics::Draw()
{
	imgD = Mat(Size(cDepthWidth, cDepthHeight), CV_8UC1, depthArr); //Must be CV_8UC1 because 1 colour channel
	//imgG = Mat(Size(cDepthWidth, cDepthHeight), CV_8UC1); //The matrix to be gaussian blurred
	GaussianBlur(imgD, imgG, Size(45, 45), 100); //65,65,150 are the current default blur parameters
	//GaussianBlur(imgG, imgG, Size(1, 45), 100); //65,65,150 are the current default blur parameters
	Mat outline = imgD.clone();
	cvtColor(imgD, imgD, COLOR_GRAY2RGB);
	cvtColor(imgG, imgG, COLOR_GRAY2RGB);

	int checkSize = 60; //The size of the area around the current center of the hand which will be checked for the next best pixel 
						//Still not sure of best size. Small medium and large seem to all have separate benefits. 120 actually works quite well.

	double multiplier = 1;

	if (refreshFrame == 0)
	{
		newPoints = findClosest2(&imgG); //Search the whole image for the closest 2 points as opposed to locally for each
	}
	else
	{
		multiplier = ((double)prevPoints.first.depth / 255.0)*0.5 + 0.5;
		newPoints.first = findClosestInRange(&imgG, prevPoints.first, checkSize, 1); //Search locally for the closest pixel around the first current closest
		multiplier = ((double)prevPoints.second.depth / 255.0)*0.5 + 0.5;
		newPoints.second = findClosestInRange(&imgG, prevPoints.second, checkSize, 2); //Search around the second
	}

	Mat hand1 = getHandArea(imgD, newPoints.first);
	Mat hand2 = getHandArea(imgD, newPoints.second);
	//cvtColor(hand1, hand1, COLOR_RGB2GRAY);
	//drawHandOutline(&imgD, &hand1);


	vector<Point> convexHull = getHull(hand1, newPoints.first, &imgD);
	for (int i = 0; i< convexHull.size(); i++)
	{
		Scalar color = Scalar(255, 255, 0);
		circle(imgD, convexHull[i], 3, Scalar(0, 0, 255));// , 1, 8, vector<Vec4i>(), 0, Point());
	}

	vector<Point> convexHull2 = getHull(hand2, newPoints.second, &imgD);
	for (int i = 0; i< convexHull2.size(); i++)
	{
		Scalar color = Scalar(255, 0, 255);
		circle(imgD, convexHull2[i], 3, Scalar(255, 0, 255));// , 1, 8, vector<Vec4i>(), 0, Point());
	}

	//drawBoxes(&imgD, newPoints.first); //Draw the boxes on the output image
	//drawBoxes(&imgD, newPoints.second);

	prevPoints = newPoints; //Make the new points become the previous points, ready for the net frame
	refreshFrame = ((refreshFrame + 1) % 100) + 1; //Remove the + 1 to enable a global search for the 2 points every 100 frames.

	imshow("window1", imgD);
	if (doVid)
	{
		vid.write(imgD);
	}
	int key = waitKey(25);//Number of ms image is displayed for. 1s = 1000ms. Increasing this decreases FPS. Perhaps do this if need to make it run better. 16.666 = 60fps, 25 = 40fps
	if (key != -1)
	{
		run = 1; //Terminate the program
	}
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

pair<pixel, pixel> CDepthBasics::findClosest2(Mat *img) //Find the closest 2 pixels, that are at least a certain distance away from each other
{
	pixel closest; //Holds the values of the current closest
	closest.depth = 0;
	pixel closest2; //Holds the values of the current second closest
	closest2.depth = 0;
	int step = 3; //The jump between pixels
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

pixel CDepthBasics::findClosestInRange(Mat *img, pixel currentPix, int radius, int point) //Find the closest pixel within a certain pixel range
{
	pixel closest;
	closest.xpos = currentPix.xpos;
	closest.ypos = currentPix.ypos;
	closest.depth = 1;
	pixel testPix;  //Holds the pixel currently being looked at
	int step = 1;
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

	int cmin = max(0, currentPix.xpos - radius);
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
	/*PRINT(xcoor1);
	PRINT("  ");
	PRINT(ycoor1);
	PRINT("  ");
	PRINT(xcoor2);
	PRINT("  ");
	PRINT(ycoor2);
	PRINT("  \n");*/
	//rectangle(imgD, Point(xcoor1, ycoor1), Point(xcoor2, ycoor2), Scalar(255, 255, 0), 3); 
	return smallMat;
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

vector<Point> CDepthBasics::getHull(Mat img, pixel p, Mat *imgDraw)
{
	double mult = ((double)p.depth / 255.0)*0.5 + 0.5;
	Mat outImg = img.clone();
	cvtColor(outImg, outImg, COLOR_RGB2GRAY);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	/*PRINT(p.xpos);
	PRINT("\n");
	PRINT(p.ypos);
	PRINT("\n");
	PRINT(imgDraw->at<Vec3b>(Point(p.xpos, p.ypos))[0]);
	PRINT("\n");
	PRINT("\n");*/
	int thresh = 100;// p.depth + 50;// imgDraw->at<Vec3b>(Point(p.xpos, p.ypos))[0] + 50;
	threshold(outImg, outImg, thresh, 255, THRESH_BINARY);
	/// Find contours
	int xoffset = max(0, p.xpos - (int)(120 * mult));
	int yoffset = max(0, p.ypos - (int)(150 * mult));
	findContours(outImg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(xoffset, yoffset));
	
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

	vector<Point> hull(maxiSize);
	convexHull(Mat(contours[maxi]), hull, false);
	
	Scalar color = Scalar(255, 255, 0);// (5 * i*i, 50 * i*i, 0);
	drawContours(*imgDraw, contours, maxi, color, 2, 8, hierarchy, 0, Point());

	return hull;
}
