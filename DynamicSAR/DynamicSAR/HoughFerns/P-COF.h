#ifndef P_COF_H
#define P_COF_H

#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <fstream>


/**
 * @brief   Perspectively-Cumulated Orientation Feature
 */
class P_COF
{
public:

	/**
	 * @brief   テンプレートの構造体
	 */
	struct Template
	{
		int width;						// 元のテンプレートの横幅
		int height;						// 元のテンプレートの縦幅
		cv::Mat_<uchar> oriMat;			// 量子化勾配方向
		cv::Mat_<int> weightMat;		// 重み
	};


	// オブジェクト毎に紐づいたテンプレート画像
	typedef std::map<std::string, std::vector<Template> > TemplatesMap;


	/**
	 * @brief   マッチングの構造体
	 */
	struct Match
	{
		int x;					// x座標
		int y;					// y座標
		int width;				// 幅
		int height;				// 高さ
		float similarity;		// 類似度
		int template_num;		// テンプレート番号
		std::string class_id;	// クラス名

		Match(int _x, int _y, int _width, int _height, float _similarity, int _template_num, std::string _class_id)
			: x (_x)
			, y (_y)
			, width (_width)
			, height (_height)
			, similarity (_similarity)
			, template_num (_template_num)
			, class_id (_class_id){}
	};


	P_COF() : gridSize (4){};
	P_COF(int _gridSize)	
		: gridSize (_gridSize)
	{}

	virtual ~P_COF(){}

	// 画像から勾配方向の積分画像を計算
	void calcIntegralOrientation(const cv::Mat& src, std::vector<cv::Mat_<int>>& integral_angle, float threshold, bool smooth=false, int kernel_size=3);

	// 積分画像から勾配方向の量子化
	void quantizedIntegraOrientation(cv::Mat& quantized_angle, const std::vector<cv::Mat_<int>>& integral_angle, int freq_th, int grid_size);

	// 勾配方向の拡張
	void spreadOrientation(const cv::Mat& src, cv::Mat& dst, int T);

	// OR演算子で勾配方向を結合
	void orCombine(const uchar * src, const int src_stride, uchar * dst, const int dst_stride, const int width, const int height);

	// 量子化勾配方向を累積
	void cumulatedOrientation(const cv::Mat& quantized_angle, std::vector<cv::Mat>& orientationFreq);

	// 類似度の計算
	void calcSimilarity(const cv::Mat& quantized_angle, const Template& templ, cv::Mat& dst);

	// 類似度の計算
	void calcSimilarity(const cv::Mat& src, const cv::Mat& oriMat, const cv::Mat& weightMat, cv::Mat& dst);

	// テンプレート画像の追加
	void addTemplate(const std::vector<cv::Mat>& src, const std::string& class_id, float mag_th, int freq_th);

	// テンプレート画像とのマッチング
	void matchTemplate(const cv::Mat& src, std::vector<Match>& matches, float mag_th, float similarity_th);
	
	// 近傍点の統合
	void nearestNeighbor(std::vector<Match>& matches, int win_th);

	// テンプレートの保存
	bool saveTemplate(const std::string& savePath, const std::string& class_id);

	// テンプレートの読み込み
	bool loadTemplate(const std::string& savePath, const std::string& class_id);


	// ラベルを返す(2進数→勾配方向)
	inline int getLabel(int quantized)
	{
		switch (quantized)
		{
			case 1:   return 0;
			case 2:   return 1;
			case 4:   return 2;
			case 8:   return 3;
			case 16:  return 4;
			case 32:  return 5;
			case 64:  return 6;
			case 128: return 7;
			default:
				std::cerr << "Invalid value of quantized parameter:" <<  quantized << std::endl;
				return -1; 
		}
	}

	// Bitの数値を返す(勾配方向→2進数)
	inline int getBitNum(int bit)
	{
		switch (bit)
		{
			case 0:   return 1;
			case 1:   return 2;
			case 2:   return 4;
			case 3:   return 8;
			case 4:   return 16;
			case 5:   return 32;
			case 6:   return 64;
			case 7:   return 128;
			default:
				std::cerr << "Invalid value of quantized parameter" << std::endl;
				return -1; 
		}
	}

	// テンプレート画像
	TemplatesMap class_templates;

	// グリッドサイズ
	int	gridSize;
};


#endif