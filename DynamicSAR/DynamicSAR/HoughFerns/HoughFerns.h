#ifndef HOUGH_FERNS_H
#define HOUGH_FERNS_H

#include <opencv2/opencv.hpp>

#include <Windows.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <glm/gtx/string_cast.hpp>

#include "sample.h"
#include "P-COF.h"
#include "FirstLayerFern.h"
#include "SecondLayerFern.h"


/**
 * @brief   Hough Fernsで画像を検出するクラス
 */
class HoughFerns
{
public:

	// コンストラクタ
	HoughFerns(int _numFerns1, int _numFerns2, int _depth1, int _depth2, int _featureTestNum1, int _featureTestNum2, int _thresholdTestNum1, int _thresholdTestNum2, 
				int _numClass, int _posLabel, int _negLabel, float _posDataPerTree1, float _posDataPerTree2, float _negDataPerTree1, float _negDataPerTree2)
		: numFerns1 (_numFerns1)
		, numFerns2 (_numFerns2)
		, depth1 (_depth1)
		, depth2 (_depth2)
		, featureTestNum1 (_featureTestNum1)
		, featureTestNum2 (_featureTestNum2)
		, thresholdTestNum1 (_thresholdTestNum1)
		, thresholdTestNum2 (_thresholdTestNum2)
		, numClass (_numClass)
		, posLabel (_posLabel)
		, negLabel (_negLabel)
		, posSample_num (0)
		, negSample_num (0)
		, posDataPerTree1 (_posDataPerTree1)
		, posDataPerTree2 (_posDataPerTree2)
		, negDataPerTree1 (_negDataPerTree1)
		, negDataPerTree2 (_negDataPerTree2)
	{}

	HoughFerns()
		: numFerns1 (20)
		, numFerns2 (20)
		, depth1 (20)
		, depth2 (20)
		, featureTestNum1 (15)
		, featureTestNum2 (15)
		, thresholdTestNum1 (6)
		, thresholdTestNum2 (6)
		, numClass (2)
		, posLabel (1)
		, negLabel (0)
		, posSample_num (0)
		, negSample_num (0)
		, posDataPerTree1 (0.25)
		, posDataPerTree2 (0.25)
		, negDataPerTree1 (0.25)
		, negDataPerTree2 (0.25)
	{}

	virtual ~HoughFerns(){}


	// 初期化
	void init(int _numFerns1, int _numFerns2, int _depth1, int _depth2, int _featureTestNum1, int _featureTestNum2, int _thresholdTestNum1, int _thresholdTestNum2, 
				int _numClass, int _posLabel, int _negLabel, float _posDataPerTree1, float _posDataPerTree2, float _negDataPerTree1, float _negDataPerTree2);

	// 設定ファイルを読み込んで初期化
	bool initFromFile(const std::string &fileName);

	// 学習
	void Learn(const std::string& posFolder, const std::string& negFolder, const std::string& saveFolder);

	// 検出
	void detect(const cv::Mat& src, float min_distance, float max_distance, int distance_num, float min_scale, float max_scale, int scale_num,
						int detect_width, int detect_height, float mag_th, int meanshift_radius=64, int win_th = 80, double likelihood_th=0.8);

	// 検出(1層目)
	void detectFirst(const cv::Mat& src, std::vector<cv::Point3f> &detect_point, std::vector<float> &detect_scales, float min_distance, float max_distance, int distance_num, 
						float mag_th, int meanshift_radius=64, int win_th = 80, double likelihood_th=0.8);
	// 検出(2層目)
	void detectSecond(const cv::Mat& src, const cv::Point3f &detect_point, float detect_scale, int detect_num, float min_scale, float max_scale, int scale_num, 
						int detect_width, int detect_height, float mag_th, int meanshift_radius=64, int win_th = 80, double likelihood_th=0.8);

	// 距離方向におけるガウシアンフィルタによる平滑化
	void smoothMap(std::vector<cv::Mat_<float>> &likelihood);

	// 尤度マップの可視化
	void visualizeLikelihood(const std::vector<cv::Mat_<float>> &likelihood, const std::vector<float> scale_rate, float max_value, bool flag);

	// Mean Shiftで極大値推定
	void meanShift(std::vector<cv::Point3f> &meanshiftPoint, std::vector<double> &detectionVote, const std::vector<float> scale_rate, int radius);

	// Nearest Neighborで近傍点の統合
	void nearestNeighbor(std::vector<cv::Point3f> &detectionPoint, std::vector<double> &detectionVote, int win_th);

	// 姿勢方向の確率の推定
	void poseEstimation(std::vector<std::vector<float>> &pose_dist, std::vector<std::vector<cv::Mat_<float>>> &poseMap, 
								const std::vector<cv::Point3f> &detectionPoint, const std::vector<int> scales, const std::vector<float> scale_rate);

	// 正確な姿勢の推定
	void poseEstimation(std::vector<glm::mat4> &pose_est, std::vector<std::vector<std::vector<glm::vec3>>> &axisMap, std::vector<std::vector<std::vector<glm::vec2>>> &angleMap, 
								const std::vector<cv::Point3f> &detectionPoint, const std::vector<int> scales, const std::vector<float> scale_rate);

	// 学習サンプルの取得
	void getLearnSamples(const std::string& posFolder, const std::string& negFolder, std::vector<std::shared_ptr<LearnSample>>& samples);

	// サブセットの作成
	void makeSubset(std::vector<std::shared_ptr<LearnSample>> &subSamples, const std::vector<std::shared_ptr<LearnSample>> &samples, float pos_rate, float neg_rate, int &posSize, int &negSize);
	void makeSubset(std::vector<std::shared_ptr<LearnSample>> &subSamples, const std::vector<std::shared_ptr<LearnSample>> &samples, int pos_num, int neg_num, float pos_rate, float neg_rate, int &posSize, int &negSize);

	// Hough Fernsの保存
	void saveFerns(const std::string &folderName);

	// Hough Fernsの読み込み
	bool loadFerns(const std::string &folderName);

	// モデルとカメラとの距離の取得
	float getDistance() { return distance; };

	// 検出用
	std::vector<cv::Mat_<float>> likelihoodmap;			// 尤度マップ

	// 位置姿勢推定用
	std::vector<cv::Point3f> detectPoint;				// 検出した座標
	std::vector<std::vector<float>> pose_distribute;	// 姿勢確率
	std::vector<glm::mat4> pose_estimate;				// 推定された姿勢


private:

	/***** メンバ変数 *****/
	int numFerns1;				// Fernの数(1層目)
	int numFerns2;				// Fernの数(2層目)
	int depth1;					// Fernの深さ(1層目)
	int depth2;					// Fernの深さ(2層目)
	int featureTestNum1;		// 特徴選択回数(1層目)
	int featureTestNum2;		// 特徴選択回数(2層目)
	int thresholdTestNum1;		// 閾値選択回数(1層目)
	int thresholdTestNum2;		// 閾値選択回数(2層目)
	int numClass;				// クラス
	int posLabel;				// ポジティブ画像のクラス番号(1層目)
	int negLabel;				// ネガティブ画像のクラス番号(2層目)
	int posSample_num;			// ポジティブサンプルの数
	int negSample_num;			// ネガティブサンプルの数
	float posDataPerTree1;		// 個々の木におけるサブセットの割合(ポジティブ)(1層目)
	float posDataPerTree2;		// 個々の木におけるサブセットの割合(ポジティブ)(2層目)
	float negDataPerTree1;		// 個々の木におけるサブセットの割合(ネガティブ)(1層目)
	float negDataPerTree2;		// 個々の木におけるサブセットの割合(ネガティブ)(2層目)

	// 特徴に関して
	P_COF p_cof;				// P-COF特徴
	int patchSize;				// パッチのサイズ
	int gridSize;				// 特徴抽出におけるグリッドサイズ
	glm::vec3 eyeVector;		// カメラの視線方向
	glm::vec3 eyeUp;			// カメラの上方向
	float distance;				// モデルとカメラの距離

	std::vector<std::unique_ptr<FirstLayerFern>> first_ferns;					// 1層目のFernの配列
	std::vector<std::vector<std::unique_ptr<SecondLayerFern>>> second_ferns;	// 2層目のFernの配列

};


#endif