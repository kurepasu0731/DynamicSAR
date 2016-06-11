#pragma once

#include <stdlib.h>
#include <algorithm>


//忘却係数付き最小二乗法クラス (1次関数近似)
class LeastSquaresMethod
{
protected:
	double n;
	double sigma_x;
	double sigma_y;
	double sigma_x_square;
	double sigma_xy;
	double a;
	double b;
	double ff;

public:
	LeastSquaresMethod();
	virtual ~LeastSquaresMethod();

	//忘却係数を設定する
	//(0.0は0割りになるので避ける)
	virtual void setForgetFactor(double factor);

	//新しいデータ(x, y)を追加する
	virtual void addData(double x, double y);

	//直線の傾きと切片を取得する
	virtual void getAB(double *dst_a, double *dst_b);

	//傾きと切片を利用してyの値を計算する
	virtual double calcYValue(double x);

	//予測データをクリアする
	virtual void clear();
};


class LeastSquaresMethodSmoothChange : public LeastSquaresMethod
{
protected:
	double a_before;
	double b_before;
	double x_before;
	double x_smoothend;
	double x_latest_calcY;
	double inter_a;
	double inter_b;

public:
	LeastSquaresMethodSmoothChange();
	virtual ~LeastSquaresMethodSmoothChange();

	virtual void addData(double x, double y);
	virtual void getAB(double *dst_a, double *dst_b);
	virtual double calcYValue(double x);
	virtual void clear();
};