//This is the file in which I will try to rebuild depthbasic.cpp but with only the stuff I need/want, and hopefully simplify it down.
//To go back to depthbasics.cpp (i.e. have that in the build), go to DepthBasics-D2D VC++ project XML file and change the file in the complilation thing at the bottom.

#include "stdafx.h" //A pre-compiled header. By precompiling it reduces a lot of overhead
#include <strsafe.h>
#include "alpha.h"
#include "resource.h"

#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include <cmath>

//#include <utility> //For the pair
#include <opencv2/opencv.hpp> //<> checks global directories first (i.e. using environment variables, "" checks local directory first

using namespace cv;
using namespace std;

#define PRINT( s ){wostringstream os_; os_ << s; OutputDebugStringW( os_.str().c_str() );} //Allows output to debug terminal below

/*struct pixel {
	int xpos;
	int ypos;
	int depth;
};*/

//Entry point for the application. Every windows application has an entry point names WinMain or wWinMain
//WinMain is used for a Windows based GUI application. main is used for a console application.
//WINAPI is the calling convention. A calling convention defines how a function receives parameters from the caller. For example, it defines the order that parameters appear on the stack.
//Parameters:
//hInstance is something called a "handle to an instance" or "handle to a module." The operating system uses this value to identify the executable (EXE) when it is loaded in memory. The instance handle is needed for certain Windows functions — for example, to load icons or bitmaps.
//hPrevInstance has no meaning.It was used in 16 - bit Windows, but is now always zero.
//pCmdLine contains the command line arguments as a Unicode string.
//nCmdShow is a flag that says whether the main application window will be minimized, maximized, or shown normally.
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	//Mat img;
	//img = imread("image.jpg");

	//namedWindow("Image", WINDOW_AUTOSIZE);
	//imshow("Image", img);
	//waitKey(0);

	HandDetector application;
	application.Run(hInstance, nCmdShow);
}

/// <summary>
/// Constructor
/// </summary>
HandDetector::HandDetector() 
{
	run = 0;
	vid = VideoWriter("handsOutput1.avi", CV_FOURCC('M', 'P', '4', '2'), 15.0, Size(640, 480), 1);
	doVid = false;
	imgD = Mat(Size(640, 480), CV_8UC3, Scalar(0,0,0));
	newPoints.first = pixel{ 0,0,0 }; //The first of the closest 2 pixels
	newPoints.second = pixel{ 0,0,0 }; //The second of the closest 2 pixels
	prevPoints.first = pixel{ 0,0,0 }; //The previous first closest pixel
	prevPoints.second = pixel{ 0,0,0 }; //The previous second closest pixel
	refreshFrame = 0;
	//m_pD2DFactory = NULL; //Equivalent to m_pD2DFactory = NULL
	//m_pDrawDepth = NULL;
	m_hNextDepthFrameEvent = INVALID_HANDLE_VALUE;
	m_pDepthStreamHandle = INVALID_HANDLE_VALUE;
	m_bNearMode = false;
	m_pNuiSensor = NULL;
	// create heap storage for depth pixel data in RGBX format
	m_depthRGBX = new BYTE[width*height*cBytesPerPixel];
}

/// <summary>
/// Destructor
/// </summary>
HandDetector::~HandDetector()
{
	if (m_pNuiSensor)
	{
		m_pNuiSensor->NuiShutdown(); //Turns the Kinect off. -> is same as . for pointer
	}

	if (m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hNextDepthFrameEvent); //Closes a handle. A handle is a reference to something. A pointer is a type of handle, but not necessarily the other way round
	}

	// clean up Direct2D renderer
	//delete m_pDrawDepth; //Deallocates a block of memory. Opposite to new.
	//m_pDrawDepth = NULL;

	// done with depth pixel data
	delete[] m_depthRGBX; //delete[] calls the deconstructor for each element in the array, then calls the  array deallocation function

						  // clean up Direct2D
	//SafeRelease(m_pD2DFactory); //Releases the pointer m_pD2D... and sets it to NULL

	SafeRelease(m_pNuiSensor);
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int HandDetector::Run(HINSTANCE hInstance, int nCmdShow) //HINSTANCE is a handle to an instance. nCmdShow determines how the window is to be shown. Values range from 0 to 11
{
	namedWindow("window1", WINDOW_AUTOSIZE);
	CreateFirstConnected();
	const int eventCount = 1;
	HANDLE hEvents[eventCount];
	if (NULL == m_pNuiSensor)
	{
		return 0;
	}

	
	while (run == 0)
	{
		hEvents[0] = m_hNextDepthFrameEvent;

		// Check to see if we have either a message (by passing in QS_ALLINPUT)
		// Or a Kinect event (hEvents)
		// Update() will check for Kinect events individually, in case more than one are signalled
		MsgWaitForMultipleObjects(eventCount, hEvents, FALSE, INFINITE, QS_ALLINPUT);
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextDepthFrameEvent, 0)) //WaitForSingleObject waits until the specified object is in the signaled state or the time-out interval elapses. m_hNext... is a handle to the object, 0 is the time-out interval.
		{
			ProcessDepth();
			//int a = img.at<uchar>(100,100); //This is how you access an element of a matrix. The type uchar is because the elements are CV_8UC3
			//PRINT(a);
			//run = false;
		}
		
	}
	return 0;
}

/// <summary>
/// Create the first connected Kinect found 
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT HandDetector::CreateFirstConnected()
{
	INuiSensor * pNuiSensor;
	HRESULT hr; //HRESULT represents error codes and warnings.

	int iSensorCount = 0;
	hr = NuiGetSensorCount(&iSensorCount); //Get the number of sensors ready for use. Returns an HRESULT, S_OK if successful, otherwise a failure code. 
	if (FAILED(hr))
	{
		return hr;
	}

	// Look at each Kinect sensor
	for (int i = 0; i < iSensorCount; ++i)
	{
		// Create the sensor so we can check status, if we can't create it, move on to the next
		hr = NuiCreateSensorByIndex(i, &pNuiSensor); //Creates an instance of the Kinect sensor with the specified index so that an application can open and use it. The snesor instance is bound to pNuiSensor
		if (FAILED(hr))
		{
			continue;
		}

		// Get the status of the sensor, and if connected, then we can initialize it
		hr = pNuiSensor->NuiStatus();
		if (S_OK == hr)
		{
			m_pNuiSensor = pNuiSensor; //The first sensor to be ready is the one we will use.
			break;
		}

		// This sensor wasn't OK, so release it since we're not using it
		pNuiSensor->Release();
	}

	if (NULL != m_pNuiSensor) //If there is at least one sensor that can be used
	{
		// Initialize the Kinect and specify that we'll be using depth
		hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH);
		if (SUCCEEDED(hr))
		{
			// Create an event that will be signaled when depth data is available by returning an event handle
			m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			// Open a depth image stream to receive depth frames
			hr = m_pNuiSensor->NuiImageStreamOpen(
				NUI_IMAGE_TYPE_DEPTH, //Type of stream
				NUI_IMAGE_RESOLUTION_640x480, //Image stream resolution
				0, //Frame event option. 0 is NUI_IMAGE_DEPTH_NO_VALUE. 
				2, //Number of frames Kinect should buffer
				m_hNextDepthFrameEvent, //A handle to a manual reset event that will be fired when the next frame in the stream is available
				&m_pDepthStreamHandle); //A pointer that contains a handle to the opened stream. This parameter cannot be NULL.
		}
	}

	if (NULL == m_pNuiSensor || FAILED(hr))
	{
		PRINT("\nNo ready Kinect found!\n\n");
		return E_FAIL;
	}

	return hr;
}

/// <summary>
/// Handle new depth data
/// </summary>
void HandDetector::ProcessDepth()
{
	HRESULT hr;
	NUI_IMAGE_FRAME imageFrame; //Struct that contains information about a depth or colour image frame

	// Attempt to get the depth frame
	hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame); //Gets the next frame of data from the stream, and stores it in imageFrame
	if (FAILED(hr))
	{
		return;
	}

	BOOL nearMode = false;
	INuiFrameTexture* pTexture; //Represents an object containing image frame data that is similar to a Direct3D texture, but has only one level (does not support mipmapping).

	// Get the depth image pixel texture
	hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture( //Stores a pointer to ptexture that contains the data of a depth frame in NUI_DEPTH_IMAGE_PIXEL format (instead of packed USHORTs).
		m_pDepthStreamHandle, &imageFrame, &nearMode, &pTexture); //NUI_DEPTH_IMAGE_PIXEL contains a pixel of depth data. Attributes are playerIndex - index of the player at this pixel, and depth - depth of the pixel
	if (FAILED(hr))
	{
		goto ReleaseFrame;
	}

	NUI_LOCKED_RECT LockedRect; //Defines the surface for a locked rectangle. Rectangle must be locked so kinect doesn't read/rewrite it

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0); //(level of texture resource to lock, pointer to D3DLOCKED_RECT structure describing locked region, pointer to rectangle to lock, combination of 0 or more locking flags that describe the type of lock to perform).

	// Make sure we've received valid data
	if (LockedRect.Pitch != 0) //LockedRect.Pitch is the number of bytes of data in a row of the locked rect
	{
		// Get the min and max reliable depth for the current frame. NUI_IMAGE... are constants
		//Near mode is not implemented for Xbox 360 Kinect (which I am currently using), but I will keep these lines of code in incase a diiferent Kinect is used
		int minDepthALT = (m_bNearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT; // = 800
		int maxDepthALT = (m_bNearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT; // = 4000 

		int minDepth = 500; //The closest any object can be recognised
		int maxDepth = 1000; //The furthest away any object can be recognised


		const NUI_DEPTH_IMAGE_PIXEL * pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits); //The pixel counter for assigning colour. LockedRect.pBits gives the position of the upper left pixel of the window 
		const NUI_DEPTH_IMAGE_PIXEL * pBufferRun2 = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits); //The pixel counter for the div method

		// end pixel is start + width*height - 1
		const NUI_DEPTH_IMAGE_PIXEL * pBufferEnd = pBufferRun + (width * height);

		//Run the division method
		//maxDepth = depthDivision(pBufferRun2, pBufferEnd, minDepth, maxDepth);
		
		//minDepth = closestDepth(pBufferRun2, pBufferEnd, maxDepth);

		//prepDrawDivision(pBufferRun, pBufferEnd, minDepth, maxDepth);
		prepDrawDepth(pBufferRun, pBufferEnd, minDepth, maxDepth); //Convert depth into an intensity value from 255 (closest) to 1 (furthest away)

		//Run the intensity maxima method
		//closestPixel(maxDepth);
		//intensityMaxima();
		//drawMaxima(closestDepth);

		//Convert the data into an OpenCV matrix format so can be used for processing.
		imgD = Mat(Size(640, 480), CV_8UC3, m_depthRGBX); //CV_8UC3 means in the image, each pixel will have 3 channels, each with an 8-bit unsigned integer. See http://docs.opencv.org/2.4/modules/core/doc/basic_structures.html for more details 
		Mat imgG = Mat(Size(640, 480), CV_8UC3); //The matrix to be gaussian blurred
		GaussianBlur(imgD, imgG, Size(65, 65), 150); //65,65,150 are the current default blur parameters

		Mat* activeMatPointer = &imgG; //The pointer to the matrix that will be used to determine which pixels are the hand centres
		Mat outputMat = imgD; //The image to which the outputs will be drawn

		int checkSize = 60; //The size of the area around the current center of the hand which will be checked for the next best pixel 
		//Still not sure of best size. Small medium and large seem to all have separate benefits. 120 actually works quite well.

		double multiplier = 1;

		if (refreshFrame == 0)
		{
			newPoints = findClosest2(activeMatPointer); //Search the whole image for the closest 2 points as opposed to locally for each
		}
		else
		{
			multiplier = ((double)prevPoints.first.depth / 255.0)*0.5 + 0.5;
			newPoints.first = findClosestInRange(activeMatPointer, prevPoints.first, checkSize, 1); //Search locally for the closest pixel around the first current closest
			multiplier = ((double)prevPoints.second.depth / 255.0)*0.5 + 0.5;
			newPoints.second = findClosestInRange(activeMatPointer, prevPoints.second, checkSize, 2); //Search around the second
		}
		//PRINT(closestPoints.first);
		drawPixels(&outputMat, newPoints.first, 4); //Draw the chosen pixel for the first hand
		drawPixels(&outputMat, newPoints.second, 4); //Draw the chosen pixel for the second
		
		multiplier = ((double)newPoints.first.depth / 255.0)*0.5 + 0.5; //The multiplier used to make the box/checking areas around the centre pixel change size as the hand moves deeper

		rectangle(outputMat, Point(newPoints.first.xpos - (int)(120*multiplier), newPoints.first.ypos - (int)(150 * multiplier)), Point(newPoints.first.xpos + (int)(120 * multiplier), newPoints.first.ypos + (int)(70 * multiplier)), Scalar(0, 255, 0), 3); //Draw a box around the first hand. This area is the image that will be used to identify the hand
		rectangle(outputMat, Point(newPoints.first.xpos - (int)(checkSize * multiplier), newPoints.first.ypos - (int)(checkSize * multiplier)), Point(newPoints.first.xpos + (int)(checkSize * multiplier), newPoints.first.ypos + (int)(checkSize * multiplier)), Scalar(0, 0, 255), 3); //Box indicating the area that is searched for the next pixel

		multiplier = ((double)newPoints.second.depth / 255.0)*0.5 + 0.5;
		rectangle(outputMat, Point(newPoints.second.xpos - (int)(120 * multiplier), newPoints.second.ypos - (int)(150 * multiplier)), Point(newPoints.second.xpos + (int)(120 * multiplier), newPoints.second.ypos + (int)(70 * multiplier)), Scalar(0, 255, 0), 3); //Secons hand
		rectangle(outputMat, Point(newPoints.second.xpos - (int)(checkSize * multiplier), newPoints.second.ypos - (int)(checkSize * multiplier)), Point(newPoints.second.xpos + (int)(checkSize * multiplier), newPoints.second.ypos + (int)(checkSize * multiplier)), Scalar(0, 0, 255), 3);
		
		//Print text to image. 
		//(image to print to, coordinate to print to, font, size, colour, text line thickness)
		//putText(outputMat, to_string(newPoints.first.xpos), Point2i(10, 50), FONT_HERSHEY_COMPLEX, 2, Scalar(0, 0, 255, 255), 5);
		//putText(outputMat, to_string(newPoints.second.xpos), Point2i(10, 100), FONT_HERSHEY_COMPLEX, 2, Scalar(0, 0, 255, 255), 5);

		prevPoints = newPoints; //Make the new points become the previous points, ready for the net frame
		refreshFrame = ((refreshFrame + 1) % 100)+1; //Remove the + 1 to enable a global search for the 2 points every 100 frames.
		
		//Change parameter to draw on Gauss (img2) or non-Gauss (img) version. 
		//drawDivisionAVG(&img2, &img);
		//(image to base off, image to draw on)
		//drawMaxima(&img2, &img);
		
		imshow("window1", outputMat); //Display the image
		if (doVid)
		{
			vid.write(outputMat);
		}
		int key = waitKey(5);//Number of ms image is displayed for. 1s = 1000ms. Increasing this decreases FPS. Perhaps do this if need to make it run better. 16.666 = 60fps, 25 = 40fps
		if (key != -1)
		{
			run = 1;
		}
	}

	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);

	pTexture->Release();


ReleaseFrame:
	// Release the frame
	m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);
}

/// <summary>
/// The hand isolation method that uses depth divisions
/// </summary>
int HandDetector::depthDivision(const NUI_DEPTH_IMAGE_PIXEL *pBufferRun, const NUI_DEPTH_IMAGE_PIXEL *pBufferEnd, int minDepth, int maxDepth)
{
	int depthDivision = 100; //The distance between each "division"
	USHORT closestDepth = (USHORT)maxDepth; //The distance of the closest (or one of the closest) pixel.

	/*
	Division method for isolating hand regions - Only works well if there is a maximum depth limit
	Works by splitting the area between camera and subject into different divisions, with each division being a range of distances away from the camera.
	Each pixel is put into a division based on its distance.
	The division with the most number of pixels is decided to be the "background", or in most cases the body. This works because the body is almost always bigger than the hands.
	To account for when the hands are much closer to the camera than the body, the number of pixels in a division is discounted if the closest pixel is in the same, or next division.
	Once the most populus division is found, anything beyond that division, and the one closer to it (to accoount for any extra bits that may be in the one closer) are turned black.
	*/
	map<int, int> divisions; //Map that stores divisions and the count for each one
	for (int i = minDepth; i < maxDepth; i = i + depthDivision)
	{
		divisions[i] = 0; //Set all divisions to having a count of 0
	}

	int division; //Saves making a new variable each time. AFAIK this is more efficient.
	while (pBufferRun < pBufferEnd) //Go through each pixel
	{
		USHORT depth = pBufferRun->depth; //Get the depth value
		if (depth < closestDepth && depth > 0) //Identifying the closet pixel. If the depth value is closer than the currrent closest, then make it the closest. Also has to be above 0.
		{
			closestDepth = depth;
		}
		if (depth < maxDepth && depth >= minDepth) //If a pixel is within the seeable range
		{
			division = ((int)(depth / depthDivision)) * depthDivision; //Turn the pixel value to match the value of the first pixel depth in a division. A division is assigned by rounding down to the nearest 100.
			divisions[division]++; //Increase the count of that division by 1.
		}
		pBufferRun = pBufferRun + 20; //Go through every 20 pixels instead of every one. Increases speed.
	}

	int highest = maxDepth - depthDivision; //Start with the last division being the highest
	int closestDiv = ((int)(closestDepth / depthDivision)) * depthDivision; //Calculate which division the closest pixel is in

	for (int i = minDepth; i < maxDepth; i = i + depthDivision) //For each division
	{
		//Issue when hands are in division before most populated division since it cuts off the division before most populated so cuts out hands
		if (divisions[i] > divisions[highest] && i != closestDiv && i != closestDiv + depthDivision) //If the number of pixels in this division is more than the current best, and the closest pixel is not in this division, or the next one further away. 
		{
			highest = i; //The new highest division value becomes this one
		}
	}

	int max = (int)highest - (depthDivision / 2); //The max distance is the one one division closer than the most populus division
												   //Divising depthDivision by 2 above might solve the issue with cutting out hands

	return max;
}

/// <summary>
/// Get m_depthRGBX ready for being drawn by OpenCV
/// </summary>
void HandDetector::prepDrawDivision(const NUI_DEPTH_IMAGE_PIXEL *pBufferRun, const NUI_DEPTH_IMAGE_PIXEL *pBufferEnd, int minDepth, int maxDepth)
{
	BYTE * rgbrun = m_depthRGBX; //m_depthBYTE is an array, so rgbrun is a pointer to that array

	while (pBufferRun < pBufferEnd)
	{
		// discard the portion of the depth that contains only the player index
		USHORT depth = pBufferRun->depth;

		if (depth > maxDepth || depth < minDepth)
		{
			depth = 0; //Make black
		}
		else
		{
			depth = 255; //Make white
		}

		BYTE intensity = static_cast<BYTE>(depth);

		// Write out blue byte
		*(rgbrun++) = intensity; //Since rgbrun is a pointer to memory, increasing it by 1 moves to the next block of memory

		// Write out green byte
		*(rgbrun++) = intensity;

		// Write out red byte
		*(rgbrun++) = intensity;

		// Increment our index into the Kinect's depth buffer
		++pBufferRun;
	}
}

void HandDetector::drawDivisionAVG(Mat *img, Mat *imgToDrawOn)
{
	int closest = 1;
	int closest2 = 1;
	
	double pixel[2] = { 0,0 };
	double pixel2[2] = { 0,0 };
	
	int numPixels = 0;
	int numPixels2 = 0;

	double sumPixel[2] = { 0,0 };
	double sumPixel2[2] = { 0,0 };

	int step = 3;
	int rad = 20; //radius for wether a pixel is part of a group or is a separate group
	int threshold = 10; //The intensity lower than maximum a pixel mut be to be considered part of the average

	for (int i = 0; i < img->rows; i = i + step)
	{
		for (int j = 0; j < img->cols; j = j + step)
		{
			double currentPix[2] = { i,j };
			int depth = img->at<Vec3b>(i, j)[0];

			if (depth > closest) //Make new pixel 1
			{
				if (!withinRange(rad, currentPix, pixel)) //Replace pixel 2 with old pixel 1 if not in range
				{
					sumPixel2[0] = sumPixel[0];
					sumPixel2[1] = sumPixel[1];
					numPixels2 = numPixels;
					pixel2[0] = pixel[0];
					pixel2[1] = pixel[1];
					closest2 = closest;
				}
				sumPixel[0] = i;
				sumPixel[1] = j;
				numPixels = 1;
				pixel[0] = sumPixel[0];
				pixel[1] = sumPixel[1];
				closest = depth;
			}
			else if (depth >= closest - threshold && depth < closest && withinRange(rad, pixel, currentPix)) //Update pixel 1
			{
				sumPixel[0] = sumPixel[0] + i;
				sumPixel[1] = sumPixel[1] + j;
				numPixels++;
				pixel[0] = sumPixel[0] / numPixels;
				pixel[1] = sumPixel[1] / numPixels;
			}
			else if(!withinRange(rad, pixel, currentPix) && depth > closest2) //New pixel 2
			{
				sumPixel2[0] = i;
				sumPixel2[1] = j;
				numPixels2 = 1;
				pixel2[0] = sumPixel2[0];
				pixel2[1] = sumPixel2[1];
				closest2 = depth;
			}
			else if (depth >= closest2 - threshold && depth < closest2 && withinRange(rad, pixel2, currentPix)) //Update pixel 2
			{
				sumPixel2[0] = sumPixel2[0] + i;
				sumPixel2[1] = sumPixel2[1] + j;
				numPixels2++;
				pixel2[0] = sumPixel2[0] / numPixels2;
				pixel2[1] = sumPixel2[1] / numPixels2;
			}
		}
	}
	
	for (int k = -4; k < 5; k++)
	{
		for (int l = -4; l < 5; l++)
		{
			int a = (int) pixel[0] + k;
			int b = (int) pixel[1] + l;
			
			int c = (int) pixel2[0] + k;
			int d = (int) pixel2[1] + l;
			if (a >= 0 && b >= 0 && a < 480 && b < 640 && c >= 0 && d >= 0 && c < 480 && d < 640)
			{
				imgToDrawOn->at<Vec3b>(a, b)[0] = 0;
				imgToDrawOn->at<Vec3b>(a, b)[1] = 255; //green
				imgToDrawOn->at<Vec3b>(a, b)[2] = 0;

				imgToDrawOn->at<Vec3b>(c, d)[0] = 0;
				imgToDrawOn->at<Vec3b>(c, d)[1] = 0;
				imgToDrawOn->at<Vec3b>(c, d)[2] = 255; //red
			}
		}
	}
}

void HandDetector::drawDivisionFIRST(Mat *img, Mat *imgToDrawOn)
{
	int step = 3;
	int closest = 1;
	int pixel[2] = { 0,0 };

	for (int i = 0; i < img->rows; i = i + step)
	{
		for (int j = 0; j < img->cols; j = j + step)
		{
			if (img->at<Vec3b>(i, j)[0] > closest)
			{
				closest = img->at<Vec3b>(i, j)[0];
				pixel[0] = i;
				pixel[1] = j;
			}
		}
	}

	for (int k = -4; k < 5; k++)
	{
		for (int l = -4; l < 5; l++)
		{
			int a = pixel[0] + k;
			int b = pixel[1] + l;
			if (a >= 0 && b >= 0 && a < 480 && b < 640)
			{
				imgToDrawOn->at<Vec3b>(a, b)[0] = 0;
				imgToDrawOn->at<Vec3b>(a, b)[1] = 255;
				imgToDrawOn->at<Vec3b>(a, b)[2] = 0;
			}
		}
	}
}

void HandDetector::prepDrawDepth(const NUI_DEPTH_IMAGE_PIXEL *pBufferRun, const NUI_DEPTH_IMAGE_PIXEL *pBufferEnd, int minDepth, int maxDepth)
{
	BYTE * rgbrun = m_depthRGBX; //m_depthBYTE is an array, so rgbrun is a pointer to that array

	while (pBufferRun < pBufferEnd)
	{
		// discard the portion of the depth that contains only the player index
		USHORT depth = pBufferRun->depth;

		if (depth > maxDepth || depth < minDepth)
		{
			depth = 1; //Make black, but not so much that causes division errors later.
		}
		else
		{
			float depth2;
			depth2 = (255.0 - ((((float)depth - minDepth) / (float)(maxDepth - minDepth)) * 254.0));
			depth = (int)depth2; //Still not sure if better to have in binary colour or gradual change. Possibly use gradual change, then identify maxial points, with 2 highest intensity points being the hands. Could possibly also do this to eliminate any need for a maximum distance, as long as nothing else was between the user and the kinect.
		}

		///
		if (depth == 0)
		{
			depth = 1;
		}
		///

		BYTE intensity = static_cast<BYTE>(depth);

		// Write out blue byte
		*(rgbrun++) = intensity; //Since rgbrun is a pointer to memory, increasing it by 1 moves to the next block of memory

		// Write out green byte
		*(rgbrun++) = intensity;

		// Write out red byte
		*(rgbrun++) = intensity;

		// Increment our index into the Kinect's depth buffer
		++pBufferRun;
	}
}

void HandDetector::drawMaxima(Mat *img, Mat *imgToDrawOn)
{
	int stepBig = 20;
	int stepSmall = 3;
	int istep = 20;
	int jstep = 20;
	bool rowHasMax;

	for (int i = 0; i < img->rows; i = i + istep) //go through each row
	{
		rowHasMax = false; //Initialise the current row to have no local maximum
		for (int j = 0; j < img->cols; j = j + jstep) //go through each column of the current row
		{
			if (img->at<Vec3b>(i, j)[0] > 0) //if the current pixel is not black
			{
				if (isMaxima(img, i, j, 51)) //If the current pixel is a local maximum within a 41 pixel radius i.e. all the pixels within a square radius 41 have a lower depth value than the current pixel
				{
					for (int k = -2; k < 3; k++) //coulour that pixel
					{
						for (int l = -2; l < 3; l++)
						{
							int a = i + k;
							int b = j + l;
							if (a >= 0 && b >= 0 && a < 480 && b < 640)
							{
								//imgToDrawOn->at<Vec3b>(a, b)[0] = 0;
								imgToDrawOn->at<Vec3b>(a, b)[1] = 0;
								//imgToDrawOn->at<Vec3b>(a, b)[2] = 255;
							}
						}
					}
					
				}
				rowHasMax = true; //if the current row has a maximum, likey the next row will, so take a small step to the next row
				jstep = stepSmall;
			}
			else
			{
				jstep = stepBig;
			}
		}
		if (rowHasMax == true)
		{
			istep = stepSmall;
		}
	}
}

bool HandDetector::isMaxima(Mat* img, int row, int col, int rad)
{
	bool isMax = true;
	int r = (int)((rad - 1) / 2);
	int pixel = img->at<Vec3b>(row, col)[0];
	int inc = 2; //The number to increment by each time for checking around a pixel
	//x x x
	//o . o
	//o o o
	for (int i = -r; i <= r; i = i + inc)
	{
		if ((row - r) >= 0 && (col + i) < width && (col + i) >= 0)
		{
			if (img->at<Vec3b>(row - r, col + i)[0] > pixel)
			{
				isMax = false;
				goto finished;
			}
		}
	}
	//o o o
	//x . o
	//o o o
	for (int i = -r + 1; i <= r - 1; i = i + inc)
	{
		if ((row + i) < height && (row + i) >= 0 && (col - r) >= 0)
		{
			if (img->at<Vec3b>(row + i, col - r)[0] > pixel)
			{
				isMax = false;
				goto finished;
			}
		}
	}
	//o o o
	//o . x
	//o o o
	for (int i = -r + 1; i <= r - 1; i = i + inc)
	{
		if ((row + i) < height && (row + i) >= 0 && (col + r) < width)
		{
			if (img->at<Vec3b>(row + i, col + r)[0] > pixel)
			{
				isMax = false;
				goto finished;
			}
		}
	}
	//o o o
	//o . o
	//x x x
	for (int i = -r; i <= r; i = i + inc)
	{
		if ((row + r) < height && (col + i) < width && (col + i) >=0)
		{
			if (img->at<Vec3b>(row + r, col + i)[0] > pixel)
			{
				isMax = false;
				goto finished;
			}
		}
	}

finished:
	return isMax;
}

int HandDetector::closestDepth(const NUI_DEPTH_IMAGE_PIXEL *pBufferRun, const NUI_DEPTH_IMAGE_PIXEL *pBufferEnd, int maxDepth)
{
	int closestDepth = maxDepth;
	while (pBufferRun < pBufferEnd) //Go through each pixel
	{
		USHORT depth = pBufferRun->depth; //Get the depth value
		if (depth < closestDepth && depth > 0) //Identifying the closet pixel. If the depth value is closer than the currrent closest, then make it the closest. Also has to be above 0.
		{
			closestDepth = depth;
		}
		pBufferRun = pBufferRun + 20; //Go through every 20 pixels instead of every one. Increases speed.
	}
	return closestDepth;
}

bool HandDetector::withinRange(int radius, double point1[2], double point2[2])
{
	bool isIn = false;
	double a = (point1[0] - point2[0]);
	double b = (point1[1] - point2[1]);
	if (sqrt(a*a + b*b) < radius)
	{
		isIn = true;
	}

	return isIn;
}

bool HandDetector::inRange(int radius, pixel pix1, pixel pix2)
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

pair<pixel, pixel> HandDetector::findClosest2(Mat *img)
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
			if (testPix.depth > closest2.depth && !inRange(75, closest, testPix))
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

pixel HandDetector::findClosestInRange(Mat *img, pixel currentPix, int radius, int point)
{
	pixel closest;
	closest.xpos = currentPix.xpos;
	closest.ypos = currentPix.ypos;
	closest.depth = 1;
	pixel testPix;
	int step = 3;
	double multiplier;

	if(point == 1) //The search area differs based on whch point this is. 
	{
		multiplier = ((double)prevPoints.first.depth / 255.0)*0.5 + 0.5;
	}
	else if (point == 2)
	{
		multiplier = ((double)prevPoints.second.depth / 255.0)*0.5 + 0.5;
	}

	radius = radius*multiplier; //Scale the radius
	int cmin = 0; //The minimum possible value to start searching from
	if (currentPix.xpos - radius > 0) //If the range would not take it beyond this minimum,
	{
		cmin = currentPix.xpos - radius; //Make this where the searching starts
	}
	int cmax = img->cols - 1; //Same with other side of the image
	if(currentPix.xpos + radius < img->cols - 1)
	{ 
		cmax = currentPix.xpos + radius;
	}

	int rmin = 0; //Same with rows
	int offset = 0;
	radius = radius - offset*multiplier;
	if (currentPix.ypos - radius > 0) //-20 means search 20 pixels above current centre pixel so that does not find arm when hand bent forward
	{
		rmin = currentPix.ypos - radius;
	}
	int rmax = img->rows - 1;
	if (currentPix.ypos + radius < img->rows - 1)
	{
		rmax = currentPix.ypos + radius;
	}

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

void HandDetector::drawPixels(Mat *img, pixel point, int size)
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