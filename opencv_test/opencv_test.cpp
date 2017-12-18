// opencv_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
using namespace std;
using namespace cv;

void CallBackFunc(int event, int x, int y, int flags, void*userdata);

Mat frame;
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

	uchar* camData = new uchar[frame.total() * 4];
	Mat frame_RGBA(frame.size(), CV_8UC4, camData);
	Mat frame_hsv(frame.size(), CV_8UC3, Scalar(0, 0, 0));


	//split the channels in order  to manipulate them
	/*split(continuousRGBA, chans);
	chans[3] = Mat::zeros(continuousRGBA.rows, continuousRGBA.cols, CV_8UC1);

	//merge the channels back
	merge(chans, 4, continuousRGBA);*/
	while (1)
	{	
		cap >> frame;
		cvtColor(frame, frame_RGBA, CV_BGR2RGBA);
		cvtColor(frame_RGBA, frame_hsv, CV_RGBA2RGB);
		cvtColor(frame_RGBA, frame_hsv, CV_RGB2HSV);

		Mat conc(frame.rows, frame.cols * 2, CV_8UC3, Scalar(0, 0, 0));
		hconcat(frame, frame_hsv, conc);

		imshow("Stuff and stuff", conc);

		if (waitKey(30) >= 0)
			break;
	}
	return 0;
}

void CallBackFunc(int event, int x, int y, int flags, void*userdata) {
	Point pt = Point(x, y);
	switch(event) {
		case EVENT_LBUTTONDOWN: cout << "Lb clicked - pos (" << pt.x << " , " << pt.y << ")" << endl;
			cout << "The color is " << int(frame.at<uchar>(y, x))<< endl;
			break;
		case EVENT_RBUTTONDOWN: cout << "Rb clicked - pos (" << pt.x << " , " << pt.y << ")" << endl;
			break;
		case EVENT_MOUSEMOVE: cout << "Mouse move over the window - pos (" << pt.x << " , " << pt.y << ")" << endl;
			break;
	}

}
