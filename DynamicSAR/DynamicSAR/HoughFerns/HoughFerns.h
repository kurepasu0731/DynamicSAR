#ifndef HOUGH_FERNS_H
#define HOUGH_FERNS_H

#include <opencv2/opencv.hpp>

#include <Windows.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <glm/gtx/string_cast.hpp>

#include "sample.h"
#include "P-COF.h"
#include "FirstLayerFern.h"
#include "SecondLayerFern.h"


/**
 * @brief   Hough Ferns�ŉ摜�����o����N���X
 */
class HoughFerns
{
public:

	// �R���X�g���N�^
	HoughFerns(int _numFerns1, int _numFerns2, int _depth1, int _depth2, int _featureTestNum1, int _featureTestNum2, int _thresholdTestNum1, int _thresholdTestNum2, 
				int _numClass, int _posLabel, int _negLabel, float _posDataPerTree1, float _posDataPerTree2, float _negDataPerTree1, float _negDataPerTree2)
		: numFerns1 (_numFerns1)
		, numFerns2 (_numFerns2)
		, depth1 (_depth1)
		, depth2 (_depth2)
		, featureTestNum1 (_featureTestNum1)
		, featureTestNum2 (_featureTestNum2)
		, thresholdTestNum1 (_thresholdTestNum1)
		, thresholdTestNum2 (_thresholdTestNum2)
		, numClass (_numClass)
		, posLabel (_posLabel)
		, negLabel (_negLabel)
		, posSample_num (0)
		, negSample_num (0)
		, posDataPerTree1 (_posDataPerTree1)
		, posDataPerTree2 (_posDataPerTree2)
		, negDataPerTree1 (_negDataPerTree1)
		, negDataPerTree2 (_negDataPerTree2)
	{}

	HoughFerns()
		: numFerns1 (20)
		, numFerns2 (20)
		, depth1 (20)
		, depth2 (20)
		, featureTestNum1 (15)
		, featureTestNum2 (15)
		, thresholdTestNum1 (6)
		, thresholdTestNum2 (6)
		, numClass (2)
		, posLabel (1)
		, negLabel (0)
		, posSample_num (0)
		, negSample_num (0)
		, posDataPerTree1 (0.25)
		, posDataPerTree2 (0.25)
		, negDataPerTree1 (0.25)
		, negDataPerTree2 (0.25)
	{}

	virtual ~HoughFerns(){}


	// ������
	void init(int _numFerns1, int _numFerns2, int _depth1, int _depth2, int _featureTestNum1, int _featureTestNum2, int _thresholdTestNum1, int _thresholdTestNum2, 
				int _numClass, int _posLabel, int _negLabel, float _posDataPerTree1, float _posDataPerTree2, float _negDataPerTree1, float _negDataPerTree2);

	// �ݒ�t�@�C����ǂݍ���ŏ�����
	bool initFromFile(const std::string &fileName);

	// �w�K
	void Learn(const std::string& posFolder, const std::string& negFolder, const std::string& saveFolder);

	// ���o
	void detect(const cv::Mat& src, float min_distance, float max_distance, int distance_num, float min_scale, float max_scale, int scale_num,
						int detect_width, int detect_height, float mag_th, int meanshift_radius=64, int win_th = 80, double likelihood_th=0.8);

	// ���o(1�w��)
	void detectFirst(const cv::Mat& src, std::vector<cv::Point3f> &detect_point, std::vector<float> &detect_scales, float min_distance, float max_distance, int distance_num, 
						float mag_th, int meanshift_radius=64, int win_th = 80, double likelihood_th=0.8);
	// ���o(2�w��)
	void detectSecond(const cv::Mat& src, const cv::Point3f &detect_point, float detect_scale, int detect_num, float min_scale, float max_scale, int scale_num, 
						int detect_width, int detect_height, float mag_th, int meanshift_radius=64, int win_th = 80, double likelihood_th=0.8);

	// ���������ɂ�����K�E�V�A���t�B���^�ɂ�镽����
	void smoothMap(std::vector<cv::Mat_<float>> &likelihood);

	// �ޓx�}�b�v�̉���
	void visualizeLikelihood(const std::vector<cv::Mat_<float>> &likelihood, const std::vector<float> scale_rate, float max_value, bool flag);

	// Mean Shift�ŋɑ�l����
	void meanShift(std::vector<cv::Point3f> &meanshiftPoint, std::vector<double> &detectionVote, const std::vector<float> scale_rate, int radius);

	// Nearest Neighbor�ŋߖT�_�̓���
	void nearestNeighbor(std::vector<cv::Point3f> &detectionPoint, std::vector<double> &detectionVote, int win_th);

	// �p�������̊m���̐���
	void poseEstimation(std::vector<std::vector<float>> &pose_dist, std::vector<std::vector<cv::Mat_<float>>> &poseMap, 
								const std::vector<cv::Point3f> &detectionPoint, const std::vector<int> scales, const std::vector<float> scale_rate);

	// ���m�Ȏp���̐���
	void poseEstimation(std::vector<glm::mat4> &pose_est, std::vector<std::vector<std::vector<glm::vec3>>> &axisMap, std::vector<std::vector<std::vector<glm::vec2>>> &angleMap, 
								const std::vector<cv::Point3f> &detectionPoint, const std::vector<int> scales, const std::vector<float> scale_rate);

	// �w�K�T���v���̎擾
	void getLearnSamples(const std::string& posFolder, const std::string& negFolder, std::vector<std::shared_ptr<LearnSample>>& samples);

	// �T�u�Z�b�g�̍쐬
	void makeSubset(std::vector<std::shared_ptr<LearnSample>> &subSamples, const std::vector<std::shared_ptr<LearnSample>> &samples, float pos_rate, float neg_rate, int &posSize, int &negSize);
	void makeSubset(std::vector<std::shared_ptr<LearnSample>> &subSamples, const std::vector<std::shared_ptr<LearnSample>> &samples, int pos_num, int neg_num, float pos_rate, float neg_rate, int &posSize, int &negSize);

	// Hough Ferns�̕ۑ�
	void saveFerns(const std::string &folderName);

	// Hough Ferns�̓ǂݍ���
	bool loadFerns(const std::string &folderName);

	// ���f���ƃJ�����Ƃ̋����̎擾
	float getDistance() { return distance; };

	// ���o�p
	std::vector<cv::Mat_<float>> likelihoodmap;			// �ޓx�}�b�v

	// �ʒu�p������p
	std::vector<cv::Point3f> detectPoint;				// ���o�������W
	std::vector<std::vector<float>> pose_distribute;	// �p���m��
	std::vector<glm::mat4> pose_estimate;				// ���肳�ꂽ�p��


private:

	/***** �����o�ϐ� *****/
	int numFerns1;				// Fern�̐�(1�w��)
	int numFerns2;				// Fern�̐�(2�w��)
	int depth1;					// Fern�̐[��(1�w��)
	int depth2;					// Fern�̐[��(2�w��)
	int featureTestNum1;		// �����I����(1�w��)
	int featureTestNum2;		// �����I����(2�w��)
	int thresholdTestNum1;		// 臒l�I����(1�w��)
	int thresholdTestNum2;		// 臒l�I����(2�w��)
	int numClass;				// �N���X
	int posLabel;				// �|�W�e�B�u�摜�̃N���X�ԍ�(1�w��)
	int negLabel;				// �l�K�e�B�u�摜�̃N���X�ԍ�(2�w��)
	int posSample_num;			// �|�W�e�B�u�T���v���̐�
	int negSample_num;			// �l�K�e�B�u�T���v���̐�
	float posDataPerTree1;		// �X�̖؂ɂ�����T�u�Z�b�g�̊���(�|�W�e�B�u)(1�w��)
	float posDataPerTree2;		// �X�̖؂ɂ�����T�u�Z�b�g�̊���(�|�W�e�B�u)(2�w��)
	float negDataPerTree1;		// �X�̖؂ɂ�����T�u�Z�b�g�̊���(�l�K�e�B�u)(1�w��)
	float negDataPerTree2;		// �X�̖؂ɂ�����T�u�Z�b�g�̊���(�l�K�e�B�u)(2�w��)

	// �����Ɋւ���
	P_COF p_cof;				// P-COF����
	int patchSize;				// �p�b�`�̃T�C�Y
	int gridSize;				// �������o�ɂ�����O���b�h�T�C�Y
	glm::vec3 eyeVector;		// �J�����̎�������
	glm::vec3 eyeUp;			// �J�����̏����
	float distance;				// ���f���ƃJ�����̋���

	std::vector<std::unique_ptr<FirstLayerFern>> first_ferns;					// 1�w�ڂ�Fern�̔z��
	std::vector<std::vector<std::unique_ptr<SecondLayerFern>>> second_ferns;	// 2�w�ڂ�Fern�̔z��

};


#endif