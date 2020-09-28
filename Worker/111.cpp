#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>  
#include  <stdlib.h>   
#define DLLEXPORT extern "C" __declspec(dllexport)

using namespace cv;

DLLEXPORT void show_image(int height, int width, uchar* data) {
	 Mat  src(height, width, CV_8UC3, data);
	//circle(src, Point(x * width, y * height), 30, Scalar(0, 0, 255), 1, 8, 0);
	imshow("video", src);
	waitKey(1);
} 