#ifndef FIRST_LAYER_FERN_H
#define FIRST_LAYER_FERN_H

#include <opencv2/opencv.hpp>
#include <memory>
#include <iostream>
#include <vector>
#include <random>
#include <glm/gtx/string_cast.hpp>


#include "sample.h"
#include "P-COF.h"



/**
 * @brief   1層目のFern
 */
class FirstLayerFern
{
public:

	/**
	 * @brief   一列のノード(Fernの深さの数用意)
	 */
	struct Node
	{
		cv::Mat_<uchar> patch;		// パッチ(累積勾配方向特徴)
		cv::Mat_<int> weight;		// 重み
		float threshold;			// 類似度の閾値
	};


	/**
	 * @brief   末端ノードのデータ(2^(深さ)のデータ)
	 */
	struct EndNodeData
	{
		std::vector<float> distribution;		// クラス確率
		std::vector<float> poseDistribution;	// 姿勢確率
		std::vector<cv::Point2f> offsetList;	// オフセットベクトル
	};


	// コンストラクタ
	FirstLayerFern(int _depth, int _featureTestNum, int _thresholdTestNum, int _numClass, int _posLabel, int _negLabel, int _gridSize)
		: depth (_depth)
		, featureTestNum (_featureTestNum)
		, thresholdTestNum (_thresholdTestNum)
		, numClass (_numClass)
		, posLabel (_posLabel)
		, negLabel (_negLabel)
		, gridSize (_gridSize)
	{
		p_cof.gridSize = gridSize;
	}

	FirstLayerFern()
		: depth (20)
		, featureTestNum (15)
		, thresholdTestNum (6)
		, numClass (2)
		, posLabel (1)
		, negLabel (0)
		, gridSize (4)
	{
		p_cof.gridSize = gridSize;
	}

	virtual ~FirstLayerFern(){}


	// 学習
	void Learn(const std::vector<std::shared_ptr<LearnSample>>& samples, int posNum, const std::vector<double>& il, const std::vector<double> &poseIl);

	// 識別と投票処理
	void Predict(const cv::Mat& src, cv::Mat_<float>& likelihoodMap, std::vector<cv::Mat_<float>> &poseMap);

	// Fernの保存
	void saveFern(const std::string &fileName, const std::string& fileName_txt);

	// Fernの読み込み
	bool loadFern(const std::string &fileName);

	// Fernの読み込み(テキスト形式)
	bool loadFern_txt(const std::string &fileName);

	/***** メンバ変数 *****/
	int depth;					// Fernの深さ
	int featureTestNum;			// 特徴選択回数
	int thresholdTestNum;		// 閾値選択回数
	int numClass;				// クラス
	int posLabel;				// ポジティブ画像のクラス番号
	int negLabel;				// ネガティブ画像のクラス番号

	P_COF p_cof;				// P-COF特徴
	int gridSize;				// 特徴抽出におけるグリッドサイズ

	// Fernのデータ
	std::vector<std::unique_ptr<Node>> splitNode;			// ノードの分岐に用いるデータ
	std::vector<std::unique_ptr<EndNodeData>> nodeData;		// 末端ノードのデータ
	std::vector<int> nodeLUT;								// 末端ノード(nodeData)の配列番号を格納(ポジティブサンプルが含まれていない場合：-1)


private:

	// 分岐後の情報利得結果(姿勢のクラスも考慮)
	inline double splitMultiInformationGain( const std::vector<std::shared_ptr<LearnSample>> &samples, const std::vector<int> &indices, const std::vector<float> &similarity, int &leftCount, int &rightCount, float threshold, const std::vector<double> &il, const std::vector<double> &poseIl);

	// 分岐後のオフセットの分散結果(姿勢のクラスも考慮)
	inline double splitMultiVariance( const std::vector<std::shared_ptr<LearnSample>> &samples, const std::vector<int> &indices, const std::vector<float> &similarity, int &leftCount, int &rightCount, double threshold);

	// シャノンエントロピー関数
	inline double computeEntropy(const std::vector<double> &dist);

	// シャノンエントロピー関数(姿勢用)
	inline double computePoseEntropy(const std::vector<double> &dist);
};


#endif