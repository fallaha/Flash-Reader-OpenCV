/*
 * Ali Fallah (c) 1397/11/24
 * New Code For Decode the FC
*/
#include "stdafx.h"
#include "opencv.hpp"
#pragma comment(lib, "opencv_world320.lib")
#include <iostream>
#include <stdio.h>
#include <vector>
#include <stdio.h>
#include <sstream>

//#define DEBUG_EN

using namespace cv;
#define TOLERANCE 2
#define ARC 255
#define SPACE 0

int read();


const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

int main(){
	read();
	std::getchar();

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

void add(cv::Point p[], int d){
	switch (d)
	{
	case LR:
		p[d].x++;
		break;
	case LDR:
		p[d].x++;
		p[d].y++;
		break;
	case LD:
		p[d].y++;
		break;
	case LDL:
		p[d].x--;
		p[d].y++;
		break;
	case LL:
		p[d].x--;
		break;
	case LUL:
		p[d].x--;
		p[d].y--;
		break;
	case LT:
		p[d].y--;
		break;
	case LUR:
		p[d].x++;
		p[d].y--;
		break;
	}
	return;
}

void add_all(cv::Point xp[8])
{
	for (int i = 0; i < 8; i++)
		add(xp, i);
}

void sub(cv::Point p[], int d){
	switch (d)
	{
	case LR:
		p[d].x--;
		break;
	case LDR:
		p[d].x--;
		p[d].y--;
		break;
	case LD:
		p[d].y--;
		break;
	case LDL:
		p[d].x++;
		p[d].y--;
		break;
	case LL:
		p[d].x++;
		break;
	case LUL:
		p[d].x++;
		p[d].y++;
		break;
	case LT:
		p[d].y++;
		break;
	case LUR:
		p[d].x--;
		p[d].y++;
		break;
	}
	return;
}


/*
 * ignore the Center of the FC
 * the Maximum Size is Smaller than image Size
 * Limited to Image Size
 */
int skip_capital(const cv::Mat_<uchar>& image, cv::Point xp[8], int width_img){

	int max = width_img / 5;
	int c;
	int radius = 0;

	for (int i = 0; i < 8; i++){
		c = max;
		while (c-- && image(xp[i]) == ARC){
			add(xp,i);
			radius++;
		}
		if (c <= 0) /* The Center Circle doesn't exist */
			return 0;
	}

	return radius / 8;
	
}

int distance_to_next_edge(const cv::Mat_<uchar>& image,cv::Point xp[8], int dir, int unit_size){
	
	Point save = xp[dir];
	int dist = 0;
	int current_color = image(xp[dir]);
	int counter = unit_size;
	while (counter > 0 && image(xp[dir]) == current_color){
		add(xp, dir);
		dist++;
		counter--;
	}

	xp[dir] = save;
	
	return dist;
}

void goto_next_edge(const cv::Mat_<uchar>& image, cv::Point xp[8], int dir){

	int current_color = image(xp[dir]);
	while (image(xp[dir]) == current_color)
		add(xp, dir);
	
}


int get_unit_size(const cv::Mat_<uchar>& image, cv::Point xp[8], int dir, int radius){

	Point p = xp[dir];
	int unit_size = 0;
	int counter = radius;

	while (image(xp[dir]) == SPACE && counter--){
		add(xp, dir);
		unit_size++;
	}

	if (counter == 0)
		return 0;

	counter = unit_size;
	while (counter > 0 && image(xp[dir]) == ARC){
		add(xp, dir);
		counter--;
	}

	
	if (counter == 0){
		if (distance_to_next_edge(image, xp, dir, unit_size) < unit_size / 2)
			goto_next_edge(image,xp, dir);
	}	
	else if (counter > unit_size / TOLERANCE){
		return 0;
	}

	return unit_size;
}

int get_entry_point_index(const cv::Mat_<uchar>& image, cv::Point xp[8]){

	int c = 0;
	int index = 0;

	for (int i = 0; i < 8; i++)
		if (image(xp[i]) == ARC){
			c++;
			index = i;
		}

	if (c > 1)
		return -1;

	return index;
}

#define ABS(a) if(a<0) a=-a

unsigned char read_data_ring(const cv::Mat_<uchar>& image, cv::Point xp[8], int unit_size[8], int ep)
{
	int counter;
	int current_color;
	int bit[8] = { 0 };

	for (int i = 0; i < 8; i++){
		current_color = image(xp[i]);
		counter = 0;
		bit[i] = current_color;
		while (image(xp[i]) == current_color){
			add(xp, i);
			counter++;
		}

		counter -= unit_size[i];

		//std::cout << "counter : " << counter << " "; /* Error Message */

		ABS(counter);

		if (counter > unit_size[i] / TOLERANCE){
#ifdef DEBUG_EN
			std::cout << "in read_data_ring1 - counter > unit_size[i] / TOLERANCE - Error occurred :( \n"; /* Error Message */
#endif
			return 0;
		}

		/* Check Bit Skip */

		counter = unit_size[i];
		while (counter > 0 && image(xp[i]) != current_color){
			add(xp, i);
			counter--;
		}



		if (counter == 0){
			if (distance_to_next_edge(image, xp, i, unit_size[i]) < unit_size[i] / 2){
				goto_next_edge(image, xp, i);
			}
		}
		else if (counter > unit_size[i] / TOLERANCE){
#ifdef DEBUG_EN
			std::cout << "in read_data_ring2 - counter > unit_size[i] / TOLERANCE - Error occurred :( \n"; /* Error Message */
#endif
			return 0;
		}

	}

	unsigned char code = 0;
	for (int i = ep; i < 8+ep; i++){
		code *= 2;
		code += ((bit[i % 8] >= unit_size[i % 8] / 2) ? 1 : 0);
	}

	//printf("Code = %d \n", code);

	return code;
}

struct information {
	unsigned rings    : 3 ;
	unsigned reserved : 2 ;
	unsigned version  : 3 ;
};



#define MAX_RING 16
const char ring_count_law[] = { 2, 4, 6, 8, 10, 12, 14, 16 };

void draw_point(Mat& flash, Point xp[8]){
	for (int i = 0; i < 8; i++)
		flash.at<uchar>(xp[i]) = 128;
}

int decode(Mat& image,unsigned char data[16],Point center){

	Point xp[8];

	/* Set The Center of Circle in All Line Process */
	for (int i = 0; i < 8; i++)
		xp[i] = center;

	int radius;
	if (!(radius = skip_capital(image, xp, (image.size().width + image.size().height) / 2))){
#ifdef DEBUG_EN
		std::cout << "The Radius Was Very Big :( \n"; /* Error Message */
#endif
		return 0; /* an error occurred */
	}
	


	int unit_size[8];
	for (int i = 0; i < 8; i++)
		if (!(unit_size[i] = get_unit_size(image, xp, i, radius))){
#ifdef DEBUG_EN
			std::cout << "in Unit Size Error occurred :( \n"; /* Error Message */
#endif
			return 0;
		}


	int ep = get_entry_point_index(image, xp);
	if (ep == -1){
#ifdef DEBUG_EN
		std::cout << "in Detect Entry Point Error occurred :( \n"; /* Error Message */
#endif
		return 0; /* an error occurred */
	}

	if ((read_data_ring(image, xp, unit_size, ep)) != 128){ /* Skip Direction Ring */
#ifdef DEBUG_EN
		std::cout << "in Compare Entry Point Error occurred :( \n" ; /* Error Message */
#endif
		return 0; /* an error occurred */
	}

	unsigned char info = read_data_ring(image, xp, unit_size, ep);
	struct information *info_s = (struct information *)&info;
	
	int ring_count = ring_count_law[info_s->rings];

/*
	printf("rings : %d\n", info_s->rings);
	printf("reserved : %d\n", info_s->reserved);
	printf("version : %d\n", info_s->version);
*/
	if (info_s->version > 0){
		std::cout << "Version Not Support!\n";
		return 0;
	}

	if (ring_count == 0){
#ifdef DEBUG_EN
		std::cout << "in Ring Count Error occurred :( \n"; /* Error Message */
#endif
		return 0;
	}

	for (int i = 0; i < ring_count; i++)
		if (!(data[i] = read_data_ring(image, xp, unit_size, ep))){
#ifdef DEBUG_EN
			std::cout << "in Ring Data Reading Error occurred :( \n"; /* Error Message */
#endif
			return 0;
		}

	/*
	for (int i = 0; i < 20; i++){
		draw_point(image, xp);
		add_all(xp);
	}
	imshow("a2", image);
	waitKey(0);
	*/

	char parity_check[8] = { 0 };

	for (int j = 7; j >= 0; j--)
		for (int i = 0; i < ring_count; i++)
			if (data[i] & 1 << j)
				parity_check[7-j]++;



	unsigned char comp = 0;
	for (int i = 0; i < 8; i++){
		comp *= 2;
		comp += parity_check[i] % 2;
	}

	char parity;
	if (!(parity = read_data_ring(image, xp, unit_size, ep))){
#ifdef DEBUG_EN
		std::cout << "in Data Parity Reading Error occurred :( \n"; /* Error Message */
#endif
		return 0;
}

	if (comp != parity){
#ifdef DEBUG_EN
		std::cout << "in Parity Error occurred :( \n"; /* Error Message */
#endif
		return 0; /* ;< */
	}
	return ring_count; /* All thing Correctly Worked :) */
}


int read(){

	unsigned char data[16] = { 0 };
	Mat cameraFeed;
	Mat gray;
	Mat image;
	int count;


	VideoCapture capture;

	capture.open(0);

	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	while (1){

		capture.read(cameraFeed);
		//cameraFeed = imread("C:\\Users\\Ali\\Desktop\\p3.jpg");

		cvtColor(cameraFeed, gray, CV_BGR2GRAY);
		cvtColor(cameraFeed, image, CV_BGR2GRAY);

		/* Reduce the noise so we avoid false circle detection */
		GaussianBlur(gray, gray, cv::Size(3, 3), 2, 2);

		std::vector<cv::Vec3f> circles;

		HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows / 8, 25, 100, 0, 0);

		bitwise_not(image, image);
		threshold(image, image, 155, 255, THRESH_BINARY);

		for (size_t i = 0; i < circles.size(); i++)
		{
			cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);

			if (count = decode(image, data, center))
				goto finish;

			circle(cameraFeed, center, 12, cv::Scalar(0, 255, 0), -1, 8, 0);
			circle(cameraFeed, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);
		}



		imshow("cameraFeed", cameraFeed);
		imshow("image", image);

		waitKey(30);
	}

	finish:
	for (int i = 0; i < count; i++){
		printf("%d", data[i]);
		if (i != count - 1)
			printf(".");
		else
			printf("\n");
	}
	return 1;
}