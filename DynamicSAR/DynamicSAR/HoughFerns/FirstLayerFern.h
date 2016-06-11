#ifndef FIRST_LAYER_FERN_H
#define FIRST_LAYER_FERN_H

#include <opencv2/opencv.hpp>
#include <memory>
#include <iostream>
#include <vector>
#include <random>
#include <glm/gtx/string_cast.hpp>


#include "sample.h"
#include "P-COF.h"



/**
 * @brief   1�w�ڂ�Fern
 */
class FirstLayerFern
{
public:

	/**
	 * @brief   ���̃m�[�h(Fern�̐[���̐��p��)
	 */
	struct Node
	{
		cv::Mat_<uchar> patch;		// �p�b�`(�ݐό��z��������)
		cv::Mat_<int> weight;		// �d��
		float threshold;			// �ގ��x��臒l
	};


	/**
	 * @brief   ���[�m�[�h�̃f�[�^(2^(�[��)�̃f�[�^)
	 */
	struct EndNodeData
	{
		std::vector<float> distribution;		// �N���X�m��
		std::vector<float> poseDistribution;	// �p���m��
		std::vector<cv::Point2f> offsetList;	// �I�t�Z�b�g�x�N�g��
	};


	// �R���X�g���N�^
	FirstLayerFern(int _depth, int _featureTestNum, int _thresholdTestNum, int _numClass, int _posLabel, int _negLabel, int _gridSize)
		: depth (_depth)
		, featureTestNum (_featureTestNum)
		, thresholdTestNum (_thresholdTestNum)
		, numClass (_numClass)
		, posLabel (_posLabel)
		, negLabel (_negLabel)
		, gridSize (_gridSize)
	{
		p_cof.gridSize = gridSize;
	}

	FirstLayerFern()
		: depth (20)
		, featureTestNum (15)
		, thresholdTestNum (6)
		, numClass (2)
		, posLabel (1)
		, negLabel (0)
		, gridSize (4)
	{
		p_cof.gridSize = gridSize;
	}

	virtual ~FirstLayerFern(){}


	// �w�K
	void Learn(const std::vector<std::shared_ptr<LearnSample>>& samples, int posNum, const std::vector<double>& il, const std::vector<double> &poseIl);

	// ���ʂƓ��[����
	void Predict(const cv::Mat& src, cv::Mat_<float>& likelihoodMap, std::vector<cv::Mat_<float>> &poseMap);

	// Fern�̕ۑ�
	void saveFern(const std::string &fileName, const std::string& fileName_txt);

	// Fern�̓ǂݍ���
	bool loadFern(const std::string &fileName);

	// Fern�̓ǂݍ���(�e�L�X�g�`��)
	bool loadFern_txt(const std::string &fileName);

	/***** �����o�ϐ� *****/
	int depth;					// Fern�̐[��
	int featureTestNum;			// �����I����
	int thresholdTestNum;		// 臒l�I����
	int numClass;				// �N���X
	int posLabel;				// �|�W�e�B�u�摜�̃N���X�ԍ�
	int negLabel;				// �l�K�e�B�u�摜�̃N���X�ԍ�

	P_COF p_cof;				// P-COF����
	int gridSize;				// �������o�ɂ�����O���b�h�T�C�Y

	// Fern�̃f�[�^
	std::vector<std::unique_ptr<Node>> splitNode;			// �m�[�h�̕���ɗp����f�[�^
	std::vector<std::unique_ptr<EndNodeData>> nodeData;		// ���[�m�[�h�̃f�[�^
	std::vector<int> nodeLUT;								// ���[�m�[�h(nodeData)�̔z��ԍ����i�[(�|�W�e�B�u�T���v�����܂܂�Ă��Ȃ��ꍇ�F-1)


private:

	// �����̏�񗘓�����(�p���̃N���X���l��)
	inline double splitMultiInformationGain( const std::vector<std::shared_ptr<LearnSample>> &samples, const std::vector<int> &indices, const std::vector<float> &similarity, int &leftCount, int &rightCount, float threshold, const std::vector<double> &il, const std::vector<double> &poseIl);

	// �����̃I�t�Z�b�g�̕��U����(�p���̃N���X���l��)
	inline double splitMultiVariance( const std::vector<std::shared_ptr<LearnSample>> &samples, const std::vector<int> &indices, const std::vector<float> &similarity, int &leftCount, int &rightCount, double threshold);

	// �V���m���G���g���s�[�֐�
	inline double computeEntropy(const std::vector<double> &dist);

	// �V���m���G���g���s�[�֐�(�p���p)
	inline double computePoseEntropy(const std::vector<double> &dist);
};


#endif