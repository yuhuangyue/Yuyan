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



struct SimuScreen {  //��ʱ��  ���¿�ʼ
	 Point3f p[4];
};

// ������
struct Nodes
{
	int id;
	float x;
	float y;
};


// �۶�ָ��
class GazeInfo {

public:
	float timestamp; // ʱ���
	Point2f gp2d; //λ������
	float gd; // ���
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
	// ��ʼ���ݼ���
	void Load_Image(int height, int width, uchar * data);
	void Valid_Gaze_Data(float x, float y, float z);
	void Show_Image(string sName,  Mat Image);

	// ͷ���˶���غ���
	//void Move(float yaw, float roll, float pitch, float distance);
	vector<vector<float>> Matrix_multiply(vector<vector<float>> arrA, vector<vector<float>> arrB);
	SimuScreen InitScreen();
	SimuScreen Move( Mat rvec,  Mat tvec, SimuScreen initPos);
	Mat & Point2Mat(Point3f input);
	Point3f GetMovePoint( Point3f input,  Mat tvec);
	void Test( Mat rvec,  Mat tvec, SimuScreen initPos);

	// ���ڻ�����Բ������ͼ��
	bool GetAlphaValue( Point2f p, int a, int b); // a b�ֱ�����Բ�ĳ�������� (mmΪ��λ)

	// marker
	void Get_Marker_Data(int n, Nodes * PointSet);
	void General_Computing(vector< Point3f> InitPoint, int flag, float step, vector< Point3f> & OutPoint);
	void Computing_3D_Coordinates();

	// ����
	 Mat mFrame; //camera 960 * 540
	 Point2f mGaze_Point;
	int mHeightOfFrame; // ��
	int mWidthOfFrame;  // ��
	double FOV; //�ӳ��� 
	float mHeightOfGlass; // ��Ƭ��������� mmΪ��λ��
	float mDistanceOfGlass; // ��Ƭ�����۾���λ�ã�������� mmΪ��λ��
	SimuScreen mInitScreenPos;
	 Mat mSurfaceBest;

	// ������
	// 3D
	vector< Point3f> mPointSet[15];


	// ��������ͼƬ
	int mScreen_Width;
	int mScreen_Height;
	int mSimulation_Screen_Width;
	int mSimulation_Screen_Height;
	 Mat mBackup_Images[4];


	// �۶�ָ��
	queue<GazeInfo> infoq;
	vector<Point2f> gazes;
	int fps;
	int vec_idx;
	void Draw_position(float x, float y); // ��ʾ�����
	void Draw_Deep(float x); // �����Ϣ
	vector<float> Speed(); // �����ٶȣ����ٶȡ�ˮƽ�ٶȡ���ֱ�ٶȣ�
	vector<float> Movement();// ����ƫ�������ĸ�����

	stack<Point2f> fixedq; // ����ע�ӵĶ���
	void UpdateFixed();
	float CalDistance(Point2f p1, Point2f p2);
	int fixedt;
	bool isFixed(); // �Ƿ���ע����Ϊ
	vector<float> FixedDur();//ע��ʱ������ע����Ϊ�ͱ�ɫ��
	Point2f FixedArea(); // ע������
	Point2f FixedPoint; // ע�����ĵ�
	void GetFixedPoint(Point2f cur);
	float FixedDurRate();
	float fixedtime;
	float totaltime;
	float scantime;
	bool isScan();//�Ƿ�ɨ��
	int scant;
	float ScanDurRate();

	// �����������
	int font_face = FONT_HERSHEY_COMPLEX;
	double font_scale = 2;
	int thickness = 2;
	int baseline;

	// ��ʾ��ͼʶ��
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