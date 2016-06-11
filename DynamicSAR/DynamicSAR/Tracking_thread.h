#ifndef TRACKING_THREAD_H
#define TRACKING_THREAD_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>

#include <gl/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <TooN/TooN.h>
#include <TooN/SVD.h>
#include <TooN/so3.h>
#include <TooN/se3.h>
#include <TooN/wls.h>

#include "GLSL/glsl_setup.h"
#include "GLSL/window.h"
#include "GLSL/camera.h"
#include "GLSL/mesh.h"
#include "GLSL/texture.h"
#include "GLSL/background.h"
#include "GLSL/offscreenRender.h"
#include "GLSL/image_rect.h"
#include "GLSL/myTimer.h"

#include "CriticalSection.h"

#include "CamDev/WebCamera.h"
#include "CamDev/CameraLinkCamera.h"
#include "CamDev/UndistortedCamera.h"

#include "LSM/LSMPoint3f.h"
#include "LSM/LSMQuatd.h"

/**
 * @brief   トラッキングスレッド
 *			オフスクリーンレンダリングでエッジ検出
 */
class TrackingThread : public Window
{
public:

	// 3Dモデルと入力画像との対応
	struct Correspond
	{
		cv::Point3f model3D;		// 3Dモデルの3次元点の位置
		cv::Point2f model2D;		// 3Dモデルの2次元点の位置
		cv::Point2f normal2D;		// 3Dモデルの2次元点における法線ベクトル
		cv::Point2f normalUnit;		// 3Dモデルの2次元点における法線の単位ベクトル
		cv::Point2f imageEdge2D;	// 対応付けられた入力画像のエッジの位置
		double dist;				// 対応点間の距離
	};


	// コンストラクタ
	TrackingThread(CriticalSection *cs, CamDev::CameraLinkCamera *cameraDevice, int width = 640, int height = 480, const char *title = "Edge image")
		: Window(width, height, title)
		, windowWidth (width)
		, windowHeight (height)
		, lightPos (0.0, 0.0, 0.0)			// 照明の初期位置(原点)
		, edge_th1 (60.f)
		, edge_th2 (100.f)
		, find_distance (0.03f)
		, error_th (0.45f)
		, trackingTime (14.0)
		, delayTime (100.0)
		, firstTime (true)
	{
		// デバイスの受け渡し
		irCamDev = cameraDevice;

		// スレッド間共有クラス
		critical_section = cs;

		// カメラの初期位置(カメラが原点)
		camera.setEyePosition(glm::vec3(0.0, 0.0, 0.0));

		// カメラの視線方向
		camera.setEyeVector(glm::vec3(0.0, 0.0, -1.0));

		// 照明の初期位置(原点)
		lightPos = glm::vec3(0.0, 0.0, 0.0);

		// オフスクリーン初期設定
		offscreen1.offscreen_width = width;
		offscreen1.offscreen_height = height;
		offscreen2.offscreen_width = width;
		offscreen2.offscreen_height = height;
		offscreen3.offscreen_width = width;
		offscreen3.offscreen_height = height;
		offscreen4.offscreen_width = width;
		offscreen4.offscreen_height = height;

		// カメラの更新
		camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);

		// 共分散の初期化
		cv::Mat cov = cv::Mat::eye(6, 6, CV_32F);
		covariance = cov;

		// 予測フィルタ
		predict_point = std::unique_ptr<LSMPoint3f> (new LSMPoint3f(LSM));
		predict_quat = std::unique_ptr<LSMQuatd> (new LSMQuatd(LSM));
		predict_point->setForgetFactor(0.6);	// 忘却係数
		predict_quat->setForgetFactor(0.6);	// 忘却係数

		// 遅延補償
		compensate_point = std::unique_ptr<LSMPoint3f> (new LSMPoint3f(LSM));
		compensate_quat = std::unique_ptr<LSMQuatd> (new LSMQuatd(LSM));
		compensate_point->setForgetFactor(0.6);	// 忘却係数
		compensate_quat->setForgetFactor(0.6);	// 忘却係数

	}


	// デストラクタ
	virtual ~TrackingThread(){}


	// 初期処理
	void init(const std::string& modelFile, const std::string& trackingFile, const cv::Matx33f& _camMat, const glm::mat4& _cameraMat);

	// 毎フレーム行う処理
	void display();

	// モデルベースのトラッキング
	void tracking();

	// オフスクリーンでの描画
	void offscreen_render(int width, int height);

	// 3Dモデルの輪郭線の検出
	void extractModelEdge();

	// 入力画像のエッジ検出
	void extractInputEdge();

	// 入力画像との対応を推定
	void estimateCorrespondence();

	// ヤコビアンの計算
	TooN::Vector<6> calcJacobian(const cv::Point3f& pts3, const cv::Point2f& pts2, const cv::Point2f& ptsnv, double ptsd, const TooN::SE3& E);

	// IRLSによる姿勢推定
	void estimatePoseIRLS();

	// エラー率の推定
	void calcError();

	// 描画シーン(元モデル)
	void scene_orig(int width, int height);

	// 描画シーン(トラッキングモデル)
	void scene_point(int width, int height);

	// 描画シーン(入力画像)
	void scene_background(int width, int height);

	// 頂点の描画
	void draw_point(const std::vector<glm::vec3>& point_cloud, int width, int height);

	// 初期姿勢のセット
	void setInitPose();

	// 2次元平面への投影
	void projection3Dto2D(const cv::Vec3f& point3D, const cv::Vec3f& normal3D, cv::Vec2f& point2D, cv::Vec2f& normal2D);

	// PLY形式で点群の読み込み
	void loadPointCloudPly(const std::string& fileName, std::vector<glm::vec3>& point_cloud, std::vector<glm::vec3>& point_normal);

	// 文字列分割
	std::vector<std::string> split(const std::string &str, char sep);

	// カラーバッファを入れ替えてイベントを取り出す
	void swapBuffers();


	/***** メンバ変数 *****/

	///// 元のCADモデル用 /////
	GLuint program_orig;		// プログラムオブジェクト

	GLuint MatrixID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
	GLuint LightID;
	GLuint LightPowerID;

	Mesh orig_mesh;				// モデルのメッシュ


	///// CADモデルエッジ検出用 /////
	GLuint program_CADedge;		// プログラムオブジェクト
	GLint edge_locW;				// シェーダ用の画像の幅
	GLint edge_locH;				// シェーダ用の画像の高さ
	GLuint edge_MatrixID;
	GLuint edge_texID;				// テクスチャID
	ImageRect edge_rect;		// シェーダエフェクト用のメッシュ


	///// トラッキングモデル用 /////
	GLuint program_point;		// プログラムオブジェクト
	GLuint pointMatrixID;
	GLuint pointViewMatrixID;
	GLuint pointModelMatrixID;
	GLuint pointVBO[3];
	GLuint pointIBO;

	// ポイントクラウド
	std::vector<glm::vec3> pointCloud;		// 頂点
	std::vector<glm::vec3> pointNormal;		// 法線


	///// シェーダエフェクト用 /////
	GLuint program_image;		// プログラムオブジェクト
	GLint locW;					// シェーダ用の画像の幅
	GLint locH;					// シェーダ用の画像の高さ
	GLuint imageMatrixID;
	GLuint tex0ID;				// テクスチャID
	GLuint tex1ID;				// テクスチャID
	GLuint tex2ID;				// テクスチャID
	GLuint tex3ID;				// テクスチャID

	cv::Mat indexImg;			// インデックス番号が格納されている
	ImageRect image_rect;		// シェーダエフェクト用のメッシュ


	///// エッジ描画用 /////
	std::vector<glm::vec3> pointEdge;
	GLuint program_edge;
	GLuint pMatrixID;
	GLuint psizeID;
	GLuint pVBO;
	GLuint pIBO;


	///// GLオブジェクト /////
	int windowWidth;
	int windowHeight;
	Camera camera;				// カメラ
	glm::vec3 lightPos;			// 光源位置
	float lightPower;			// 光源の強さ

	cv::Matx33f camMat;			// カメラの内部パラメータ
	glm::mat4 cameraMat;		// カメラの内部パラメータ

	OffscreenRender offscreen1;	// オフスクリーンレンダリング
	OffscreenRender offscreen2;	// オフスクリーンレンダリング
	OffscreenRender offscreen3;	// オフスクリーンレンダリング
	OffscreenRender offscreen4;	// オフスクリーンレンダリング

	///// トラッキング用 /////
	cv::Matx44f estimatedPose;	// 推定結果
	cv::Matx66f covariance;		// 共分散
	float find_distance;		// 探索する距離(m)
	double max_distance;		// 考慮する最大の距離
	float error_th;				// エラーの閾値

	std::vector<Correspond> correspondPoints;		// 対応点

	///// 予測フィルタ用 /////
	double trackingTime;		// トラッキングにかかる時間
	std::unique_ptr<LSMPoint3f> predict_point;	// 位置の最小二乗法
	std::unique_ptr<LSMQuatd> predict_quat;		// 姿勢の最小二乗法
	bool firstTime;								// 1回目かどうか

	///// 遅延補償用 /////
	MyTimer timer;				// 起動時間からの時間計測
	double delayTime;			// システムの遅延時間
	std::unique_ptr<LSMPoint3f> compensate_point;	// 位置の最小二乗法
	std::unique_ptr<LSMQuatd> compensate_quat;		// 姿勢の最小二乗法


	///// 入力画像用 /////
	CamDev::CameraLinkCamera *irCamDev;		// IRカメラ
	cv::Mat inputImg;			// 入力画像
	cv::Mat cannyImg;			// エッジ画像
	cv::Mat edge_img;			// 入力画像とモデルのエッジの対応画像
	float edge_th1;				// エッジの閾値1
	float edge_th2;				// エッジの閾値2
	cv::Mat sobelImgX;			// sobel検出
	cv::Mat sobelImgY;			// sobel検出

	Background input_mesh;		// 入力画像用のメッシュ

	///// スレッド間の共有クラス用 //////
	CriticalSection* critical_section;
};


#endif