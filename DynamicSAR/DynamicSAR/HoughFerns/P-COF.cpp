#include "P-COF.h"



/**
 * @brief   画像から勾配方向の積分画像を計算
 * 
 * @param   src[in]					入力画像
 * @param   integral_angle[in,out]	勾配方向の積分画像		
 * @param   threshold[in]			勾配強度の閾値(dx*dx+dy*dy)
 * @param   smooth[in]				ガウシアンフィルタをするかどうか
 * @param   kernel_size[in]			ガウシアンフィルタのカーネルサイズ
 *
 * @note	3channelの場合は最も勾配強度の大きいものを用いる
 */
void P_COF::calcIntegralOrientation(const cv::Mat& src, std::vector<cv::Mat_<int>>& integral_angle, float threshold, bool smooth, int kernel_size)
{
	// 勾配強度の初期化
	cv::Mat angle;
	cv::Mat magnitude(src.size(), CV_32F);

	cv::Size size = src.size();
	cv::Mat sobel_3dx;					// x方向の勾配強度(3channel)
	cv::Mat sobel_3dy;					// y方向の勾配強度(3channel)
	cv::Mat sobel_dx(size, CV_32F);		// x方向の勾配強度(1channel)
	cv::Mat sobel_dy(size, CV_32F);		// y方向の勾配強度(1channel)
	cv::Mat sobel_ag;					// 勾配方向(量子化前)
	cv::Mat smoothed;

	if (smooth) {
		// ガウシアンフィルタ
		cv::GaussianBlur(src, smoothed, cv::Size(kernel_size, kernel_size), 0, 0, cv::BORDER_REPLICATE);
	} else {
		smoothed = src.clone();
	}

	// 3channelの場合
	if (src.channels() == 3)
	{
		// ソーベルフィルタ
		cv::Sobel(smoothed, sobel_3dx, CV_16S, 1, 0, 3, 1.0, 0.0, cv::BORDER_REPLICATE);
		cv::Sobel(smoothed, sobel_3dy, CV_16S, 0, 1, 3, 1.0, 0.0, cv::BORDER_REPLICATE);

		// ポインタ
		short * ptrx  = (short *)sobel_3dx.data;
		short * ptry  = (short *)sobel_3dy.data;
		float * ptr0x = (float *)sobel_dx.data;
		float * ptr0y = (float *)sobel_dy.data;
		float * ptrmg = (float *)magnitude.data;

		// 1ステップのチャンネル総数
		const int length1 = static_cast<const int>(sobel_3dx.step1());
		const int length2 = static_cast<const int>(sobel_3dy.step1());
		const int length3 = static_cast<const int>(sobel_dx.step1());
		const int length4 = static_cast<const int>(sobel_dy.step1());
		const int length5 = static_cast<const int>(magnitude.step1());
		const int length0 = sobel_3dy.cols * 3;

		// BGRのうち最も大きい勾配を用いる
		for (int r = 0; r < sobel_3dy.rows; ++r)
		{
			int ind = 0;

			for (int i = 0; i < length0; i += 3)
			{
				// BGRそれぞれの勾配強度
				int magB = ptrx[i + 0] * ptrx[i + 0] + ptry[i + 0] * ptry[i + 0];
				int magG = ptrx[i + 1] * ptrx[i + 1] + ptry[i + 1] * ptry[i + 1];
				int magR = ptrx[i + 2] * ptrx[i + 2] + ptry[i + 2] * ptry[i + 2];

				if (magB >= magG && magB >= magR)
				{
					ptr0x[ind] = ptrx[i];
					ptr0y[ind] = ptry[i];
					ptrmg[ind] = (float)magB;
				}
				else if (magG >= magB && magG >= magR)
				{
					ptr0x[ind] = ptrx[i + 1];
					ptr0y[ind] = ptry[i + 1];
					ptrmg[ind] = (float)magG;
				}
				else
				{
					ptr0x[ind] = ptrx[i + 2];
					ptr0y[ind] = ptry[i + 2];
					ptrmg[ind] = (float)magR;
				}
				++ind;
			}
			// 次の行に移動
			ptrx += length1;
			ptry += length2;
			ptr0x += length3;
			ptr0y += length4;
			ptrmg += length5;
		}
	}
	// 1channelの場合
	else if(src.channels() == 1)
	{
		// ソーベルフィルタ
		cv::Sobel(smoothed, sobel_dx, CV_32F, 1, 0, 3, 1.0, 0.0, cv::BORDER_REPLICATE);
		cv::Sobel(smoothed, sobel_dy, CV_32F, 0, 1, 3, 1.0, 0.0, cv::BORDER_REPLICATE);

		// ポインタ
		float * ptr0x = (float *)sobel_dx.data;
		float * ptr0y = (float *)sobel_dy.data;
		float * ptrmg = (float *)magnitude.data;

		// 1ステップのチャンネル総数
		const int length1 = static_cast<const int>(sobel_dx.step1());
		const int length2 = static_cast<const int>(sobel_dy.step1());
		const int length3 = static_cast<const int>(magnitude.step1());

		// 勾配強度
		for (int r = 0; r < sobel_dy.rows; ++r)
		{
			for (int c = 0; c < sobel_dy.cols; ++c)
			{
				ptrmg[c] = ptr0x[c] * ptr0x[c] + ptr0y[c] * ptr0y[c];
			}
			// 次の行に移動
			ptr0x += length1;
			ptr0y += length2;
			ptrmg += length3;
		}
	}

	// 勾配方向の推定
	cv::phase(sobel_dx, sobel_dy, angle, true);


	// 360°を16分割
	cv::Mat_<unsigned char> quantized_unfiltered;
	angle.convertTo(quantized_unfiltered, CV_8U, 16.0 / 360.0);

	// 180°反対側の勾配方向を同じ方向とする(16分割→8分割)
	for (int r = 0; r < angle.rows; ++r)
	{
		uchar* quant_r = quantized_unfiltered.ptr<uchar>(r);
		for (int c = 0; c < angle.cols; ++c)
		{
			// 4ビット目を無視
			quant_r[c] &= 7;
		}
	}


	// 角度別の積分画像の初期化
    std::vector<cv::Mat_<uchar>> bins(8);
	for(int i = 0; i < 8; ++i){
		bins[i] = cv::Mat_<uchar>::zeros(smoothed.size());
	}

	// 勾配方向別に頻度を格納
    for (int r = 0; r < quantized_unfiltered.rows; ++r) 
	{
		float* mag_r = magnitude.ptr<float>(r);					// 勾配強度
        for (int c = 0; c < quantized_unfiltered.cols; ++c) 
		{
			// 勾配強度が閾値以上であれば
			if (mag_r[c] > threshold)
			{
				int ind = (int)quantized_unfiltered(r, c);
				bins[ind](r, c)++;		// 勾配数でインクリメント
			}
        }
    }
 
    // 勾配方向別に積分画像生成
    integral_angle.resize(8);

    for (int i = 0; i < 8; ++i) 
	{
        cv::integral(bins[i], integral_angle[i]);
    }
}


/**
 * @brief   積分画像から勾配方向の量子化(グリッドサイズ内の勾配方向を頻度の多い勾配方向で代表する)
 * 
 * @param   quantized_angle[in,out]		量子化後の勾配方向(8bit)	
 * @param   integral_angle[in]			勾配方向の積分画像		
 * @param   freq_th[in]					勾配方向の頻度による閾値	
 * @param   grid_size[in]				グリッドサイズ
 */
void P_COF::quantizedIntegraOrientation(cv::Mat& quantized_angle, const std::vector<cv::Mat_<int>>& integral_angle, int freq_th, int grid_size)
{
	// 新しいウィンドウサイズ(グリッドサイズで割った際の余り部分は除外)
	int window_width = (integral_angle[0].cols-1) / grid_size;
	int window_height = (integral_angle[0].rows-1) / grid_size;

	
	// 初期化
	quantized_angle = cv::Mat::zeros(window_height, window_width, CV_8U);


	// グリッド領域内の勾配方向を推定
	for (int r = 0; r < window_height; ++r)
	{
		uchar* quant_r = quantized_angle.ptr<uchar>(r);
		for (int c = 0; c < window_width; ++c)
		{
			uchar ori = 0;	// 勾配方向

			// 勾配方向の頻度
			for (int i = 0; i < 8; ++i)
			{
				// 積分画像から矩形内の総和を得る
				int ori_freq = integral_angle[i](r*grid_size, c*grid_size) + integral_angle[i]((r+1)*grid_size, (c+1)*grid_size) - (integral_angle[i]((r+1)*grid_size, c*grid_size) + integral_angle[i](r*grid_size, (c+1)*grid_size));
				
				// 閾値以上であれば
				if (ori_freq > freq_th)
				{
					ori += uchar(1 << i);		// 勾配方向の追加
				}
			}

			// 領域内の勾配方向
			quant_r[c] = ori;
		}
	}
}


/**
 * @brief   勾配方向の拡張
 *			Implements "2.3 Spreading the Orientations"
 * 
 * @param   src[in]				量子化された勾配方向画像(8bit)
 * @param   dst[in,out]			拡張後の勾配方向画像		
 * @param   T[in]				拡張ステップ数(左上方向にT pixelずらしてOR演算子で方向を結合	※2.3節の図とは少し異なる)	
 */
void P_COF::spreadOrientation(const cv::Mat& src, cv::Mat& dst, int T)
{
	// 初期化
	dst = cv::Mat::zeros(src.size(), CV_8U);

	// 勾配方向を拡張
	for (int r = 0; r < T; ++r)
	{
		int height = src.rows - r;
		for (int c = 0; c < T; ++c)
		{
			// ずらした勾配方向をORで結合
			orCombine(&src.at<unsigned char>(r, c), static_cast<const int>(src.step1()), dst.ptr(), static_cast<const int>(dst.step1()), src.cols - c, height);
		}
	}
}


/**
 * @brief   OR演算子で勾配方向を結合
 *			Implements "2.3 Spreading the Orientations"
 * 
 * @param   src[in]				ずらした勾配方向画像(8bit)
 * @param   src_stride[in]		入力画像の1ステップ数
 * @param   dst[in,out]			結合後の勾配方向画像		
 * @param   dst_stride[in]		出力画像の1ステップ数
 * @param   width[in]			入力画像の横幅
 * @param   height[in]			入力画像の縦幅
 */
void P_COF::orCombine(const uchar * src, const int src_stride, uchar * dst, const int dst_stride, const int width, const int height)
{
#if 1
	volatile bool haveSSE3 = cv::checkHardwareSupport(CV_CPU_SSE3);
#endif
#if 0
	volatile bool haveSSE2 = cv::checkHardwareSupport(CV_CPU_SSE2);
#endif


	// SIMD命令を使用しOR演算
	for (int r = 0; r < height; ++r)
	{
		int c = 0;

#if 1
		// SSE3が使えるとき
		if (haveSSE3)
		{
			for ( ; c < width - 15; c += 16)
			{
				__m128i val = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src + c));
				__m128i dst_ptr = _mm_lddqu_si128(reinterpret_cast<__m128i*>(dst + c));
				dst_ptr = _mm_or_si128(dst_ptr, val);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst + c), dst_ptr);
			}
		}
		else
#endif
#if 0
		// SSE2が使えるとき
		if (haveSSE2)
		{
			for ( ; c < width - 15; c += 16)
			{
				__m128i val = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src + c));
				__m128i dst_ptr = _mm_loadu_si128(reinterpret_cast<__m128i*>(dst + c));
				dst_ptr = _mm_or_si128(dst_ptr, val);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst + c), dst_ptr);
			}
		}
#endif
		for ( ; c < width; ++c) {
			dst[c] |= src[c];
		}

		// Advance to next row
		src += src_stride;
		dst += dst_stride;
	}
}


/**
 * @brief   量子化勾配方向を累積
 * 
 * @param	quantized_angle[in]			量子化勾配方向
 * @param	orientationFreq[in,out]		勾配の頻度
 */
void P_COF::cumulatedOrientation(const cv::Mat& quantized_angle, std::vector<cv::Mat>& orientationFreq)
{
	// 勾配を累積させる
	for (int r = 0; r < quantized_angle.rows; ++r) 
	{
		const uchar* quant_r = quantized_angle.ptr<uchar>(r);
		for (int c = 0; c < quantized_angle.cols; ++c) 
		{
			if (quant_r[c] != 0) {
				int histogram[8] = {0, 0, 0, 0, 0, 0, 0, 0};
				for (int h = 0; h < 8; ++h)
				{
					if ( (quant_r[c] & uchar(1 << h)) > 0 ) {
						// 勾配方向のカウント
						orientationFreq[h].at<int>(r,c) += 1;
					}
				}
			}
		}
	}
}


/**
 * @brief   類似度の計算
 * 
 * @param	quantized_angle[in]		量子化勾配方向
 * @param	templ[in]				テンプレート
 * @param	dst[in,out]				類似度画像(CV_32F)
 */
void P_COF::calcSimilarity(const cv::Mat& quantized_angle, const Template& templ, cv::Mat& dst)
{
#if 1
	volatile bool haveSSE3 = cv::checkHardwareSupport(CV_CPU_SSE3);
#endif

	// スライディングウィンドウの範囲
	int height = quantized_angle.rows - templ.oriMat.rows + 1;
	int width = quantized_angle.cols - templ.oriMat.cols + 1;
	int t_height = templ.oriMat.rows;
	int t_width = templ.oriMat.cols;

	// 類似度マップの初期化
	dst = cv::Mat::zeros(height, width, CV_32F);

	// 重みの総和
	cv::Scalar s = cv::sum(templ.weightMat);
	int sum_weight = s(0);


	// テンプレート
	for (int tr = 0; tr < t_height; ++tr)
	{
		const uchar* ori_r = templ.oriMat.ptr<uchar>(tr);
		const int* weight_r = templ.weightMat.ptr<int>(tr);
		
		for (int tc = 0; tc < t_width; ++tc)
		{
			// 勾配があれば
			if (ori_r[tc] > 0)
			{
				// スライディングウィンドウ
				for (int r = 0; r < height; ++r)
				{
					float* dst_ptr = dst.ptr<float>(r);
					const uchar* quant_r = quantized_angle.ptr<uchar>(r+tr);

					int c = 0;

#if 1
					// SSE3が使えるとき
					if (haveSSE3)
					{
						for ( ; c < width - 15; c += 16)
						{
							__m128i quant_sse = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(quant_r + c + tc));	// 量子化勾配方向
							__m128 weight_sse = _mm_set1_ps(static_cast<float>(weight_r[tc]));		// 特徴の重み
							__m128i and = _mm_and_si128( quant_sse, _mm_set1_epi8(ori_r[tc]));		// 論理積

							// 8ビットから32ビットへ変換
							__m128i and1 = _mm_cvtepu8_epi32(and);		
							__m128 and1f = _mm_cvtepi32_ps(and1);								// 整数型から浮動小数点型へ
							__m128 mask1 = _mm_cmpgt_ps( and1f, _mm_set1_ps(0.f));				// 条件判定
							__m128 dst_sse1 = _mm_loadu_ps(dst_ptr + c);
							dst_sse1 = _mm_add_ps(dst_sse1, _mm_and_ps(mask1, weight_sse));		// 重みの和
							_mm_storeu_ps((dst_ptr + c), dst_sse1);

							// 8ビットから32ビットへ変換
							and = _mm_srli_si128(and, 4);
							__m128i and2 = _mm_cvtepu8_epi32(and);
							__m128 and2f = _mm_cvtepi32_ps(and2);
							__m128 mask2 = _mm_cmpgt_ps( and2f, _mm_set1_ps(0.f));
							__m128 dst_sse2 = _mm_loadu_ps(dst_ptr + c + 4);
							dst_sse2 = _mm_add_ps(dst_sse2, _mm_and_ps(mask2, weight_sse));
							_mm_storeu_ps((dst_ptr + c + 4), dst_sse2);

							// 8ビットから32ビットへ変換
							and = _mm_srli_si128(and, 4);
							__m128i and3 = _mm_cvtepu8_epi32(and);
							__m128 and3f = _mm_cvtepi32_ps(and3);
							__m128 mask3 = _mm_cmpgt_ps( and3f, _mm_set1_ps(0.f));
							__m128 dst_sse3 = _mm_loadu_ps(dst_ptr + c + 8);
							dst_sse3 = _mm_add_ps(dst_sse3, _mm_and_ps(mask3, weight_sse));
							_mm_storeu_ps((dst_ptr + c + 8), dst_sse3);

							// 8ビットから32ビットへ変換
							and = _mm_srli_si128(and, 4);
							__m128i and4 = _mm_cvtepu8_epi32(and);
							__m128 and4f = _mm_cvtepi32_ps(and4);
							__m128 mask4 = _mm_cmpgt_ps( and4f, _mm_set1_ps(0.f));
							__m128 dst_sse4 = _mm_loadu_ps(dst_ptr + c + 12);
							dst_sse4 = _mm_add_ps(dst_sse4, _mm_and_ps(mask4, weight_sse));
							_mm_storeu_ps((dst_ptr + c + 12), dst_sse4);
						}
					}
#endif


					for ( ; c < width; ++c)
					{	
						// 論理積が0より大きければ
						if ((quant_r[c+tc] & ori_r[tc]) > 0) 
						{
							dst_ptr[c] += weight_r[tc];
						}
					}
				}
			}
		}
	}

	// 総和で除算
	dst /= (float)sum_weight;
}



/**
 * @brief   類似度の計算
 * 
 * @param	src[in]					量子化勾配方向
 * @param	oriMat[in]				累積勾配方向特徴(テンプレート)
 * @param	weightMat[in]			重み(テンプレート)
 * @param	dst[in,out]				類似度画像(CV_32F)
 */
void P_COF::calcSimilarity(const cv::Mat& src, const cv::Mat& oriMat, const cv::Mat& weightMat, cv::Mat& dst)
{
#if 1
	volatile bool haveSSE3 = cv::checkHardwareSupport(CV_CPU_SSE3);
#endif

	// スライディングウィンドウの範囲
	int height = src.rows - oriMat.rows + 1;
	int width = src.cols - oriMat.cols + 1;
	int t_height = oriMat.rows;
	int t_width = oriMat.cols;

	// 類似度マップの初期化
	dst = cv::Mat::zeros(height, width, CV_32F);

	// 重みの総和
	cv::Scalar s = cv::sum(weightMat);
	int sum_weight = s(0);


	// テンプレート
	for (int tr = 0; tr < t_height; ++tr)
	{
		const uchar* ori_r = oriMat.ptr<uchar>(tr);
		const int* weight_r = weightMat.ptr<int>(tr);
		
		for (int tc = 0; tc < t_width; ++tc)
		{
			// 勾配があれば
			if (ori_r[tc] > 0)
			{
				// スライディングウィンドウ
				for (int r = 0; r < height; ++r)
				{
					float* dst_ptr = dst.ptr<float>(r);
					const uchar* quant_r = src.ptr<uchar>(r+tr);

					int c = 0;

#if 1
					// SSE3が使えるとき
					if (haveSSE3)
					{
						for ( ; c < width - 15; c += 16)
						{
							__m128i quant_sse = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(quant_r + c + tc));	// 量子化勾配方向
							__m128 weight_sse = _mm_set1_ps(static_cast<float>(weight_r[tc]));		// 特徴の重み
							__m128i and = _mm_and_si128( quant_sse, _mm_set1_epi8(ori_r[tc]));		// 論理積

							// 8ビットから32ビットへ変換
							__m128i and1 = _mm_cvtepu8_epi32(and);		
							__m128 and1f = _mm_cvtepi32_ps(and1);								// 整数型から浮動小数点型へ
							__m128 mask1 = _mm_cmpgt_ps( and1f, _mm_set1_ps(0.f));				// 条件判定
							__m128 dst_sse1 = _mm_loadu_ps(dst_ptr + c);
							dst_sse1 = _mm_add_ps(dst_sse1, _mm_and_ps(mask1, weight_sse));		// 重みの和
							_mm_storeu_ps((dst_ptr + c), dst_sse1);

							// 8ビットから32ビットへ変換
							and = _mm_srli_si128(and, 4);
							__m128i and2 = _mm_cvtepu8_epi32(and);
							__m128 and2f = _mm_cvtepi32_ps(and2);
							__m128 mask2 = _mm_cmpgt_ps( and2f, _mm_set1_ps(0.f));
							__m128 dst_sse2 = _mm_loadu_ps(dst_ptr + c + 4);
							dst_sse2 = _mm_add_ps(dst_sse2, _mm_and_ps(mask2, weight_sse));
							_mm_storeu_ps((dst_ptr + c + 4), dst_sse2);

							// 8ビットから32ビットへ変換
							and = _mm_srli_si128(and, 4);
							__m128i and3 = _mm_cvtepu8_epi32(and);
							__m128 and3f = _mm_cvtepi32_ps(and3);
							__m128 mask3 = _mm_cmpgt_ps( and3f, _mm_set1_ps(0.f));
							__m128 dst_sse3 = _mm_loadu_ps(dst_ptr + c + 8);
							dst_sse3 = _mm_add_ps(dst_sse3, _mm_and_ps(mask3, weight_sse));
							_mm_storeu_ps((dst_ptr + c + 8), dst_sse3);

							// 8ビットから32ビットへ変換
							and = _mm_srli_si128(and, 4);
							__m128i and4 = _mm_cvtepu8_epi32(and);
							__m128 and4f = _mm_cvtepi32_ps(and4);
							__m128 mask4 = _mm_cmpgt_ps( and4f, _mm_set1_ps(0.f));
							__m128 dst_sse4 = _mm_loadu_ps(dst_ptr + c + 12);
							dst_sse4 = _mm_add_ps(dst_sse4, _mm_and_ps(mask4, weight_sse));
							_mm_storeu_ps((dst_ptr + c + 12), dst_sse4);
						}
					}
#endif


					for ( ; c < width; ++c)
					{	
						// 論理積が0より大きければ
						if ((quant_r[c+tc] & ori_r[tc]) > 0) 
						{
							dst_ptr[c] += weight_r[tc];
						}
					}
				}
			}
		}
	}

	// 総和で除算
	dst /= (float)sum_weight;
}



/**
 * @brief   テンプレート画像の追加
 * 
 * @param   src[in]			摂動画像
 * @param   class_id[in]	追加する画像のクラス
 * @param   mag_th[in]		勾配強度による閾値
 * @param   freq_th[in]		頻度による閾値
 */
void P_COF::addTemplate( const std::vector<cv::Mat>& src, const std::string& class_id, float mag_th, int freq_th)
{	
	// 各勾配方向の頻度
	std::vector<cv::Mat> orientationFreq(8);
	int window_width = src[0].cols/gridSize;
	int window_height = src[0].rows/gridSize;

	for (int i = 0; i < 8; ++i){
        orientationFreq[i] = cv::Mat::zeros(window_height, window_width, CV_32S);
	}

	// 各摂動画像の累積
	for (int i = 0; i < (int)src.size(); ++i)
	{
		std::vector<cv::Mat_<int>> integ_angle;
		cv::Mat quant_angle;

		// 勾配方向の計算
		calcIntegralOrientation( src[i], integ_angle, mag_th, true, 7);

		// 勾配方向の量子化
		int freq_th = gridSize * gridSize * 0.3;
		quantizedIntegraOrientation(quant_angle, integ_angle, freq_th, gridSize);

		// 勾配方向の累積
		cumulatedOrientation(quant_angle, orientationFreq);
	}


	// テンプレート
	std::vector<Template>& templs = class_templates[class_id];

	Template templ;
	templ.oriMat = cv::Mat_<uchar>::zeros(window_height,window_width);
	templ.weightMat = cv::Mat_<int>::zeros(window_height,window_width);
	templ.width = src[0].cols;
	templ.height = src[0].rows;

	// 摂動画像を探索
	for (int r = 0; r < window_height; ++r) 
	{
		uchar* ori_r = templ.oriMat.ptr<uchar>(r);
		int* weight_r = templ.weightMat.ptr<int>(r);

		for (int c = 0; c < window_width; ++c) 
		{
			uchar ori = 0;			// 勾配方向
			int max_freq = 0;		// 最大の頻度

			// 方向毎に探索
			for (int i = 0; i < 8; ++i) 
			{
				// 頻度が閾値以上であれば追加
				if (orientationFreq[i].at<int>(r,c) > freq_th)
				{
					ori += getBitNum(i);		// 勾配の追加

					if (max_freq < orientationFreq[i].at<int>(r,c)) {
						max_freq = orientationFreq[i].at<int>(r,c);
					}
				}
			}

			// テンプレートに追加
			if ( ori != 0 && max_freq != 0)
			{
				ori_r[c] = ori;
				weight_r[c] = max_freq;
			}
		}
	}
	templs.emplace_back(templ);
}


/**
 * @brief   テンプレート画像とのマッチング
 * 
 * @param   src[in]					入力画像
 * @param   matches[in,out]			マッチングの結果
 * @param   mag_th[in]				勾配強度による閾値
 * @param   similarity_rate[in]		類似度による閾値(0.0～1.0)
 */
void P_COF::matchTemplate(const cv::Mat& src, std::vector<Match>& matches, float mag_th, float similarity_th)
{
	// 初期化
	matches.clear();

	// 変数の用意
	std::vector<cv::Mat_<int>> integ_angle;
	cv::Mat quant_angle;

	// 勾配方向の計算
	calcIntegralOrientation( src, integ_angle, mag_th, true, 7);

	// 勾配方向の量子化
	int freq_th = gridSize * gridSize * 0.3;
	quantizedIntegraOrientation(quant_angle, integ_angle, freq_th, gridSize);


	// 格納しているテンプレートとのマッチング
	TemplatesMap::const_iterator it = class_templates.begin(), itend = class_templates.end();
	for ( ; it != itend; ++it) 
	{
		for (int i = 0; i < (int)it->second.size(); ++i)
		{
			cv::Mat similarity_map;		// 類似度マップ

			// 類似度マップの計算
			calcSimilarity(quant_angle, it->second[i], similarity_map);

			// 閾値以上の点を探索
			for (int r = 0; r < similarity_map.rows; ++r)
			{
				float* row = similarity_map.ptr<float>(r);

				for (int c = 0; c < similarity_map.cols; ++c)
				{
					float score = row[c];

					// 閾値以上であれば検出
					if (score > similarity_th)
					{
						int width = it->second[i].width;
						int height = it->second[i].height;
						matches.emplace_back(Match(c*gridSize, r*gridSize, width, height, score, i, it->first));
					}
				}
			}
		}
	}
}



/**
 * @brief  近傍点の統合
 * 
 * @param   matches[in,out]		マッチング結果
 * @param   win_th[in]			統合する領域
 */
void P_COF::nearestNeighbor(std::vector<Match>& matches, int win_th)
{
	int x, y;
	double distance;
	double dist_th = win_th * win_th;

	for (int n = 0; n < (int)matches.size()-1; ++n)
	{
		for (int m = n+1; m < (int)matches.size(); ++m)
		{
			x = matches[n].x - matches[m].x;
			y = matches[n].y - matches[m].y;
			distance = x*x + y*y;

			// 距離が閾値以内かつ類似度が小さくテンプレート番号が同じであれば統合
			if (distance < dist_th && matches[n].similarity <= matches[n].similarity && matches[n].template_num == matches[m].template_num)
			{
				matches[n].x = -1.f;	// 削除フラグ
			}
		}
	}

	// 要素の削除
	auto itr = matches.begin();
	while (itr != matches.end())
	{
		if (itr->x == -1.f)
		{
			itr = matches.erase(itr);
		} else {
			itr++;
		}
	}
}


/**
 * @brief   テンプレートの保存
 * 
 * @param   savePath[in]	保存先
 * @param   class_id[in]	画像のクラス
 */
bool P_COF::saveTemplate( const std::string& savePath, const std::string& class_id)
{
	// テンプレート
	std::vector<Template>& templs = class_templates[class_id];

	if (templs.size() == 0){
		std::cout << "テンプレートが見つかりません" << std::endl;
		return false;
	}


	// テンプレートの枚数
	for (int i = 0; i < (int)templs.size(); ++i)
	{
		// ファイル名
		std::string fileName = savePath + std::to_string(i) + ".xml";
		cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);

		// ヘッダ
		cv::write(cvfs, "grid_size", gridSize);
		cv::write(cvfs, "template_width", templs[i].width);
		cv::write(cvfs, "template_height", templs[i].height);
		
		// テンプレート
		cv::write(cvfs, "orientation", templs[i].oriMat);
		cv::write(cvfs, "weight", templs[i].weightMat);

		cvfs.release();
	}

	return true;
}


/**
 * @brief   テンプレートの読み込み
 * 
 * @param   savePath[in]	保存先のフォルダ名
 * @param   class_id[in]	画像のクラス
 */
bool P_COF::loadTemplate( const std::string& savePath, const std::string& class_id)
{
	// フォルダからファイル名を取得
	std::vector<std::string> fileNames;
	WIN32_FIND_DATA ffd;
	HANDLE hF;

	std::string folderName = savePath + "/*.xml";
	hF = FindFirstFile( folderName.c_str(), &ffd);
	if (hF != INVALID_HANDLE_VALUE) {
		// フォルダ内のファイルの探索
		do {
			std::string fullpath = savePath + "/" + ffd.cFileName;

			// フォルダ名の格納
			fileNames.emplace_back(fullpath);

		} while (FindNextFile(hF, &ffd ) != 0);
		FindClose(hF);
	}

	// テンプレート
	std::vector<Template>& templs = class_templates[class_id];

	// ファイル数分読み込む
	for (int f = 0; f < (int)fileNames.size(); ++f)
	{
		// ファイルの読み込み
		cv::FileStorage cvfs(fileNames[f], cv::FileStorage::READ);

		// ヘッダ
		Template templ;
		cvfs["grid_size"] >> gridSize;
		cvfs["template_width"] >> templ.width;
		cvfs["template_height"] >> templ.height;

		// テンプレートの格納
		cvfs["orientation"] >> templ.oriMat;
		cvfs["weight"] >> templ.weightMat;

		templs.emplace_back(templ);
	}

	return true;
}

