// opencv_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <cmath>
#include <iostream>
using namespace std;
using namespace cv;

void CallBackFunc(int event, int x, int y, int flags, void*userdata);
Vec3b NewThresholdsLow(Vec3b old_val, Vec3b new_val);
Vec3b NewThresholdsUp(Vec3b old_val, Vec3b new_val);

Mat frame;
Point pt;
uchar clicked = 0, first = 1;

//THRESHOLDING Values
int THRESH_HUE = 5;
int THRESH_SAT = 5;
int THRESH_VAL = 10;

int main()
{
	VideoCapture cap(1);
	//Check if video device has been initialized
	if (!cap.isOpened())
		cout << "cannot open camera";

	//Declare needed variables
	
	//Create a window
	namedWindow("Stuff and stuff");

	//set the callback function for any mouse event
	setMouseCallback("Stuff and stuff", CallBackFunc, NULL);

	cap >> frame;
	uchar* camData = new uchar[frame.total() * 4];
	Mat frame_RGBA(frame.size(), CV_8UC4, camData);
	Mat frame_hsv(frame.size(), CV_8UC3, Scalar(0, 0, 0));


	//Colors
	Vec3b WHITE_HSV = (0, 0, 255);
	Vec3b WHITE_BGR = (255, 255, 255);



	//Thresholding color for first run
	Vec3b lower_bound = (0,0,0), upper_bound = (179, 255, 255);/
	while (1)
	{	
		cap >> frame;
		cvtColor(frame, frame_RGBA, CV_BGR2RGBA);
		cvtColor(frame_RGBA, frame_hsv, CV_RGBA2RGB);
		cvtColor(frame_RGBA, frame_hsv, CV_RGB2HSV);
		Mat frame_black(frame.size(), CV_8UC3, Scalar(0, 0, 0));
		if (clicked) {
			Vec3b new_color = frame_hsv.at<Vec3b>(pt.y, pt.x);
			clicked = 0;
			if (first) {
				lower_bound = (abs(int(new_color[0]) - THRESH_HUE), abs(int(new_color[1]) - THRESH_SAT), abs(int(new_color[2]) - THRESH_VAL));
				if (new_color[0] + THRESH_HUE < 180) {
					upper_bound[0] = new_color[0] + THRESH_HUE;
				}
				else {
					upper_bound[0] = 179;
				}
				if (new_color[1] + THRESH_SAT < 256) {
					upper_bound[1] = new_color[1] + THRESH_SAT;
				}
				else {
					upper_bound[1] = 255;
				}
				if (new_color[2] + THRESH_VAL < 256) {
					upper_bound[2] = new_color[2] + THRESH_VAL;
				}
				else {
					upper_bound[2] = 255;
				}
				first = 0;
			}
			else {
				lower_bound = NewThresholdsLow(lower_bound, new_color);
				upper_bound = NewThresholdsUp(upper_bound, new_color);
			}
		}
		Mat out;
		inRange(frame_hsv, lower_bound, upper_bound, out);
		Mat conc(frame.rows, frame.cols * 2, CV_8UC3, Scalar(0, 0, 0));

		//hconcat(frame, out, conc);
		imshow("Stuff and stuff", frame);
		imshow("threshed", out);

		if (waitKey(30) >= 0)
			break;
	}
	return 0;
}

void CallBackFunc(int event, int x, int y, int flags, void*userdata) {
	pt = Point(x, y);
	switch(event) {
		case EVENT_LBUTTONDOWN: cout << "Lb clicked - pos (" << pt.x << " , " << pt.y << ")" << endl;
			clicked = 1;
			break;
		/*case EVENT_RBUTTONDOWN: cout << "Rb clicked - pos (" << pt.x << " , " << pt.y << ")" << endl;
			break;
		case EVENT_MOUSEMOVE: cout << "Mouse move over the window - pos (" << pt.x << " , " << pt.y << ")" << endl;
			break;*/
	}

}

Vec3b NewThresholdsLow(Vec3b old_val, Vec3b new_val) {
	Vec3b low_bound = (0,0,0);
	if((abs(int(old_val[0]) - int(new_val[0]) < THRESH_HUE)) || (int(new_val[0]) - int(old_val[0]) < 0)) {
		if (int(new_val[0]) - THRESH_HUE > 0) {
			low_bound[0] = int(new_val[0]) - THRESH_HUE;
		}
		else {
			low_bound[0] = 0;
		}
	}
	else {
		low_bound[0] = old_val[0];
	}
	if((abs(int(old_val[1]) - int(new_val[1]) < THRESH_SAT)) || (int(new_val[1]) - int(old_val[1]) < 0)) {
		if (int(new_val[1]) - THRESH_SAT > 0) {
			low_bound[1] = int(new_val[1]) - THRESH_SAT;
		}
		else {
			low_bound[1] = 0;
		}
	}
	else {
		low_bound[1] = old_val[1];
	}
	if((abs(int(old_val[2]) - int(new_val[2]) < THRESH_VAL)) || (int(new_val[2]) - int(old_val[2]) < 0)) {
		if (int(new_val[2]) - THRESH_VAL > 0) {
			low_bound[2] = int(new_val[2]) - THRESH_HUE;
		}
		else {
			low_bound[2] = 0;
		}
	}
	else {
		low_bound[2] = old_val[2];
	}

return low_bound;
}

Vec3b NewThresholdsUp(Vec3b old_val, Vec3b new_val) {

	Vec3b upper_bound = (0, 0, 0);
	if ((abs(int(old_val[0]) - int(new_val[0]) < THRESH_HUE)) || (int(new_val[0]) - int(old_val[0]) > 0)) {
		if (int(new_val[0]) + THRESH_HUE < 180) {
			upper_bound[0] = int(new_val[0]) + THRESH_HUE;
		}
		else {
			upper_bound[0] = 179;
		}
	}
	else {
		upper_bound[0] = old_val[0];
	}
	if ((abs(int(old_val[1]) - int(new_val[1]) < THRESH_SAT)) || (int(new_val[1]) - int(old_val[1]) > 0)) {
		if (int(new_val[1]) + THRESH_SAT < 256) {
			upper_bound[1] = int(new_val[1]) + THRESH_SAT;
		}
		else {
			upper_bound[1] = 255;
		}
	}
	else {
		upper_bound[1] = old_val[1];
	}
	if ((abs(int(old_val[2]) - int(new_val[2]) < THRESH_VAL)) || (int(new_val[2]) - int(old_val[2]) > 0)) {
		if (int(new_val[2]) + THRESH_VAL < 256) {
			upper_bound[2] = int(new_val[2]) + THRESH_VAL;
		}
		else {
			upper_bound[2] = 256;
		}
	}
	else {
		upper_bound[2] = old_val[2];
	}
	return upper_bound;
}
