#include "Detection_thread.h"



/**
 * @brief   初期処理
 * 
 * @param   paramFile[in]	パラメータファイル	
 * @param   saveFolder[in]	Fernsのフォルダ
 */
void DetectionThread::init(const std::string &paramFile, const std::string &saveFolder)
{

	// パラメータの読み込み
	hough_ferns->initFromFile(paramFile);
	hough_ferns->loadFerns(saveFolder);

	critical_section->ready_detect_flag = true;
}


/**
 * @brief   検出		
 */
void DetectionThread::detection()
{
	// 入力画像
	cv::Mat cam_img;
	irCamDev->getImage(cam_img);

	// 歪み補正
	if(critical_section->use_calib_flag)
	{
		cv::Mat map1, map2;
		critical_section->getUndistortMap(map1, map2);
		cv::remap(cam_img, cam_img, map1, map2, cv::INTER_LINEAR);
	}

	///// 位置姿勢推定処理 /////
	detectPoseMat.clear();

	// 検出パラメータの取得
	float min_distance1;				// 1層目の考慮する最小距離
	float max_distance1;				// 1層目の考慮する最大距離
	int distance_step;					// 1層目の距離変化のステップ数
	float min_scale2;					// 2層目の考慮する最小サイズ
	float max_scale2;					// 2層目の考慮する最大サイズ
	int scale_step;						// 2層目のスケールのステップ数
	int detect_width;					// 2層目の検出する矩形領域
	int detect_height;					// 2層目の検出する矩形領域
	float grad_th;						// 輝度勾配の閾値
	int meanshift_radius;				// Mean Shiftのカーネル幅
	int win_th;							// Nearest Neighborで統合するサイズ
	double likelihood_th;				// 投票による尤度の閾値

	critical_section->getDetectParams(min_distance1, max_distance1, distance_step, min_scale2, max_scale2, scale_step,
									detect_width, detect_height, grad_th, meanshift_radius, win_th, likelihood_th);

	// Hough Fernsによる検出
	hough_ferns->detect(cam_img, min_distance1, max_distance1, distance_step, min_scale2, max_scale2, scale_step,
									detect_width, detect_height, grad_th, meanshift_radius, win_th, likelihood_th);

	// 位置姿勢結果
	resultPose( hough_ferns->detectPoint, hough_ferns->pose_estimate,  cam_img.cols, cam_img.rows);

	// 結果のセット
	critical_section->setDetectPoseMatrix(detectPoseMat);

	// トラッキング開始
	if(critical_section->detect_tracking_flag)
	{
		critical_section->tracking_flag = true;
	}
}



/**
 * @brief   位置姿勢検出結果の描画
 * 
 * @param   detectPoint[in]		検出座標
 * @param   detectPose[in]		検出した位置に対する姿勢
 * @param   width[in]			ウィンドウの幅
 * @param   height[in]			ウィンドウの高さ	
 */
void DetectionThread::resultPose(const std::vector<cv::Point3f> &detectPoint, const std::vector<glm::mat4> &detectPose, int width, int height)
{
	// 位置姿勢推定結果を描画
	for (int i = 0; i < (int)detectPoint.size(); ++i)
	{
		// 姿勢
		glm::mat4 estimate = detectPose[i];

		// xy座標をワールド座標に変換
		glm::mat4 projection = critical_section->getCameraIntrinsicMatrix();
		glm::mat4 view(1.0f);

		// ビューポート逆変換
		glm::vec4 device_vec;
		device_vec.w = detectPoint[i].z;
		device_vec.x = device_vec.w * (float)( (2.0 * detectPoint[i].x / width) - 1.0);
		device_vec.y = device_vec.w * (float)( (2.0 * (height-detectPoint[i].y) / height) - 1.0);
		device_vec.z = (float)( (camera.getNear()+camera.getFar())*device_vec.w - 2*camera.getNear()*camera.getFar()) / (camera.getNear() - camera.getFar());

		// 正規化デバイス座標系からワールド座標系へ変換
		glm::vec4 world_vec = glm::inverse(view) * glm::inverse(projection) * device_vec;

		estimate[3][0] = world_vec.x;
		estimate[3][1] = world_vec.y;
		estimate[3][2] = world_vec.z;


		// 追加
		detectPoseMat.emplace_back(estimate);
	}
}