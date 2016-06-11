#include "LSMQuatd.h"

LSMQuatd::LSMQuatd(LSMKind lsmkind)
{
	switch (lsmkind)
	{
	case LSM:
		lsm_x = new LeastSquaresMethod();
		lsm_y = new LeastSquaresMethod();
		lsm_z = new LeastSquaresMethod();
		lsm_w = new LeastSquaresMethod();
		break;
	case LSM_SmoothChange:
		lsm_x = new LeastSquaresMethodSmoothChange();
		lsm_y = new LeastSquaresMethodSmoothChange();
		lsm_z = new LeastSquaresMethodSmoothChange();
		lsm_w = new LeastSquaresMethodSmoothChange();
		break;
	case LSMQuadratic:
		lsm_x = new LeastSquaresMethodQuadratic();
		lsm_y = new LeastSquaresMethodQuadratic();
		lsm_z = new LeastSquaresMethodQuadratic();
		lsm_w = new LeastSquaresMethodQuadratic();
		break;
	case LSMQuadratic_SmoothChange:
		lsm_x = new LSMQuadraticSmoothChange();
		lsm_y = new LSMQuadraticSmoothChange();
		lsm_z = new LSMQuadraticSmoothChange();
		lsm_w = new LSMQuadraticSmoothChange();
		break;
	default:
		lsm_x = new LeastSquaresMethod();
		lsm_y = new LeastSquaresMethod();
		lsm_z = new LeastSquaresMethod();
		lsm_w = new LeastSquaresMethod();
		break;
	}
}

LSMQuatd::~LSMQuatd()
{
	delete lsm_x;
	delete lsm_y;
	delete lsm_z;
	delete lsm_w;
}

void LSMQuatd::setForgetFactor(double factor)
{
	lsm_x->setForgetFactor(factor);
	lsm_y->setForgetFactor(factor);
	lsm_z->setForgetFactor(factor);
	lsm_w->setForgetFactor(factor);
}

void LSMQuatd::addData(double x, glm::quat y)
{
	y = glm::normalize(y);

	lsm_x->addData(x, y.x);
	lsm_y->addData(x, y.y);
	lsm_z->addData(x, y.z);
	lsm_w->addData(x, y.w);
}


glm::quat LSMQuatd::calcYValue(double x)
{
	glm::quat q(
		lsm_w->calcYValue(x),
		lsm_x->calcYValue(x),
		lsm_y->calcYValue(x),
		lsm_z->calcYValue(x)
		);

	return glm::normalize(q);
}

void LSMQuatd::clear()
{
	lsm_x->clear();
	lsm_y->clear();
	lsm_z->clear();
	lsm_w->clear();
}

void LSMQuatd::debug_getABC(double *a, double *b, double *c)
{
	if ((a != NULL) && (b != NULL))
	{
		lsm_x->getAB(a, b);
	}

	LeastSquaresMethodQuadratic *qlsm = dynamic_cast<LeastSquaresMethodQuadratic *>(lsm_x);
	if ((qlsm != NULL) && (c != NULL))
	{
		qlsm->getC(c);
	}
}