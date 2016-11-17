//#pragma once //Means header will only be used once

#include "resource.h"
#include "NuiApi.h"
#include "ImageRenderer.h"
#include <opencv2/opencv.hpp>

//dangerous to use "using namespace ..." in header files

struct pixel {
	int xpos;
	int ypos;
	int depth;
};

class HandDetector
{
	//The width and height of the depth image to be taken, and also the output image. Static means value is same accross all code, const means value cannot be changed, and there will be an error if some code tries to change it.
	static const int width = 640;
	static const int height = 480;
	static const int cBytesPerPixel = 3; //Must be 3 since 3 channels - RGB. Is 4 on original DepthBasics-2D.

private:
	cv::Mat						imgD;
	bool					doVid;
	int						run;

	std::pair<pixel, pixel> newPoints;
	std::pair<pixel, pixel> prevPoints;
	int					    refreshFrame;

	cv::VideoWriter			vid;

	bool                    m_bNearMode; //If near mode is active
	
	INuiSensor*             m_pNuiSensor; // Current Kinect

	// Direct2D - a hardware-accelerated, immediate-mode, 2-D graphics API that provides high performance and high-quality rendering for 2-D geometry, bitmaps, and text.
	//ImageRenderer*          m_pDrawDepth; //ImageRenderer deals with the drawing of image data. May not be needed as its only two functions are to set the window to draw in, and then to draw it, both of which can be done in OpenCV.
	//ID2D1Factory*           m_pD2DFactory; //The starting point for using Direct2D - an ID2D1Factory object is used to create Direct2D resources. This object is created by calling D2D1CreateFactory(singleOrDoubleThreaded, m_pD2DFactory)

	HANDLE                  m_pDepthStreamHandle; //I think this is the stream of depth data. A handle is an abstract reference to a resource
	HANDLE                  m_hNextDepthFrameEvent; //Think this is the handle to the next snapshot of depth data

	BYTE*                   m_depthRGBX; //Storage for depth pixel data in RGBX format

	

public:
	/// <summary>
	/// Constructor
	/// </summary>
	HandDetector();

	/// <summary>
	/// Destructor
	/// </summary>
	~HandDetector();

	/// <summary>
	/// Creates the main window and begins processing
	/// </summary>
	/// <param name="hInstance"></param>
	/// <param name="nCmdShow"></param>
	int Run(HINSTANCE hInstance, int nCmdShow);

	/// <summary>
	/// Create the first connected Kinect found 
	/// </summary>
	/// <returns>S_OK on success, otherwise failure code</returns>
	HRESULT CreateFirstConnected();

	/// <summary>
	/// Main processing function
	/// </summary>
	//void Update();

	/// <summary>
	/// Handle new depth data
	/// </summary>
	void ProcessDepth();

	/// <summary>
	/// The hand isolation method that uses depth divisions
	/// </summary>
	int depthDivision(const NUI_DEPTH_IMAGE_PIXEL *pBufferRun, const NUI_DEPTH_IMAGE_PIXEL *pBufferEnd, int minDepth, int maxDepth);

	/// <summary>
	/// Draws the depth image for the depth division method
	/// </summary>
	void prepDrawDivision(const NUI_DEPTH_IMAGE_PIXEL *pBufferRun, const NUI_DEPTH_IMAGE_PIXEL *pBufferEnd, int minDepth, int maxDepth);

	/// <summary>
	/// Draw the image for the division method
	/// </summary>
	/// <param name="img"></param> The image that chosen part of the hand is identified from
	/// <param name="imgToDrawOn"></param> //The image the chosen part of the hind will be highlighted on
	void drawDivisionAVG(cv::Mat *img, cv::Mat *imgToDrawOn);

	void drawDivisionFIRST(cv::Mat *img, cv::Mat *imgToDrawOn);

	void prepDrawDepth(const NUI_DEPTH_IMAGE_PIXEL *pBufferRun, const NUI_DEPTH_IMAGE_PIXEL *pBufferEnd, int minDepth, int maxDepth);

	void drawMaxima(cv::Mat *img, cv::Mat *imgToDrawOn);

	//Whether a given pixel is a local maxima (every pixel around it in a square of radius rad has a lower of equal depth)
	bool isMaxima(cv::Mat *img, int row, int col, int rad);

	int closestDepth(const NUI_DEPTH_IMAGE_PIXEL *pBufferRun, const NUI_DEPTH_IMAGE_PIXEL *pBufferEnd, int maxDepth);

	bool withinRange(int radius, double point1[2], double point2[2]);

	bool inRange(int radius, pixel pix1, pixel pix2);

	std::pair<pixel, pixel> findClosest2(cv::Mat *img);

	pixel findClosestInRange(cv::Mat *img, pixel currentPix, int radius, int point);

	void drawPixels(cv::Mat* img, pixel point, int size);
};