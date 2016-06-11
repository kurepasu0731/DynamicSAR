#ifndef GRAYCODE_H
#define GRAYCODE_H

#pragma once


#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>  // ������X�g���[��
#include <direct.h> // create dir
#include <memory>

#include "../CamDev/WebCamera.h"
#include "../CamDev/CameraLinkCamera.h"
#include "../CamDev/UndistortedCamera.h"

class ProjectionThread;

/**
 * @brief   GrayCode��p�����􉽕␳
 *
 * @note	�O���C�R�[�h�́C�c�C���ʂɍ쐬���C��ō���
 *			�p�^�[���摜�̓r�b�g���Z��p���č쐬
 *			�ŏI�I�ȃv���W�F�N�^�ƃJ������Ή��t�����z���c->CamPro�Ɋi�[�����
 */
class GRAYCODE
{

public:

	// �O���C�R�[�h�쐬�ɕK�v�ȍ\����
	typedef struct _Graycode 
	{
		std::vector<std::vector<int>> graycode;			// �O���C�R�[�h�i�v���W�F�N�^�𑜓x[����][��]�j
		unsigned int h_bit, w_bit;						// �����C���̕K�v�r�b�g��
		unsigned int all_bit;							// ���v�r�b�g���ih_bit + w_bit�j
	} Graycode;

	// �v���W�F�N�^ - �J�����Ή��ɕK�v�ȍ\����
	typedef struct _correspondence 
	{
		std::vector<std::vector<int>> graycode;			// 2�l���R�[�h�𕜌��������̂��J������f�Ɋi�[
		std::vector<std::vector<cv::Point>> CamPro;		// �v���W�F�N�^��f�ɑ΂���J�����Ή���f
		std::vector<std::vector<cv::Point>> ProCam;		// �J������f�ɑ΂���v���W�F�N�^�Ή���f 
		std::unique_ptr<std::map<int, cv::Point>> code_map;
		Graycode g;
	} correspondence;

	std::unique_ptr<correspondence> c;

	// �R���X�g���N�^
	GRAYCODE(CamDev::Camera *camera, int _prj_width, int _prj_height, int _cam_width, int _cam_height, double _delay=80.0, std::string _saveFolder=".")
		: prj_width (_prj_width)
		, prj_height (_prj_height)
		, cam_width (_cam_width)
		, cam_height (_cam_height)
		, saveFolder (_saveFolder)
	{
		// �f�o�C�X�̎󂯓n��
		camDev = camera;

		// ������
		GC = "Graycode";
		MP = "Measure";
		delay = _delay;
		g = std::unique_ptr<Graycode>(new Graycode());
		c = std::unique_ptr<correspondence>(new correspondence());

		// �T�C�Y�̏�����
		g->graycode.resize(prj_height, std::vector<int>(prj_width, 0));
		c->graycode.resize(cam_height, std::vector<int>(cam_width, 0));
		c->CamPro.resize(prj_height, std::vector<cv::Point>(prj_width, cv::Point(0,0)));
		c->ProCam.resize(cam_height, std::vector<cv::Point>(cam_width, cv::Point(0,0)));
		c->g.graycode.resize(prj_height, std::vector<int>(prj_width, 0));
		c->code_map = std::unique_ptr<std::map<int, cv::Point>>(new std::map<int, cv::Point>());

		// �\���̂̏�����
		c->g.h_bit = (int)ceil( log(prj_height+1) / log(2) );
		c->g.w_bit = (int)ceil( log(prj_width+1) / log(2) );
		c->g.all_bit = c->g.h_bit + c->g.w_bit;

		// �ۑ���̃t�H���_����
		createDirs();
	}

	virtual ~GRAYCODE(){}


	// �p�^�[���R�[�h���e & �B�e
	void code_projection(ProjectionThread *projection);
	// 2�l��
	void make_thresh();
	// ������
	void makeCorrespondence();

	//// �摜�ό`�E����
	//// �J�����B�e�̈悩��v���W�F�N�^���e�̈��؂�o��
	void transport_camera_projector(const cv::Mat &src, cv::Mat &dst);
	//// ���͉摜���J�����B�e�̈�ɕό`
	void transport_projector_camera(const cv::Mat &src, cv::Mat &dst);

	// �J�������W�ɑ΂���v���W�F�N�^�̑Ή��_��Ԃ�
	void getCorrespondProjPoints(std::vector<cv::Point2f> &projPoint, const std::vector<cv::Point2f> &imagePoint);
	// �J�������W�ɑ΂���v���W�F�N�^�̑Ή��_��Ԃ�(�����x��)
	void getCorrespondSubPixelProjPoints(std::vector<cv::Point2f> &projPoint, const std::vector<cv::Point2f> &imagePoint, int size = 20);

	// �Ή��̂Ƃꂽ�_��S�ĕԂ�
	void getCorrespondAllPoints(std::vector<cv::Point2f> &projPoint, std::vector<cv::Point2f> &imagePoint);


	// �v���W�F�N�^�𑜓x
	int prj_width;
	int prj_height;
	int cam_width;
	int cam_height;


private:
	// �E�B���h�E�l�[��
	char* GC;
	char* MP;
	float SHUTTER;	// �V���b�^�[���x
	double delay;

	std::unique_ptr<Graycode> g;

	///// �O���C�R�[�h�̍쐬�֘A /////
	// �J�����̏�����
	void initCamera();
	// �O���C�R�[�h�쐬
	void initGraycode();
	// �p�^�[���R�[�h�摜�쐬
	void makeGraycodeImage();
	// �f�B���N�g���̍쐬
	void createDirs();

	///// ��l���֘A /////
	// �J�����B�e�摜��ǂݍ��ފ֐�
	void loadCam(cv::Mat &mat, int div_bin, bool flag, bool pattern);
	// �ŏI�I�Ɏg�p����}�X�N�𐶐�����֐�
	void makeMask(cv::Mat &mask);
	// �O���C�R�[�h�̉摜�𗘗p���ă}�X�N�𐶐�����֐�
	void makeMaskFromCam(cv::Mat &posi, cv::Mat &nega, cv::Mat &result, int thresholdValue = 25);
	// 2�l�������֐� 
	void thresh( cv::Mat &posi, cv::Mat &nega, cv::Mat &thresh_img, int thresh_value );

	///// ���̑� /////
	// �v���W�F�N�^ - �J�����\���̏�����
	void initCorrespondence();
	// 2�l���R�[�h����
	void code_restore();

	// �ۑ��t�H���_
	std::string saveFolder;

	CamDev::Camera *camDev;				// �J�����f�o�C�X
};


#endif