#ifndef GRAYCODE_H
#define GRAYCODE_H

#pragma once


#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>  // 文字列ストリーム
#include <direct.h> // create dir
#include <memory>

#include "../CamDev/WebCamera.h"
#include "../CamDev/CameraLinkCamera.h"
#include "../CamDev/UndistortedCamera.h"

class ProjectionThread;

/**
 * @brief   GrayCodeを用いた幾何補正
 *
 * @note	グレイコードは，縦，横別に作成し，後で合成
 *			パターン画像はビット演算を用いて作成
 *			最終的なプロジェクタとカメラを対応付けた配列はc->CamProに格納される
 */
class GRAYCODE
{

public:

	// グレイコード作成に必要な構造体
	typedef struct _Graycode 
	{
		std::vector<std::vector<int>> graycode;			// グレイコード（プロジェクタ解像度[高さ][幅]）
		unsigned int h_bit, w_bit;						// 高さ，幅の必要ビット数
		unsigned int all_bit;							// 合計ビット数（h_bit + w_bit）
	} Graycode;

	// プロジェクタ - カメラ対応に必要な構造体
	typedef struct _correspondence 
	{
		std::vector<std::vector<int>> graycode;			// 2値化コードを復元したものをカメラ画素に格納
		std::vector<std::vector<cv::Point>> CamPro;		// プロジェクタ画素に対するカメラ対応画素
		std::vector<std::vector<cv::Point>> ProCam;		// カメラ画素に対するプロジェクタ対応画素 
		std::unique_ptr<std::map<int, cv::Point>> code_map;
		Graycode g;
	} correspondence;

	std::unique_ptr<correspondence> c;

	// コンストラクタ
	GRAYCODE(CamDev::Camera *camera, int _prj_width, int _prj_height, int _cam_width, int _cam_height, double _delay=80.0, std::string _saveFolder=".")
		: prj_width (_prj_width)
		, prj_height (_prj_height)
		, cam_width (_cam_width)
		, cam_height (_cam_height)
		, saveFolder (_saveFolder)
	{
		// デバイスの受け渡し
		camDev = camera;

		// 初期化
		GC = "Graycode";
		MP = "Measure";
		delay = _delay;
		g = std::unique_ptr<Graycode>(new Graycode());
		c = std::unique_ptr<correspondence>(new correspondence());

		// サイズの初期化
		g->graycode.resize(prj_height, std::vector<int>(prj_width, 0));
		c->graycode.resize(cam_height, std::vector<int>(cam_width, 0));
		c->CamPro.resize(prj_height, std::vector<cv::Point>(prj_width, cv::Point(0,0)));
		c->ProCam.resize(cam_height, std::vector<cv::Point>(cam_width, cv::Point(0,0)));
		c->g.graycode.resize(prj_height, std::vector<int>(prj_width, 0));
		c->code_map = std::unique_ptr<std::map<int, cv::Point>>(new std::map<int, cv::Point>());

		// 構造体の初期化
		c->g.h_bit = (int)ceil( log(prj_height+1) / log(2) );
		c->g.w_bit = (int)ceil( log(prj_width+1) / log(2) );
		c->g.all_bit = c->g.h_bit + c->g.w_bit;

		// 保存先のフォルダ生成
		createDirs();
	}

	virtual ~GRAYCODE(){}


	// パターンコード投影 & 撮影
	void code_projection(ProjectionThread *projection);
	// 2値化
	void make_thresh();
	// 初期化
	void makeCorrespondence();

	//// 画像変形・処理
	//// カメラ撮影領域からプロジェクタ投影領域を切り出し
	void transport_camera_projector(const cv::Mat &src, cv::Mat &dst);
	//// 入力画像をカメラ撮影領域に変形
	void transport_projector_camera(const cv::Mat &src, cv::Mat &dst);

	// カメラ座標に対するプロジェクタの対応点を返す
	void getCorrespondProjPoints(std::vector<cv::Point2f> &projPoint, const std::vector<cv::Point2f> &imagePoint);
	// カメラ座標に対するプロジェクタの対応点を返す(高精度版)
	void getCorrespondSubPixelProjPoints(std::vector<cv::Point2f> &projPoint, const std::vector<cv::Point2f> &imagePoint, int size = 20);

	// 対応のとれた点を全て返す
	void getCorrespondAllPoints(std::vector<cv::Point2f> &projPoint, std::vector<cv::Point2f> &imagePoint);


	// プロジェクタ解像度
	int prj_width;
	int prj_height;
	int cam_width;
	int cam_height;


private:
	// ウィンドウネーム
	char* GC;
	char* MP;
	float SHUTTER;	// シャッター速度
	double delay;

	std::unique_ptr<Graycode> g;

	///// グレイコードの作成関連 /////
	// カメラの初期化
	void initCamera();
	// グレイコード作成
	void initGraycode();
	// パターンコード画像作成
	void makeGraycodeImage();
	// ディレクトリの作成
	void createDirs();

	///// 二値化関連 /////
	// カメラ撮影画像を読み込む関数
	void loadCam(cv::Mat &mat, int div_bin, bool flag, bool pattern);
	// 最終的に使用するマスクを生成する関数
	void makeMask(cv::Mat &mask);
	// グレイコードの画像を利用してマスクを生成する関数
	void makeMaskFromCam(cv::Mat &posi, cv::Mat &nega, cv::Mat &result, int thresholdValue = 25);
	// 2値化処理関数 
	void thresh( cv::Mat &posi, cv::Mat &nega, cv::Mat &thresh_img, int thresh_value );

	///// その他 /////
	// プロジェクタ - カメラ構造体初期化
	void initCorrespondence();
	// 2値化コード復元
	void code_restore();

	// 保存フォルダ
	std::string saveFolder;

	CamDev::Camera *camDev;				// カメラデバイス
};


#endif