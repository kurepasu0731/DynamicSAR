#pragma once

#include "CameraDevice.h"

#include <Windows.h>
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <atomic>
#include <xcliball.h>


namespace CamDev
{

	/**
	 * @brief   Camera Linkで画像を撮影するクラス
	 */
	class CameraLinkCamera : public Camera
	{
	protected:
		bool ready;

		int width, height;

		cv::Mat *latestImagePtr;		// 最新の画像のポインタ(メインスレッドからのアクセス用)
		cv::Mat *threadImagePtr;		// 撮影スレッドからアクセス用の画像バッファのポインタ
		cv::Mat img1, img2;				// 画像バッファ

		// スレッド
		boost::thread thread;

		// ミューテックス
		boost::shared_mutex mtx;

		//スレッド停止フラグ
		boost::atomic<bool> threadStopFlag;

	protected:

		// 撮影スレッド関数
		void thread_capture();


	public:
		// コンストラクタ
		CameraLinkCamera(const std::string &fmtFilePath);
		virtual ~CameraLinkCamera();

		// 撮影メソッド
		bool getImage(cv::Mat &dst);

		int getWidth() { return width; }
		int getHeight() { return height; }
	};
}