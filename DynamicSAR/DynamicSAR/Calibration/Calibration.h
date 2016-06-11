#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Windows.h>
#include <opencv2/opencv.hpp>
#include "Graycode.h"

#include "../CamDev/WebCamera.h"
#include "../CamDev/CameraLinkCamera.h"
#include "../CamDev/UndistortedCamera.h"


class ProjectionThread;



/**
 * @brief   GrayCode��p�����v���W�F�N�^�ƃJ�����̃L�����u���[�V����
 */
class Calibration
{
public:
	Calibration(CamDev::Camera *camera, int _prj_width, int _prj_height, int _cam_width, int _cam_height, bool _use_chessboard, int _cornerCol, int _cornerRow, float _cornerInterval_m, double _delay=80.0, std::string _saveCalibFolder=".")
		: cornerSize (cv::Size(_cornerCol, _cornerRow))
		, cornerInterval_m (_cornerInterval_m)
		, saveCalibFolder (_saveCalibFolder)
		, calib_flag (false)
		, calib_count (0)
	{
		// �f�o�C�X�̎󂯓n��
		camDev = camera;

		// �O���C�R�[�h�N���X
		graycode = std::unique_ptr<GRAYCODE>( new GRAYCODE(camDev, _prj_width, _prj_height, _cam_width, _cam_height, _delay, "../../"));

		// ���E���W�ɂ�����`�F�b�J�[�p�^�[���̌�_���W������
		// �`�F�X�{�[�h�̏ꍇ
		if (use_chessboard)
		{
			for( int i = 0; i < cornerSize.area(); ++i ) {
				worldPoint.push_back( cv::Point3f(	static_cast<float>( i % cornerSize.width * cornerInterval_m ),
														static_cast<float>( i / cornerSize.width * cornerInterval_m ), 0.0 ) );
			}
		}
		// �T�[�N���O���b�h�̏ꍇ
		else
		{
			for (int j = 0; j < _cornerRow; j++)
			{
				for (int i = 0; i < _cornerCol; i++)
				{
					worldPoint.push_back(cv::Point3f(static_cast<float>((2*i + j%2) * cornerInterval_m), static_cast<float>(j * cornerInterval_m), 0.0));
				}
			}
		}
	}

	virtual ~Calibration(){}


	// �O���C�R�[�h���e��ProCam�Ԃ̊􉽑Ή����擾
	void runGetCorrespond(const cv::Mat& src, const cv::Mat& ProCam_src, ProjectionThread *projection);

	// �Ή��_�̃t�@�C������ProCam�L�����u���[�V����
	void runCorrespondFileCalib(const cv::Size &camSize, const cv::Size &projSize);


	// �摜����`�F�b�J�[�p�^�[���̌�_���擾
	bool getCorners(std::vector<cv::Point2f> &imagePoint, const cv::Mat &image, cv::Mat &draw_image);		// �摜����`�F�b�J�[�p�^�[���̌�_���擾

	// �ē��e�덷�̌v�Z
	void calcReprojectionError(const std::vector<std::vector<cv::Point3f>> &worldPoints, const std::vector<std::vector<cv::Point2f>> &cameraPoints, const std::vector<std::vector<cv::Point2f>> &projectorPoints,double &cam_error, double &proj_error);

	// �v���W�F�N�^�ƃJ�����̃L�����u���[�V����
	void proCamCalibration(const std::vector<std::vector<cv::Point3f>> &worldPoints, const std::vector<std::vector<cv::Point2f>> &cameraPoints, const std::vector<std::vector<cv::Point2f>> &projectorPoints,
							const cv::Size &camSize, const cv::Size &projSize);

	// �L�����u���[�V�������ʂ̓ǂݍ���
	void loadCalibParam(const std::string &fileName);

	// �������e�ϊ��s��̎擾(�J����)
	cv::Mat getCamPerspectiveMat();
	// �������e�ϊ��s��̎擾(�v���W�F�N�^)
	cv::Mat getProjPerspectiveMat();

	// �J�����ʒu�����[���h���W�Ƃ����ۂ̑Ώە��̂̈ʒu�̎擾
	void getCameraWorldPoint(std::vector<cv::Point3f> &camWorldPoint, const std::vector<cv::Point2f> &imagePoint);

	// �ۑ���t�H���_�̃Z�b�g
	inline void setSaveFolder(std::string& folder){ saveCalibFolder = folder; }


	/***** �����o�ϐ� *****/
	bool use_chessboard;		// true:�`�F�X�{�[�h, false:�T�[�N���O���b�h
	cv::Size cornerSize;		// �R�[�i�[��
	float cornerInterval_m;		// �R�[�i�[�̊Ԋu�̑傫��(m)

	std::vector<cv::Point3f> worldPoint;						// �`�F�b�J�[��_���W�ƑΉ����鐢�E���W�̒l���i�[����s��

	std::vector<std::vector<cv::Point3f>> myWorldPoints;		// ���E���W�̓_
	std::vector<std::vector<cv::Point2f>> myCameraPoints;		// �J�����摜��̑Ή��_
	std::vector<std::vector<cv::Point2f>> myProjectorPoints;	// �v���W�F�N�^�摜��̑Ή��_

	// �J����
	cv::Mat cam_K;					// �����p�����[�^�s��
	cv::Mat cam_dist;				// �����Y�c��
	std::vector<cv::Mat> cam_R;		// ��]�x�N�g��
	std::vector<cv::Mat> cam_T;		// ���s�ړ��x�N�g��

	// �v���W�F�N�^
	cv::Mat proj_K;					// �����p�����[�^�s��
	cv::Mat proj_dist;				// �����Y�c��
	std::vector<cv::Mat> proj_R;	// ��]�x�N�g��
	std::vector<cv::Mat> proj_T;	// ���s�ړ��x�N�g��

	// �X�e���I�p�����[�^
	cv::Mat R;						// �J����-�v���W�F�N�^�Ԃ̉�]�s��
	cv::Mat T;						// �J����-�v���W�F�N�^�Ԃ̕��i�x�N�g��
	cv::Mat E;						// ��{�s��
	cv::Mat F;						// ��b�s��

	// �t���O
	bool calib_flag;

	int calib_count;				// �L�����u���[�V������
	std::string saveCalibFolder;		// �Ή��_��ۑ�����t�H���_

	// �O���C�R�[�h
	std::unique_ptr<GRAYCODE> graycode;

	CamDev::Camera *camDev;			// �J�����f�o�C�X
};


#endif