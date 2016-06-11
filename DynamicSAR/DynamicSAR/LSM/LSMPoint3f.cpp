#include "LSMPoint3f.h"

LSMPoint3f::LSMPoint3f(LSMKind lsmkind)
{
	switch (lsmkind)
	{
	case LSM:
		lsm_x = new LeastSquaresMethod();
		lsm_y = new LeastSquaresMethod();
		lsm_z = new LeastSquaresMethod();
		break;
	case LSM_SmoothChange:
		lsm_x = new LeastSquaresMethodSmoothChange();
		lsm_y = new LeastSquaresMethodSmoothChange();
		lsm_z = new LeastSquaresMethodSmoothChange();
		break;
	case LSMQuadratic:
		lsm_x = new LeastSquaresMethodQuadratic();
		lsm_y = new LeastSquaresMethodQuadratic();
		lsm_z = new LeastSquaresMethodQuadratic();
		break;
	case LSMQuadratic_SmoothChange:
		lsm_x = new LSMQuadraticSmoothChange();
		lsm_y = new LSMQuadraticSmoothChange();
		lsm_z = new LSMQuadraticSmoothChange();
		break;
	default:
		lsm_x = new LeastSquaresMethod();
		lsm_y = new LeastSquaresMethod();
		lsm_z = new LeastSquaresMethod();
		break;
	}
}

LSMPoint3f::~LSMPoint3f()
{
	delete lsm_x;
	delete lsm_y;
	delete lsm_z;
}


void LSMPoint3f::setForgetFactor(double factor)
{
	lsm_x->setForgetFactor(factor);
	lsm_y->setForgetFactor(factor);
	lsm_z->setForgetFactor(factor);
}

void LSMPoint3f::addData(double x, cv::Point3f y)
{
	lsm_x->addData(x, y.x);
	lsm_y->addData(x, y.y);


	//////////debug
	//genelib::genelibdebug_global_out_sigmas = true;
	lsm_z->addData(x, y.z);
	//genelib::genelibdebug_global_out_sigmas = false;
}


cv::Point3f LSMPoint3f::calcYValue(double x)
{
	cv::Point3f p(
		(float)lsm_x->calcYValue(x),
		(float)lsm_y->calcYValue(x),
		(float)lsm_z->calcYValue(x)
		);

	return p;
}

void LSMPoint3f::clear()
{
	lsm_x->clear();
	lsm_y->clear();
	lsm_z->clear();
}

void LSMPoint3f::debug_getABC(double *a, double *b, double *c)
{
	if ((a != NULL) && (b != NULL))
	{
		lsm_z->getAB(a, b);
	}

	LeastSquaresMethodQuadratic *qlsm = dynamic_cast<LeastSquaresMethodQuadratic *>(lsm_z);
	if ((qlsm != NULL) && (c != NULL))
	{
		qlsm->getC(c);
	}
}