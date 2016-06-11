#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Windows.h>
#include <opencv2/opencv.hpp>
#include "Graycode.h"

#include "../CamDev/WebCamera.h"
#include "../CamDev/CameraLinkCamera.h"
#include "../CamDev/UndistortedCamera.h"


class ProjectionThread;



/**
 * @brief   GrayCodeを用いたプロジェクタとカメラのキャリブレーション
 */
class Calibration
{
public:
	Calibration(CamDev::Camera *camera, int _prj_width, int _prj_height, int _cam_width, int _cam_height, bool _use_chessboard, int _cornerCol, int _cornerRow, float _cornerInterval_m, double _delay=80.0, std::string _saveCalibFolder=".")
		: cornerSize (cv::Size(_cornerCol, _cornerRow))
		, cornerInterval_m (_cornerInterval_m)
		, saveCalibFolder (_saveCalibFolder)
		, calib_flag (false)
		, calib_count (0)
	{
		// デバイスの受け渡し
		camDev = camera;

		// グレイコードクラス
		graycode = std::unique_ptr<GRAYCODE>( new GRAYCODE(camDev, _prj_width, _prj_height, _cam_width, _cam_height, _delay, "../../"));

		// 世界座標におけるチェッカーパターンの交点座標を決定
		// チェスボードの場合
		if (use_chessboard)
		{
			for( int i = 0; i < cornerSize.area(); ++i ) {
				worldPoint.push_back( cv::Point3f(	static_cast<float>( i % cornerSize.width * cornerInterval_m ),
														static_cast<float>( i / cornerSize.width * cornerInterval_m ), 0.0 ) );
			}
		}
		// サークルグリッドの場合
		else
		{
			for (int j = 0; j < _cornerRow; j++)
			{
				for (int i = 0; i < _cornerCol; i++)
				{
					worldPoint.push_back(cv::Point3f(static_cast<float>((2*i + j%2) * cornerInterval_m), static_cast<float>(j * cornerInterval_m), 0.0));
				}
			}
		}
	}

	virtual ~Calibration(){}


	// グレイコード投影でProCam間の幾何対応を取得
	void runGetCorrespond(const cv::Mat& src, const cv::Mat& ProCam_src, ProjectionThread *projection);

	// 対応点のファイルからProCamキャリブレーション
	void runCorrespondFileCalib(const cv::Size &camSize, const cv::Size &projSize);


	// 画像からチェッカーパターンの交点を取得
	bool getCorners(std::vector<cv::Point2f> &imagePoint, const cv::Mat &image, cv::Mat &draw_image);		// 画像からチェッカーパターンの交点を取得

	// 再投影誤差の計算
	void calcReprojectionError(const std::vector<std::vector<cv::Point3f>> &worldPoints, const std::vector<std::vector<cv::Point2f>> &cameraPoints, const std::vector<std::vector<cv::Point2f>> &projectorPoints,double &cam_error, double &proj_error);

	// プロジェクタとカメラのキャリブレーション
	void proCamCalibration(const std::vector<std::vector<cv::Point3f>> &worldPoints, const std::vector<std::vector<cv::Point2f>> &cameraPoints, const std::vector<std::vector<cv::Point2f>> &projectorPoints,
							const cv::Size &camSize, const cv::Size &projSize);

	// キャリブレーション結果の読み込み
	void loadCalibParam(const std::string &fileName);

	// 透視投影変換行列の取得(カメラ)
	cv::Mat getCamPerspectiveMat();
	// 透視投影変換行列の取得(プロジェクタ)
	cv::Mat getProjPerspectiveMat();

	// カメラ位置をワールド座標とした際の対象物体の位置の取得
	void getCameraWorldPoint(std::vector<cv::Point3f> &camWorldPoint, const std::vector<cv::Point2f> &imagePoint);

	// 保存先フォルダのセット
	inline void setSaveFolder(std::string& folder){ saveCalibFolder = folder; }


	/***** メンバ変数 *****/
	bool use_chessboard;		// true:チェスボード, false:サークルグリッド
	cv::Size cornerSize;		// コーナー数
	float cornerInterval_m;		// コーナーの間隔の大きさ(m)

	std::vector<cv::Point3f> worldPoint;						// チェッカー交点座標と対応する世界座標の値を格納する行列

	std::vector<std::vector<cv::Point3f>> myWorldPoints;		// 世界座標の点
	std::vector<std::vector<cv::Point2f>> myCameraPoints;		// カメラ画像上の対応点
	std::vector<std::vector<cv::Point2f>> myProjectorPoints;	// プロジェクタ画像上の対応点

	// カメラ
	cv::Mat cam_K;					// 内部パラメータ行列
	cv::Mat cam_dist;				// レンズ歪み
	std::vector<cv::Mat> cam_R;		// 回転ベクトル
	std::vector<cv::Mat> cam_T;		// 平行移動ベクトル

	// プロジェクタ
	cv::Mat proj_K;					// 内部パラメータ行列
	cv::Mat proj_dist;				// レンズ歪み
	std::vector<cv::Mat> proj_R;	// 回転ベクトル
	std::vector<cv::Mat> proj_T;	// 平行移動ベクトル

	// ステレオパラメータ
	cv::Mat R;						// カメラ-プロジェクタ間の回転行列
	cv::Mat T;						// カメラ-プロジェクタ間の並進ベクトル
	cv::Mat E;						// 基本行列
	cv::Mat F;						// 基礎行列

	// フラグ
	bool calib_flag;

	int calib_count;				// キャリブレーション回数
	std::string saveCalibFolder;		// 対応点を保存するフォルダ

	// グレイコード
	std::unique_ptr<GRAYCODE> graycode;

	CamDev::Camera *camDev;			// カメラデバイス
};


#endif