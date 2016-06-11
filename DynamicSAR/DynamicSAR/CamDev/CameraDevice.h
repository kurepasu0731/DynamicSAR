#pragma once

#include <opencv2/opencv.hpp>


/**
 * @brief   �J�����f�o�C�X�̒��ۃN���X
 */
namespace CamDev
{
	/**
	 * @brief   �J�����f�o�C�X�͂�����p������
	 */
	class Camera
	{
	protected:

		// �B�e���\�b�h
		virtual bool getImage(cv::Mat &dst) = 0;

	public:
		virtual ~Camera();

		// �摜���B�e
		bool captureImage(cv::Mat &dst);
	};
}