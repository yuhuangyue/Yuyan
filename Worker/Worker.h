#pragma once
#include <stdlib.h> 
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>  
#include <iostream>
#include <cstdio>
#include <vector>
#include <cmath>
#include <queue>
#include <stack>
//#include "marker_detect.h"
#include "putText.h"
using namespace std;
using namespace cv;
#define DLLEXPORT __declspec(dllexport)
#define pi 3.1415926



struct SimuScreen {  //逆时针  右下开始
	 Point3f p[4];
};

// 特征点
struct Nodes
{
	int id;
	float x;
	float y;
};


// 眼动指标
class GazeInfo {

public:
	float timestamp; // 时间戳
	Point2f gp2d; //位置坐标
	float gd; // 深度
	bool isFixed;
	float FixedDur;
	Point2f FixedArea;
	int FixedPoints;
	float FixedRate;
	bool isScan;
	float avgSpeed;
	float speedX, speedY;
	float movex1, movex2, movey1, movey2;
	float ScanRate;
	bool isblink;
	bool isblinkr, isblinkl;
	bool dblink;
	GazeInfo() {
		timestamp = 0;
		gp2d = Point2f(0, 0); FixedArea = Point2f(0, 0);
		gd = 0; FixedPoints = 0;
		isFixed = false; isScan = false;
		FixedDur = 0;
		FixedRate = 0; ScanRate = 0;
		avgSpeed = 0;
		speedX = 0; speedY = 0;
		movex1 = 0; movex2 = 0; movey1 = 0; movey2 = 0;
		isblink = false; isblinkr = false; isblinkl = false; dblink = false;
	}

	friend ostream &operator<<(ostream &output,
		const GazeInfo &D)
	{
		output << "timestamp: " << D.timestamp << " , GazePosition: " << D.gp2d.x << "," << D.gp2d.y << "," << D.gd << endl;
		output << "isFixed: " << D.isFixed << " , FixedDur: " << D.FixedDur << " , FixedPoints: " << D.FixedPoints << " , FixedArea: " << D.FixedArea.x << "," << D.FixedArea.y << endl;
		output << "isScan: " << D.isScan << " , AvgSpeed:" << D.avgSpeed << " , XYSpeed: " << D.speedX << "," << D.speedY << endl;
		output << "Moving: " << D.movex1 << "," << D.movex2 << "," << D.movey1 << "," << D.movey2 << endl;
		output << "FixedRate: " << D.FixedRate << " , ScanRate: " << D.ScanRate << endl;

		return output;
	}

};

class Worker
{
public:
	Worker();
	~Worker();
	// 初始数据加载
	void Load_Image(int height, int width, uchar * data);
	void Valid_Gaze_Data(float x, float y, float z);
	void Show_Image(string sName,  Mat Image);

	// 头部运动相关函数
	//void Move(float yaw, float roll, float pitch, float distance);
	vector<vector<float>> Matrix_multiply(vector<vector<float>> arrA, vector<vector<float>> arrB);
	SimuScreen InitScreen();
	SimuScreen Move( Mat rvec,  Mat tvec, SimuScreen initPos);
	Mat & Point2Mat(Point3f input);
	Point3f GetMovePoint( Point3f input,  Mat tvec);
	void Test( Mat rvec,  Mat tvec, SimuScreen initPos);

	// 用于绘制椭圆及其他图形
	bool GetAlphaValue( Point2f p, int a, int b); // a b分别是椭圆的长轴与短轴 (mm为单位)

	// marker
	void Get_Marker_Data(int n, Nodes * PointSet);
	void General_Computing(vector< Point3f> InitPoint, int flag, float step, vector< Point3f> & OutPoint);
	void Computing_3D_Coordinates();

	// 变量
	 Mat mFrame; //camera 960 * 540
	 Point2f mGaze_Point;
	int mHeightOfFrame; // 高
	int mWidthOfFrame;  // 宽
	double FOV; //视场角 
	float mHeightOfGlass; // 镜片（物理距离 mm为单位）
	float mDistanceOfGlass; // 镜片距离眼镜的位置（物理距离 mm为单位）
	SimuScreen mInitScreenPos;
	 Mat mSurfaceBest;

	// 特征点
	// 3D
	vector< Point3f> mPointSet[15];


	// 加载四张图片
	int mScreen_Width;
	int mScreen_Height;
	int mSimulation_Screen_Width;
	int mSimulation_Screen_Height;
	 Mat mBackup_Images[4];


	// 眼动指标
	queue<GazeInfo> infoq;
	vector<Point2f> gazes;
	int fps;
	int vec_idx;
	void Draw_position(float x, float y); // 显示坐标点
	void Draw_Deep(float x); // 深度信息
	vector<float> Speed(); // 计算速度（总速度、水平速度、垂直速度）
	vector<float> Movement();// 计算偏移量（四个方向）

	stack<Point2f> fixedq; // 计算注视的队列
	void UpdateFixed();
	float CalDistance(Point2f p1, Point2f p2);
	int fixedt;
	bool isFixed(); // 是否有注视行为
	vector<float> FixedDur();//注视时长（有注视行为就变色）
	Point2f FixedArea(); // 注视区域
	Point2f FixedPoint; // 注视中心点
	void GetFixedPoint(Point2f cur);
	float FixedDurRate();
	float fixedtime;
	float totaltime;
	float scantime;
	bool isScan();//是否扫视
	int scant;
	float ScanDurRate();

	// 界面字体相关
	int font_face = FONT_HERSHEY_COMPLEX;
	double font_scale = 2;
	int thickness = 2;
	int baseline;

	// 显示意图识别
	void isSearch();
	vector<GazeInfo> searchList;
	Point2f SearchValid();
	void DrawSearchArea();
	Rect searchArea;
	

};

extern "C" {
	DLLEXPORT Worker* Worker_new()
	{
		return new Worker();
	}
	DLLEXPORT void Worker_Load_Image(Worker* worker, int height, int width, uchar * data)
	{
		worker->Load_Image(height, width, data);
	}
	DLLEXPORT void Worker_Valid_Gaze_Data(Worker* worker, float x, float y, float z)
	{
		worker->Valid_Gaze_Data(x, y, z);
	}
	DLLEXPORT void Delete_Worker_new(Worker * worker)
	{
		if (worker == NULL) return;
		delete worker;
	}
	/*
	DLLEXPORT void Worker_Get_Marker_Data(Worker* worker, int n, Nodes * PointSet)
	{
		worker->Get_Marker_Data(n, PointSet);
	}*/
}