#ifndef DERECTION_THREAD_H
#define DERECTION_THREAD_H

#include "HoughFerns/HoughFerns.h"

#include "CriticalSection.h"

#include "CamDev/WebCamera.h"
#include "CamDev/CameraLinkCamera.h"
#include "CamDev/UndistortedCamera.h"

#include "GLSL/camera.h"

class DetectionThread
{
public:
	// �R���X�g���N�^
	DetectionThread(CriticalSection *cs, CamDev::CameraLinkCamera *cameraDevice)
	{
		// �f�o�C�X�̎󂯓n��
		irCamDev = cameraDevice;

		// �X���b�h�ԋ��L�N���X
		critical_section = cs;

		// Hough Ferns
		hough_ferns = std::unique_ptr<HoughFerns> (new HoughFerns);
	}

	// �f�X�g���N�^
	virtual ~DetectionThread(){}

	// ��������
	void init(const std::string &paramFile, const std::string &saveFolder);

	// ���o(���t���[������)
	void detection();

	// �ʒu�p������
	void resultPose(const std::vector<cv::Point3f> &detectPoint, const std::vector<glm::mat4> &detectPose, int width, int height);


	/***** �����o�ϐ� *****/

	//////////// Hough Ferns ////////////
	std::unique_ptr<HoughFerns> hough_ferns;
	std::vector<glm::mat4> detectPoseMat;	// ���o�����ʒu�p��	

	///// GL�I�u�W�F�N�g�p /////
	Camera camera;					// �J����

	///// �J�����f�o�C�X /////
	CamDev::CameraLinkCamera *irCamDev;		// IR�J����

	///// �X���b�h�Ԃ̋��L�N���X�p //////
	CriticalSection* critical_section;
};


#endif