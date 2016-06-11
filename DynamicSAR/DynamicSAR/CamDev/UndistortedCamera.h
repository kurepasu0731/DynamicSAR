#pragma once

#include "CameraDevice.h"


namespace CamDev
{
	/**
	 * @brief   �c�ݕ␳����N���X
	 */
	class UndistortedCamera : public Camera
	{
	protected:
		bool ready;

		Camera *cam;			// �J�����N���X
		cv::Mat map1, map2;		// �c�ݕ␳�}�b�v

		// �B�e���\�b�h
		virtual bool getImage(cv::Mat &dst);

	public:

		// �R���X�g���N�^
		UndistortedCamera(Camera *cameraInstance, const std::string &cameraParamFileName);

		virtual ~UndistortedCamera(){}

		// �J�����̃C���X�^���X���擾
		Camera *getCameraInstance() { return cam; }

		// �c�ݕ␳�}�b�v���擾
		cv::Mat *getUndistortMap_x() { return &map1; }
		cv::Mat *getUndistortMap_y() { return &map2; }
	};
}