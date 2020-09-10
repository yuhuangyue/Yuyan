#include "marker_detect.h"

namespace h1114 {
	void Marker_Detect( Mat frame,  Point2f markers[])
	{
		 Mat gray_img;
		cvtColor(frame, gray_img, CV_BGR2GRAY);
		
	}
}