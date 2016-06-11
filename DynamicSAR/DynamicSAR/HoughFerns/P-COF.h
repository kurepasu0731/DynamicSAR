#ifndef P_COF_H
#define P_COF_H

#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <fstream>


/**
 * @brief   Perspectively-Cumulated Orientation Feature
 */
class P_COF
{
public:

	/**
	 * @brief   �e���v���[�g�̍\����
	 */
	struct Template
	{
		int width;						// ���̃e���v���[�g�̉���
		int height;						// ���̃e���v���[�g�̏c��
		cv::Mat_<uchar> oriMat;			// �ʎq�����z����
		cv::Mat_<int> weightMat;		// �d��
	};


	// �I�u�W�F�N�g���ɕR�Â����e���v���[�g�摜
	typedef std::map<std::string, std::vector<Template> > TemplatesMap;


	/**
	 * @brief   �}�b�`���O�̍\����
	 */
	struct Match
	{
		int x;					// x���W
		int y;					// y���W
		int width;				// ��
		int height;				// ����
		float similarity;		// �ގ��x
		int template_num;		// �e���v���[�g�ԍ�
		std::string class_id;	// �N���X��

		Match(int _x, int _y, int _width, int _height, float _similarity, int _template_num, std::string _class_id)
			: x (_x)
			, y (_y)
			, width (_width)
			, height (_height)
			, similarity (_similarity)
			, template_num (_template_num)
			, class_id (_class_id){}
	};


	P_COF() : gridSize (4){};
	P_COF(int _gridSize)	
		: gridSize (_gridSize)
	{}

	virtual ~P_COF(){}

	// �摜������z�����̐ϕ��摜���v�Z
	void calcIntegralOrientation(const cv::Mat& src, std::vector<cv::Mat_<int>>& integral_angle, float threshold, bool smooth=false, int kernel_size=3);

	// �ϕ��摜������z�����̗ʎq��
	void quantizedIntegraOrientation(cv::Mat& quantized_angle, const std::vector<cv::Mat_<int>>& integral_angle, int freq_th, int grid_size);

	// ���z�����̊g��
	void spreadOrientation(const cv::Mat& src, cv::Mat& dst, int T);

	// OR���Z�q�Ō��z����������
	void orCombine(const uchar * src, const int src_stride, uchar * dst, const int dst_stride, const int width, const int height);

	// �ʎq�����z������ݐ�
	void cumulatedOrientation(const cv::Mat& quantized_angle, std::vector<cv::Mat>& orientationFreq);

	// �ގ��x�̌v�Z
	void calcSimilarity(const cv::Mat& quantized_angle, const Template& templ, cv::Mat& dst);

	// �ގ��x�̌v�Z
	void calcSimilarity(const cv::Mat& src, const cv::Mat& oriMat, const cv::Mat& weightMat, cv::Mat& dst);

	// �e���v���[�g�摜�̒ǉ�
	void addTemplate(const std::vector<cv::Mat>& src, const std::string& class_id, float mag_th, int freq_th);

	// �e���v���[�g�摜�Ƃ̃}�b�`���O
	void matchTemplate(const cv::Mat& src, std::vector<Match>& matches, float mag_th, float similarity_th);
	
	// �ߖT�_�̓���
	void nearestNeighbor(std::vector<Match>& matches, int win_th);

	// �e���v���[�g�̕ۑ�
	bool saveTemplate(const std::string& savePath, const std::string& class_id);

	// �e���v���[�g�̓ǂݍ���
	bool loadTemplate(const std::string& savePath, const std::string& class_id);


	// ���x����Ԃ�(2�i�������z����)
	inline int getLabel(int quantized)
	{
		switch (quantized)
		{
			case 1:   return 0;
			case 2:   return 1;
			case 4:   return 2;
			case 8:   return 3;
			case 16:  return 4;
			case 32:  return 5;
			case 64:  return 6;
			case 128: return 7;
			default:
				std::cerr << "Invalid value of quantized parameter:" <<  quantized << std::endl;
				return -1; 
		}
	}

	// Bit�̐��l��Ԃ�(���z������2�i��)
	inline int getBitNum(int bit)
	{
		switch (bit)
		{
			case 0:   return 1;
			case 1:   return 2;
			case 2:   return 4;
			case 3:   return 8;
			case 4:   return 16;
			case 5:   return 32;
			case 6:   return 64;
			case 7:   return 128;
			default:
				std::cerr << "Invalid value of quantized parameter" << std::endl;
				return -1; 
		}
	}

	// �e���v���[�g�摜
	TemplatesMap class_templates;

	// �O���b�h�T�C�Y
	int	gridSize;
};


#endif