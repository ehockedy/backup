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
	/// Performs the initialisation for the hand detection system. Requred to be done before any detecting takes place
	/// </summary>
	/// <param name="hands">true if detecting two hands</param>
	/// <param name="k">the kernel sixe for Gaussian blur</param>
	/// <param name="s">the sigma value for Gaussian blur</param>
	/// <param name="check">The radius of the square area to check around the previous hand centre pixel for the centre of the new hand</param>
	/// <param name="draw">Whether to output details to the image</param>
	/// <param name="step">Skips every "step" pixels in order to reduce search time</param>
	void SetUp(bool hands, int k, int s, int check, bool draw, int step, int depth);

	/// <summary>
	/// Main processing function
	/// </summary>
	void                    Update();

	/// <param name=""></param>
	/// <param name=""></param>
	/// <param name=""></param>
	void FindHands(bool refreshFrame, int step, bool twohands);

	void UpdatePreviousHands(pixel h1, pixel h2);

	/// <summary>
	/// Get the useful information describing the hands
	/// <param name="pix">The pixel that is decided to be the centre of the hand</param>
	/// <param name="draw">Whether to output to the image</param>
	/// <returns>The 25 points that describe the hand shape, and the point that is the furthest from the hand centre i.e. the tip of the longest finger</returns>
	/// </summary>
	std::pair<std::vector<cv::Point>, cv::Point> ProcessHand(pixel pix, bool draw, int handNum);

	/// <summary>
	/// Output the identified pose
	/// <param name="c">The classification to output</param>
	/// </summary>
	void DrawLeftClassification(int c);

	void DrawRightClassification(int c);

	/// <summary>
	/// <param name="divisor">The dimensionf of the image are divided by this value</param>
	/// </summary>
	cv::Mat Draw(int divisor);

	cv::Mat DrawTrain(int divisor);

	/// <summary>
	/// Returns the first had info
	/// </summary>
	pixel getHand1();

	/// <summary>
	/// Returns the second hand info
	/// </summary>
	pixel getHand2();

	int getWidth() { return cDepthWidth; }
	int getHeight() { return cDepthHeight; }
	int getMinDepth() { return minDepth; }
	int getMaxDepth() { return maxDepth; }

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

	///
	IColorFrameReader* m_pColourFrameReader;
	///

    RGBQUAD*                m_pDepthRGBX; //Hold Colour info
	BYTE*					depthArr;	//Holds the depth info straight from the Kinect

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

	/// <summary>
	/// Do some preprocessing to the depth image received from the kinect
	/// </summary>
	void PrepareImage();

	/// <summary>
	/// Checks if pix1 is in range of pix2 i.e. whether pix 2 lies withn an area around pix1
	/// <param name="radius">The area to check around to see if pix1 is in range of pix2</param>
	/// <param name="pix1">The first pixel, checks to see if it in range of pix2</param>
	/// <param name="pix2">Second pixel</param>
	/// </summary>
	bool inRange(int radius, pixel pix1, pixel pix2);

	/// <summary>
	/// Find the position of a single hand
	/// <param name="img">pointer to the image</param>
	/// <param name="s">Jump between pixels when looking through them all</param>
	/// </summary>
	pixel initOneHand(cv::Mat *img, int s);

	/// <summary>
	/// Find the position of two hands
	/// <param name=""></param>
	/// <param name=""></param>
	/// </summary>
	std::pair<pixel, pixel> initTwoHands(cv::Mat *img, int s);

	/// <summary>
	/// Find the closest pixel (depth wise) within a certain range
	/// <param name="img">The image to search in</param>
	/// <param name="currentPix">The pixel to search around</param>
	/// <param name="radius">The radius of search</param>
	/// <param name="point">The hand that is being searched around. Options are 1 (first hand) or 2 (second hand)</param>
	/// <param name="s">Search jump amount</param>
	/// </summary>
	pixel findClosestInRange(cv::Mat *img, pixel currentPix, int radius, int point, int s);

	/// <summary>
	/// Extracts from the image the area around the hand and returns it
	/// <param name="img">The image being searched in</param>
	/// <param name="point">The point around whihc the part of th eimage will be extracted from</param>
	/// </summary>
	cv::Mat getHandArea(cv::Mat img, pixel point);

	/// <summary>
	/// Calculates the proportion of pixels that are within a certain range of depth. The intuition behind this is that the 3 poses should take up a porportion of the pixels, so by classifing based on this, it helps to identify pose more accurately
	/// <param name="img">The image being searched in</param>
	/// <param name="minDepth">The depth of the centre pixel which is the maximum in the range </param>
	/// </summary>
	float getHandProportion(cv::Mat img, float minDepth);

	/// <summary>
	/// Draws some pixels
	/// <param name="img">The image to draw on</param>
	/// <param name="point">The point to draw on</param>
	/// <param name="size">The size of the point to draw</param>
	/// </summary>
	void drawPixels(cv::Mat* img, pixel point, int size);

	/// <summary>
	/// Draws some boxes of search range and part of the image used for segmenting hand
	/// <param name="img">The image to draw on </param>
	/// <param name="point">The point to search around</param>
	/// </summary>
	void drawBoxes(cv::Mat* img, pixel point);

	/// <summary>
	/// Gets the contours of the hand. Used in finding the hull points that describe the hand
	/// <param name="img">The image to search in</param>
	/// <param name="p">The pixel to search around</param>
	/// <param name="imgDraw">The image to draw to</param>
	/// </summary>
	std::vector<std::vector<cv::Point> > getContours(cv::Mat img, pixel p, cv::Mat *imgDraw);

	/// <summary>
	/// FInd the biggest group of contours i.e. the hand
	/// <param name="contours">The set of contours</param>
	/// </summary>
	int getContourIndex(std::vector<std::vector<cv::Point> > contours);

	/// <summary>
	/// Calculates the hull around the hand
	/// <param name="contours">The contours calculated from earlier</param>
	/// </summary>
	std::vector<cv::Point> getHull(std::vector<cv::Point> contours);

	/// <summary>
	/// Draws on the hull
	/// <param name="img">Image to draw on</param>
	/// <param name="hull">The hull points to draw on</param>
	/// </summary>
	void drawHull(cv::Mat *img, std::vector<cv::Point> hull);

	/// <summary>
	/// Gets the furthest away point that represents the longest finger tip
	/// <param name="img">Image to search in</param>
	/// <param name="hull">Hull points to search in</param>
	/// <param name="centre">The centre point that distance is measured from</param>
	/// </summary>
	cv::Point getMaxPoint(cv::Mat *img, std::vector<cv::Point> hull, pixel centre);

};


