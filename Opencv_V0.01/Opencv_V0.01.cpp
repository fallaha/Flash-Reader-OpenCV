/*
* Ali Fallah (c) 1397/11/25
* Flash Code Generator
*/

#include "stdafx.h"
#include "opencv.hpp"
#pragma comment(lib, "opencv_world320.lib")
#include <iostream>
#include <stdio.h>
#include <vector>

using namespace cv;

const int xsize = 2480;
const int ysize = 3508;

#define MAX_DATA_RING 16

int generate(unsigned char data[], int count);
void conver_to_code_string(unsigned char data[], int count, char buffer[]);

int main(){
	unsigned char data[MAX_DATA_RING];
	int count = 3;

	while (!((count % 2) - 1 && count <= 16 && count >= 2)){
		printf("Enter Count of Byte (2,4,8..16) : ");
		scanf_s("%d", &count);
	}

	for (int i = 0; i < count; i++)
		scanf_s("%d", &data[i]);


	generate(data, count);

	return 0;
}


struct information {
	unsigned rings : 3;
	unsigned reserved : 2;
	unsigned version : 3;
};


int generate(unsigned char data[], int count){

	char buffer[MAX_DATA_RING * 3 + 17];
	Mat FC = Mat::zeros(ysize, xsize, CV_8UC1);
	int deg = 0;
	int margin = (xsize / 10) / 2;
	int radius = xsize / 2 - margin;

	unsigned char ring_val[MAX_DATA_RING + 1 + 3];

	int capital_radius;
	capital_radius = xsize / 10.6666;

	int data_ring_size;
	data_ring_size = xsize / 2 - capital_radius - margin;

	int thinkness = data_ring_size / ((count + 4) * 2);



	/* Calculate Parity */
	char parity_check[8] = { 0 };

	for (int j = 7; j >= 0; j--)
		for (int i = 0; i < count; i++)
			if (data[i] & 1 << j)
				parity_check[7 - j]++;



	unsigned char parity = 0;
	for (int i = 0; i < 8; i++){
		parity *= 2;
		parity += parity_check[i] % 2;
	}

	data[count] = parity;
	//count += 1; /* for Parity */

	ring_val[0] = parity;

	int index = 1;
	for (int i = count - 1; i >= 0; i--, index++)
		ring_val[index] = data[i];

	struct information *info = (struct information *) &ring_val[index];
	info->version = 0;
	info->rings = (count / 2) - 1;
	info->reserved = 0;

	index++; /* Slip info byte */

	ring_val[index] = 128;
	index++; /* Slip Direction byte */

	ring_val[index] = 0; /* Distance Byte */




	for (int j = 0; j < (count + 4 - 1) * 2; j++){
		for (int i = 0; i < 8; i++){
			ellipse(FC, Point(xsize / 2, ysize / 2), Size(radius, radius), 0, deg, deg + 38, Scalar(((j % 2) ? (ring_val[j / 2] & 1 << 7 - i) * 255 : !(ring_val[j / 2] & 1 << 7 - i) * 255), 0, 0), thinkness, CV_AA);
			deg += 45;
		}

		radius -= thinkness;
		deg = 0;
	}

	/* For Distance Ring */
	ellipse(FC, Point(xsize / 2, ysize / 2), Size(radius, radius), 0, 0, 360, Scalar(255, 0, 0), thinkness, CV_AA);
	radius -= thinkness;
	ellipse(FC, Point(xsize / 2, ysize / 2), Size(radius, radius), 0, 0, 360, Scalar(0, 0, 0), thinkness, CV_AA);
	radius -= thinkness;


	radius += thinkness / 2;
	ellipse(FC, Point(xsize / 2, ysize / 2), Size(radius, radius), 0, 0, 360, Scalar(255, 0, 0), -1, CV_AA);

	/* Rotate */
	warpAffine(FC, FC, (getRotationMatrix2D(Point2f(FC.cols / 2, FC.rows / 2), 19, 1)), Size(FC.cols, FC.rows));

	bitwise_not(FC, FC);
	conver_to_code_string(data, count, buffer);
	putText(FC, buffer, Point(xsize / 4, ysize - ysize / 20), FONT_HERSHEY_SIMPLEX, 5, (128, 0, 0), 5, LINE_AA);

	imwrite("C:\\Users\\Ali\\Desktop\\FCG.jpg", FC);
	//imshow("FC", FC);
	//waitKey(0);

	return 0;
}

int length(int a){
	int c = 0;
	while (a != 0){
		a /= 10;
		c++;
	}
	return c;
}

void conver_to_code_string(unsigned char data[], int count, char buffer[]){
	int index = 0;
	for (int i = 0; i < count; i++){
		std::sprintf(&buffer[index], "%d", data[i]);
		index += length(data[i]);
		if (i != count - 1)
			buffer[index] = '.';
		else
			buffer[index] = '\0';
		index++;
	}
}

