#include "UndistortedCamera.h"


namespace CamDev
{
	/**
	 * @brief   �c�ݕ␳��̉摜���擾
	 *
	 * @param	dst[in,out]		�摜
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

		// ��x�B�e���ĉ摜�T�C�Y���擾
		cv::Mat img;
		if (!cameraInstance->captureImage(img))
		{
			std::cerr << "�J�����̎B�e�Ɏ��s" << std::endl;
			return;
		}

		// �J�����p�����[�^�̓ǂݍ���
		cv::FileStorage fs(cameraParamFileName, cv::FileStorage::READ);
		cv::FileNode node(fs.fs, NULL);
		cv::Mat intrinsic, distortion;

		read(node["cameraMatrix"], intrinsic);
		read(node["distCoeffs"], distortion);

		if (intrinsic.empty() || distortion.empty())
		{
			std::cerr << "�J�����p�����[�^�t�@�C���̓ǂݍ��݂Ɏ��s (" << cameraParamFileName << ")" << std::endl;
			return;
		}

		//�}�b�v�̍쐬
		initUndistortRectifyMap(intrinsic, distortion, cv::Mat(), intrinsic, cv::Size(img.cols, img.rows), CV_32FC1, map1, map2);

		ready = true;
	}
}