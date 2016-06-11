#pragma once

#include "LeastSquaresMethod.h"
#include "LeastSquaresMethodQuadratic.h"
#include <opencv2/opencv.hpp>



//Point3fの最小二乗法クラス
class LSMPoint3f
{
protected:
	LeastSquaresMethod *lsm_x, *lsm_y, *lsm_z;

public:
	//コンストラクタ
	//lsmkind: LSMの種類
	LSMPoint3f(LSMKind lsmkind);
	~LSMPoint3f();

	//忘却係数を設定する
	//(0.0は0割りになるので避ける)
	void setForgetFactor(double factor);

	//新しいデータを追加する
	void addData(double x, cv::Point3f y);

	//傾きと切片を利用してyの値を計算する
	cv::Point3f calcYValue(double x);

	//予測データをすべてクリアする
	void clear();

	//////debug
	void debug_getABC(double *a, double *b, double *c);
};