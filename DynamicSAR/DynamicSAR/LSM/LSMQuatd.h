#pragma once

#include "LeastSquaresMethod.h"
#include "LeastSquaresMethodQuadratic.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>



//クォータニオンの最小二乗法クラス
class LSMQuatd
{
protected:
	LeastSquaresMethod *lsm_x, *lsm_y, *lsm_z, *lsm_w;

public:
	//コンストラクタ
	//lsmkind: LSMの種類
	LSMQuatd(LSMKind lsmkind);
	~LSMQuatd();

	//忘却係数を設定する
	//(0.0は0割りになるので避ける)
	void setForgetFactor(double factor);

	//新しいデータを追加する
	void addData(double x, glm::quat y);

	//傾きと切片を利用してyの値を計算する
	glm::quat calcYValue(double x);

	//予測データをすべてクリアする
	void clear();


	//////debug
	void debug_getABC(double *a, double *b, double *c);
};