//------------------------------------------------------------------------------
// <copyright file="DepthBasics.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "resource.h"
#include "ImageRenderer.h"
#include <fstream>
#include <opencv2/opencv.hpp>

struct pixel {
	int xpos;
	int ypos;
	int depth;
};

class CDepthBasics
{
	static const int        cDepthWidth = 512;
	static const int        cDepthHeight = 424;

public:
    /// <summary>
    /// Constructor
    /// </summary>
    CDepthBasics();

    /// <summary>
    /// Destructor
    /// </summary>
    ~CDepthBasics();

    /// <summary>
    /// Creates the main window and begins processing
    /// </summary>
    /// <param name="hInstance"></param>
    /// <param name="nCmdShow"></param>
    int                     Run(HINSTANCE hInstance, int nCmdShow);

private:
	//The minium and maximum depths
	int						minDepth;
	int						maxDepth;

	std::fstream output;

    // Current Kinect
    IKinectSensor*          m_pKinectSensor;

    // Depth reader
    IDepthFrameReader*      m_pDepthFrameReader;

    RGBQUAD*                m_pDepthRGBX;
	BYTE*					depthArr;

	int						run;
	cv::Mat					imgD;
	cv::Mat					imgG;

	bool					doVid;
	cv::VideoWriter			vid;

	std::pair<pixel, pixel> newPoints;
	std::pair<pixel, pixel> prevPoints;
	int					    refreshFrame;

    /// <summary>
    /// Main processing function
    /// </summary>
    void                    Update();

    /// <summary>
    /// Initializes the default Kinect sensor
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                 InitializeDefaultSensor();

    /// <summary>
    /// Handle new depth data
    /// <param name="nTime">timestamp of frame</param>
    /// <param name="pBuffer">pointer to frame data</param>
    /// <param name="nWidth">width (in pixels) of input image data</param>
    /// <param name="nHeight">height (in pixels) of input image data</param>
    /// <param name="nMinDepth">minimum reliable depth</param>
    /// <param name="nMaxDepth">maximum reliable depth</param>
    /// </summary>
    void                    ProcessDepth(INT64 nTime, const UINT16* pBuffer, int nHeight, int nWidth, USHORT nMinDepth, USHORT nMaxDepth);

	void Draw();

	bool inRange(int radius, pixel pix1, pixel pix2);

	std::pair<pixel, pixel> findClosest2(cv::Mat *img);

	pixel findClosestInRange(cv::Mat *img, pixel currentPix, int radius, int point);

	cv::Mat getHandArea(cv::Mat img, pixel point);

	void drawPixels(cv::Mat* img, pixel point, int size);

	void drawBoxes(cv::Mat* img, pixel point);

	std::vector<cv::Point> getHull(cv::Mat img, pixel point, cv::Mat *imgD);

	void drawHull(cv::Mat *img, std::vector<cv::Point> hull);

	std::string getMLdata(std::vector<cv::Point> hullPoints, pixel centralPoint);

	//void drawHandOutline(cv::Mat *imgDraw, cv:: Mat *imgEdit);
};


