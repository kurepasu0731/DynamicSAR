#include "CameraLinkCamera.h"


namespace CamDev
{
	/**
	 * @brief   コンストラクタ
	 *
	 * @param	fmtFilePath[in]		カメラリンクのfmtファイル
	 */
	CameraLinkCamera::CameraLinkCamera(const std::string &fmtFilePath)
	{
		ready = false;
		width = height = 0;
		latestImagePtr = threadImagePtr = NULL;
		threadStopFlag = false;


		// PCIバス全てオープン
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

		//CameraLink初期化
		char driverparms[80];
		driverparms[sizeof(driverparms)-1] = 0;			// PCIバス全て選択
		_snprintf(driverparms, sizeof(driverparms)-1, "-DM 0x%x %s", UNITSOPENMAP, DRIVERPARMS);
		int openerr = pxd_PIXCIopen(driverparms, "", fmtFilePath.c_str());

		if (openerr != 0)
		{
			std::cerr << "Error: Camera Link Open Error (" << openerr << ")" << std::endl;
			return;
		}

		// キャプチャ開始
		pxd_goLiveSeq(0x1, 1, pxd_imageZdim(), 1, 0, 1);		//リングバッファを使用

		//画像サイズ取得
		width = pxd_imageXdim();
		height = pxd_imageYdim();

		//画像バッファ作成
		img1 = cv::Mat(height, width, CV_8UC1, cv::Scalar(0));
		img2 = cv::Mat(height, width, CV_8UC1, cv::Scalar(0));
		latestImagePtr = &img1;
		threadImagePtr = &img2;


		// 撮影スレッド開始
		thread = boost::thread( &CameraLinkCamera::thread_capture, this);

		ready = true;
	}

	CameraLinkCamera::~CameraLinkCamera()
	{
		// 撮影スレッドの終了
		threadStopFlag = true;
		thread.join();

		// CameraLink終了
		pxd_goUnLive(0x1);
		pxd_PIXCIclose();
	}


	/**
	 * @brief   撮影スレッド
	 */
	void CameraLinkCamera::thread_capture()
	{
		pxbuffer_t latest_buffer = -1;

		while(!threadStopFlag)
		{
			// 最新画像がキャプチャされるまで待つ
			if (pxd_capturedBuffer(0x1) == latest_buffer) {
				continue;
			}
			latest_buffer = pxd_capturedBuffer(0x1);

			// グレイスケールデータの1ラインへアクセス
			pxd_readuchar(0x1, latest_buffer, 0, 0, -1, -1, (unsigned char *)(threadImagePtr->data), threadImagePtr->size().area(), "Grey");


			// 最新の画像と入れ替え
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
	 * @brief   画像を取得
	 *
	 * @param	dst[in,out]		画像
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