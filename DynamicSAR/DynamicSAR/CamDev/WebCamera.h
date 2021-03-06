#pragma once


#include "CameraDevice.h"

namespace CamDev
{
	/**
	 * @brief   Webカメラで画像を撮影するクラス
	 */
	class WebCamera: public Camera
	{
	private:
		cv::VideoCapture *cap;

	protected:

		// 撮影メソッド
		bool getImage(cv::Mat &dst);

	public:
		WebCamera(int cameraIndex = 0, int width=-1, int height=-1);
		~WebCamera();
	};
}