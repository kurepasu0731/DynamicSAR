#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H


#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <opencv2/opencv.hpp>


/**
 * @brief	マルチスレッド間の変数の共有クラス\n
 * 			クリティカルセクションの確保
 */
class CriticalSection
{
public:

	CriticalSection()
		: graycode_flag (false)
		, calib_flag (false)
		, load_calib_flag (false)
		, run_reprojection_flag (false)
		, use_calib_flag (false)
		, use_projCalib_flag (false)
		, project_white_flag (false)
		, ready_detect_flag (false)
		, tracking_flag (false)
		, predict_flag (true)
		, detect_tracking_flag (false)
		, tracking_success_flag (false)
		, compensation_delay_flag (true)
		, lightPower (0.7f)
		, min_distance1 (1.0)
		, max_distance1 (1.3)
		, distance_step (1)
		, min_scale2 (0.9)
		, max_scale2 (1.1)
		, scale_step (2)
		, detect_width (200)
		, detect_height (200)
		, grad_th (200.f)
		, meanshift_radius (64)
		, win_th (80)
		, likelihood_th (0.8)
		, edge_th1 (60.f)
		, edge_th2 (100.f)
		, find_distance (0.05f)
		, error_th (0.75f)
		, trackingTime (24.0)
		, delayTime (120.0)
		//**written by fujisawa**//
		, movie_flag(false)
		, movieFileName("movie02.avi")
		, reloadMovie_flag(false)
		//, textureFileName("test.png") //Debug用
		//***********************//

	{}

	virtual ~CriticalSection(){}

	// モデルの座標変換のセット
	inline void setModelMatrix(const glm::mat4& _model_matrix)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(model_matrix_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		model_matrix = _model_matrix;
	}

	// 光源の強さのセット
	inline void setLightPower(const float& _lightPower)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(light_power_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		lightPower = _lightPower;
	}


	// カメラの内部パラメータのセット(CV)
	inline void setCameraIntrinsicCV(const cv::Matx33f& _camera_matrix)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(camera_intrinsic_cv_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		cameraIntrinsicCV = _camera_matrix;
	}

	// カメラの内部パラメータのセット
	inline void setCameraIntrinsicMatrix(const glm::mat4& _camera_matrix)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(camera_intrinsic_matrix_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		cameraIntrinsicMatrix = _camera_matrix;
	}

	// カメラ歪み補正マップのセット
	inline void setUndistortMap(const cv::Mat& map1, const cv::Mat& map2)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(undistortMap_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		undistortMap1 = map1.clone();
		undistortMap2 = map2.clone();
	}

	// プロジェクタ歪み補正マップのセット
	inline void setProjUndistortMap(const cv::Mat& map1, const cv::Mat& map2)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(projUndistortMap_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		projUndistortMap1 = map1.clone();
		projUndistortMap2 = map2.clone();
	}

	// 検出した位置姿勢のセット
	inline void setDetectPoseMatrix(const std::vector<glm::mat4>& _detectPose)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(detectPose_matrix_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		
		detectPose.clear();
		detectPose.assign(_detectPose.begin(), _detectPose.end());
	}

	// 検出に用いるパラメータのセット
	inline void setDetectParams(float _min_distance1, float _max_distance1, int _distance_step, float _min_scale2, float _max_scale2, int _scale_step,
								int _detect_width, int _detect_height, float _grad_th, int _meanshift_radius, int _win_th, double _likelihood_th)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(detect_params_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		min_distance1 = _min_distance1;
		max_distance1 = _max_distance1;
		distance_step = _distance_step;
		min_scale2 = _min_scale2;
		max_scale2 = _max_scale2;
		scale_step = _scale_step;
		detect_width = _detect_width;
		detect_height = _detect_height;
		grad_th = _grad_th;
		meanshift_radius = _meanshift_radius;
		win_th = _win_th;
		likelihood_th = _likelihood_th;
	}

	// 追跡中の位置姿勢のセット
	inline void setTrackingPoseMatrix(const glm::mat4& _trackingPose)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(trackingPose_matrix_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		
		trackingPose = _trackingPose;
	}

	// 遅延補償した位置姿勢のセット
	inline void setCompensatePoseMatrix(const glm::mat4& _predicctPose)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(compensatePose_matrix_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		
		compensatePose = _predicctPose;
	}

	// 追跡に用いるパラメータのセット
	inline void setTrackingParams(float _find_distance, float _error_th, float _edge_th1, float _edge_th2, double _trackingTime, double _delayTime)
	{
		boost::upgrade_lock<boost::shared_mutex> up_lock(tracking_params_mtx);
		boost::upgrade_to_unique_lock<boost::shared_mutex> write_lock(up_lock);
		find_distance = _find_distance;
		edge_th1 = _edge_th1;
		edge_th1 = _edge_th1;
		edge_th2 = _edge_th2;
		trackingTime = _trackingTime;
		delayTime = _delayTime;
	}


	// モデルの座標変換の取得
	inline glm::mat4 getModelMatrix()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(model_matrix_mtx);
		return model_matrix;
	}

	// 光源の強さの取得
	inline float getLightPower()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(light_power_mtx);
		return lightPower;
	}

	// カメラの内部パラメータの取得
	inline cv::Matx33f getCameraIntrinsicCV()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(camera_intrinsic_cv_mtx);
		return cameraIntrinsicCV;
	}

	// カメラの内部パラメータの取得
	inline glm::mat4 getCameraIntrinsicMatrix()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(camera_intrinsic_matrix_mtx);
		return cameraIntrinsicMatrix;
	}

	// カメラ歪み補正マップの取得
	inline void getUndistortMap(cv::Mat& map1, cv::Mat& map2)
	{
		boost::shared_lock<boost::shared_mutex> read_lock(undistortMap_mtx);
		map1 = undistortMap1.clone();
		map2 = undistortMap2.clone();
	}

	// カメラ歪み補正マップの取得
	inline void getProjUndistortMap(cv::Mat& map1, cv::Mat& map2)
	{
		boost::shared_lock<boost::shared_mutex> read_lock(projUndistortMap_mtx);
		map1 = projUndistortMap1.clone();
		map2 = projUndistortMap2.clone();
	}

	// 検出した位置姿勢の取得
	inline void getDetectPoseMatrix(std::vector<glm::mat4>& _detectPose)
	{
		boost::shared_lock<boost::shared_mutex> read_lock(detectPose_matrix_mtx);
		_detectPose.clear();
		_detectPose.assign(detectPose.begin(), detectPose.end());
	}

	// 検出に用いるパラメータの取得
	inline void getDetectParams(float& _min_distance1, float& _max_distance1, int& _distance_step, float& _min_scale2, float& _max_scale2, int& _scale_step,
								int& _detect_width, int& _detect_height, float& _grad_th, int& _meanshift_radius, int& _win_th, double& _likelihood_th)
	{
		boost::shared_lock<boost::shared_mutex> read_lock(detect_params_mtx);
		_min_distance1 = min_distance1;
		_max_distance1 = max_distance1;
		_distance_step = distance_step;
		_min_scale2 = min_scale2;
		_max_scale2 = max_scale2;
		_scale_step = scale_step;
		_detect_width = detect_width;
		_detect_height = detect_height;
		_grad_th = grad_th;
		_meanshift_radius = meanshift_radius;
		_win_th = win_th;
		_likelihood_th = likelihood_th;
	}

	// 追跡中の位置姿勢の取得
	inline glm::mat4 getTrackingPoseMatrix()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(trackingPose_matrix_mtx);
		return trackingPose;
	}

	// 遅延補償した位置姿勢の取得
	inline glm::mat4 getCompensatePoseMatrix()
	{
		boost::shared_lock<boost::shared_mutex> read_lock(compensatePose_matrix_mtx);
		return compensatePose;
	}

	// 追跡に用いるパラメータの取得
	inline void getTrackingParams(float& _find_distance, float& _error_th, float& _edge_th1, float& _edge_th2, double& _trackingTime, double& _delayTime)
	{
		boost::shared_lock<boost::shared_mutex> read_lock(tracking_params_mtx);
		_find_distance = find_distance;
		_edge_th1 = edge_th1;
		_edge_th1 = edge_th1;
		_edge_th2 = edge_th2;
		_trackingTime = trackingTime;
		_delayTime = delayTime;
	}



	///// フラグ /////

	// キャリブレーション
	boost::atomic<bool> graycode_flag;			// グレイコード投影フラグ
	boost::atomic<bool> calib_flag;				// キャリブレーションフラグ
	boost::atomic<bool> load_calib_flag;		// キャリブレーションファイルの読み込みフラグ
	boost::atomic<bool> run_reprojection_flag;	// チェッカーボードに再投影するフラグ
	boost::atomic<bool> use_calib_flag;			// カメラキャリブレーション済みか確認フラグ
	boost::atomic<bool> use_projCalib_flag;		// プロジェクタキャリブレーション済みか確認フラグ
	boost::atomic<bool> project_white_flag;		// 白を投影するフラグ

	// 検出
	boost::atomic<bool> ready_detect_flag;		// 検出の準備確認フラグ

	// 追跡
	boost::atomic<bool> tracking_flag;			// 追跡開始フラグ
	boost::atomic<bool> predict_flag;			// 予測フィルタフラグ

	// 検出＆追跡
	boost::atomic<bool> detect_tracking_flag;	// 検出＆追跡フラグ
	boost::atomic<bool> tracking_success_flag;	// 追跡成功フラグ

	// 遅延補償
	boost::atomic<bool> compensation_delay_flag;	// 遅延補償フラグ

	//**written by fujisawa***************************************************//
	// 動画
	boost::atomic<bool> movie_flag;				//動画再生する/しない
	boost::atomic<const char*> movieFileName;	//動画ファイル場所
	boost::atomic<bool> reloadMovie_flag;		//動画の再読み込みする/しない
	//boost::atomic<const char*> textureFileName; //チェンジするテクスチャファイル場所 //Debug用
	//************************************************************************//

private:

	///// ミューテックス /////
	boost::shared_mutex model_matrix_mtx;
	boost::shared_mutex light_power_mtx;
	boost::shared_mutex camera_intrinsic_cv_mtx;
	boost::shared_mutex camera_intrinsic_matrix_mtx;
	boost::shared_mutex undistortMap_mtx;
	boost::shared_mutex projUndistortMap_mtx;
	boost::shared_mutex detectPose_matrix_mtx;
	boost::shared_mutex detect_params_mtx;
	boost::shared_mutex trackingPose_matrix_mtx;
	boost::shared_mutex compensatePose_matrix_mtx;
	boost::shared_mutex tracking_params_mtx;


	///// 共有変数 /////
	glm::mat4 model_matrix;				// 3Dモデルの座標変換
	float lightPower;					// 光源の強さ

	// キャリブレーション
	cv::Matx33f cameraIntrinsicCV;		// IRカメラの内部パラメータ(CV用)
	glm::mat4 cameraIntrinsicMatrix;	// IRカメラの内部パラメータ
	cv::Mat undistortMap1;				// IRカメラの歪み補正マップ
	cv::Mat undistortMap2;				// IRカメラの歪み補正マップ
	cv::Mat projUndistortMap1;			// プロジェクタの歪み補正マップ
	cv::Mat projUndistortMap2;			// プロジェクタの歪み補正マップ

	// 検出
	std::vector<glm::mat4> detectPose;	// 検出した位置姿勢
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

	// 追跡
	glm::mat4 trackingPose;				// 追跡中の位置姿勢
	glm::mat4 compensatePose;			// 遅延時間補償した位置姿勢
	float find_distance;				// 探索する距離(m)	
	float error_th;						// エラーの閾値
	float edge_th1;						// エッジの閾値1
	float edge_th2;						// エッジの閾値2
	double trackingTime;				// 追跡処理時間
	double delayTime;					// システム遅延時間
};



#endif