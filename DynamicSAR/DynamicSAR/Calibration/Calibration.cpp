#include "Calibration.h"
#include "../Projection_thread.h"


/**
 * @brief   グレイコード投影でProCam間の幾何対応を取得
 * 
 * @param	src[in]				入力画像(IRカメラ用)
 * @param	ProCam_src[in]		入力画像(ProCam用)
 * @param	projection[in,out]	投影用クラス
 */
void Calibration::runGetCorrespond(const cv::Mat& src, const cv::Mat& ProCam_src, ProjectionThread *projection)
{
	// チェッカーパターンの交点を描画(カメラ)
	std::vector<cv::Point2f> procam_imagePoint;
	std::vector<cv::Point2f> imagePoint;
	std::vector<cv::Point2f> projPoint;
	cv::Mat color_src, draw_corner, draw_corner2;
	if (src.channels() == 1)
		cv::cvtColor(src, color_src, CV_GRAY2BGR);
	else
		color_src = src.clone();
	bool detect_flag = getCorners(procam_imagePoint, ProCam_src, draw_corner);
	bool detect_flag2 = getCorners(imagePoint, color_src, draw_corner2);
	cv::namedWindow("Image Corner", cv::WINDOW_NORMAL);
	cv::imshow( "Image Corner", draw_corner);
	cv::namedWindow("IR Image Corner", cv::WINDOW_NORMAL);
	cv::imshow( "IR Image Corner", draw_corner2);

	// カメラ上でチェッカーパターンを検出できたら
	if (detect_flag && detect_flag2)
	{
		// グレイコード投影
		graycode->code_projection(projection);
		graycode->make_thresh();
		graycode->makeCorrespondence();


		// チェッカーパターンの交点を描画(プロジェクタ)
		cv::Mat dst;
		graycode->getCorrespondSubPixelProjPoints(projPoint, procam_imagePoint, 20);
		graycode->transport_camera_projector(ProCam_src, dst);

		if(procam_imagePoint.size() == projPoint.size()) {
			cv::drawChessboardCorners( dst, cornerSize, projPoint, true );
		} else {
			cv::drawChessboardCorners( dst, cornerSize, projPoint, false );
		}

		cv::namedWindow("prj", cv::WINDOW_NORMAL);
		cv::imshow("prj",dst);
		cv::imwrite("./prj.jpg", dst );

		// チェッカーパターンの交点を投影
		projection->img_projection(dst);

		Sleep(3000);

		// プロジェクタ上でチェッカーパターンを検出できたら
		if(procam_imagePoint.size() == projPoint.size())
		{
			std::cout << calib_count+1 << "回目の検出成功" << std::endl;

			// 追加
			myWorldPoints.emplace_back(worldPoint);
			myCameraPoints.emplace_back(imagePoint);
			myProjectorPoints.emplace_back(projPoint);

			// 検出点の保存
			std::string fileName = saveCalibFolder + "/checkerPoint" + std::to_string(calib_count) + ".xml";
			cv::FileStorage fs(fileName, cv::FileStorage::WRITE);
			cv::write(fs,"world", worldPoint);
			cv::write(fs,"camera", imagePoint);
			cv::write(fs,"projector", projPoint);

			calib_count++;
		}else{
			std::cout << "プロジェクタ上のチェッカーパターンの検出失敗" << std::endl;
		}
	} else {
		std::cout << "カメラ上のチェッカーパターンの検出失敗" << std::endl;
	}
}


/**
 * @brief   対応点のファイルからProCamキャリブレーション	
 *
 * @param   camSize[in]				カメラのサイズ
 * @param   projSize[in]			プロジェクタのサイズ
 */
void Calibration::runCorrespondFileCalib(const cv::Size &camSize, const cv::Size &projSize)
{
	myWorldPoints.clear();
	myCameraPoints.clear();
	myProjectorPoints.clear();

	// ファイルの探索
	WIN32_FIND_DATA ffd;
	HANDLE hF;

	std::string imageFlieName = saveCalibFolder+"/checkerPoint*.*";
	hF = FindFirstFile( imageFlieName.c_str(), &ffd);
	if (hF != INVALID_HANDLE_VALUE) {
		// フォルダ内のファイルの探索
		do {
			std::string fullpath = ffd.cFileName;

			std::vector<cv::Point3f> worldPoint;
			std::vector<cv::Point2f> imagePoint;
			std::vector<cv::Point2f> projPoint;

			// xmlファイルの読み込み
			cv::FileStorage cvfs(fullpath, cv::FileStorage::READ);

			cvfs["world"] >> worldPoint;
			cvfs["camera"] >> imagePoint;
			cvfs["projector"] >> projPoint;
						
			// 追加
			myWorldPoints.emplace_back(worldPoint);
			myCameraPoints.emplace_back(imagePoint);
			myProjectorPoints.emplace_back(projPoint);

		} while (FindNextFile(hF, &ffd ) != 0);
		FindClose(hF);
	}

	if(myWorldPoints.size() > 0)
	{
		std::cout << "キャリブレーション中…" << std::endl;

		// キャリブレーション
		proCamCalibration(myWorldPoints, myCameraPoints, myProjectorPoints, camSize, projSize);

	}
}



/**
 * @brief   画像からチェッカーパターンの交点を取得
 * 
 * @param   imagePoint[in,out]		画像の座標値
 * @param   image[in]				画像
 * @param   draw_image[in,out]		交点描画画像
 */
bool Calibration::getCorners(std::vector<cv::Point2f> &imagePoint, const cv::Mat &image, cv::Mat &draw_image)
{
	// 交点検出
	bool detect;

	// チェスボードの場合
	if (use_chessboard)
		detect = cv::findChessboardCorners( image, cornerSize, imagePoint);
	else
		detect = cv::findCirclesGrid( image, cornerSize, imagePoint, cv::CALIB_CB_ASYMMETRIC_GRID );

	// 検出点の描画
	image.copyTo(draw_image);
	if(detect) {

		// サブピクセル推定
		cv::Mat	gray;
		if(image.channels() == 3) {
			cv::cvtColor( image, gray, cv::COLOR_BGR2GRAY );
		} else {
			gray = image.clone();
		}

		cv::Size winSize;
		// 解像度によってサブピクセル推定の探索範囲を変える
		if (image.cols > 640) {
			winSize = cv::Size(11, 11);
		} else {
			winSize = cv::Size(3, 3);
		}
		if (use_chessboard)
			cv::cornerSubPix( gray, imagePoint, winSize, cv::Size( -1, -1 ), cv::TermCriteria( cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 20, 0.001 ) );

		cv::drawChessboardCorners( draw_image, cornerSize, imagePoint, true );
	} else {
		cv::drawChessboardCorners( draw_image, cornerSize, imagePoint, false );
	}

	return detect;
}


/**
 * @brief   再投影誤差の計算
 * 
 * @param   worldPoints[in]			世界座標値
 * @param   cameraPoints[in]		カメラ座標値
 * @param   projectorPoints[in]		プロジェクタ座標値
 * @param   cam_error[in,out]		カメラの再投影誤差
 * @param   proj_error[in,out]		プロジェクタの再投影誤差
 */
void Calibration::calcReprojectionError(const std::vector<std::vector<cv::Point3f>> &worldPoints, const std::vector<std::vector<cv::Point2f>> &cameraPoints, const std::vector<std::vector<cv::Point2f>> &projectorPoints,
											double &cam_error, double &proj_error)
{
	cv::Mat camera_R = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat camera_T = cv::Mat::zeros(3, 1, CV_64F);

	// カメラの再投影誤差
	for(int i=0; i<worldPoints.size(); ++i)
	{
		cv::Mat rvec, tvec;
		cv::solvePnP(worldPoints[i], cameraPoints[i], cam_K, cam_dist, rvec, tvec);		// チェッカーパターンの位置検出

		cv::Mat rmat;
		cv::Rodrigues(rvec, rmat);

		// チェッカーパターン中心からカメラ中心に座標変換
		rmat = rmat.t();	// 転置行列

		cv::Mat extrinsic(4, 4, CV_64F);
		extrinsic.at<double>(0,0) = rmat.at<double>(0,0);
		extrinsic.at<double>(0,1) = rmat.at<double>(0,1);
		extrinsic.at<double>(0,2) = rmat.at<double>(0,2);
		extrinsic.at<double>(1,0) = rmat.at<double>(1,0);
		extrinsic.at<double>(1,1) = rmat.at<double>(1,1);
		extrinsic.at<double>(1,2) = rmat.at<double>(1,2);
		extrinsic.at<double>(2,0) = rmat.at<double>(2,0);
		extrinsic.at<double>(2,1) = rmat.at<double>(2,1);
		extrinsic.at<double>(2,2) = rmat.at<double>(2,2);
		extrinsic.at<double>(0,3) = cv::Mat(-rmat*tvec).at<double>(0,0);
		extrinsic.at<double>(1,3) = cv::Mat(-rmat*tvec).at<double>(1,0);
		extrinsic.at<double>(2,3) = cv::Mat(-rmat*tvec).at<double>(2,0);
		extrinsic.at<double>(3,0) = 0.0;
		extrinsic.at<double>(3,1) = 0.0;
		extrinsic.at<double>(3,2) = 0.0;
		extrinsic.at<double>(3,3) = 1.0;

		// チェッカーパターンの交点位置
		std::vector<cv::Point3f> new_worldPoint;
		for(int j=0; j<worldPoints[0].size(); ++j)
		{
			cv::Mat checker_pos = extrinsic.inv() * cv::Mat((cv::Mat_<double>(4,1) << worldPoints[i][j].x, worldPoints[i][j].y, worldPoints[i][j].z, 1.0));		// チェッカーパターンの位置
			new_worldPoint.emplace_back(cv::Point3f(checker_pos.at<double>(0)/checker_pos.at<double>(3), checker_pos.at<double>(1)/checker_pos.at<double>(3), checker_pos.at<double>(2)/checker_pos.at<double>(3)));
		}

		// カメラ座標への投影
		std::vector<cv::Point2f> cam_projection;
		cv::projectPoints(new_worldPoint, camera_R, camera_T, cam_K, cam_dist, cam_projection);

		// プロジェクタ座標への投影
		std::vector<cv::Point2f> proj_projection;
		cv::projectPoints(new_worldPoint, R, T, proj_K, proj_dist, proj_projection);

		// カメラ座標への再投影誤差
		for(int j=0; j<cameraPoints[0].size(); ++j)
		{
			cam_error += std::sqrt((cameraPoints[i][j].x - cam_projection[j].x)*(cameraPoints[i][j].x - cam_projection[j].x) + (cameraPoints[i][j].y - cam_projection[j].y)*(cameraPoints[i][j].y - cam_projection[j].y));
		}

		// プロジェクタ座標への再投影誤差
		for(int j=0; j<projectorPoints[0].size(); ++j)
		{
			proj_error += std::sqrt((projectorPoints[i][j].x - proj_projection[j].x)*(projectorPoints[i][j].x - proj_projection[j].x) + (projectorPoints[i][j].y - proj_projection[j].y)*(projectorPoints[i][j].y - proj_projection[j].y));
		}
	}

	double sum = worldPoints.size() * worldPoints[0].size();

	cam_error /= sum;
	proj_error /= sum;
}



/**
 * @brief   プロジェクタとカメラのキャリブレーション
 * 
 * @param   worldPoints[in]			世界座標値
 * @param   cameraPoints[in]		カメラ座標値
 * @param   projectorPoints[in]		プロジェクタ座標値
 * @param   camSize[in]				カメラのサイズ
 * @param   projSize[in]			プロジェクタのサイズ
 */
void Calibration::proCamCalibration(const std::vector<std::vector<cv::Point3f>> &worldPoints, const std::vector<std::vector<cv::Point2f>> &cameraPoints, const std::vector<std::vector<cv::Point2f>> &projectorPoints,
										const cv::Size &camSize, const cv::Size &projSize)
{
	// カメラキャリブレーション
	double cam_error = cv::calibrateCamera(worldPoints, cameraPoints, camSize, cam_K, cam_dist, cam_R, cam_T, cv::CALIB_FIX_K3, 
									cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 200, DBL_EPSILON));

	// プロジェクタキャリブレーション
	double proj_error = cv::calibrateCamera(worldPoints, projectorPoints, projSize, proj_K, proj_dist, proj_R, proj_T, cv::CALIB_FIX_K3, 
									cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 200, DBL_EPSILON));

	// ステレオ最適化
	double stereo_error = cv::stereoCalibrate(worldPoints, cameraPoints, projectorPoints, cam_K, cam_dist, proj_K, proj_dist, camSize, R, T, E, F, 
                                                cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 200, DBL_EPSILON), cv::CALIB_FIX_INTRINSIC /*cv::CALIB_USE_INTRINSIC_GUESS*/+cv::CALIB_FIX_K3);

	// 最適化後の再投影誤差の計算
	double cam_error2 = 0;
	double proj_error2 = 0;
	calcReprojectionError(worldPoints, cameraPoints, projectorPoints, cam_error2, proj_error2);
	
	// 結果
	std::cout << "***** Calibration results *****" << std::endl << std::endl;

	std::cout	<< "Camera Calibration results:" << std::endl
				<< " - Reprojection error: " << cam_error << std::endl
				<< " - Reprojection error2: " << cam_error2 << std::endl
				<< " - K:\n" << cam_K << std::endl
				<< " - Distortion:" << cam_dist << std::endl << std::endl;

	std::cout	<< "Projector Calibration results:" << std::endl
				<< " - Reprojection error: " << proj_error << std::endl
				<< " - Reprojection error2: " << proj_error2 << std::endl
				<< " - K:\n" << proj_K << std::endl
				<< " - Distortion:" << proj_dist << std::endl << std::endl;

	std::cout	<< "Stereo Calibration results:" << std::endl
				<< " - Reprojection error: " << stereo_error << std::endl
				<< " - R:\n" << R << std::endl
				<< " - T:" << T << std::endl << std::endl;


	// 結果の保存
	cv::FileStorage fs(saveCalibFolder+"/calibration.xml", cv::FileStorage::WRITE);
	fs << "cam_reprojection_error" << cam_error
	   << "cam_reprojection_error2" << cam_error2
	   << "proj_reprojection_error" << proj_error
	   << "proj_reprojection_error2" << proj_error2
	   << "stereo_reprojection_error" << stereo_error
	   << "cam_K" << cam_K << "cam_dist" << cam_dist
	   << "cam_R" << cam_R << "cam_T" << cam_T
       << "proj_K" << proj_K << "proj_dist" << proj_dist
       << "proj_R" << proj_R << "proj_T" << proj_T
	   << "R" << R << "T" << T << "E" << E << "F" << F;
	fs.release();


	// カメラパラメータの保存
	cv::FileStorage fs2(saveCalibFolder+"/camera.xml", cv::FileStorage::WRITE);

	fs2 << "cameraMatrix" << cam_K;
	fs2 << "distCoeffs" << cam_dist;
	fs2 << "imageWidth" << camSize.width;
	fs2 << "imageHeight" << camSize.height;

	calib_flag = true;
}


/**
 * @brief   キャリブレーション結果の読み込み
 * 
 * @param   fileName[in]		キャリブレーションファイル
 */
void Calibration::loadCalibParam(const std::string &fileName)
{
	// xmlファイルの読み込み
	cv::FileStorage cvfs(fileName, cv::FileStorage::READ);

	cvfs["cam_K"] >> cam_K;
	cvfs["cam_dist"] >> cam_dist;
	cvfs["cam_R"] >> cam_R;
	cvfs["cam_T"] >> cam_T;
	cvfs["proj_K"] >> proj_K;
	cvfs["proj_dist"] >> proj_dist;
	cvfs["proj_R"] >> proj_R;
	cvfs["proj_T"] >> proj_T;
	cvfs["R"] >> R;
	cvfs["T"] >> T;
	cvfs["E"] >> E;
	cvfs["F"] >> F;

	calib_flag = true;
}


/**
 * @brief   透視投影変換行列の取得(カメラ)	
 */
cv::Mat Calibration::getCamPerspectiveMat()
{
	// 回転と並進を結合
	cv::Mat extrinsic = cv::Mat::eye(4, 4, CV_64F);

	// 内部パラメータの変形
	cv::Mat intrinsic(3, 4, CV_64F);
	intrinsic.at<double>(0,0) = cam_K.at<double>(0,0);
	intrinsic.at<double>(0,1) = cam_K.at<double>(0,1);
	intrinsic.at<double>(0,2) = cam_K.at<double>(0,2);
	intrinsic.at<double>(1,0) = cam_K.at<double>(1,0);
	intrinsic.at<double>(1,1) = cam_K.at<double>(1,1);
	intrinsic.at<double>(1,2) = cam_K.at<double>(1,2);
	intrinsic.at<double>(2,0) = cam_K.at<double>(2,0);
	intrinsic.at<double>(2,1) = cam_K.at<double>(2,1);
	intrinsic.at<double>(2,2) = cam_K.at<double>(2,2);
	intrinsic.at<double>(0,3) = 0.0;
	intrinsic.at<double>(1,3) = 0.0;
	intrinsic.at<double>(2,3) = 0.0;

	return intrinsic * extrinsic;
}


/**
 * @brief   透視投影変換行列の取得(プロジェクタ)	
 */
cv::Mat Calibration::getProjPerspectiveMat()
{
	// 回転と並進を結合
	cv::Mat extrinsic(4, 4, CV_64F);
	extrinsic.at<double>(0,0) = R.at<double>(0,0);
	extrinsic.at<double>(0,1) = R.at<double>(0,1);
	extrinsic.at<double>(0,2) = R.at<double>(0,2);
	extrinsic.at<double>(1,0) = R.at<double>(1,0);
	extrinsic.at<double>(1,1) = R.at<double>(1,1);
	extrinsic.at<double>(1,2) = R.at<double>(1,2);
	extrinsic.at<double>(2,0) = R.at<double>(2,0);
	extrinsic.at<double>(2,1) = R.at<double>(2,1);
	extrinsic.at<double>(2,2) = R.at<double>(2,2);
	extrinsic.at<double>(0,3) = T.at<double>(0,0);
	extrinsic.at<double>(1,3) = T.at<double>(1,0);
	extrinsic.at<double>(2,3) = T.at<double>(2,0);
	extrinsic.at<double>(3,0) = 0.0;
	extrinsic.at<double>(3,1) = 0.0;
	extrinsic.at<double>(3,2) = 0.0;
	extrinsic.at<double>(3,3) = 1.0;

	// 内部パラメータの変形
	cv::Mat intrinsic(3, 4, CV_64F);
	intrinsic.at<double>(0,0) = proj_K.at<double>(0,0);
	intrinsic.at<double>(0,1) = proj_K.at<double>(0,1);
	intrinsic.at<double>(0,2) = proj_K.at<double>(0,2);
	intrinsic.at<double>(1,0) = proj_K.at<double>(1,0);
	intrinsic.at<double>(1,1) = proj_K.at<double>(1,1);
	intrinsic.at<double>(1,2) = proj_K.at<double>(1,2);
	intrinsic.at<double>(2,0) = proj_K.at<double>(2,0);
	intrinsic.at<double>(2,1) = proj_K.at<double>(2,1);
	intrinsic.at<double>(2,2) = proj_K.at<double>(2,2);
	intrinsic.at<double>(0,3) = 0.0;
	intrinsic.at<double>(1,3) = 0.0;
	intrinsic.at<double>(2,3) = 0.0;

	return intrinsic * extrinsic;
}



/**
 * @brief   カメラ位置をワールド座標とした際の対象物体の位置の取得
 * 
 * @param   camWorldPoint[in,out]		ワールド座標値
 * @param   imagePoint[in]				画像座標値
 */
void Calibration::getCameraWorldPoint(std::vector<cv::Point3f> &camWorldPoint, const std::vector<cv::Point2f> &imagePoint)
{
	cv::Mat rvec, tvec, rmat;

	// チェッカーパターンの位置検出
	cv::solvePnP(worldPoint, imagePoint, cam_K, cv::Mat(), rvec, tvec);		

	cv::Rodrigues(rvec, rmat);		// 回転行列に変換

	// チェッカーパターン中心からカメラ中心に座標変換
	rmat = rmat.t();	// 転置行列

	cv::Mat extrinsic(4, 4, CV_64F);
	extrinsic.at<double>(0,0) = rmat.at<double>(0,0);
	extrinsic.at<double>(0,1) = rmat.at<double>(0,1);
	extrinsic.at<double>(0,2) = rmat.at<double>(0,2);
	extrinsic.at<double>(1,0) = rmat.at<double>(1,0);
	extrinsic.at<double>(1,1) = rmat.at<double>(1,1);
	extrinsic.at<double>(1,2) = rmat.at<double>(1,2);
	extrinsic.at<double>(2,0) = rmat.at<double>(2,0);
	extrinsic.at<double>(2,1) = rmat.at<double>(2,1);
	extrinsic.at<double>(2,2) = rmat.at<double>(2,2);
	extrinsic.at<double>(0,3) = cv::Mat(-rmat*tvec).at<double>(0,0);
	extrinsic.at<double>(1,3) = cv::Mat(-rmat*tvec).at<double>(1,0);
	extrinsic.at<double>(2,3) = cv::Mat(-rmat*tvec).at<double>(2,0);
	extrinsic.at<double>(3,0) = 0.0;
	extrinsic.at<double>(3,1) = 0.0;
	extrinsic.at<double>(3,2) = 0.0;
	extrinsic.at<double>(3,3) = 1.0;

	// チェッカーパターンの交点位置
	for(int i=0; i<worldPoint.size(); ++i)
	{
		cv::Mat checker_pos = extrinsic.inv() * cv::Mat((cv::Mat_<double>(4,1) << worldPoint[i].x, worldPoint[i].y, worldPoint[i].z, 1.0));		// チェッカーパターンの位置
		camWorldPoint.emplace_back(cv::Point3f(checker_pos.at<double>(0)/checker_pos.at<double>(3), checker_pos.at<double>(1)/checker_pos.at<double>(3), checker_pos.at<double>(2)/checker_pos.at<double>(3)));
	}
}