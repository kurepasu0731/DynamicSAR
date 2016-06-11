#include "CameraLinkCamera.h"


namespace CamDev
{
	/**
	 * @brief   �R���X�g���N�^
	 *
	 * @param	fmtFilePath[in]		�J���������N��fmt�t�@�C��
	 */
	CameraLinkCamera::CameraLinkCamera(const std::string &fmtFilePath)
	{
		ready = false;
		width = height = 0;
		latestImagePtr = threadImagePtr = NULL;
		threadStopFlag = false;


		// PCI�o�X�S�ăI�[�v��
		#if !defined(UNITS)
		#define UNITS	1
		#endif
		#define UNITSMAP    ((1<<UNITS)-1)  /* shorthand - bitmap of all units */
		#if !defined(UNITSOPENMAP)
			#define UNITSOPENMAP UNITSMAP
		#endif

		#if !defined(DRIVERPARMS)
			#define DRIVERPARMS ""	  // default
		#endif

		//CameraLink������
		char driverparms[80];
		driverparms[sizeof(driverparms)-1] = 0;			// PCI�o�X�S�đI��
		_snprintf(driverparms, sizeof(driverparms)-1, "-DM 0x%x %s", UNITSOPENMAP, DRIVERPARMS);
		int openerr = pxd_PIXCIopen(driverparms, "", fmtFilePath.c_str());

		if (openerr != 0)
		{
			std::cerr << "Error: Camera Link Open Error (" << openerr << ")" << std::endl;
			return;
		}

		// �L���v�`���J�n
		pxd_goLiveSeq(0x1, 1, pxd_imageZdim(), 1, 0, 1);		//�����O�o�b�t�@���g�p

		//�摜�T�C�Y�擾
		width = pxd_imageXdim();
		height = pxd_imageYdim();

		//�摜�o�b�t�@�쐬
		img1 = cv::Mat(height, width, CV_8UC1, cv::Scalar(0));
		img2 = cv::Mat(height, width, CV_8UC1, cv::Scalar(0));
		latestImagePtr = &img1;
		threadImagePtr = &img2;


		// �B�e�X���b�h�J�n
		thread = boost::thread( &CameraLinkCamera::thread_capture, this);

		ready = true;
	}

	CameraLinkCamera::~CameraLinkCamera()
	{
		// �B�e�X���b�h�̏I��
		threadStopFlag = true;
		thread.join();

		// CameraLink�I��
		pxd_goUnLive(0x1);
		pxd_PIXCIclose();
	}


	/**
	 * @brief   �B�e�X���b�h
	 */
	void CameraLinkCamera::thread_capture()
	{
		pxbuffer_t latest_buffer = -1;

		while(!threadStopFlag)
		{
			// �ŐV�摜���L���v�`�������܂ő҂�
			if (pxd_capturedBuffer(0x1) == latest_buffer) {
				continue;
			}
			latest_buffer = pxd_capturedBuffer(0x1);

			// �O���C�X�P�[���f�[�^��1���C���փA�N�Z�X
			pxd_readuchar(0x1, latest_buffer, 0, 0, -1, -1, (unsigned char *)(threadImagePtr->data), threadImagePtr->size().area(), "Grey");


			// �ŐV�̉摜�Ɠ���ւ�
			boost::upgrade_lock<boost::shared_mutex> up_lock(mtx);
			boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
			{
				cv::Mat *tmp = latestImagePtr;
				latestImagePtr = threadImagePtr;
				threadImagePtr = tmp;
			}
		}
	}


	/**
	 * @brief   �摜���擾
	 *
	 * @param	dst[in,out]		�摜
	 */
	bool CameraLinkCamera::getImage(cv::Mat &dst)
	{
		boost::shared_lock<boost::shared_mutex> read_lock(mtx);

		if (!ready) {
			return false;
		}

		dst = latestImagePtr->clone();

		return true;
	}
}