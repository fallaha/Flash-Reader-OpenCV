﻿
#include "stdafx.h"
#include "opencv.hpp"
#pragma comment(lib, "opencv_world320.lib")

#define SPACE 0
#define ARC 255
#define RING 5

void read_flash(cv::Mat& image);
int main(){
	cv::Mat image;
	image = cv::imread("flashcode4.jpg");
	read_flash(image);
	cv::imwrite("bar.jpg", image);
	cv::imshow("image", image);
	cv::waitKey(0);
	return 0;
}

enum direction {
	LR = 0,
	LD,
	LL,
	LU,
	LDR,
	LDL,
	LUL,
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
	for (int i = 0; i < 4; i++)
		add(xp[i], i);
}

void add_ext(cv::Point xp[8])
{
	for (int i = 4; i < 8; i++)
		add(xp[i], i);
}


int test_all(const cv::Mat_<uchar>& image, cv::Point xp[8]){
	int t = 0;
	for (int i = 0; i < 8; i++)
		t += image(xp[i])>0 ? 1 : 0;
	return t;
}

void draw_point(const cv::Mat_<uchar>& image, cv::Mat& flash, cv::Point xp[8]){
	for (int i = 0; i < 8; i++){
		//int t = image(xp[i]);
		flash.at<uchar>(xp[i]) = 128;
		//std::cout << xp[i] << t <<" ";
	}
}

/*
int GetUnitSize(const cv::Mat_<uchar>& image, cv::Point xp[8], int dir){
cv::Point p = xp[dir];
int size = 0;
while (image(xp[dir]) == ARC)
add(xp[dir],dir);

while (image(xp[dir]) == SPACE){
add(xp[dir], dir);
size++;
}
xp[dir] = p;
return size;
}
*/

int GetUnitSize(const cv::Mat_<uchar>& image, cv::Point xp[8], int dir){
	cv::Point p = xp[dir];
	int size = 0;
	int pattern[2] = { SPACE, ARC };
	int t = 0;

	for (int i = 0; i < 2; i++){
		size = 0;
		while (image(xp[dir]) == pattern[i]){
			add(xp[dir], dir);
			size++;
		}
		t += size;
		//  std::cout << size << " ";
	}
	t /= 2;
	//std::cout << std::endl;
	xp[dir] = p;
	return t;
}



void skip_capital(const cv::Mat_<uchar>& image, cv::Point xp[8]){

	while (image(xp[LU]) == ARC || image(xp[LR]) == ARC || image(xp[LL]) == ARC || image(xp[LD]) == ARC)
		add_org(xp);

	while (image(xp[LUR]) != SPACE || image(xp[LUL]) != SPACE || image(xp[LDR]) != SPACE || image(xp[LDL]) != SPACE)
		add_ext(xp);


}

void LineProcess(const cv::Mat_<uchar>& image, int arc[], cv::Point xp[8], int usize, int dir, int nring){

	for (int i = 0; i < 5; i++){
		for (int j = 0; j < usize; j++){ /* Skip Space */
			add(xp[dir], dir);
		}
		for (int j = 0; j < usize; j++){ /* Count Arc Pixel */
			if (image(xp[dir]) == ARC)
				arc[i] ++;
			add(xp[dir], dir);
		}
		arc[i] = (arc[i] >= usize / 2) ? 1 : 0;

		cv::Point x = xp[dir]; int c = 0;

		if (image(xp[dir]) == arc[i] * 255){
			while (image(xp[dir]) == arc[i] * 255 && c <2 * usize){
				add(xp[dir], dir);
				c++;
			}
			if (c >= usize / 2)
				xp[dir] = x;
		}
		else {
			while (image(xp[dir]) != arc[i] * 255){
				sub(xp[dir], dir);
				c++;
			}

			c--;
			add(xp[dir], dir);

			if (c >= usize / 2)
				xp[dir] = x;
		}
	}
}

void read_flash(cv::Mat& image){

	cv::cvtColor(image, image, CV_BGR2GRAY);
	cv::bitwise_not(image, image);
	cv::threshold(image, image, 25, 255, cv::THRESH_BINARY);
	cv::Size size = image.size();
	cv::Point def = cv::Point(size.width / 2, size.height / 2);

	cv::Point xp[8] = { def, def, def, def, def, def, def, def };

	skip_capital(image, xp);

	int unit_size[8];
	for (int i = 0; i < 8; i++)
		unit_size[i] = GetUnitSize(image, xp, i);

	int arc[8][RING] = { 0 };

	for (int i = 0; i < 8; i++)
		LineProcess(image, arc[i], xp, unit_size[i], i, RING);

	int code[5] = { 0 };

	for (int i = 0; i < RING; i++){
		for (int j = 0; j < 8 / 2; j++){
			code[i] *= 2;
			code[i] += arc[j][i];
			code[i] *= 2;
			code[i] += arc[j + 4][i];

		}
	}


	for (int i = 1; i < RING; i++)
		std::cout << code[i] << ".";

	unsigned int iCode = code[1] | code[2] << 8 | code[3] << 8 * 2 | code[4] << 8 * 3;

	std::cout << std::endl << iCode;


}