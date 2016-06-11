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
	 * @brief   Camera Link�ŉ摜���B�e����N���X
	 */
	class CameraLinkCamera : public Camera
	{
	protected:
		bool ready;

		int width, height;

		cv::Mat *latestImagePtr;		// �ŐV�̉摜�̃|�C���^(���C���X���b�h����̃A�N�Z�X�p)
		cv::Mat *threadImagePtr;		// �B�e�X���b�h����A�N�Z�X�p�̉摜�o�b�t�@�̃|�C���^
		cv::Mat img1, img2;				// �摜�o�b�t�@

		// �X���b�h
		boost::thread thread;

		// �~���[�e�b�N�X
		boost::shared_mutex mtx;

		//�X���b�h��~�t���O
		boost::atomic<bool> threadStopFlag;

	protected:

		// �B�e�X���b�h�֐�
		void thread_capture();


	public:
		// �R���X�g���N�^
		CameraLinkCamera(const std::string &fmtFilePath);
		virtual ~CameraLinkCamera();

		// �B�e���\�b�h
		bool getImage(cv::Mat &dst);

		int getWidth() { return width; }
		int getHeight() { return height; }
	};
}