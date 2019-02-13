/*
 * Ali Fallah (c) 1397/11/23
 * + Video Capture and RealTime Detect FlashCode
*/
#include "stdafx.h"
#include "opencv.hpp"
#pragma comment(lib, "opencv_world320.lib")
#include <iostream>
#include <stdio.h>
#include <vector>
#include <stdio.h>

//objectTrackingTutorial.cpp

//Written by  Kyle Hounslow 2013

//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software")
//, to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//IN THE SOFTWARE.

#include <sstream>
#include <string>
#include <iostream>
#include <opencv\highgui.h>
#include <opencv\cv.h>

using namespace cv;
//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;
int Circle_min = 25;
int Circle_max = 500;
//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS = 50;
//minimum and maximum object area
const int MIN_OBJECT_AREA = 20 * 20;
const int MAX_OBJECT_AREA = FRAME_HEIGHT*FRAME_WIDTH / 1.5;
//names that will appear at the top of each window
const std::string windowName = "Original Image";
const std::string windowName1 = "HSV Image";
const std::string windowName2 = "Thresholded Image";
const std::string windowName3 = "After Morphological Operations";
const std::string trackbarWindowName = "Trackbars";

#define SPACE 0
#define ARC 255
#define RING 5

void read_flash(cv::Mat image, cv::Point def);

void on_trackbar(int, void*)
{//This function gets called whenever a
	// trackbar position is changed





}
std::string intToString(int number){


	std::stringstream ss;
	ss << number;
	return ss.str();
}
void createTrackbars(){
	//create window for trackbars


	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf_s(TrackbarName, "H_MIN", H_MIN);
	sprintf_s(TrackbarName, "H_MAX", H_MAX);
	sprintf_s(TrackbarName, "S_MIN", S_MIN);
	sprintf_s(TrackbarName, "S_MAX", S_MAX);
	sprintf_s(TrackbarName, "V_MIN", V_MIN);
	sprintf_s(TrackbarName, "V_MAX", V_MAX);
	sprintf_s(TrackbarName, "C_MIN", Circle_min);
	sprintf_s(TrackbarName, "C_MAX", Circle_max);

	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar);

	createTrackbar("C_MIN", trackbarWindowName, &Circle_min, Circle_max, on_trackbar);
	createTrackbar("C_MAX", trackbarWindowName, &Circle_max, Circle_max, on_trackbar);


}
void drawObject(int x, int y, Mat &frame){

	//use some of the openCV drawing functions to draw crosshairs
	//on your tracked image!

	//UPDATE:JUNE 18TH, 2013
	//added 'if' and 'else' statements to prevent
	//memory errors from writing off the screen (ie. (-25,-25) is not within the window!)

	circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
	if (y - 25>0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
	if (y + 25<FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	if (x - 25>0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	if (x + 25<FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);

}
void morphOps(Mat &thresh){

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle

	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(8, 8));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);


	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);



}
void trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed){

	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	std::vector< std::vector<Point> > contours;
	std::vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		//if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
		if (numObjects<MAX_NUM_OBJECTS){
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;

				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
				if (area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
					x = moment.m10 / area;
					y = moment.m01 / area;
					objectFound = true;
					refArea = area;
				}
				else objectFound = false;


			}
			//let user know you found an object
			if (objectFound == true){
				putText(cameraFeed, "Tracking Object", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
				//draw object location on screen
				drawObject(x, y, cameraFeed);
			}

		}
		else putText(cameraFeed, "TOO MUCH NOISE! ADJUST FILTER", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
	}
}
int main(int argc, char* argv[])
{
	//some boolean variables for different functionality within this
	//program
	bool trackObjects = false;
	bool useMorphOps = false;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	Mat gray;

	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold;
	//x and y values for the location of the object
	int x = 0, y = 0;
	//create slider bars for HSV filtering
	createTrackbars();
	Circle_max = 100;
	//video capture object to acquire webcam feed
	VideoCapture capture;
	//open capture object at location zero (default location for webcam)
	capture.open(0);
	//set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	while (1){
		//store image to matrix
		capture.read(cameraFeed);
		//convert frame from BGR to HSV colorspace



		cvtColor(cameraFeed, gray, CV_BGR2GRAY);

		/// Reduce the noise so we avoid false circle detection
		GaussianBlur(gray, gray, cv::Size(3, 3), 2, 2);

		//Canny(gray, gray, 100, 120);

		//Canny(src_gray, src_gray,10,120);
		std::vector<cv::Vec3f> circles;

		//cv::pyrDown(gray, gray, cv::Size(gray.cols / 2, gray.rows / 2));
		//cv::pyrDown(gray, gray, cv::Size(gray.cols / 2, gray.rows / 2));


		HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows / 8, Circle_min, Circle_max, 0, 0);
		for (size_t i = 0; i < circles.size(); i++)
		{
			cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			read_flash(cameraFeed, center);
			// circle center
			circle(cameraFeed, center, 12, cv::Scalar(0, 255, 0), -1, 8, 0);
			// circle outline
			circle(cameraFeed, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
		}


		//cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix
		//inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
		//perform morphological operations on thresholded image to eliminate noise
		//and emphasize the filtered object(s)
		if (useMorphOps)
			morphOps(threshold);
		//pass in thresholded frame to our object tracking function
		//this function will return the x and y coordinates of the
		//filtered object
		if (trackObjects)
			trackFilteredObject(x, y, threshold, cameraFeed);

		//show frames 
		//imshow(windowName2, threshold);
		imshow(windowName, cameraFeed);
		imshow(windowName1, gray);


		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		waitKey(30);
	}






	return 0;
}



enum direction {
	LR = 0,
	LDR,
	LD,
	LDL,
	LL,
	LUL,
	LT,
	LUR
};

void add(cv::Point& p, int d){
	switch (d)
	{
	case LR:
		p.x++;
		break;
	case LDR:
		p.x++;
		p.y++;
		break;
	case LD:
		p.y++;
		break;
	case LDL:
		p.x--;
		p.y++;
		break;
	case LL:
		p.x--;
		break;
	case LUL:
		p.x--;
		p.y--;
		break;
	case LT:
		p.y--;
		break;
	case LUR:
		p.x++;
		p.y--;
		break;
	}
	return;
}

void sub(cv::Point& p, int d){
	switch (d)
	{
	case LR:
		p.x--;
		break;
	case LDR:
		p.x--;
		p.y--;
		break;
	case LD:
		p.y--;
		break;
	case LDL:
		p.x++;
		p.y--;
		break;
	case LL:
		p.x++;
		break;
	case LUL:
		p.x++;
		p.y++;
		break;
	case LT:
		p.y++;
		break;
	case LUR:
		p.x--;
		p.y++;
		break;
	}
	return;
}



void add_all(cv::Point xp[8])
{
	for (int i = 0; i < 8; i++)
		add(xp[i], i);
}

void add_org(cv::Point xp[8])
{
	for (int i = 0; i < 8; i += 2)
		add(xp[i], i);
}

void add_ext(cv::Point xp[8])
{
	for (int i = 1; i < 8; i += 2)
		add(xp[i], i);
}

void draw_point(cv::Mat& flash, cv::Point xp[8]){
	for (int i = 0; i < 8; i++)
		flash.at<uchar>(xp[i]) = 128;
}

int GetUnitSize(const cv::Mat_<uchar>& image, cv::Point xp[8], int dir, int r){
	cv::Point p = xp[dir];
	int size = 0;
	int pattern[2] = { SPACE, ARC };
	int t = 0;
	int c;
	//int corruption_pixel = 0;
	for (int i = 0; i < 2; i++){
		size = 0;
		c = r;
		/* size < r/10 is for corruption pixel (pic 3) */
		while (size < r / 10 || (image(xp[dir]) == pattern[i] && c--)){
			add(xp[dir], dir);
			size++;
		}
		t += size;

		if (c <= 0)
			return 0;

		//  std::cout << size << " ";
	}
	t /= 2;
	//std::cout << std::endl;
	xp[dir] = p;
	return t;
}


int skip_capital(const cv::Mat_<uchar>& image, cv::Point xp[8], int width_img){

	int max = (width_img * 3) / 4;
	int c;
	int size = 0;

	for (int i = 0; i < 8; i++){
		c = max / 10;
		while (c-- && image(xp[i]) == ARC){
			add(xp[i], i);
			size++;
		}
		if (c <= 0)
			return 0;
	}

	return size / 8;


}

void LineProcess(const cv::Mat_<uchar>& image, int arc[], cv::Point xp[8], int usize, int dir, int nring){
	int pattern[2 * RING] = { SPACE, ARC, SPACE, ARC, SPACE, ARC, SPACE, ARC, SPACE, ARC };

	for (int i = 0; i < 2 * nring; i++){

		for (int j = 0; j < usize; j++){ /* Skip Space or Count Arc Pixel */
			if (pattern[i] == ARC && image(xp[dir]) == ARC)
				arc[i / 2]++;
			add(xp[dir], dir);

		}

		if (pattern[i] == ARC){
			//std::cout << arc[i / 2] << " ";
			arc[i / 2] = (arc[i / 2] >= usize / 2) ? 1 : 0;
		}

		/* Align Pointer  */
		cv::Point x = xp[dir]; int c = 0;

		if (image(xp[dir]) == pattern[i]){ /* if Pointer, point to Previous Area */
			while (image(xp[dir]) == pattern[i] && c < usize){
				add(xp[dir], dir);
				c++;
			}

			/* If the size of the remaining area is more than C/2, then there will be an arc in this area
			so back value */
			if (c >= (usize * 3) / 4)
				xp[dir] = x;
			//else 
			//	std::cout << "dir : " << dir << " ring : " << i << " Arc or Ring : " << pattern[i] << " and " << c << " forward" << std::endl;

		}
		else {	/* if Pointer, point to invers Area */
			while (image(xp[dir]) != pattern[i]){ /* Back to Area to correctly point*/
				sub(xp[dir], dir);
				c++;
			}

			c--;
			add(xp[dir], dir);

			if (c >= (usize * 3) / 4)
				xp[dir] = x;
			//else if (xp[dir] != x)
			//std::cout << "dir : " << dir << " ring : " << i << " Arc or Ring : " << pattern[i] << " and " << c << " backward" << std::endl;

		}
	}
	//std::cout << std::endl;
}


void read_flash(cv::Mat image, cv::Point def){
	cv::cvtColor(image, image, CV_BGR2GRAY);
	cv::bitwise_not(image, image);
	/* good treshold
	190 for pic 1,2
	120 for pic 3*/
	cv::threshold(image, image, 155, 255, cv::THRESH_BINARY);

	cv::Size size = image.size();

	if (def == cv::Point(-1, -1))
		def = cv::Point(size.width / 2, size.height / 2);

	cv::Point xp[8];

	//cv::namedWindow("qwe", CV_WINDOW_NORMAL);
	//cv::imshow("qwe", image);

	//return;

	for (int i = 0; i < 8; i++)
		xp[i] = def;

	int capital;
	if (!(capital = skip_capital(image, xp, (size.height + size.width) / 2)))
		return;

	//	std::cout << "capital size = " << capital << std::endl;


	/* Get Unit Size of each wave */
	int unit_size[8];
	for (int i = 0; i < 8; i++)
		if (!(unit_size[i] = GetUnitSize(image, xp, i, capital)))
			return;

	//for (int i = 4; i < 8; i++)
	//	unit_size[i] = (unit_size[0] + unit_size[4]) / 2;

	//for (int i = 0; i < 8; i++)
	//	std::cout << unit_size[i] << std::endl;

	/* arc variable is array of 8 bit per each ring that save each bit in each rings */
	int arc[8][RING] = { 0 };
	for (int i = 0; i < 8; i++)
		LineProcess(image, arc[i], xp, unit_size[i], i, RING);

	//std::cout << " BitMap Array Code \n";
	//for (int j = 0; j < 8; j++){
	//	for (int i = 0; i < RING; i++){
	//		std::cout << arc[j][i] << " ";
	//	}
	//	std::cout << std::endl;
	//}

	/* code variable is array of 4 unsigned char (1 Byte) and save the 4 value of ring */
	/* note : the first ring must be 255 (and the last 4 ring is the real value) */
	int code[5] = { 0 };
	for (int i = 0; i < RING; i++)
		for (int j = 0; j < 8; j++){
			code[i] *= 2;
			code[i] += arc[j][i];
		}

	/* Show the code with .*/
	for (int i = 1; i < RING; i++)
		std::cout << code[i] << ".";

	std::cout << std::endl;
	/* Convert code array to Unsigned integer */
	unsigned int iCode = code[1] | code[2] << 8 | code[3] << 8 * 2 | code[4] << 8 * 3;


	//cv::namedWindow("as", CV_WINDOW_NORMAL);
	//cv::imwrite("C:\\Users\\Ali\\Desktop\\ali\\00out.jpg",image);
	//cv::imshow("as", image);
}