#ifndef PROJECTION_THREAD_H
#define PROJECTION_THREAD_H

#include <memory>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "GLSL/window.h"
#include "GLSL/background.h"
#include "GLSL/camera.h"
#include "GLSL/mesh.h"
#include "GLSL/texture.h"
#include "GLSL/offscreenRender.h"

#include "Calibration/Calibration.h"

#include "CriticalSection.h"

// 全画面表示用
namespace Monitor
{

	typedef struct disp_prop
	{
		int index;
		int x,y,width,height;
	} Disp_Prop;

	static int dispCount=-1;
	static std::vector<Disp_Prop> Disps_Prop;

	//ディスプレイの情報入手
	inline BOOL CALLBACK DispEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData ) 
	{
		Disp_Prop di;
		di.index = dispCount++;
		di.x = lprcMonitor->left;
		di.y = lprcMonitor->top;
		di.width = lprcMonitor->right - di.x;
		di.height = lprcMonitor->bottom - di.y;
		Disps_Prop.push_back(di);

		return TRUE; // TRUEは探索継続，FALSEで終了
	}

	//ディスプレイ検出
	inline void SearchDisplay(void) 
	{
		// 一度だけ実行する
		if (dispCount == -1) {
			dispCount = 0;
			Disps_Prop = std::vector<Disp_Prop>();
			EnumDisplayMonitors(NULL, NULL, DispEnumProc, 0);
			Sleep(200);
		}
	}
}



// 投影スレッドクラス
class ProjectionThread : public Window
{

public:
	// コンストラクタ
	//**written by fujisawa****************************************************************************//
	ProjectionThread(CriticalSection *cs, CamDev::CameraLinkCamera *cameraDevice, CamDev::Camera *RGBcameraDevice = NULL, bool _use_chessboard=false, int _cornerCol=4, int _cornerRow=11, float _cornerInterval_m=0.0225f, double _delay=80.0, std::string _saveCalibFolder=".", int width=640, int height=480, std::string mFileName = "movie.avi", const char *title = "Projection")
		: Window( width, height, title)
		, calib_able_flag (false)
		, lightPower(0.7f)
		//**written by fujisawa*******//
		, frame_count(0)
		, modelID(0)
		, original_fixed_flag(false)
		, FILENAME(mFileName)
		//****************************//
	{
		// デバイスの受け渡し
		irCamDev = cameraDevice;
		rgbCamDev = RGBcameraDevice;

		// 保存フォルダ
		saveCalibFolder = _saveCalibFolder;

		// カメラが接続されていたらキャリブレーション可能
		if (rgbCamDev != NULL)
		{
			cv::Mat img;
			if (rgbCamDev->captureImage(img)) 
			{
				calib = std::unique_ptr<Calibration> (new Calibration(rgbCamDev, width, height, img.cols, img.rows, _use_chessboard, _cornerCol, _cornerRow, _cornerInterval_m, _delay, saveCalibFolder));
				calib_able_flag = true;
			}		
		}

		// スレッド間共有クラス
		critical_section = cs;

		// 姿勢の初期化
		quatOrientation = glm::quat(1.0, 0.0, 0.0, 0.0);

		// 照明の初期位置
		lightPos = glm::vec3(0.0, 0.0, 1.0);

		// オフスクリーン初期設定
		offscreen.offscreen_width = width;
		offscreen.offscreen_height = height;

		// ウィンドウの垂直同期off
		glfwSwapInterval(0);

		// カメラの更新
		camera.setEyePosition(glm::vec3(0.0f, 0.0f, 1.5f));
		camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);

		// インスタンスのthisポインタを記録
		glfwSetWindowUserPointer(window, this);

		//**written by fujisawa*******//
		// 動画テクスチャ初期処理
		capture.open(FILENAME);
		//****************************//
	}

	// デストラクタ
	virtual ~ProjectionThread(){}

	// 初期処理
	void init(const std::string& modelFile);

	// 毎フレーム行う処理
	void display();

	// モデルの描画
	void draw_model();

	// 画像の投影
	void img_projection(const cv::Mat& projMat);

	// 頂点の描画
	void draw_point(const std::vector<glm::vec3>& point_cloud);

	// キャリブレーション結果を用いてチェッカーボードに再投影
	void runCheckerReprojection();

	// プロジェクタパラメータの読み込み
	void loadProjectorParam();

	// カラーバッファを入れ替えてイベントを取り出す
	void swapBuffers();

	//**written by fujisawa***************************************//
	//////////動画関連//////////
	//動画再生
	void playMovie();

	//動画停止(一時停止含む)
	void stopMovie();

	//動画再読み込み
	void reloadMovie(const std::string mFileName);

	//現在のフレーム数取得
	int getCurrentFrameCount();

	//デフォルトのテクスチャに戻す
	void setDefaultTexture(int model_id);

	//テクスチャ画像の変更
	void setTexture(int model_id, std::string fileName);

	//動画を1フレーム分読み込む
	void updateMovieFrame();

	//モデルテクスチャ画像の変更
	void exChangeTexture(int model_id, cv::Mat m_image); 

	//////////動画関連//////////
	//***********************************************************//

	/***** メンバ変数 *****/

	///// CADモデル用 /////
	GLuint program;					// プログラムオブジェクト
	// シェーダのuniform変数のID
	GLuint MatrixID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
	GLuint LightID;
	GLuint LightPowerID;
	Mesh mesh;						// モデルのメッシュ

	glm::mat4 ProjectionMatrix;		// 透視投影
	glm::mat4 ViewMatrix;			// カメラ行列
	glm::quat quatOrientation;		// モデルの姿勢


	///// 投影画像描画用 /////
	Background projection_background;		


	///// キャリブレーション確認用の点群 /////
	GLuint program_point;					// 頂点のみ描画用
	std::vector<glm::vec3> checkerPoint;	// 頂点
	std::vector<glm::vec3> checkerNormal;	// 法線
	GLuint PointMatrixID;
	GLuint PointVBO;						// 点の描画用
	GLuint PointIBO;						// 点の描画用
	Background camera_background;			// カメラ確認用
	OffscreenRender camera_offscreen;


	///// GLオブジェクト用 /////
	Camera camera;					// カメラ
	glm::vec3 lightPos;				// 光源位置
	float lightPower;				// 光源の強さ

	OffscreenRender offscreen;		// オフスクリーンレンダリング


	///// プロジェクタパラメータ用 /////
	glm::mat4 proj_extrinsicMat;			// プロジェクタの位置姿勢
	glm::mat4 proj_intrinsicMat;			// カメラの内部パラメータ
	cv::Matx33f proj_intrinsic;				// カメラの内部パラメータ
	glm::mat4 projection;


	///// ProCamキャリブレーション用 /////
	std::unique_ptr<Calibration> calib;
	std::string saveCalibFolder;				// キャリブレーションフォルダ
	CamDev::CameraLinkCamera *irCamDev;			// IRカメラ
	CamDev::Camera *rgbCamDev;					// 投影光撮影用デバイス
	bool calib_able_flag;						// キャリブレーションが可能か判定


	///// スレッド間の共有クラス用 //////
	CriticalSection* critical_section;

	//**written by fujisawa***************************************//
	//////////動画関連//////////
	std::string FILENAME;						//! 動画ファイル名
	cv::Mat movie_image;						//! 動画
	int frame_count;							//! 動画のフレーム数
	cv::VideoCapture capture;					//! 動画を開く
	int modelID;								//! ターゲットモデル番号
	bool original_fixed_flag;					//! 投影用モデルのテクスチャがオリジナルのものかどうか
	//***********************************************************//

};



#endif