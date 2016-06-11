#ifndef SAMPLE_H
#define SAMPLE_H

#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>



// 学習用のサンプルデータ
class LearnSample
{
public:
	LearnSample()
		: label (0)
	{}

	/***** メンバ変数  *****/
	cv::Mat_<uchar> gradOri;				// 輝度勾配方向
	cv::Mat_<uchar> pcofOri;				// 累積輝度勾配方向
	cv::Mat_<int> weight;					// 重み
	int label;								// 教師信号
	int poseLabel;							// 姿勢ラベル
	cv::Point2f offset;						// 物体重心からのオフセット
	glm::vec3 axis;							// 回転軸
	glm::vec2 rollAngle;					// 回転角

	// 値を入力(ポジティブサンプル用)
	inline void posInsert(const cv::Mat& _gradOri, const cv::Mat& _pcofOri, const cv::Mat& _weight, int _label, int _poseLabel, const cv::Point2f &_offset, const glm::vec3 &_axis, const glm::vec2 &_roll)
	{
		gradOri = _gradOri.clone();
		pcofOri = _pcofOri.clone();
		weight = _weight.clone();
		label = _label;
		poseLabel = _poseLabel;
		offset = _offset;
		axis = _axis;
		rollAngle = _roll;
	};

	// 値を入力(ネガティブサンプル用)
	inline void negInsert(const cv::Mat& _gradOri, const int &_label)
	{
		gradOri = _gradOri.clone();
		label = _label;
	};
};



#endif