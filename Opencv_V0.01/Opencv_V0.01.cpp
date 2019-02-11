/* Ali Fallah (c) 1397/11/22 */
#include "stdafx.h"
#include "opencv.hpp"
#pragma comment(lib, "opencv_world320.lib")
#include <iostream>
#include <stdio.h>
#include <vector>


#define SPACE 0
#define ARC 255
#define RING 5

void read_flash(cv::Mat image, cv::Point def);


void show_dart(cv::Mat& image){
	//cv::cvtColor(image, image, CV_BGR2GRAY);
	cv::bitwise_not(image, image);

	cv::threshold(image, image, 120, 255, cv::THRESH_BINARY);

	cv::Size size = image.size();
	cv::namedWindow("qwe", CV_WINDOW_NORMAL);
	cv::imshow("qwe", image);
	cv::waitKey(0);
	return;

}
/** @function main */
int main(int argc, char** argv)
{
	cv::Mat src, src_gray;

	/// Read the image
	src = cv::imread("C:\\Users\\Ali\\Desktop\\ali\\1.jpg", 1);

	if (!src.data)
	{
		return -1;
	}

	/// Convert it to gray
	cvtColor(src, src_gray, CV_BGR2GRAY);

	/// Reduce the noise so we avoid false circle detection
	GaussianBlur(src_gray, src_gray, cv::Size(21, 21), 2, 2);

	/// Do the operation new_image(i,j) = alpha*image(i,j) + beta
	//for (int y = 0; y < src_gray.rows; y++)
	//	for (int x = 0; x < src_gray.cols; x++)
	//		src_gray.at<uchar>(y, x) =
	//			cv::saturate_cast<uchar>(1*(src_gray.at<uchar>(y, x)) + 0);



	//Canny(src_gray, src_gray,10,120);
	std::vector<cv::Vec3f> circles;

	//show_dart(src_gray);

	/// Apply the Hough Transform to find the circles
	/* good low 25 for pic 1,2*/

	cv::pyrDown(src_gray, src_gray, cv::Size(src_gray.cols / 2, src_gray.rows / 2));
	cv::pyrDown(src_gray, src_gray, cv::Size(src_gray.cols / 2, src_gray.rows / 2));


	//	cv::pyrDown(src, src, cv::Size(src.cols / 2, src.rows / 2));


	//cv::namedWindow("salam", CV_WINDOW_NORMAL);
	//cv::imshow("salam", src_gray);
	//cv:cvWaitKey(0);

	HoughCircles(src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.rows / 8, 25, 100, 0, 0);
#define DEVIDE_SIZE 4
	/// Draw the circles detected
	for (size_t i = 0; i < circles.size(); i++)
	{
		std::cout << " for \n";
		cv::Point center(cvRound(circles[i][0])*DEVIDE_SIZE, cvRound(circles[i][1]) * DEVIDE_SIZE);
		int radius = cvRound(circles[i][2])*DEVIDE_SIZE;
		read_flash(src, center);
		// circle center
		circle(src, center, 12, cv::Scalar(0, 255, 0), -1, 8, 0);
		// circle outline
		circle(src, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
	}

	/// Show your results
	cv::namedWindow("Result", CV_WINDOW_NORMAL);
	imshow("Result", src);
	cv::waitKey(0);
	return 0;

}

enum direction {
	LR = 0,
	LDR,
	LD,
	LDL,
	LL,
	LUL,
	LU,
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
	case LU:
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
	case LU:
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
			std::cout << arc[i / 2] << " ";
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
	std::cout << std::endl;
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

	std::cout << "capital size = " << capital << std::endl;


	/* Get Unit Size of each wave */
	int unit_size[8];
	for (int i = 0; i < 8; i++)
		if (!(unit_size[i] = GetUnitSize(image, xp, i, capital)))
			return;

	//for (int i = 4; i < 8; i++)
	//	unit_size[i] = (unit_size[0] + unit_size[4]) / 2;

	for (int i = 0; i < 8; i++)
		std::cout << unit_size[i] << std::endl;

	/* arc variable is array of 8 bit per each ring that save each bit in each rings */
	int arc[8][RING] = { 0 };
	for (int i = 0; i < 8; i++)
		LineProcess(image, arc[i], xp, unit_size[i], i, RING);

	std::cout << " BitMap Array Code \n";
	for (int j = 0; j < 8; j++){
		for (int i = 0; i < RING; i++){
			std::cout << arc[j][i] << " ";
		}
		std::cout << std::endl;
	}

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

	/* Convert code array to Unsigned integer */
	unsigned int iCode = code[1] | code[2] << 8 | code[3] << 8 * 2 | code[4] << 8 * 3;
	std::cout << std::endl << iCode;

	for (int i = 0; i < 300; i++){
		draw_point(image, xp);
		add_all(xp);
	}

	//cv::namedWindow("as", CV_WINDOW_NORMAL);
	//cv::imwrite("C:\\Users\\Ali\\Desktop\\ali\\00out.jpg",image);
	//cv::imshow("as", image);
}