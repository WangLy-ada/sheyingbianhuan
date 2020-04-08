#include<opencv2/opencv.hpp>
using namespace cv;
#define CV_SHOW(x) imshow("df",x);waitKey(0)
static void testImageRectification(cv::Mat &I);
void main()
{
	Mat I;
	I = imread("sss.jpg");
	testImageRectification(I);
}
static void testImageRectification(cv::Mat &image_original)
{
	double scale = 0.5;
	Size dsize = Size(image_original.cols*scale, image_original.rows*scale);
	resize(image_original, image_original, dsize);
	CV_SHOW(image_original); // CV_SHOW是cv::imshow的一个自定义宏
	cv::waitKey(1000);
	cv::imwrite("src.jpg", image_original);
	cv::Mat &&image = image_original.clone();

	cv::Mat image_gray;
	cv::cvtColor(image, image_gray, cv::COLOR_BGR2GRAY);
	//cv::threshold(image_gray, image_gray, g_threshVal, g_threshMax, cv::THRESH_BINARY);
	cv::threshold(image_gray, image_gray, 110, 250, cv::THRESH_BINARY);

	//查找图像的轮廓
	std::vector< std::vector<cv::Point> > contours_list;
	{
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(image_gray, contours_list, hierarchy,
			cv::RetrievalModes::RETR_EXTERNAL, cv::ContourApproximationModes::CHAIN_APPROX_NONE);
	}

	for (uint32_t index = 0; index < contours_list.size(); ++index) {
		cv::RotatedRect &&rect = cv::minAreaRect(contours_list[index]);
		if (rect.size.area() > 1000) {
			if (rect.angle != 0.) {
				
			} //if

			cv::Mat &mask = image_gray;
			cv::drawContours(mask, contours_list, static_cast<int>(index), cv::Scalar(255), cv::FILLED);

			cv::Mat extracted(image_gray.rows, image_gray.cols, CV_8UC1, cv::Scalar(0));
			image.copyTo(extracted, mask);
			//CV_SHOW(extracted);

			std::vector<cv::Point2f> poly;
			cv::approxPolyDP(contours_list[index], poly, 30, true); // 多边形逼近，精度(即最小边长)设为30是为了得到4个角点
			cv::Point2f pts_src[] = { 
				poly[1],
				poly[0],
				poly[3],
				poly[2]
			};

			cv::Rect &&r = rect.boundingRect();
			cv::Point2f pts_dst[] = {
				cv::Point(r.x, r.y),
				cv::Point(r.x + r.width, r.y),
				cv::Point(r.x + r.width, r.y + r.height) ,
				cv::Point(r.x, r.y + r.height)
			};
			cv::Mat &&M = cv::getPerspectiveTransform(pts_dst, pts_src); 

			cv::Mat warp; cv::warpPerspective(image, warp, M, image.size(), cv::INTER_LINEAR + cv::WARP_INVERSE_MAP, cv::BORDER_REPLICATE);
			//cv::resize(warp, warp,);
			//double scale = 0.5;
			//Size dsize = Size(warp.cols*scale, warp.rows*scale);
			//resize(warp, warp, dsize);
			CV_SHOW(warp);
			cv::imwrite("res.jpg", warp);
		} //if
	}
}
