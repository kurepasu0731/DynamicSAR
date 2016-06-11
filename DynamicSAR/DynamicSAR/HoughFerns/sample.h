#ifndef SAMPLE_H
#define SAMPLE_H

#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>



// �w�K�p�̃T���v���f�[�^
class LearnSample
{
public:
	LearnSample()
		: label (0)
	{}

	/***** �����o�ϐ�  *****/
	cv::Mat_<uchar> gradOri;				// �P�x���z����
	cv::Mat_<uchar> pcofOri;				// �ݐϋP�x���z����
	cv::Mat_<int> weight;					// �d��
	int label;								// ���t�M��
	int poseLabel;							// �p�����x��
	cv::Point2f offset;						// ���̏d�S����̃I�t�Z�b�g
	glm::vec3 axis;							// ��]��
	glm::vec2 rollAngle;					// ��]�p

	// �l�����(�|�W�e�B�u�T���v���p)
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

	// �l�����(�l�K�e�B�u�T���v���p)
	inline void negInsert(const cv::Mat& _gradOri, const int &_label)
	{
		gradOri = _gradOri.clone();
		label = _label;
	};
};



#endif