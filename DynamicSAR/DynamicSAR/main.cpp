#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <memory>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL/glsl_setup.h"
#include "Main_thread.h"


#include "CamDev/WebCamera.h"
#include "CamDev/CameraLinkCamera.h"
#include "CamDev/UndistortedCamera.h"


#pragma	comment(lib,"glfw3dll.lib")
#pragma comment(lib,"opengl32.lib")
#pragma	comment(lib,"glew32.lib")


int DISPLAY_NUM = 2;					// プロジェクタの番号
int USE_CHESSBOARD = 0;					// 0:サークルグリッド, 1:チェスボード
double CORNERS_INTERVAL_m = 0.0225;		// コーナーの間隔の大きさ[m]
int CORNERS_COLS = 4;					// 横のコーナー数
int CORNERS_ROWS = 11;					// 縦のコーナー数

int CAMERA_INDEX = 0;					// RGBカメラ番号
int CAMERA_WIDTH = 1920;				// RGBカメラ画像の幅
int CAMERA_HEIGHT = 1080;				// RGBカメラ画像の高さ

double GRAYCODE_DELAY = 80.0;			// グレイコード投影の待機時間

const std::string CALIB_SAVE_FOLDER = "../../Calibration";
const std::string MODEL_FOLDER = "../../Model/";

#define SAVE_FOLDER "../../Save/mannequinCAD"
#define SAVE_FILE "../../Save/mannequinCAD/ferns_param.csv"

bool loadConfigFile(const std::string &fileName);

// プログラム終了時の処理
static void cleanup()
{
	// GLFWの終了処理
	glfwTerminate();
}


int main()
{
	// 設定ファイルの読み込み
	loadConfigFile("../../param.csv");

	// GLFWの初期化
	if(!glfwInit())
	{
		std::cerr << "Can't initialize GLFW" << std::endl;
		return -1;
	}

	// プログラムの終了処理
	atexit(cleanup);

	// OpenGLのバージョンの選択
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// 赤外カメラデバイス
	std::unique_ptr<CamDev::CameraLinkCamera> IR_camera;
	IR_camera = std::unique_ptr<CamDev::CameraLinkCamera>(new CamDev::CameraLinkCamera("../../CameraLink HXC20 VGA.fmt"));

	cv::Mat ir_img;
	if (!IR_camera->captureImage(ir_img)) 
	{
		std::cerr << "capture error" << std::endl;
		return -1;
	}

	// キャリブレーション用カメラデバイス
	std::unique_ptr<CamDev::Camera> RGB_camera;
	RGB_camera = std::unique_ptr<CamDev::Camera>(new CamDev::WebCamera(CAMERA_INDEX, CAMERA_WIDTH, CAMERA_HEIGHT));

	cv::Mat rgb_img;
	if (!RGB_camera->captureImage(rgb_img)) 
	{
		std::cerr << "capture error" << std::endl;
		return -1;
	}

	// ウィンドウを作成
	MainThread main_thread(IR_camera.get(), RGB_camera.get(), DISPLAY_NUM, USE_CHESSBOARD, CORNERS_COLS, CORNERS_ROWS, CORNERS_INTERVAL_m, GRAYCODE_DELAY, CALIB_SAVE_FOLDER, SAVE_FILE, SAVE_FOLDER, ir_img.cols, ir_img.rows);


	// GLEWの初期化
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		std::cerr << "Can't initialize GLEW" << std::endl;
		return -1;
	}

	// 初期処理
	main_thread.init(MODEL_FOLDER+"mannequinCAD.obj", MODEL_FOLDER+"mannequinCAD.ply");



	// 繰り返し処理
	while (main_thread.shouldClose() == GL_FALSE)
	{
		// 描画
		main_thread.display();
	}


	main_thread.end();


	return 0;
}



/**
 * @brief   設定ファイルの読み込み
 * 
 * @param   fileName[in]	設定ファイル名   
 */
bool loadConfigFile(const std::string &fileName)
{
	// 設定ファイルの読み込み
	std::ifstream ifs(fileName);

	if( !ifs ) {
		std::cerr << "設定ファイルを読み込めません" << std::endl;
		return false;
	}

	std::vector<std::vector<std::string>> params;	// 文字の格納

	// 文字列の取得
	std::string str;
	while (std::getline(ifs, str))
	{
		std::vector<std::string> param;

		std::string tmp;
		std::istringstream stream(str);
		while (std::getline(stream, tmp, ','))
		{
			param.emplace_back(tmp);
		}
		params.emplace_back(param);
	}

	DISPLAY_NUM = atoi(params[0][1].c_str());
	USE_CHESSBOARD = atoi(params[1][1].c_str());
	CORNERS_INTERVAL_m = atof(params[2][1].c_str());
	CORNERS_COLS = atoi(params[3][1].c_str());
	CORNERS_ROWS = atoi(params[4][1].c_str());
	CAMERA_INDEX = atoi(params[5][1].c_str());
	CAMERA_WIDTH = atoi(params[6][1].c_str());
	CAMERA_HEIGHT = atoi(params[7][1].c_str());
	GRAYCODE_DELAY = atof(params[8][1].c_str());

	return true;
}