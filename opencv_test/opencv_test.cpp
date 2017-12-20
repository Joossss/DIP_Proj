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
	float x;
	float y;
	bool key_pushed;
};

void CallBackFunc(int event, int x, int y, int flags, void*userdata);
Vec3b NewThresholdsLow(Vec3b old_val, Vec3b new_val);
Vec3b NewThresholdsUp(Vec3b old_val, Vec3b new_val);
void FingerMovement(FINGER * old_fingers, FINGER * new_fingers, int size);
Mat draw_numbers(FINGER * old_fingers, Mat frame, int size);

Mat frame;
Point pt;
uchar rclicked = 0, lclicked = 0, first_L = 1, first_R = 1;

//THRESHOLDING Values
uint8_t THRESH_HUE = 5;
uint8_t THRESH_SAT = 5;
uint8_t THRESH_VAL = 30;

int main()
{
	VideoCapture cap(0);
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

	//Thresholding color for first run
	Vec3b lower_bound = (0, 0, 0);
	Vec3b upper_bound = (179, 255, 255);

	//Blob detector parameters
	SimpleBlobDetector::Params params;

	//Filter by area
	params.filterByArea = true;
	params.minArea = 70;

	//Filter by color
	params.filterByColor = true;
	params.blobColor = 255;

	//Not filtering by
	params.filterByArea = false;
	params.filterByCircularity = false;
	params.filterByConvexity = false;
	params.filterByInertia = false;

	//Fingers
	FINGER new_fingers[5];
	FINGER old_fingers[5];

	while (1)
	{	
		cap >> frame;
		GaussianBlur(frame, frame, Size(11,11), 0);
		cvtColor(frame, frame_RGBA, CV_BGR2RGBA);
		cvtColor(frame_RGBA, frame_hsv, CV_RGBA2RGB);
		cvtColor(frame_RGBA, frame_hsv, CV_RGB2HSV);
		Mat frame_black(frame.size(), CV_8UC3, Scalar(0, 0, 0));
		Mat out;

		inRange(frame_hsv, lower_bound, upper_bound, out);

		//Detect blobs.
		vector<KeyPoint> keypoints;
		Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);
		detector->detect(out, keypoints);

		//Draw detected blobs as red circles
		//DrawMatchFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the
		//corresponds to the size of the blob
		Mat out_with_keypoints;
		drawKeypoints(out, keypoints, out_with_keypoints, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

		//Mouseclick routine
		if (lclicked) {
			Vec3b new_color = frame_hsv.at<Vec3b>(pt.y, pt.x);
			cout << new_color << endl;

			//If it's the first mouseclick of the session
			if (first_L) {
				lower_bound[0] = uint8_t(abs(new_color[0] - THRESH_HUE));
				lower_bound[1] = uint8_t(abs(new_color[1] - THRESH_SAT));
				lower_bound[2] = uint8_t(abs(new_color[2] - THRESH_VAL));
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
				for (int i = 0; i < keypoints.size(); i++) {
					old_fingers[i].x = keypoints[i].pt.x;
					old_fingers[i].y = keypoints[i].pt.y;
					old_fingers[i].key_pushed = false;
				}
				first_R = 0;
			}
			else {
				for (int i = 0; i < keypoints.size(); i++) {
					new_fingers[i].x = keypoints[i].pt.x;
					new_fingers[i].y = keypoints[i].pt.y;
					new_fingers[i].key_pushed = false;
					FingerMovement(old_fingers, new_fingers, 5);
					out_with_keypoints = draw_numbers(old_fingers, out_with_keypoints, 5);
					
				}
			}
		}

		//hconcat(frame, out, conc);
		imshow("Stuff and stuff", frame);
		imshow("threshed", out_with_keypoints);

		if (waitKey(30) >= 0)
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
			upper_bound[0] = uint8_t(new_val[2] + THRESH_VAL);
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

void FingerMovement(FINGER * old_fingers, FINGER * new_fingers, int size) {
	for (int i = 0; i < size; i++) {
		FINGER holder;
		for (int j = 0; j < size; j++) {
			if (j == 0)
				holder = *new_fingers;
			else {
				int dxn = pow((new_fingers->x - old_fingers->x),2);
				int dyn = pow((new_fingers->y - old_fingers->y),2);
				int dxh = pow((holder.x - old_fingers->x), 2);
				int dyh = pow((holder.y - old_fingers->y), 2);
				if ((dxn + dyn) < (dxh - dyh))
					holder = *new_fingers;
			}
			*old_fingers = holder;
			new_fingers++;
		}
		old_fingers++;
	}
	return;
}

Mat draw_numbers(FINGER * old_fingers, Mat frame, int size) {
	for (int i = 0; i < size; i++) {
		putText(frame, to_string(i), cvPoint(old_fingers->x, old_fingers->y),
			FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255, 0 ,0), 1, CV_AA);
		old_fingers++;
	}
	return frame;
}


