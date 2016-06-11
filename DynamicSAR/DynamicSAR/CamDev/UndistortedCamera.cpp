#include "UndistortedCamera.h"


namespace CamDev
{
	/**
	 * @brief   歪み補正後の画像を取得
	 *
	 * @param	dst[in,out]		画像
	 */
	bool UndistortedCamera::getImage(cv::Mat &dst)
	{
		if (!ready) {
			return false;
		}

		if (!cam->captureImage(dst)) {
			return false;
		}
		remap(dst, dst, map1, map2, cv::INTER_LINEAR);

		return true;
	}

	UndistortedCamera::UndistortedCamera(Camera *cameraInstance, const std::string &cameraParamFileName)
	{
		ready = false;
		cam = cameraInstance;

		// 一度撮影して画像サイズを取得
		cv::Mat img;
		if (!cameraInstance->captureImage(img))
		{
			std::cerr << "カメラの撮影に失敗" << std::endl;
			return;
		}

		// カメラパラメータの読み込み
		cv::FileStorage fs(cameraParamFileName, cv::FileStorage::READ);
		cv::FileNode node(fs.fs, NULL);
		cv::Mat intrinsic, distortion;

		read(node["cameraMatrix"], intrinsic);
		read(node["distCoeffs"], distortion);

		if (intrinsic.empty() || distortion.empty())
		{
			std::cerr << "カメラパラメータファイルの読み込みに失敗 (" << cameraParamFileName << ")" << std::endl;
			return;
		}

		//マップの作成
		initUndistortRectifyMap(intrinsic, distortion, cv::Mat(), intrinsic, cv::Size(img.cols, img.rows), CV_32FC1, map1, map2);

		ready = true;
	}
}