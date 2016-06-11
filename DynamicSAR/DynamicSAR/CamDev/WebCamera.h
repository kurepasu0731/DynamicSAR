#pragma once


#include "CameraDevice.h"

namespace CamDev
{
	/**
	 * @brief   Web�J�����ŉ摜���B�e����N���X
	 */
	class WebCamera: public Camera
	{
	private:
		cv::VideoCapture *cap;

	protected:

		// �B�e���\�b�h
		bool getImage(cv::Mat &dst);

	public:
		WebCamera(int cameraIndex = 0, int width=-1, int height=-1);
		~WebCamera();
	};
}