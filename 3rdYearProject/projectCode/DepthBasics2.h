#pragma once
#include "resource.h"
#include "ImageRenderer.h"
#include <fstream>
#include "header.h"
//#include <Kinect.h>

#include <opencv2/opencv.hpp>
#include <shark/Data/Dataset.h>
#include <shark/Algorithms/Trainers/RFTrainer.h> //the random forest trainer
#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h>



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
    //int                     Run(HINSTANCE hInstance, int nCmdShow);

	void SetUp(bool hands, int k, int s, int check);

	/// <summary>
	/// Main processing function
	/// </summary>
	void                    Update();

	void FindHands(bool refreshFrame);

	std::pair<std::vector<cv::Point>, cv::Point> ProcessHand(pixel pix, bool draw);

	void DrawClassification(int c);

	void Draw();

	pixel getHand1();

	pixel getHand2();

private:
	//The minium and maximum depths
	int						minDepth;
	int						maxDepth;

	//std::fstream output;

	//shark::RFClassifier model;

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

	bool train;

	std::pair<pixel, pixel> newPoints;
	std::pair<pixel, pixel> prevPoints;
	int					    refreshFrame;

	bool twoHands;
	int checkRadius;
	int sigma;
	int ksize;

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

	void PrepareImage();

	bool inRange(int radius, pixel pix1, pixel pix2);

	pixel initOneHand(cv::Mat *img);

	std::pair<pixel, pixel> initTwoHands(cv::Mat *img);

	pixel findClosestInRange(cv::Mat *img, pixel currentPix, int radius, int point);

	cv::Mat getHandArea(cv::Mat img, pixel point);

	void drawPixels(cv::Mat* img, pixel point, int size);

	void drawBoxes(cv::Mat* img, pixel point);

	std::vector<std::vector<cv::Point> > getContours(cv::Mat img, pixel p, cv::Mat *imgDraw);

	int getContourIndex(std::vector<std::vector<cv::Point> > contours);

	std::vector<cv::Point> getHull(std::vector<cv::Point> contours);

	void drawHull(cv::Mat *img, std::vector<cv::Point> hull);

	//std::string getMLTrainData(std::vector<cv::Point> hullPoints, pixel centralPoint);

	//shark::Data<shark::RealVector> getMLdata(std::vector<cv::Point> hullPoints, pixel centralPoint);

	//int classify(shark::Data<shark::RealVector> prediction, int sampleNum, double confidenceThreshold);

	//float getDist(cv::Point p, pixel p2);

	cv::Point getMaxPoint(cv::Mat *img, std::vector<cv::Point> hull, pixel centre);

	//void drawHandOutline(cv::Mat *imgDraw, cv:: Mat *imgEdit);
};


