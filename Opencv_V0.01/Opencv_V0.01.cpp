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

using namespace cv;

#define ARC 255
#define SPACE 0

int read();

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
	else if (counter > radius / 10){
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

		if (counter > unit_size[i] / 3){
			std::cout << "in read_data_ring1 - counter > unit_size[i] / 3 - Error occurred :( \n"; /* Error Message */
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
		else if (counter > unit_size[i] / 5){
			std::cout << "in read_data_ring2 - counter > unit_size[i] / 5 - Error occurred :( \n"; /* Error Message */
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
	unsigned version  : 3 ;
	unsigned reserved : 2 ;
	unsigned rings    : 3 ;
};



#define MAX_RING 16
const char ring_count_law[] = { 5, 4, 6, 8, 10, 12, 14, 16 };

struct  ali
{
	int a;
	char b;
};

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
		std::cout << "The Radius Was Very Big :( \n"; /* Error Message */
		return 0; /* an error occurred */
	}
	


	int unit_size[8];
	for (int i = 0; i < 8; i++)
		if (!(unit_size[i] = get_unit_size(image, xp, i, radius))){
			std::cout << "in Unit Size Error occurred :( \n"; /* Error Message */
			return 0;
		}


	int ep = get_entry_point_index(image, xp);
	if (ep == -1){
		std::cout << "in Detect Entry Point Error occurred :( \n"; /* Error Message */
		return 0; /* an error occurred */
	}

	if ((read_data_ring(image, xp, unit_size, ep)) != 128){ /* Skip Direction Ring */
		std::cout << "in Compare Entry Point Error occurred :( \n" ; /* Error Message */
		return 0; /* an error occurred */
	}

	unsigned char info = read_data_ring(image, xp, unit_size, ep);
	struct information *info_s = (struct information *)&info;
	
	int ring_count = ring_count_law[info_s->rings];

	if (ring_count == 0){
		std::cout << "in Ring Count Error occurred :( \n"; /* Error Message */
		return 0;
	}

	for (int i = 0; i < ring_count; i++)
		if (!(data[i] = read_data_ring(image, xp, unit_size, ep))){
			std::cout << "in Ring Data Reading Error occurred :( \n"; /* Error Message */
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

	char parity = read_data_ring(image, xp, unit_size, ep);

	if (comp != parity){
		std::cout << "in Parity Error occurred :( \n"; /* Error Message */
		return 0; /* ;< */
	}
	return ring_count; /* All thing Correctly Worked :) */
}


int read(){
	/* Read the Image and Process on it to Decode! */
	Mat image;
	unsigned char data[16] = { 0 };

	image = imread("C:\\Users\\Ali\\Desktop\\FC-2-p.jpg");
	Point center = Point(image.size().width / 2, image.size().height / 2);

	cv::cvtColor(image, image, CV_BGR2GRAY);
	cv::bitwise_not(image, image);
	/* good treshold
	190 for pic 1,2
	120 for pic 3*/
	cv::threshold(image, image, 155, 255, cv::THRESH_BINARY);



	/* Set Center of The FC */
	int count;
	if (!(count = decode(image,data,center)))
		return 0;

	for (int i = 0; i < count; i++){
		printf("%d", data[i]);
		if (i != count - 1)
			printf(".");
		else
			printf("\n");
	}

	std::cout << "Program RUN\n";



	

	return 1;
}