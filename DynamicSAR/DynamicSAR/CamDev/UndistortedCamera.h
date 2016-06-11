#pragma once

#include "CameraDevice.h"


namespace CamDev
{
	/**
	 * @brief   歪み補正するクラス
	 */
	class UndistortedCamera : public Camera
	{
	protected:
		bool ready;

		Camera *cam;			// カメラクラス
		cv::Mat map1, map2;		// 歪み補正マップ

		// 撮影メソッド
		virtual bool getImage(cv::Mat &dst);

	public:

		// コンストラクタ
		UndistortedCamera(Camera *cameraInstance, const std::string &cameraParamFileName);

		virtual ~UndistortedCamera(){}

		// カメラのインスタンスを取得
		Camera *getCameraInstance() { return cam; }

		// 歪み補正マップを取得
		cv::Mat *getUndistortMap_x() { return &map1; }
		cv::Mat *getUndistortMap_y() { return &map2; }
	};
}