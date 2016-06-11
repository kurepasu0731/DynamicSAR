#pragma once

#include "LeastSquaresMethod.h"
#include <opencv2/opencv.hpp>


//忘却係数付き最小二乗法クラス (2次関数近似)
class LeastSquaresMethodQuadratic : public LeastSquaresMethod
{
protected:
	double sigma_x3;
	double sigma_x4;
	double sigma_x2y;
	double c;


public:
	LeastSquaresMethodQuadratic();
	virtual ~LeastSquaresMethodQuadratic();

	//新しいデータ(x, y)を追加する
	virtual void addData(double x, double y);

	//aとbを取得する (y = ax^2 + bx + c)
	virtual void getAB(double *dst_a, double *dst_b);

	//cを取得する (y = ax^2 + bx + c)
	virtual void getC(double *dst_c);

	//近似曲線を利用してyの値を計算する
	virtual double calcYValue(double x);

	//予測データをクリアする
	virtual void clear();
};


/////////debug
//extern bool genelibdebug_global_out_sigmas;
	
class LSMQuadraticSmoothChange : public LeastSquaresMethodQuadratic
{
protected:
	double a_before;
	double b_before;
	double c_before;
	double x_before;
	double x_smoothend;
	double x_latest_calcY;

public:
	LSMQuadraticSmoothChange();
	virtual ~LSMQuadraticSmoothChange();

	virtual void addData(double x, double y);
	virtual double calcYValue(double x);
	virtual void clear();
};


//最小二乗法の種類
enum LSMKind
{
	LSM,
	LSM_SmoothChange,
	LSMQuadratic,
	LSMQuadratic_SmoothChange
};