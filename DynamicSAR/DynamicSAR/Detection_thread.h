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
	// コンストラクタ
	DetectionThread(CriticalSection *cs, CamDev::CameraLinkCamera *cameraDevice)
	{
		// デバイスの受け渡し
		irCamDev = cameraDevice;

		// スレッド間共有クラス
		critical_section = cs;

		// Hough Ferns
		hough_ferns = std::unique_ptr<HoughFerns> (new HoughFerns);
	}

	// デストラクタ
	virtual ~DetectionThread(){}

	// 初期処理
	void init(const std::string &paramFile, const std::string &saveFolder);

	// 検出(毎フレーム処理)
	void detection();

	// 位置姿勢結果
	void resultPose(const std::vector<cv::Point3f> &detectPoint, const std::vector<glm::mat4> &detectPose, int width, int height);


	/***** メンバ変数 *****/

	//////////// Hough Ferns ////////////
	std::unique_ptr<HoughFerns> hough_ferns;
	std::vector<glm::mat4> detectPoseMat;	// 検出した位置姿勢	

	///// GLオブジェクト用 /////
	Camera camera;					// カメラ

	///// カメラデバイス /////
	CamDev::CameraLinkCamera *irCamDev;		// IRカメラ

	///// スレッド間の共有クラス用 //////
	CriticalSection* critical_section;
};


#endif