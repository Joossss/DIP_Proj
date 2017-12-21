// opencv_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include <cmath>
#include <iostream>
using namespace std;
using namespace cv;

struct FINGER {
	int x;
	int y;
	bool key_pushed;
	int ID;
};

void CallBackFunc(int event, int x, int y, int flags, void*userdata);
Vec3b NewThresholdsLow(Vec3b old_val, Vec3b new_val);
Vec3b NewThresholdsUp(Vec3b old_val, Vec3b new_val);
void FingerMovement(vector<FINGER> &old_fingers, vector<FINGER> new_fingers);
void draw_numbers(vector<FINGER> old_fingers, Mat &frame);

Mat frame;
Point pt;
uchar rclicked = 0, lclicked = 0, first_L = 1, first_R = 1;

//THRESHOLDING Values
uint8_t THRESH_HUE = 5;
uint8_t THRESH_SAT = 5;
uint8_t THRESH_VAL = 10;

int main()
{
	VideoCapture cap(0);
	
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	//Check if video device has been initialized
	if (!cap.isOpened())
		cout << "cannot open camera";

	//Declare needed variables

	//Create a window
	namedWindow("Stuff and stuff");

	//set the callback function for any mouse event
	setMouseCallback("Stuff and stuff", CallBackFunc, NULL);

	cap >> frame;
	cout << frame.size << endl;

	//Thresholding color for first run
	Vec3b lower_bound = (0, 0, 0);
	Vec3b upper_bound = (179, 255, 255);

	//Blob detector parameters
	SimpleBlobDetector::Params params;

	//Filter by area
	params.filterByArea = true;
	params.minArea = 1000;
	params.maxArea = 6000;

	//Filter by color
	params.filterByColor = true;
	params.blobColor = 255;

	//Not filtering by

	params.filterByCircularity = false;
	params.filterByConvexity = false;
	params.filterByInertia = false;

	//Fingers
	int num_fingers = 4;
	vector<FINGER> new_fingers(num_fingers);
	vector<FINGER> old_fingers(num_fingers);

	//For blurring
	int blurSize = 5;

	RNG rng(12345);

	while (1)
	{
		cap >> frame;
		cvtColor(frame, frame, CV_BGR2RGBA);
		cvtColor(frame, frame, CV_RGBA2RGB);
		cvtColor(frame, frame, CV_RGB2HSV);

		Mat out;

		inRange(frame, lower_bound, upper_bound, out);


		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		medianBlur(out, out, blurSize);

		/// Detect edges using canny
		//Canny(out, out, 100, 200, 3);
		/// Find contours
		findContours(out, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);


		sort(contours.begin(), contours.end(), [](const vector<Point>& c1, const vector<Point>& c2) {
			return contourArea(c1, false) < contourArea(c2, false);
		});
		Mat drawing = Mat::zeros(out.size(), CV_8UC3);

		for (int i = 0; i < contours.size(); i++) {
			drawContours(drawing, contours, i, Scalar(255, 255, 255), 2, 8);
		}
		vector<vector<Point>> big_cont;
		num_fingers = 4;
		for (int i = 0; i < contours.size(); i++) {
			if(contourArea(contours[contours.size()-i-1]) > 100)
				big_cont.push_back(contours[contours.size()-i-1]);
		}
		contours = big_cont;
		num_fingers = MIN(num_fingers, contours.size());
		/// Draw contours
		for (int i = 0; i < num_fingers; i++) {
			drawContours(drawing, contours, i, Scalar(255,0, 0 ), 2, 8);
		}
		
		
		//Mouseclick routine
		if (lclicked) {
			Vec3b new_color = frame.at<Vec3b>(pt.y, pt.x);
			cout << new_color << endl;

			//If it's the first mouseclick of the session
			if (first_L) {
				lower_bound[0] = uint8_t(abs(new_color[0] - THRESH_HUE));
				lower_bound[1] = uint8_t(abs(new_color[1] - THRESH_SAT));
				lower_bound[2] = uint8_t(abs(new_color[2] - THRESH_VAL));
				if (new_color[0] + THRESH_HUE < 180) {
					upper_bound[0] = uint8_t(new_color[0] + THRESH_HUE);
				}
				else {
					upper_bound[0] = 179;
				}
				if (new_color[1] + THRESH_SAT < 256) {
					upper_bound[1] = uint8_t(new_color[1] + THRESH_SAT);
				}
				else {
					upper_bound[1] = 255;
				}
				if (new_color[2] + THRESH_VAL < 256) {
					upper_bound[2] = uint8_t(new_color[2] + THRESH_VAL);
				}
				else {
					upper_bound[2] = 255;
				}
				first_L = 0;
			}
			else {
				lower_bound = NewThresholdsLow(lower_bound, new_color);
				upper_bound = NewThresholdsUp(upper_bound, new_color);
			}
			cout << "Lower bound: " << lower_bound << ", upper bound: " << upper_bound << endl;
			lclicked = 0;
		}
		if (rclicked) {
			if (first_R) {
				for (int i = 0; i < contours.size(); i++) {
					Moments M = moments(contours[i], false);
					Point P = Point(round(M.m10 / M.m00), round(M.m01 / M.m00));
					old_fingers[i].x = P.x;
					old_fingers[i].y = P.y;
					old_fingers[i].key_pushed = false;
					old_fingers[i].ID = i;
				}
				first_R = 0;
			}
			else {
				for (int i = 0; i < contours.size(); i++) {
					Moments M = moments(contours[i], false);
					Point P = Point(round(M.m10 / M.m00), round(M.m01 / M.m00));
					new_fingers[i].x = P.x;
					new_fingers[i].y = P.y;
					new_fingers[i].key_pushed = false;
				}
				FingerMovement(old_fingers, new_fingers);
			}
			draw_numbers(old_fingers, drawing);
		}

		cv::imshow("Stuff and stuff", frame);
		cv::imshow("threshed", drawing);

		if (waitKey(10) >= 0)
			break;
	}
	return 0;
}

//Mouseclick callback function
void CallBackFunc(int event, int x, int y, int flags, void*userdata) {
	pt = Point(x, y);
	switch(event) {
		case EVENT_LBUTTONDOWN: cout << "Lb clicked - pos (" << pt.x << " , " << pt.y << ")" << endl;
			lclicked = 1;
			break;
		case EVENT_RBUTTONDOWN: cout << "Rb clicked - pos (" << pt.x << " , " << pt.y << ")" << endl;
			rclicked = 1;
			break;
		//case EVENT_MOUSEMOVE: cout << "Mouse move over the window - pos (" << pt.x << " , " << pt.y << ")" << endl;
			break;
	}

}

//Finding the new lower bound for inrange masking
Vec3b NewThresholdsLow(Vec3b old_val, Vec3b new_val) {
	Vec3b low_bound = (0,0,0);
	if(new_val[0] < (old_val[0] + THRESH_HUE)) {
		if (new_val[0] - THRESH_HUE > 0) {
			low_bound[0] = uint8_t(new_val[0] - THRESH_HUE);
		}
		else {
			low_bound[0] = 0;
		}
	}
	else {
		low_bound[0] = old_val[0];
	}
	if (new_val[1] < (old_val[1] + THRESH_SAT)) {
		if (new_val[1] - THRESH_SAT > 0) {
			low_bound[1] = uint8_t(new_val[1] - THRESH_SAT);
		}
		else {
			low_bound[1] = 0;
		}
	}
	else {
		low_bound[1] = old_val[1];
	}
	if (new_val[2] < (old_val[2] + THRESH_VAL)) {
		if (new_val[2] - THRESH_VAL > 0) {
			low_bound[2] = uint8_t(new_val[2] - THRESH_VAL);
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

//Finding the new upperbound for inrange masking
Vec3b NewThresholdsUp(Vec3b old_val, Vec3b new_val) {

	Vec3b upper_bound = (0, 0, 0);
	if (new_val[0] > (old_val[0] - THRESH_HUE)) {
		if ((new_val[0] + THRESH_HUE) < 180) {
			upper_bound[0] = uint8_t(new_val[0] + THRESH_HUE);
		}
		else {
			upper_bound[0] = 179;
		}
	}
	else {
		upper_bound[0] = old_val[0];
	}
	if (new_val[1] > (old_val[1] - THRESH_SAT)) {
		if ((new_val[1] + THRESH_SAT) < 256) {
			upper_bound[1] = uint8_t(new_val[1] + THRESH_SAT);
		}
		else {
			upper_bound[1] = 255;
		}
	}
	else {
		upper_bound[1] = old_val[1];
	}
	if (new_val[2] > (old_val[2] - THRESH_VAL)) {
		if ((new_val[2] + THRESH_VAL) < 256) {
			upper_bound[2] = uint8_t(new_val[2] + THRESH_VAL);
		}
		else {
			upper_bound[2] = 255;
		}
	}
	else {
		upper_bound[2] = old_val[2];
	}
	return upper_bound;
}

void FingerMovement(vector<FINGER> &old_fingers, vector<FINGER>new_fingers) {
	for (int i = 0; i < old_fingers.size(); i++) {
		FINGER holder = new_fingers[0];
		int chosen = 0;
		for (int j = 0; j < new_fingers.size(); j++) {
			cout << "i: " << i << endl;
			if (abs(new_fingers[j].x) < 1280 && abs(new_fingers[j].y) < 720) {
				float dxn = pow((new_fingers[j].x - old_fingers[j].x), 2);
				float dyn = pow((new_fingers[j].y - old_fingers[j].y), 2);
				float dxh = pow((holder.x - old_fingers[j].x), 2);
				float dyh = pow((holder.y - old_fingers[j].y), 2);
				if ((dxn + dyn) < (dxh + dyh)) {
					holder = new_fingers[j];
					chosen = j;
				}
			}
		}
		old_fingers[i].x = holder.x;
		old_fingers[i].y = holder.y;
		new_fingers.erase(new_fingers.begin() + chosen);
	}
}

void draw_numbers(vector<FINGER> old_fingers, Mat &frame) {
	for (int i = 0; i < old_fingers.size(); i++) {
		putText(frame, to_string(old_fingers[i].ID), cvPoint(old_fingers[i].x, old_fingers[i].y),
			FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255, 0 ,0), 1, CV_AA);
		cout << "ID: " << old_fingers[i].ID << ", x: " << old_fingers[i].x << ", y:" << old_fingers[i].y << endl;

	}
	//return frame;
}


