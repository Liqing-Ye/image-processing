#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc.hpp>
//#include <opencv2/opencv.hpp>
#include <opencv2\imgproc\types_c.h>
#include <iostream>
#include <chrono>
using namespace std;

// ��������ڴ�Žŵ�ѹ������
class PressureData
{
public:
	// �ֱ��Ǻ����Ĵ�С��ѹ�����ĵ�x���꣬ѹ�����ĵ�y����
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
		// �������Ҹ�ֵ
		if (this == &pData) return *this;
		force = pData.force;
		Fx = pData.Fx;
		Fy = pData.Fy;
		// ������ʽ��ֵ
		return *this;
	}
	// ��Ԫ���������Խ������������ֵ
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

		PressureData leftData; // ���ڴ����ŵ�����
		PressureData rightData; // ���ڴ���ҽŵ�����
		cap >> frame;
		cv::Scalar Sum1 = cv::sum(frame);
		double pressure0 = Sum1[0];

		if (frame.empty()) break;  //���ͼ��û������
		else if (pressure0 == 0)  //���ͼ��û������ֵ���򷵻أ�-1��-1��-1��
		{
			cv::imshow("Frame", frame);
			cout << "Left Foot" << endl;
			leftData.printData();
			cout << "Right Foot" << endl;
			rightData.printData();
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time = end - start;
			// �����ʱ����λ���룩
			std::cout << "Time in milliseconds: " << time.count() * 1000 << std::endl;
			if (cv::waitKey(33) == 27) break;
		}
		else  //���ͼ��������ֵ
		{
			cv::Mat image_gray;
			cvtColor(frame, image_gray, CV_BGR2GRAY);   //ת�Ҷ�ͼ

			cv::Mat image_binary;
			cv::threshold(image_gray, image_binary, 0, 255, cv::THRESH_BINARY);   //ת��ֵͼ
			cv::Mat structureElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(-1, -1));
			cv::dilate(image_binary, image_binary, structureElement, cv::Point(-1, -1), 2); //��������
			vector<vector<cv::Point>>contours;
			cv::findContours(image_binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);  //Ѱ��ͼ������
			cv::Rect rectbounding1 = cv::boundingRect(contours[0]);  //Ѱ�Ҿ��α߿�
			cv::Mat image_rect1;
			image_rect1 = cv::Mat(frame, rectbounding1);
			cv::Mat Sum1 = image_rect1;
			cv::Scalar sum1 = sum(Sum1);  //����α߿���ͼ������غ�
			double pressure1 = sum1[0];   //���غ�ת��double��������
			//������������������
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
			//�жϾ��α߿��ڵ�ͼ�񲿷�������ֻ��
			if (x1 > 25)
			{
				cv::rectangle(frame, rectbounding1, cv::Scalar(0, 0, 255));
				PressureData leftData(pressure1 / 10, x1, y1); // ���ڴ����ŵ�����
				cout << "Left Foot" << endl;
				leftData.printData();
			}
			else
			{
				cv::rectangle(frame, rectbounding1, cv::Scalar(255, 0, 0));
				PressureData rightData(pressure1 / 10, x1, y1); // ���ڴ���ҽŵ�����
				cout << "Right Foot" << endl;
				rightData.printData();
			}

			//���ͼƬ��ͬʱ������ֻ��
			double x2 = 0, y2 = 0;
			if (contours.size() > 1)
			{
				cv::Rect rectbounding2 = cv::boundingRect(contours[1]); //Ѱ��ͼ���ڵڶ��������ľ��α߿�
				cv::Mat image_rect2;
				image_rect2 = cv::Mat(frame, rectbounding2);
				cv::Mat Sum2 = image_rect2;
				cv::Scalar sum2 = sum(Sum2);  //��ڶ������α߿�������ֵ�ĺ�
				double pressure2 = sum2[0];   //���غ�תdouble��������
				//��������ڶ������α߿���ͼ�񲿷ֵ������ĵ�����
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
				//�жϵڶ������α߿��ڵ�ͼ������ҽ�
				if (x2 > 25)
				{
					cv::rectangle(frame, rectbounding2, cv::Scalar(0, 0, 255));
					PressureData leftData(pressure2 / 10, x2, y2); // ���ڴ����ŵ�����
					cout << "Left Foot" << endl;
					leftData.printData();
				}
				else
				{
					cv::rectangle(frame, rectbounding2, cv::Scalar(255, 0, 0));
					PressureData rightData(pressure2 / 10, x2, y2); // ���ڴ���ҽŵ�����
					cout << "Right Foot" << endl;
					rightData.printData();
				}
			}
			else //���ͼ����ֻ��һ�����α߿�����һֻ����ͼ���ڵĽŵ�ֵ����Ϊ��-1��-1��-1��
			{
				if (x1 > 50)
				{
					PressureData rightData(-1, -1, -1); // ���ڴ���ҽŵ�����
					cout << "Right Foot" << endl;
					rightData.printData();
				}
				else
				{
					PressureData leftData(-1, -1, -1); // ���ڴ����ŵ�����
					cout << "Left Foot" << endl;
					leftData.printData();
				}
			}

			cv::imshow("Frame", frame);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> time = end - start;
			// �����ʱ����λ���룩
			std::cout << "Time in milliseconds: " << time.count() * 1000 << std::endl;
			if (cv::waitKey(33) == 27) break;
		}
	}
}