#include "Worker.h"


Worker::Worker()
{
	// 加载和显示背景图
	//mSurfaceBest = imread("D:\\5Yuyan\\map.jpg");
	string name = "surface_best";
	namedWindow(name,  WINDOW_NORMAL);
	 //setWindowProperty(name, WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
	 //Show_Image(name, mSurfaceBest);
	 //imshow(name, mSurfaceBest);

	 
	// 定义视场角
	FOV = pi / 8; // 约45°视场角

	mDistanceOfGlass = 50; //约为7mm
	mHeightOfGlass = 10; //约为30mm

	// 生成marker 3D坐标
	//Computing_3D_Coordinates();

	// 定义一些常量
	fps = 30;
	vec_idx = 0;
	fixedt = 100;
	font_face = FONT_HERSHEY_COMPLEX;
	font_scale = 2;
	thickness = 2;
	fixedtime = 0;
	totaltime = 0;
	scantime = 0;
	scant = 200;
	//searchArea = new Rect(0, 0, 0, 0);
	mInitScreenPos = InitScreen();
}

Worker::~Worker()
{
}

SimuScreen Worker::InitScreen() { // 计算相机初始位置 
	// p1 (h/2 , -y ,distance)
	// p2 (h/2, y , distance)
	float cos_FOV = cos(FOV);
	float c = pow(mHeightOfGlass, 2) / 4 + pow(mDistanceOfGlass, 2);
	//cout << c << endl;
	float x = sqrt((1 - cos_FOV)* c / (cos_FOV + 1));
	SimuScreen initScreenPos;
	initScreenPos.p[0] = { x, -mHeightOfGlass / 2, mDistanceOfGlass };
	initScreenPos.p[1] = { x, mHeightOfGlass / 2, mDistanceOfGlass };
	initScreenPos.p[2] = { -x,mHeightOfGlass / 2, mDistanceOfGlass };
	initScreenPos.p[3] = { -x, -mHeightOfGlass / 2, mDistanceOfGlass };
	//cout << "Init done" << endl;

	return initScreenPos;
}


void Worker::Load_Image(int height, int width, uchar * data)
{
	Mat src(height, width, CV_8UC3, data); // 记录原始图片
	mFrame = src;
	mHeightOfFrame = height; 
	mWidthOfFrame = width;  // 照片已经缩小为原先的 1 / 4
	//cout << "Load_Image done" << endl;
	
}

void Worker::Valid_Gaze_Data(float x, float y, float z)
{

	GazeInfo info;//构造所有数据

	mGaze_Point.x = x * mWidthOfFrame;
	mGaze_Point.y = y * mHeightOfFrame;
	info.gp2d = mGaze_Point;
	info.gd = z;

	totaltime = totaltime + 1 * 1.0 / fps * 1.0;
	info.timestamp = totaltime;

	if (gazes.size() < fps) {
		gazes.push_back(mGaze_Point);
	}
	else {
		gazes[vec_idx] = mGaze_Point;
		vec_idx++;
		vec_idx = vec_idx % fps;
	}
	
	fixedq.push(mGaze_Point);
	

	Mat temp =  Mat::zeros(600, 1280, CV_8UC3);


	// 绘制眼动指标
	Draw_position(mGaze_Point.x, mGaze_Point.y);

	// 判断是否注视行为
	UpdateFixed();
	info.isFixed = isFixed();
	// 计算注视时长和注视点
	vector<float> fixans(2);
	fixans = FixedDur();
	info.FixedDur = fixans[0];
	info.FixedPoints = fixans[1];

	// 计算注视区域
	info.FixedArea = FixedArea();
	// 注视时长占比
	info.FixedRate = FixedDurRate();

	// 是否扫视
	info.isScan = isScan();
	// 扫视时长占比
	info.ScanRate = ScanDurRate();

	// 绘制深度信息
	Draw_Deep(z);
	// 计算速度（平均速度，水平速度，垂直速度）
	vector<float> speeds(3);
	speeds = Speed();
	info.avgSpeed = speeds[0];
	info.speedX = speeds[1];
	info.speedY = speeds[2];

	// 计算偏移量
	vector<float> moves(4);
	moves = Movement();
	info.movex1 = moves[0];
	info.movex2 = moves[1];
	info.movey1 = moves[2];
	info.movey2 = moves[3];

	cout << info << endl;
	if (infoq.size() < fps) {
		infoq.push(info);
	}
	else {
		infoq.pop();
		infoq.push(info);
	}

	// 意图识别important!
	isSearch();
	DrawSearchArea();

	// 绘制眼动坐标点
	circle(mFrame,  Point(mGaze_Point.x, mGaze_Point.y), 28,  Scalar(255, 255, 255), 5, 8, 0); // 标记视线方向
	resize(mFrame, temp, temp.size(), 0, 0,  INTER_CUBIC);
	//cout << "show me " << mGaze_Point.x << " " << mGaze_Point.y << endl;
	
	Show_Image("test", temp);



	/*
	// marker_detect
	Point2f marker[4];
	h1114::Marker_Detect(mFrame, marker);*/
	
}

void Worker::Show_Image(string sName,  Mat Image) // show 图片
{
	 imshow(sName, Image);
	 waitKey(1);
}


void Worker::Draw_position(float x, float y) {

	stringstream streamx;
	stringstream streamy;
	streamx << fixed << setprecision(2) << x;
	streamy << fixed << setprecision(2) << y;

	string position_str = "视线坐标：" + streamx.str() + " , " + streamy.str();
	Size text_size =  getTextSize(position_str, font_face, font_scale, thickness, &baseline);

	Point origin;
	origin.x = mFrame.cols / 2 - text_size.width / 2;
	origin.y = mFrame.rows / 2 + text_size.height / 2;
	//putText(mFrame, position_str, origin, font_face, font_scale,  Scalar(0, 255, 255), thickness, 8, 0);
	char* str_input = (char*)position_str.c_str();
	putTextZH(mFrame, str_input, Point(50,50),  Scalar(255, 255, 255), 30, "Arial");
}

void Worker::Draw_Deep(float z) {

	stringstream streamz;
	streamz << fixed << setprecision(2) << z;

	string deep_str = "视线深度：" + streamz.str();
	Size text_size = getTextSize(deep_str, font_face, font_scale, thickness, &baseline);

	//putText(mFrame, position_str, origin, font_face, font_scale,  Scalar(0, 255, 255), thickness, 8, 0);
	char* str_input = (char*)deep_str.c_str();
	putTextZH(mFrame, str_input, Point(50, 80), Scalar(255, 255, 255), 30, "Arial");

}

vector<float> Worker::Speed() {
	

	vector<float> output(3);

	int len = gazes.size();
	if (len != fps) { return output; }

	float speed_x = 0, speed_y = 0;
	int cnt = vec_idx;
	int nums = len - 1;
	while (nums) {
		if (cnt == len) {
			speed_x = speed_x + abs(gazes[0].x - gazes[cnt - 1].x);
			speed_y = speed_y + abs(gazes[0].y - gazes[cnt - 1].y);
			cnt = 1;
			nums--;
		}
		else if (cnt==0) {
			cnt++;
		}
		else {
			speed_x = speed_x + abs(gazes[cnt].x - gazes[cnt - 1].x);
			speed_y = speed_y + abs(gazes[cnt].y - gazes[cnt - 1].y);
			cnt++;
			nums--;
		}
	}

	speed_x = speed_x / len;
	speed_y = speed_y / len;
	output[1] = speed_x;
	output[2] = speed_y;

	// 平均速度
	float speed_avg = sqrt(speed_x* speed_x + speed_y * speed_y);
	output[0] = speed_avg;
	stringstream stream_avg;
	stream_avg << fixed << setprecision(2) << speed_avg;
	string avg_speed = "平均速度：" + stream_avg.str();

	Size text_size = getTextSize(avg_speed, font_face, font_scale, thickness, &baseline);
	char* str_input = (char*)avg_speed.c_str();
	putTextZH(mFrame, str_input, Point(50, 110), Scalar(255, 255, 255), 30, "Arial");


	// x方向速度
	stringstream stream_x;
	stream_x << fixed << setprecision(2) << speed_x;
	avg_speed = "水平速度：" + stream_x.str();
	text_size = getTextSize(avg_speed, font_face, font_scale, thickness, &baseline);
	str_input = (char*)avg_speed.c_str();
	putTextZH(mFrame, str_input, Point(50, 140), Scalar(255, 255, 255), 30, "Arial");


	// y方向速度
	stringstream stream_y;
	stream_y << fixed << setprecision(2) << speed_y;
	avg_speed = "垂直速度：" + stream_y.str();
	text_size = getTextSize(avg_speed, font_face, font_scale, thickness, &baseline);
	str_input = (char*)avg_speed.c_str();
	putTextZH(mFrame, str_input, Point(50, 170), Scalar(255, 255, 255), 30, "Arial");
	
	return output;
}

vector<float> Worker::Movement() {

	vector<float> output(4);

	int len = gazes.size();
	if (len != fps) { return output; }
	int cur_idx = vec_idx == 0 ? fps - 1 : vec_idx - 1;
	
	Point2f cur = gazes[cur_idx];
	Point2f pre = gazes[cur_idx == 0 ? fps - 1 : cur_idx - 1];

	float movex = cur.x - pre.x;
	float movey = cur.y - pre.y;


	// x方向偏移
	stringstream stream_x;
	string move1,move2;
	if (movex >= 0) {
		stream_x << fixed << setprecision(2) << movex;
		move1 = "向右偏移：" + stream_x.str();
		move2 = "向左偏移：0.00";
		output[0] = movex;
		output[1] = 0;
	}
	else {
		stream_x << fixed << setprecision(2) << (-movex);
		move1 = "向右偏移：0.00";
		move2 = "向左偏移：" + stream_x.str();
		output[1] = -movex;
		output[0] = 0;
	}
	Size text_size = getTextSize(move1, font_face, font_scale, thickness, &baseline);
	char* str_input1 = (char*)move1.c_str();
	char* str_input2 = (char*)move2.c_str();
	putTextZH(mFrame, str_input1, Point(50, 200), Scalar(255, 255, 255), 30, "Arial");
	putTextZH(mFrame, str_input2, Point(50, 230), Scalar(255, 255, 255), 30, "Arial");

	// y方向偏移
	stringstream stream_y;
	if (movey >= 0) {
		stream_y << fixed << setprecision(2) << movey;
		move1 = "向上偏移：" + stream_y.str();
		move2 = "向下偏移：0.00";
		output[2] = movey;
		output[3] = 0;
	}
	else {
		stream_y << fixed << setprecision(2) << (-movey);
		move1 = "向上偏移：0.00";
		move2 = "向下偏移：" + stream_y.str();
		output[3] = -movey;
		output[2] = 0;
	}
	text_size = getTextSize(move1, font_face, font_scale, thickness, &baseline);
	str_input1 = (char*)move1.c_str();
	str_input2 = (char*)move2.c_str();
	putTextZH(mFrame, str_input1, Point(50, 260), Scalar(255, 255, 255), 30, "Arial");
	putTextZH(mFrame, str_input2, Point(50, 290), Scalar(255, 255, 255), 30, "Arial");

	return output;
}

float Worker::CalDistance(Point2f p1, Point2f p2) {

	return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));

}


void Worker::GetFixedPoint(Point2f cur) {
	int s = fixedq.size() - 1;
	FixedPoint.x = (FixedPoint.x*s + cur.x) / (s + 1);
	FixedPoint.y = (FixedPoint.y*s + cur.y) / (s + 1);
}

void Worker::UpdateFixed() {

	if (fixedq.size() == 1) { FixedPoint = fixedq.top(); }

	if (fixedq.size() < 2) { return; }

	Point2f cur = fixedq.top();
	fixedq.pop();
	Point2f pre = fixedq.top();
	float dis = CalDistance(cur, pre);
	//cout << "distance :" << dis << " size:"<< fixedq.size()<<endl;
	if (dis < fixedt) {
		fixedq.push(cur);
		GetFixedPoint(cur);
	}
	else {
		float time = fixedq.size() / fps;
		if (time > 3) {
			fixedtime = fixedtime + time;
		}
		while (!fixedq.empty()) { fixedq.pop(); }
		FixedPoint = Point2f(-1, -1);
	}
}


bool Worker::isFixed() {
	
	// 是否存在注视行为
	string isfixed;
	bool output;
	if (fixedq.size() > fps * 3) {
		isfixed = "是否注视：True";
		output = true;
	}
	else {
		isfixed = "是否注视：False";
		output = false;
	}
	Size text_size = getTextSize(isfixed, font_face, font_scale, thickness, &baseline);
	char* str_input = (char*)isfixed.c_str();
	putTextZH(mFrame, str_input, Point(50, 320), Scalar(255, 255, 255), 30, "Arial");

	return output;
}


vector<float> Worker::FixedDur() {


	vector<float> output(2);

	// 注视时长 以及 注视点个数
	float duration = fixedq.size()*1.0 / fps;
	int fixedpoints = fixedq.size();
	output[0] = duration;
	output[1] = fixedpoints;
	stringstream stream_d;
	stream_d << fixed << setprecision(2) << duration;
	string str_d = "注视时长："+stream_d.str();
	string str_p = "注视点数："+to_string(fixedpoints);
	Size text_size = getTextSize(str_p, font_face, font_scale, thickness, &baseline);
	char* str_input = (char*)str_d.c_str();
	char* str_input_points = (char*)str_p.c_str();

	if (duration > 3) {
		putTextZH(mFrame, str_input, Point(50, 350), Scalar(255, 255, 255), 30, "Arial");
		putTextZH(mFrame, str_input_points, Point(50, 380), Scalar(255, 255, 255), 30, "Arial");
	}
	else {
		putTextZH(mFrame, str_input, Point(50, 350), Scalar(0, 255, 255), 30, "Arial");
		putTextZH(mFrame, str_input_points, Point(50, 380), Scalar(0, 255, 255), 30, "Arial");
	}

	return output;
}


Point2f Worker::FixedArea() {

	Point2f output(0, 0);

	string str_a;
	if (fixedq.size() < fps * 3) {
		str_a = "注视区域：无注视行为";
	}
	else {
		output = FixedPoint;
		circle(mFrame, FixedPoint, 15, Scalar(0, 255, 255), -1, 8, 0); // 填充圆点，表示注视区域
		stringstream streamx;
		stringstream streamy;
		streamx << fixed << setprecision(2) << FixedPoint.x;
		streamy << fixed << setprecision(2) << FixedPoint.y;

		str_a = "注视区域：" + streamx.str() + " , " + streamy.str();
	}
	Size text_size = getTextSize(str_a, font_face, font_scale, thickness, &baseline);
	char* str_input = (char*)str_a.c_str();
	putTextZH(mFrame, str_input, Point(50, 410), Scalar(255, 255, 255), 30, "Arial");

	return output;
}

float Worker::FixedDurRate() {
	//cout << fixedtime << " " << totaltime << endl;
	float rate = fixedtime / totaltime * 100;
	stringstream stream;
	stream << fixed << setprecision(2) << rate;
	string str_r = "注视时长百分比：" + stream.str() + "%";
	Size text_size = getTextSize(str_r, font_face, font_scale, thickness, &baseline);
	char* str_input = (char*)str_r.c_str();
	putTextZH(mFrame, str_input, Point(50, 440), Scalar(255, 255, 255), 30, "Arial");
	return rate;
}

bool Worker::isScan() {

	bool output;
	if (gazes.size() < fps) { return false; }

	int cur_idx = vec_idx == 0 ? fps - 1 : vec_idx - 1;

	Point2f cur = gazes[cur_idx];
	Point2f pre = gazes[cur_idx == 0 ? fps - 1 : cur_idx - 1];
	string str_s = "是否扫视：";
	if (CalDistance(cur, pre)> scant) {
		str_s = str_s + "True";
		scantime = scantime + 1.0 / fps;
		output = true;
	}
	else {
		str_s = str_s + "False";
		output = false;
	}

	Size text_size = getTextSize(str_s, font_face, font_scale, thickness, &baseline);
	char* str_input = (char*)str_s.c_str();
	putTextZH(mFrame, str_input, Point(50, 470), Scalar(255, 255, 255), 30, "Arial");

	return output;
}


float Worker::ScanDurRate() {
	
	float rate = scantime / totaltime * 100;
	stringstream stream;
	stream << fixed << setprecision(2) << rate;
	string str_r = "扫视时长百分比：" + stream.str() + "%";
	Size text_size = getTextSize(str_r, font_face, font_scale, thickness, &baseline);
	char* str_input = (char*)str_r.c_str();
	putTextZH(mFrame, str_input, Point(50, 500), Scalar(255, 255, 255), 30, "Arial");
	return rate;
}


void Worker::isSearch() {
	GazeInfo cur = infoq.back();
	if (cur.avgSpeed > 30 && !cur.isFixed && cur.speedY > 30 && cur.speedY*0.7 > cur.speedX) {
		cout << "==>start to search" << endl;
		searchList.push_back(cur);

	}
	else {
		Point2f res = SearchValid();
		if (res != Point2f(0, 0)) {
			Rect newr(res.x - 50, res.y - 100, 100, 200);
			searchArea = newr;
		}
		searchList.clear();
		cout << "==>no search" << endl;
	}

}

Point2f Worker::SearchValid() {

	Point2f output = Point2f(0, 0);

	if (searchList.size() < fps * 3) {
		cout << "====>too short" << endl;
		return output;
	}

	int scan = 0;
	for (int i = 0; i < searchList.size(); i++) {
		if (searchList[i].isScan) { scan++; }
		output.x = searchList[i].gp2d.x + output.x;
		output.y = searchList[i].gp2d.y + output.y;
	}

	if (scan < searchList.size()*0.5) {
		cout << "====>no scan" << endl;
		return Point2f(0, 0);
	}
	else {
		cout << "====>done" << endl;
		output.x = output.x / searchList.size();
		output.y = output.y / searchList.size();
		return output;
	}

}

void Worker::DrawSearchArea(){
	rectangle(mFrame, searchArea, Scalar(0, 255, 255), 3);
}

