#pragma once

#include <opencv2/opencv.hpp>


/**
 * @brief   カメラデバイスの抽象クラス
 */
namespace CamDev
{
	/**
	 * @brief   カメラデバイスはこれを継承する
	 */
	class Camera
	{
	protected:

		// 撮影メソッド
		virtual bool getImage(cv::Mat &dst) = 0;

	public:
		virtual ~Camera();

		// 画像を撮影
		bool captureImage(cv::Mat &dst);
	};
}