#include "LeastSquaresMethodQuadratic.h"

////////debug
//#include <Windows.h>


/////////debug
//extern bool genelibdebug_global_out_sigmas = false;

LeastSquaresMethodQuadratic::LeastSquaresMethodQuadratic() : LeastSquaresMethod()
{
	clear();
}

LeastSquaresMethodQuadratic::~LeastSquaresMethodQuadratic()
{
}

void LeastSquaresMethodQuadratic::addData(double x, double y)
{
	LeastSquaresMethod::addData(x, y);

	//シグマ値の更新
	sigma_x3 = sigma_x3 * ff + x * x * x;
	sigma_x4 = sigma_x4 * ff + x * x * x * x;
	sigma_x2y = sigma_x2y * ff + x * x * y;

	//係数の計算
	double denominator =
		2.0 * sigma_x * sigma_x_square * sigma_x3 + 
		n * sigma_x_square * sigma_x4 -
		sigma_x * sigma_x * sigma_x4 - 
		n * sigma_x3 * sigma_x3 - 
		sigma_x_square * sigma_x_square * sigma_x_square;

	double numerator_a = 
		n * sigma_x_square * sigma_x2y -
		sigma_x * sigma_x * sigma_x2y +
		sigma_x * sigma_x_square * sigma_xy -
		n * sigma_x3 * sigma_xy +
		sigma_x * sigma_x3 * sigma_y -
		sigma_x_square * sigma_x_square * sigma_y;

	double numerator_b = 
		sigma_x * sigma_x_square * sigma_x2y -
		n * sigma_x3 * sigma_x2y + 
		n * sigma_x4 * sigma_xy - 
		sigma_x_square * sigma_x_square * sigma_xy + 
		sigma_x_square * sigma_x3 * sigma_y - 
		sigma_x * sigma_x4 * sigma_y;

	double numerator_c = 
		-sigma_x_square * sigma_x_square * sigma_x2y + 
		sigma_x * sigma_x3 * sigma_x2y - 
		sigma_x * sigma_x4 * sigma_xy + 
		sigma_x_square * sigma_x3 * sigma_xy -
		sigma_x3 * sigma_x3 * sigma_y + 
		sigma_x_square * sigma_x4 * sigma_y;

	if (denominator != 0.0)
	{
		a = numerator_a / denominator;
		b = numerator_b / denominator;
		c = numerator_c / denominator;
	}
	else
	{
		a = b = c = 0.0;
	}


	////debvug
	//sigma_x = (double)sigma_x;
	//sigma_x_square = (double)sigma_x_square;
	//sigma_x3 = (double)sigma_x3;
	//sigma_x4 = (double)sigma_x4;
	//sigma_y = (double)sigma_y;
	//sigma_xy = (double)sigma_xy;
	//sigma_x2y = (double)sigma_x2y;

	//////debug Matで求める
	//Mat lhand = (Mat_<double>(3, 3) << 
	//	sigma_x4, sigma_x3, sigma_x_square,
	//	sigma_x3, sigma_x_square, sigma_x,
	//	sigma_x_square, sigma_x, n);
	//Mat rhand = (Mat_<double>(3, 1) << 
	//	sigma_x2y, sigma_xy, sigma_y);
	//Mat ans;
	//solve(lhand, rhand, ans, DECOMP_CHOLESKY);

	//double __a = ans.at<double>(0, 0);
	//double __b = ans.at<double>(1, 0);
	//double __c = ans.at<double>(2, 0);

	//////debug
	//printf("%.15lf %.15lf %.15lf %.15lf\n", sigma_x, sigma_x_square, sigma_x3, sigma_x4);
	//printf("  %.15lf %.15lf %.15lf %.15lf\n", n, sigma_y, sigma_xy, sigma_x2y);
	//printf("  %.15lf %.15lf %.15lf / %.15lf\n  %.15lf %.15lf %.15lf\n", numerator_a, numerator_b, numerator_c, denominator, a, b, c);
	//printf("  %.15lf %.15lf %.15lf\n", __a, __b, __c);
	//printf("  %.15lf %.15lf %.15lf\n", __a - a, __b - b, __c - c);

	//a = __a;
	//b = __b;
	//c = __c;

	//if (genelibdebug_global_out_sigmas)
	//{
	//	//コンソールのカーソルを0, 0に
	//	//HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	//	//COORD coord = {0, 0};
	//	//SetConsoleCursorPosition(hConsole, coord);
	//	system("cls");

	//	printf("%.5lf %.5lf\n  %.5lf %.5lf\n", sigma_x, sigma_x_square, sigma_x3, sigma_x4);
	//	printf("  %.5lf %.5lf\n  %.5lf %.5lf\n", n, sigma_y, sigma_xy, sigma_x2y);
	//}
}

void LeastSquaresMethodQuadratic::getAB(double *dst_a, double *dst_b)
{
	LeastSquaresMethod::getAB(dst_a, dst_b);
}

void LeastSquaresMethodQuadratic::getC(double *dst_c)
{
	if (dst_c != NULL)
		*dst_c = c;
}

double LeastSquaresMethodQuadratic::calcYValue(double x)
{
	return a * x * x + b * x + c;
}

void LeastSquaresMethodQuadratic::clear()
{
	LeastSquaresMethod::clear();

	sigma_x3 = sigma_x4 = sigma_x2y = c = 0.0;
}


LSMQuadraticSmoothChange::LSMQuadraticSmoothChange() : LeastSquaresMethodQuadratic()
{
	clear();
}

LSMQuadraticSmoothChange::~LSMQuadraticSmoothChange()
{
}

void LSMQuadraticSmoothChange::addData(double x, double y)
{
	a_before = a;
	b_before = b;
	c_before = c;

	x_smoothend = x_latest_calcY + (x_latest_calcY - x_before) / 1.0;
	x_before = x_latest_calcY;

	LeastSquaresMethodQuadratic::addData(x, y);
}

double LSMQuadraticSmoothChange::calcYValue(double x)
{
	x_latest_calcY = x;

	double ratio = 1.0;

	if ((x_before <= x) && (x <= x_smoothend))
	{
		if (x_smoothend - x_before != 0.0)
		{
			ratio = std::min(1.0, std::max(0.0, (x - x_before) / (x_smoothend - x_before)));
		}
	}

	double inter_a = (1.0 - ratio) * a_before + ratio * a;
	double inter_b = (1.0 - ratio) * b_before + ratio * b;
	double inter_c = (1.0 - ratio) * c_before + ratio * c;

	return inter_a * x * x + inter_b * x + inter_c;
}

void LSMQuadraticSmoothChange::clear()
{
	LeastSquaresMethodQuadratic::clear();
	a_before = b_before = c_before = x_before = x_smoothend = x_latest_calcY = 0.0;
}