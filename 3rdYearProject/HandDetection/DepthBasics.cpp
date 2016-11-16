//------------------------------------------------------------------------------
// <copyright file="DepthBasics.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

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
	imgG(Mat(Size(cDepthWidth, cDepthHeight), CV_8UC4, Scalar(0, 0, 0))),
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
        USHORT nDepthMaxDistance = 0;
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
	imgG = Mat(Size(cDepthWidth, cDepthHeight), CV_8UC1); //The matrix to be gaussian blurred
	GaussianBlur(imgD, imgG, Size(45, 45), 100); //65,65,150 are the current default blur parameters
	//GaussianBlur(imgG, imgG, Size(1, 45), 100); //65,65,150 are the current default blur parameters

	imshow("window1", imgD);
	int key = waitKey(25);//Number of ms image is displayed for. 1s = 1000ms. Increasing this decreases FPS. Perhaps do this if need to make it run better. 16.666 = 60fps, 25 = 40fps
	if (key != -1)
	{
		run = 1; //Terminate the program
	}
}
