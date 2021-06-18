#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc.hpp>
//#include <opencv2/opencv.hpp>
#include <opencv2\imgproc\types_c.h>
#include <iostream>
#include <chrono>
using namespace std;

// 这个类用于存放脚的压力数据
class PressureData
{
public:
	// 分别是合力的大小，压力中心的x坐标，压力中心的y坐标
	double force, Fx, Fy;
	PressureData()
	{
		force = Fx = Fy = -1;
	}
	PressureData(double _force, double _Fx, double _Fy)
	{
		force = _force;
		Fx = _Fx;
		Fy = _Fy;
	}
	PressureData& operator=(const PressureData& pData)
	{
		// 处理自我赋值
		if (this == &pData) return *this;
		force = pData.force;
		Fx = pData.Fx;
		Fy = pData.Fy;
		// 处理链式赋值
		return *this;
	}
	// 友元函数，可以交换两个对象的值
	inline friend void swap(PressureData& p1, PressureData& p2)
	{
		swap(p1.force, p2.force);
		swap(p1.Fx, p2.Fx);
		swap(p1.Fy, p2.Fy);
	}
	inline void printData()
	{
		printf("Force: %f\tFx: %f\tFy: %f\n", force, Fx, Fy);
	}
};

int main()
{
	cv::VideoCapture cap("demo.avi");
	cv::Mat frame;
	cv::namedWindow("Frame", cv::WINDOW_NORMAL);


	while (true)
	{
		auto start = std::chrono::high_resolution_clock::now();

		PressureData leftData; // 用于存放左脚的数据
		PressureData rightData; // 用于存放右脚的数据
		cap >> frame;
		cv::Scalar Sum1 = cv::sum(frame);
		double pressure0 = Sum1[0];

		if (frame.empty()) break;  //如果图像没有载入
		else if (pressure0 == 0)  //如果图像没有像素值，则返回（-1，-1，-1）
		{
			cv::imshow("Frame", frame);
			cout << "Left Foot" << endl;
			leftData.printData();
			cout << "Right Foot" << endl;
			rightData.printData();
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time = end - start;
			// 输出用时（单位毫秒）
			std::cout << "Time in milliseconds: " << time.count() * 1000 << std::endl;
			if (cv::waitKey(33) == 27) break;
		}
		else  //如果图像有像素值
		{
			cv::Mat image_gray;
			cvtColor(frame, image_gray, CV_BGR2GRAY);   //转灰度图

			cv::Mat image_binary;
			cv::threshold(image_gray, image_binary, 0, 255, cv::THRESH_BINARY);   //转二值图
			cv::Mat structureElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(-1, -1));
			cv::dilate(image_binary, image_binary, structureElement, cv::Point(-1, -1), 2); //膨胀两次
			vector<vector<cv::Point>>contours;
			cv::findContours(image_binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);  //寻找图像轮廓
			cv::Rect rectbounding1 = cv::boundingRect(contours[0]);  //寻找矩形边框
			cv::Mat image_rect1;
			image_rect1 = cv::Mat(frame, rectbounding1);
			cv::Mat Sum1 = image_rect1;
			cv::Scalar sum1 = sum(Sum1);  //求矩形边框内图像的像素和
			double pressure1 = sum1[0];   //像素和转成double数据类型
			//以下是求力中心坐标
			double x = rectbounding1.x, y = rectbounding1.y, w = rectbounding1.width, h = rectbounding1.height;
			double a = 0, b = 0, x1 = 0, y1 = 0;
			for (int i = y - 1; i < y + h; i++)
			{
				uchar* uc_pixel = image_gray.data + i * image_gray.step;
				for (int j = x - 1; j < x + w; j++)
				{
					a = uc_pixel[j] * (j + 1);
					x1 = a + x1;
					b = uc_pixel[j] * (i + 1);
					y1 = b + y1;
				}
			}
			x1 = x1 / pressure1;
			y1 = y1 / pressure1;
			//判断矩形边框内的图像部分属于哪只脚
			if (x1 > 25)
			{
				cv::rectangle(frame, rectbounding1, cv::Scalar(0, 0, 255));
				PressureData leftData(pressure1 / 10, x1, y1); // 用于存放左脚的数据
				cout << "Left Foot" << endl;
				leftData.printData();
			}
			else
			{
				cv::rectangle(frame, rectbounding1, cv::Scalar(255, 0, 0));
				PressureData rightData(pressure1 / 10, x1, y1); // 用于存放右脚的数据
				cout << "Right Foot" << endl;
				rightData.printData();
			}

			//如果图片内同时出现两只脚
			double x2 = 0, y2 = 0;
			if (contours.size() > 1)
			{
				cv::Rect rectbounding2 = cv::boundingRect(contours[1]); //寻找图像内第二个轮廓的矩形边框
				cv::Mat image_rect2;
				image_rect2 = cv::Mat(frame, rectbounding2);
				cv::Mat Sum2 = image_rect2;
				cv::Scalar sum2 = sum(Sum2);  //求第二个矩形边框内像素值的和
				double pressure2 = sum2[0];   //像素和转double数据类型
				//以下是求第二个矩形边框内图像部分的力中心点坐标
				double x = rectbounding2.x, y = rectbounding2.y, w = rectbounding2.width, h = rectbounding2.height;
				double a = 0, b = 0;
				for (int i = y - 1; i < y + h; i++)
				{
					uchar* uc_pixel = image_gray.data + i * image_gray.step;
					for (int j = x - 1; j < x + w; j++)
					{
						a = uc_pixel[j] * (j + 1);
						x2 = a + x2;
						b = uc_pixel[j] * (i + 1);
						y2 = b + y2;
					}
				}
				x2 = x2 / pressure2;
				y2 = y2 / pressure2;
				//判断第二个矩形边框内的图像的左右脚
				if (x2 > 25)
				{
					cv::rectangle(frame, rectbounding2, cv::Scalar(0, 0, 255));
					PressureData leftData(pressure2 / 10, x2, y2); // 用于存放左脚的数据
					cout << "Left Foot" << endl;
					leftData.printData();
				}
				else
				{
					cv::rectangle(frame, rectbounding2, cv::Scalar(255, 0, 0));
					PressureData rightData(pressure2 / 10, x2, y2); // 用于存放右脚的数据
					cout << "Right Foot" << endl;
					rightData.printData();
				}
			}
			else //如果图像内只有一个矩形边框，则另一只不在图像内的脚的值返回为（-1，-1，-1）
			{
				if (x1 > 50)
				{
					PressureData rightData(-1, -1, -1); // 用于存放右脚的数据
					cout << "Right Foot" << endl;
					rightData.printData();
				}
				else
				{
					PressureData leftData(-1, -1, -1); // 用于存放左脚的数据
					cout << "Left Foot" << endl;
					leftData.printData();
				}
			}

			cv::imshow("Frame", frame);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time = end - start;
			// 输出用时（单位毫秒）
			std::cout << "Time in milliseconds: " << time.count() * 1000 << std::endl;
			if (cv::waitKey(33) == 27) break;
		}
	}
}