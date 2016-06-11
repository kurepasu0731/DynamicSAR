#ifndef MAIN_THREAD_H
#define MAIN_THREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/atomic.hpp>


#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <AntTweakBar.h>

#include "GLSL/glsl_setup.h"
#include "GLSL/window.h"
#include "GLSL/camera.h"
#include "GLSL/mesh.h"
#include "GLSL/texture.h"
#include "GLSL/background.h"
#include "GLSL/myTimer.h"

#include "CamDev/WebCamera.h"
#include "CamDev/CameraLinkCamera.h"
#include "CamDev/UndistortedCamera.h"

#include "CriticalSection.h"
#include "Projection_thread.h"
#include "Detection_thread.h"
#include "Tracking_thread.h"

// メインスレッドクラス
class MainThread : public Window
{

public:
	// コンストラクタ
	MainThread(CamDev::CameraLinkCamera *cameraDevice, CamDev::Camera *RGBCameraDevice = NULL, int _disp_num = 0, bool _use_chessboard=false, int _cornerCol=4, int _cornerRow=11, float _cornerInterval_m=0.0225f, double _graycode_delay=80.0,
				std::string _saveCalibFolder=".", std::string _fernsParamFile="ferns_param.csv", std::string _lerningDataFolder=".", int width = 640, int height = 480, const char *title = "Main Thread")
		: Window( width, height, title)
		, mousePress_flag ( false )
		, lightPower(0.7f)
		, model_trans (0.0, 0.0, 0.0)
		, model_pose (1.0, 0.0, 0.0, 0.0)
		, disp_num (_disp_num)
		, projThread_end_flag (false)
		, use_chessboard (_use_chessboard)
		, cornerCol (_cornerCol)
		, cornerRow (_cornerRow)
		, cornerInterval_m (_cornerInterval_m)
		, graycode_delay (_graycode_delay)
		, saveCalibFolder (_saveCalibFolder)
		, use_calib_flag (false)
		, detection_flag (false)
		, detectThread_end_flag (false)
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
		, ready_tracking_flag (false)
		, trackingThread_end_flag (false)
		, edge_th1 (60.f)
		, edge_th2 (100.f)
		, find_distance (0.05f)
		, error_th (0.75f)
		, trackingTime (24.0)
		, delayTime (120.0)
	{
		// スレッド間共有クラス
		critical_section = std::unique_ptr<CriticalSection> (new CriticalSection);

		// デバイスの受け渡し
		irCamDev = cameraDevice;
		rgbCamDev = RGBCameraDevice;


		///// 投影スレッドの開始 /////
		projThread = boost::thread( &MainThread::createProjectionThread, this);

		///// 検出スレッドの開始 /////
		detectThread = boost::thread( &MainThread::createDetectionThread, this);

		///// 追跡スレッドの開始 /////
		trackingThread = boost::thread( &MainThread::createTrackingThread, this);


		// GUIの初期化
		TwInit(TW_OPENGL_CORE, NULL);
		TwWindowSize(width, height);
		myGUI = TwNewBar("GUI");
		TwDefine(" GLOBAL help='This example shows a model loader' ");
		TwDefine(" GUI size='200 400' color='41 126 231' ");
		TwAddVarRW(myGUI, "Background color", TW_TYPE_COLOR3F, &background_color, NULL);
		TwAddVarCB(myGUI, "Quaternion", TW_TYPE_QUAT4F, SetQuatNormalizeCB, GetQuatNormalizeCB, &model_pose, "showval=true opened=true");
		TwAddSeparator(myGUI, NULL, " group='Camera' ");
		TwAddVarRW(myGUI, "Position", TW_TYPE_DIR3F, &camera.eyePosition, " group='Camera' ");
		TwAddVarRW(myGUI, "Vector", TW_TYPE_DIR3F, &camera.eyeVector, " group='Camera' ");
		TwAddVarRW(myGUI, "Up", TW_TYPE_DIR3F, &camera.eyeUp, " group='Camera' ");
		TwAddVarRW(myGUI, "FoV", TW_TYPE_FLOAT, &camera.FoV, " group='Camera' ");
		TwAddSeparator(myGUI, NULL, " group='Light' ");
		TwAddVarRW(myGUI, "Light pos", TW_TYPE_DIR3F, &lightPos, " group='Light' ");
		TwAddVarRW(myGUI, "Power", TW_TYPE_FLOAT, &lightPower, " group='Light' min=0.0 max=100.0 step=0.01");

		// キャリブレーション用GUI
		calibGUI = TwNewBar("Calibration");
		TwDefine(" Calibration position ='430 10' size='200 140' color='41 250 100' ");
		TwAddVarRW(calibGUI, "Project graycode", TW_TYPE_BOOLCPP, &critical_section->graycode_flag, NULL);
		TwAddVarRW(calibGUI, "Run calibration", TW_TYPE_BOOLCPP, &critical_section->calib_flag, NULL);
		TwAddVarRW(calibGUI, "Run reprojection", TW_TYPE_BOOLCPP, &critical_section->run_reprojection_flag, NULL);
		TwAddVarRW(calibGUI, "Load projector calibration", TW_TYPE_BOOLCPP, &critical_section->load_calib_flag, NULL);
		TwAddVarCB(calibGUI, "Load camera calibration", TW_TYPE_BOOLCPP, SetCalibCB, GetCalibCB, this, NULL);
		TwAddVarRW(calibGUI, "Project white", TW_TYPE_BOOLCPP, &critical_section->project_white_flag, NULL);

		// 検出用GUI
		detectGUI = TwNewBar("Detectioin");
		TwDefine(" Detectioin position ='430 160' size='200 140' color='241 50 100' ");
		TwAddVarRW(detectGUI, "DetectAndTrack", TW_TYPE_BOOLCPP, &critical_section->detect_tracking_flag, NULL);
		TwAddVarRW(detectGUI, "Detect", TW_TYPE_BOOLCPP, &detection_flag, NULL);
		TwAddButton(detectGUI, "Update parameter", SetDetectParamCB, this, NULL);
		TwAddVarRW(detectGUI, "min_distance1", TW_TYPE_FLOAT, &min_distance1, "min=0.5 max=2.0 step=0.1");
		TwAddVarRW(detectGUI, "max_distance1", TW_TYPE_FLOAT, &max_distance1, "min=0.5 max=2.0 step=0.1");
		TwAddVarRW(detectGUI, "distance_step", TW_TYPE_INT32, &distance_step, "min=1 max=8 step=1");
		TwAddVarRW(detectGUI, "min_scale2", TW_TYPE_FLOAT, &min_scale2, "min=0.5 max=1.6 step=0.1");
		TwAddVarRW(detectGUI, "max_scale2", TW_TYPE_FLOAT, &max_scale2, "min=0.5 max=1.6 step=0.1");
		TwAddVarRW(detectGUI, "scale_step", TW_TYPE_INT32, &scale_step, "min=1 max=8 step=1");
		TwAddVarRW(detectGUI, "detect_width", TW_TYPE_INT32, &detect_width, "min=20 max=400 step=10");
		TwAddVarRW(detectGUI, "detect_height", TW_TYPE_INT32, &detect_height, "min=20 max=400 step=10");
		TwAddVarRW(detectGUI, "gradient threashold", TW_TYPE_FLOAT, &grad_th, "min=0.0 max=4000.0 step=1.0");
		TwAddVarRW(detectGUI, "meanshift_radius", TW_TYPE_INT32, &meanshift_radius, "min=10 max=100 step=1");
		TwAddVarRW(detectGUI, "nearest neighbor threshold", TW_TYPE_INT32, &win_th, "min=10 max=200 step=1");
		TwAddVarRW(detectGUI, "likelihood threshold", TW_TYPE_DOUBLE, &likelihood_th, "min=0.1 max=1.0 step=0.01");

		// 追跡用GUI
		trackingGUI = TwNewBar("Tracking");
		TwDefine(" Tracking position ='430 310' size='200 140' color='101 50 200' ");
		TwAddVarRW(trackingGUI, "Track", TW_TYPE_BOOLCPP, &critical_section->tracking_flag, NULL);
		TwAddVarRW(trackingGUI, "Predict filter", TW_TYPE_BOOLCPP, &critical_section->predict_flag, NULL);
		TwAddVarRW(trackingGUI, "Compensation delay", TW_TYPE_BOOLCPP, &critical_section->compensation_delay_flag, NULL);
		TwAddButton(trackingGUI, "Update parameter", SetTrackingParamCB, this, NULL);
		TwAddVarRW(trackingGUI, "find distance", TW_TYPE_FLOAT, &find_distance, "min=0.01 max=0.2 step=0.01");
		TwAddVarRW(trackingGUI, "error threshold", TW_TYPE_FLOAT, &error_th, "min=0.05 max=1.0 step=0.05");
		TwAddVarRW(trackingGUI, "edge threshold1", TW_TYPE_FLOAT, &edge_th1, "min=0.0 max=400.0 step=1.0");
		TwAddVarRW(trackingGUI, "edge threshold2", TW_TYPE_FLOAT, &edge_th2, "min=0.0 max=400.0 step=1.0");
		TwAddVarRW(trackingGUI, "tracking time", TW_TYPE_DOUBLE, &trackingTime, "min=0.0 max=1000.0 step=1.0");
		TwAddVarRW(trackingGUI, "System delay time", TW_TYPE_DOUBLE, &delayTime, "min=0.0 max=1000.0 step=1.0");

		// 照明の初期位置
		lightPos = glm::vec3(0.0, 0.0, 1.0);

		// ウィンドウの垂直同期off
		glfwSwapInterval(0);

		// ウィンドウの背景色
		background_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		// カメラの更新
		camera.setEyePosition(glm::vec3(0.0f, 0.0f, 1.5f));
		camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);

		// 検出用ファイル
		fernsParamFile = _fernsParamFile;
		lerningDataFolder = _lerningDataFolder;

		// コールバック
		glfwSetFramebufferSizeCallback(window, ResizeCB);
		glfwSetMouseButtonCallback(window, MouseButtonCB);
		glfwSetCursorPosCallback(window , MousePosCB);
		glfwSetScrollCallback(window, MouseScrollCB);
		glfwSetKeyCallback(window, KeyFunCB);

		// インスタンスのthisポインタを記録
		glfwSetWindowUserPointer(window, this);
	}

	// デストラクタ
	virtual ~MainThread()
	{
		// GUIの終了
		TwTerminate();
	}

	// 初期処理
	void init(const std::string& modelFile, const std::string& trackingFile);

	// 終了処理
	void end();

	// 毎フレーム行う処理
	void display();

	// モデルの描画
	void draw_model();

	// 検出結果の描画
	void draw_detectPose(const glm::mat4 &poseMatrix);

	// 背景の描画
	void draw_background();

	// 投影用スレッドの生成
	void createProjectionThread();

	// 検出用スレッドの生成
	void createDetectionThread();

	// 追跡用スレッドの生成
	void createTrackingThread();

	// カメラパラメータの読み込み
	bool loadCameraParam(const std::string& camFile);

	// カラーバッファを入れ替えてイベントを取り出す
	void swapBuffers();

	// コールバック処理
	static void ResizeCB(GLFWwindow *const window, int width, int height);
	static void MouseButtonCB(GLFWwindow *const window, int button, int action, int mods);
	static void MousePosCB(GLFWwindow *const window, double x, double y);
	static void MouseScrollCB(GLFWwindow *window, double x, double y);
	static void KeyFunCB(GLFWwindow *window, int key, int scancode, int action, int mods);

	// 自作コールバック関数
	static void TW_CALL SetQuatNormalizeCB(const void *value, void *clientData);
	static void TW_CALL GetQuatNormalizeCB(void *value, void *clientData);
	static void TW_CALL SetCalibCB(const void *value, void* clientData);
	static void TW_CALL GetCalibCB(void *value, void* clientData);
	static void TW_CALL SetDetectParamCB(void* clientData);
	static void TW_CALL SetTrackingParamCB(void* clientData);

	/***** メンバ変数 *****/

	///// GUI用 /////
	TwBar *myGUI;					// GUI
	TwBar *calibGUI;				
	TwBar *detectGUI;	
	TwBar *trackingGUI;	



	///// CADモデル用 /////
	GLuint program;					// プログラムオブジェクト
	// シェーダのuniform変数のID
	GLuint MatrixID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
	GLuint LightID;
	GLuint LightPowerID;
	Mesh mesh;						// モデルのメッシュ

	glm::vec3 model_trans;			// 初期位置
	glm::quat model_pose;			// 初期姿勢
	std::string modelFilePath;		// モデルのファイル名


	///// カメラ画像描画用 /////
	Background camera_background;	// 背景の描画


	///// GLオブジェクト用 /////
	Camera camera;					// カメラ
	glm::vec4 background_color;		// 背景色
	glm::vec3 lightPos;				// 光源位置
	float lightPower;			// 光源の強さ

	bool use_calib_flag;		// 内部パラメータの使用フラグ
	cv::Matx33f camMat;			// カメラの内部パラメータ
	glm::mat4 cameraMat;		// カメラの内部パラメータ

	// 前回のマウス位置
	double lastMousePosX;
	double lastMousePosY;
	bool mousePress_flag;			// マウスが押されているか

	MyTimer FPS;					// fps計測


	///// カメラデバイス /////
	CamDev::CameraLinkCamera *irCamDev;		// IRカメラ
	CamDev::Camera *rgbCamDev;				// プロジェクタキャリブレーション用のカメラ


	///// スレッド間の共有クラス用 //////
	std::unique_ptr<CriticalSection> critical_section;

	///// 投影スレッド用 /////
	boost::thread projThread;
	int disp_num;
	bool use_chessboard;
	int cornerCol;
	int cornerRow;
	float cornerInterval_m;
	double graycode_delay;
	std::string saveCalibFolder;					// キャリブレーション結果を保存するフォルダ
	boost::atomic<bool> projThread_end_flag;		// スレッド終了フラグ


	///// 検出スレッド用 /////
	boost::thread detectThread;
	boost::atomic<bool> detection_flag;				// 検出開始フラグ
	boost::atomic<bool> detectThread_end_flag;		// スレッド終了フラグ
	std::string fernsParamFile;						// Hough Fernsのパラメータファイル
	std::string lerningDataFolder;					// 学習データのあるフォルダ名
	
	// 検出パラメータ
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



	///// 追跡スレッド用 /////
	boost::thread trackingThread;
	boost::atomic<bool> ready_tracking_flag;		// 初期処理完了フラグ
	boost::atomic<bool> trackingThread_end_flag;	// スレッド終了フラグ
	std::string trackingFilePath;					// 追跡用のモデルファイル

	// 追跡パラメータ
	float find_distance;				// 探索する距離(m)	
	float error_th;						// エラーの閾値
	float edge_th1;						// エッジの閾値1
	float edge_th2;						// エッジの閾値2
	double trackingTime;				// 追跡処理時間
	double delayTime;					// システム遅延時間
};


#endif