#pragma once


#include "CameraDevice.h"

namespace CamDev
{
	/**
	 * @brief   WebƒJƒƒ‰‚Å‰æ‘œ‚ğB‰e‚·‚éƒNƒ‰ƒX
	 */
	class WebCamera: public Camera
	{
	private:
		cv::VideoCapture *cap;

	protected:

		// B‰eƒƒ\ƒbƒh
		bool getImage(cv::Mat &dst);

	public:
		WebCamera(int cameraIndex = 0, int width=-1, int height=-1);
		~WebCamera();
	};
}