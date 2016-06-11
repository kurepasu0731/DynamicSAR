#include "LeastSquaresMethod.h"


LeastSquaresMethod::LeastSquaresMethod()
{
	ff = 1.0;
	clear();
}

LeastSquaresMethod::~LeastSquaresMethod()
{
}

void LeastSquaresMethod::setForgetFactor(double factor)
{
	ff = factor;
}


void LeastSquaresMethod::addData(double x, double y)
{
	n = n * ff + 1.0;

	sigma_x = sigma_x * ff + x;
	sigma_y = sigma_y * ff + y;
	sigma_x_square = sigma_x_square * ff + x * x;
	sigma_xy = sigma_xy * ff + x * y;

	double denominator = n * sigma_x_square - sigma_x * sigma_x;

	if (denominator != 0.0)
	{
		a = (n * sigma_xy - sigma_x * sigma_y) / denominator;
		b = (sigma_x_square * sigma_y - sigma_xy * sigma_x) / denominator;
	}
	else
	{
		a = b = 0.0;
	}
}

void LeastSquaresMethod::getAB(double *dst_a, double *dst_b)
{
	if (dst_a != NULL)
		*dst_a = a;

	if (dst_b != NULL)
		*dst_b = b;
}

double LeastSquaresMethod::calcYValue(double x)
{
	return a * x + b;
}

void LeastSquaresMethod::clear()
{
	n = 0.0;
	sigma_x = sigma_y = sigma_x_square = sigma_xy = 0.0;
	a = b = 0.0;
}




LeastSquaresMethodSmoothChange::LeastSquaresMethodSmoothChange() : LeastSquaresMethod()
{
	clear();
}

LeastSquaresMethodSmoothChange::~LeastSquaresMethodSmoothChange()
{
}

void LeastSquaresMethodSmoothChange::addData(double x, double y)
{
	a_before = a;
	b_before = b;

	x_smoothend = x_latest_calcY + (x_latest_calcY - x_before) / 1.0;
	x_before = x_latest_calcY;

	LeastSquaresMethod::addData(x, y);
}

void LeastSquaresMethodSmoothChange::getAB(double *dst_a, double *dst_b)
{
	if (dst_a != NULL)
		*dst_a = inter_a;

	if (dst_b != NULL)
		*dst_b = inter_b;
}

double LeastSquaresMethodSmoothChange::calcYValue(double x)
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

	inter_a = (1.0 - ratio) * a_before + ratio * a;
	inter_b = (1.0 - ratio) * b_before + ratio * b;
		
	return inter_a * x + inter_b;
}

void LeastSquaresMethodSmoothChange::clear()
{
	LeastSquaresMethod::clear();
	a_before = b_before = x_before = x_smoothend = x_latest_calcY = 0.0;
	inter_a = inter_b = 0.0;
}
