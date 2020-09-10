#pragma once
#ifndef Marker_Detect_h
#define Marker_Detect_h
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>  
using namespace cv;
namespace h1114 {
	void Marker_Detect( Mat frame,  Point2f markers[]);

}

#endif // !Marker_Detect_h
