#include "P-COF.h"



/**
 * @brief   �摜������z�����̐ϕ��摜���v�Z
 * 
 * @param   src[in]					���͉摜
 * @param   integral_angle[in,out]	���z�����̐ϕ��摜		
 * @param   threshold[in]			���z���x��臒l(dx*dx+dy*dy)
 * @param   smooth[in]				�K�E�V�A���t�B���^�����邩�ǂ���
 * @param   kernel_size[in]			�K�E�V�A���t�B���^�̃J�[�l���T�C�Y
 *
 * @note	3channel�̏ꍇ�͍ł����z���x�̑傫�����̂�p����
 */
void P_COF::calcIntegralOrientation(const cv::Mat& src, std::vector<cv::Mat_<int>>& integral_angle, float threshold, bool smooth, int kernel_size)
{
	// ���z���x�̏�����
	cv::Mat angle;
	cv::Mat magnitude(src.size(), CV_32F);

	cv::Size size = src.size();
	cv::Mat sobel_3dx;					// x�����̌��z���x(3channel)
	cv::Mat sobel_3dy;					// y�����̌��z���x(3channel)
	cv::Mat sobel_dx(size, CV_32F);		// x�����̌��z���x(1channel)
	cv::Mat sobel_dy(size, CV_32F);		// y�����̌��z���x(1channel)
	cv::Mat sobel_ag;					// ���z����(�ʎq���O)
	cv::Mat smoothed;

	if (smooth) {
		// �K�E�V�A���t�B���^
		cv::GaussianBlur(src, smoothed, cv::Size(kernel_size, kernel_size), 0, 0, cv::BORDER_REPLICATE);
	} else {
		smoothed = src.clone();
	}

	// 3channel�̏ꍇ
	if (src.channels() == 3)
	{
		// �\�[�x���t�B���^
		cv::Sobel(smoothed, sobel_3dx, CV_16S, 1, 0, 3, 1.0, 0.0, cv::BORDER_REPLICATE);
		cv::Sobel(smoothed, sobel_3dy, CV_16S, 0, 1, 3, 1.0, 0.0, cv::BORDER_REPLICATE);

		// �|�C���^
		short * ptrx  = (short *)sobel_3dx.data;
		short * ptry  = (short *)sobel_3dy.data;
		float * ptr0x = (float *)sobel_dx.data;
		float * ptr0y = (float *)sobel_dy.data;
		float * ptrmg = (float *)magnitude.data;

		// 1�X�e�b�v�̃`�����l������
		const int length1 = static_cast<const int>(sobel_3dx.step1());
		const int length2 = static_cast<const int>(sobel_3dy.step1());
		const int length3 = static_cast<const int>(sobel_dx.step1());
		const int length4 = static_cast<const int>(sobel_dy.step1());
		const int length5 = static_cast<const int>(magnitude.step1());
		const int length0 = sobel_3dy.cols * 3;

		// BGR�̂����ł��傫�����z��p����
		for (int r = 0; r < sobel_3dy.rows; ++r)
		{
			int ind = 0;

			for (int i = 0; i < length0; i += 3)
			{
				// BGR���ꂼ��̌��z���x
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
			// ���̍s�Ɉړ�
			ptrx += length1;
			ptry += length2;
			ptr0x += length3;
			ptr0y += length4;
			ptrmg += length5;
		}
	}
	// 1channel�̏ꍇ
	else if(src.channels() == 1)
	{
		// �\�[�x���t�B���^
		cv::Sobel(smoothed, sobel_dx, CV_32F, 1, 0, 3, 1.0, 0.0, cv::BORDER_REPLICATE);
		cv::Sobel(smoothed, sobel_dy, CV_32F, 0, 1, 3, 1.0, 0.0, cv::BORDER_REPLICATE);

		// �|�C���^
		float * ptr0x = (float *)sobel_dx.data;
		float * ptr0y = (float *)sobel_dy.data;
		float * ptrmg = (float *)magnitude.data;

		// 1�X�e�b�v�̃`�����l������
		const int length1 = static_cast<const int>(sobel_dx.step1());
		const int length2 = static_cast<const int>(sobel_dy.step1());
		const int length3 = static_cast<const int>(magnitude.step1());

		// ���z���x
		for (int r = 0; r < sobel_dy.rows; ++r)
		{
			for (int c = 0; c < sobel_dy.cols; ++c)
			{
				ptrmg[c] = ptr0x[c] * ptr0x[c] + ptr0y[c] * ptr0y[c];
			}
			// ���̍s�Ɉړ�
			ptr0x += length1;
			ptr0y += length2;
			ptrmg += length3;
		}
	}

	// ���z�����̐���
	cv::phase(sobel_dx, sobel_dy, angle, true);


	// 360����16����
	cv::Mat_<unsigned char> quantized_unfiltered;
	angle.convertTo(quantized_unfiltered, CV_8U, 16.0 / 360.0);

	// 180�����Α��̌��z�����𓯂������Ƃ���(16������8����)
	for (int r = 0; r < angle.rows; ++r)
	{
		uchar* quant_r = quantized_unfiltered.ptr<uchar>(r);
		for (int c = 0; c < angle.cols; ++c)
		{
			// 4�r�b�g�ڂ𖳎�
			quant_r[c] &= 7;
		}
	}


	// �p�x�ʂ̐ϕ��摜�̏�����
    std::vector<cv::Mat_<uchar>> bins(8);
	for(int i = 0; i < 8; ++i){
		bins[i] = cv::Mat_<uchar>::zeros(smoothed.size());
	}

	// ���z�����ʂɕp�x���i�[
    for (int r = 0; r < quantized_unfiltered.rows; ++r) 
	{
		float* mag_r = magnitude.ptr<float>(r);					// ���z���x
        for (int c = 0; c < quantized_unfiltered.cols; ++c) 
		{
			// ���z���x��臒l�ȏ�ł����
			if (mag_r[c] > threshold)
			{
				int ind = (int)quantized_unfiltered(r, c);
				bins[ind](r, c)++;		// ���z���ŃC���N�������g
			}
        }
    }
 
    // ���z�����ʂɐϕ��摜����
    integral_angle.resize(8);

    for (int i = 0; i < 8; ++i) 
	{
        cv::integral(bins[i], integral_angle[i]);
    }
}


/**
 * @brief   �ϕ��摜������z�����̗ʎq��(�O���b�h�T�C�Y���̌��z������p�x�̑������z�����ő�\����)
 * 
 * @param   quantized_angle[in,out]		�ʎq����̌��z����(8bit)	
 * @param   integral_angle[in]			���z�����̐ϕ��摜		
 * @param   freq_th[in]					���z�����̕p�x�ɂ��臒l	
 * @param   grid_size[in]				�O���b�h�T�C�Y
 */
void P_COF::quantizedIntegraOrientation(cv::Mat& quantized_angle, const std::vector<cv::Mat_<int>>& integral_angle, int freq_th, int grid_size)
{
	// �V�����E�B���h�E�T�C�Y(�O���b�h�T�C�Y�Ŋ������ۂ̗]�蕔���͏��O)
	int window_width = (integral_angle[0].cols-1) / grid_size;
	int window_height = (integral_angle[0].rows-1) / grid_size;

	
	// ������
	quantized_angle = cv::Mat::zeros(window_height, window_width, CV_8U);


	// �O���b�h�̈���̌��z�����𐄒�
	for (int r = 0; r < window_height; ++r)
	{
		uchar* quant_r = quantized_angle.ptr<uchar>(r);
		for (int c = 0; c < window_width; ++c)
		{
			uchar ori = 0;	// ���z����

			// ���z�����̕p�x
			for (int i = 0; i < 8; ++i)
			{
				// �ϕ��摜�����`���̑��a�𓾂�
				int ori_freq = integral_angle[i](r*grid_size, c*grid_size) + integral_angle[i]((r+1)*grid_size, (c+1)*grid_size) - (integral_angle[i]((r+1)*grid_size, c*grid_size) + integral_angle[i](r*grid_size, (c+1)*grid_size));
				
				// 臒l�ȏ�ł����
				if (ori_freq > freq_th)
				{
					ori += uchar(1 << i);		// ���z�����̒ǉ�
				}
			}

			// �̈���̌��z����
			quant_r[c] = ori;
		}
	}
}


/**
 * @brief   ���z�����̊g��
 *			Implements "2.3 Spreading the Orientations"
 * 
 * @param   src[in]				�ʎq�����ꂽ���z�����摜(8bit)
 * @param   dst[in,out]			�g����̌��z�����摜		
 * @param   T[in]				�g���X�e�b�v��(���������T pixel���炵��OR���Z�q�ŕ���������	��2.3�߂̐}�Ƃ͏����قȂ�)	
 */
void P_COF::spreadOrientation(const cv::Mat& src, cv::Mat& dst, int T)
{
	// ������
	dst = cv::Mat::zeros(src.size(), CV_8U);

	// ���z�������g��
	for (int r = 0; r < T; ++r)
	{
		int height = src.rows - r;
		for (int c = 0; c < T; ++c)
		{
			// ���炵�����z������OR�Ō���
			orCombine(&src.at<unsigned char>(r, c), static_cast<const int>(src.step1()), dst.ptr(), static_cast<const int>(dst.step1()), src.cols - c, height);
		}
	}
}


/**
 * @brief   OR���Z�q�Ō��z����������
 *			Implements "2.3 Spreading the Orientations"
 * 
 * @param   src[in]				���炵�����z�����摜(8bit)
 * @param   src_stride[in]		���͉摜��1�X�e�b�v��
 * @param   dst[in,out]			������̌��z�����摜		
 * @param   dst_stride[in]		�o�͉摜��1�X�e�b�v��
 * @param   width[in]			���͉摜�̉���
 * @param   height[in]			���͉摜�̏c��
 */
void P_COF::orCombine(const uchar * src, const int src_stride, uchar * dst, const int dst_stride, const int width, const int height)
{
#if 1
	volatile bool haveSSE3 = cv::checkHardwareSupport(CV_CPU_SSE3);
#endif
#if 0
	volatile bool haveSSE2 = cv::checkHardwareSupport(CV_CPU_SSE2);
#endif


	// SIMD���߂��g�p��OR���Z
	for (int r = 0; r < height; ++r)
	{
		int c = 0;

#if 1
		// SSE3���g����Ƃ�
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
		// SSE2���g����Ƃ�
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
 * @brief   �ʎq�����z������ݐ�
 * 
 * @param	quantized_angle[in]			�ʎq�����z����
 * @param	orientationFreq[in,out]		���z�̕p�x
 */
void P_COF::cumulatedOrientation(const cv::Mat& quantized_angle, std::vector<cv::Mat>& orientationFreq)
{
	// ���z��ݐς�����
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
						// ���z�����̃J�E���g
						orientationFreq[h].at<int>(r,c) += 1;
					}
				}
			}
		}
	}
}


/**
 * @brief   �ގ��x�̌v�Z
 * 
 * @param	quantized_angle[in]		�ʎq�����z����
 * @param	templ[in]				�e���v���[�g
 * @param	dst[in,out]				�ގ��x�摜(CV_32F)
 */
void P_COF::calcSimilarity(const cv::Mat& quantized_angle, const Template& templ, cv::Mat& dst)
{
#if 1
	volatile bool haveSSE3 = cv::checkHardwareSupport(CV_CPU_SSE3);
#endif

	// �X���C�f�B���O�E�B���h�E�͈̔�
	int height = quantized_angle.rows - templ.oriMat.rows + 1;
	int width = quantized_angle.cols - templ.oriMat.cols + 1;
	int t_height = templ.oriMat.rows;
	int t_width = templ.oriMat.cols;

	// �ގ��x�}�b�v�̏�����
	dst = cv::Mat::zeros(height, width, CV_32F);

	// �d�݂̑��a
	cv::Scalar s = cv::sum(templ.weightMat);
	int sum_weight = s(0);


	// �e���v���[�g
	for (int tr = 0; tr < t_height; ++tr)
	{
		const uchar* ori_r = templ.oriMat.ptr<uchar>(tr);
		const int* weight_r = templ.weightMat.ptr<int>(tr);
		
		for (int tc = 0; tc < t_width; ++tc)
		{
			// ���z�������
			if (ori_r[tc] > 0)
			{
				// �X���C�f�B���O�E�B���h�E
				for (int r = 0; r < height; ++r)
				{
					float* dst_ptr = dst.ptr<float>(r);
					const uchar* quant_r = quantized_angle.ptr<uchar>(r+tr);

					int c = 0;

#if 1
					// SSE3���g����Ƃ�
					if (haveSSE3)
					{
						for ( ; c < width - 15; c += 16)
						{
							__m128i quant_sse = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(quant_r + c + tc));	// �ʎq�����z����
							__m128 weight_sse = _mm_set1_ps(static_cast<float>(weight_r[tc]));		// �����̏d��
							__m128i and = _mm_and_si128( quant_sse, _mm_set1_epi8(ori_r[tc]));		// �_����

							// 8�r�b�g����32�r�b�g�֕ϊ�
							__m128i and1 = _mm_cvtepu8_epi32(and);		
							__m128 and1f = _mm_cvtepi32_ps(and1);								// �����^���畂�������_�^��
							__m128 mask1 = _mm_cmpgt_ps( and1f, _mm_set1_ps(0.f));				// ��������
							__m128 dst_sse1 = _mm_loadu_ps(dst_ptr + c);
							dst_sse1 = _mm_add_ps(dst_sse1, _mm_and_ps(mask1, weight_sse));		// �d�݂̘a
							_mm_storeu_ps((dst_ptr + c), dst_sse1);

							// 8�r�b�g����32�r�b�g�֕ϊ�
							and = _mm_srli_si128(and, 4);
							__m128i and2 = _mm_cvtepu8_epi32(and);
							__m128 and2f = _mm_cvtepi32_ps(and2);
							__m128 mask2 = _mm_cmpgt_ps( and2f, _mm_set1_ps(0.f));
							__m128 dst_sse2 = _mm_loadu_ps(dst_ptr + c + 4);
							dst_sse2 = _mm_add_ps(dst_sse2, _mm_and_ps(mask2, weight_sse));
							_mm_storeu_ps((dst_ptr + c + 4), dst_sse2);

							// 8�r�b�g����32�r�b�g�֕ϊ�
							and = _mm_srli_si128(and, 4);
							__m128i and3 = _mm_cvtepu8_epi32(and);
							__m128 and3f = _mm_cvtepi32_ps(and3);
							__m128 mask3 = _mm_cmpgt_ps( and3f, _mm_set1_ps(0.f));
							__m128 dst_sse3 = _mm_loadu_ps(dst_ptr + c + 8);
							dst_sse3 = _mm_add_ps(dst_sse3, _mm_and_ps(mask3, weight_sse));
							_mm_storeu_ps((dst_ptr + c + 8), dst_sse3);

							// 8�r�b�g����32�r�b�g�֕ϊ�
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
						// �_���ς�0���傫�����
						if ((quant_r[c+tc] & ori_r[tc]) > 0) 
						{
							dst_ptr[c] += weight_r[tc];
						}
					}
				}
			}
		}
	}

	// ���a�ŏ��Z
	dst /= (float)sum_weight;
}



/**
 * @brief   �ގ��x�̌v�Z
 * 
 * @param	src[in]					�ʎq�����z����
 * @param	oriMat[in]				�ݐό��z��������(�e���v���[�g)
 * @param	weightMat[in]			�d��(�e���v���[�g)
 * @param	dst[in,out]				�ގ��x�摜(CV_32F)
 */
void P_COF::calcSimilarity(const cv::Mat& src, const cv::Mat& oriMat, const cv::Mat& weightMat, cv::Mat& dst)
{
#if 1
	volatile bool haveSSE3 = cv::checkHardwareSupport(CV_CPU_SSE3);
#endif

	// �X���C�f�B���O�E�B���h�E�͈̔�
	int height = src.rows - oriMat.rows + 1;
	int width = src.cols - oriMat.cols + 1;
	int t_height = oriMat.rows;
	int t_width = oriMat.cols;

	// �ގ��x�}�b�v�̏�����
	dst = cv::Mat::zeros(height, width, CV_32F);

	// �d�݂̑��a
	cv::Scalar s = cv::sum(weightMat);
	int sum_weight = s(0);


	// �e���v���[�g
	for (int tr = 0; tr < t_height; ++tr)
	{
		const uchar* ori_r = oriMat.ptr<uchar>(tr);
		const int* weight_r = weightMat.ptr<int>(tr);
		
		for (int tc = 0; tc < t_width; ++tc)
		{
			// ���z�������
			if (ori_r[tc] > 0)
			{
				// �X���C�f�B���O�E�B���h�E
				for (int r = 0; r < height; ++r)
				{
					float* dst_ptr = dst.ptr<float>(r);
					const uchar* quant_r = src.ptr<uchar>(r+tr);

					int c = 0;

#if 1
					// SSE3���g����Ƃ�
					if (haveSSE3)
					{
						for ( ; c < width - 15; c += 16)
						{
							__m128i quant_sse = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(quant_r + c + tc));	// �ʎq�����z����
							__m128 weight_sse = _mm_set1_ps(static_cast<float>(weight_r[tc]));		// �����̏d��
							__m128i and = _mm_and_si128( quant_sse, _mm_set1_epi8(ori_r[tc]));		// �_����

							// 8�r�b�g����32�r�b�g�֕ϊ�
							__m128i and1 = _mm_cvtepu8_epi32(and);		
							__m128 and1f = _mm_cvtepi32_ps(and1);								// �����^���畂�������_�^��
							__m128 mask1 = _mm_cmpgt_ps( and1f, _mm_set1_ps(0.f));				// ��������
							__m128 dst_sse1 = _mm_loadu_ps(dst_ptr + c);
							dst_sse1 = _mm_add_ps(dst_sse1, _mm_and_ps(mask1, weight_sse));		// �d�݂̘a
							_mm_storeu_ps((dst_ptr + c), dst_sse1);

							// 8�r�b�g����32�r�b�g�֕ϊ�
							and = _mm_srli_si128(and, 4);
							__m128i and2 = _mm_cvtepu8_epi32(and);
							__m128 and2f = _mm_cvtepi32_ps(and2);
							__m128 mask2 = _mm_cmpgt_ps( and2f, _mm_set1_ps(0.f));
							__m128 dst_sse2 = _mm_loadu_ps(dst_ptr + c + 4);
							dst_sse2 = _mm_add_ps(dst_sse2, _mm_and_ps(mask2, weight_sse));
							_mm_storeu_ps((dst_ptr + c + 4), dst_sse2);

							// 8�r�b�g����32�r�b�g�֕ϊ�
							and = _mm_srli_si128(and, 4);
							__m128i and3 = _mm_cvtepu8_epi32(and);
							__m128 and3f = _mm_cvtepi32_ps(and3);
							__m128 mask3 = _mm_cmpgt_ps( and3f, _mm_set1_ps(0.f));
							__m128 dst_sse3 = _mm_loadu_ps(dst_ptr + c + 8);
							dst_sse3 = _mm_add_ps(dst_sse3, _mm_and_ps(mask3, weight_sse));
							_mm_storeu_ps((dst_ptr + c + 8), dst_sse3);

							// 8�r�b�g����32�r�b�g�֕ϊ�
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
						// �_���ς�0���傫�����
						if ((quant_r[c+tc] & ori_r[tc]) > 0) 
						{
							dst_ptr[c] += weight_r[tc];
						}
					}
				}
			}
		}
	}

	// ���a�ŏ��Z
	dst /= (float)sum_weight;
}



/**
 * @brief   �e���v���[�g�摜�̒ǉ�
 * 
 * @param   src[in]			�ۓ��摜
 * @param   class_id[in]	�ǉ�����摜�̃N���X
 * @param   mag_th[in]		���z���x�ɂ��臒l
 * @param   freq_th[in]		�p�x�ɂ��臒l
 */
void P_COF::addTemplate( const std::vector<cv::Mat>& src, const std::string& class_id, float mag_th, int freq_th)
{	
	// �e���z�����̕p�x
	std::vector<cv::Mat> orientationFreq(8);
	int window_width = src[0].cols/gridSize;
	int window_height = src[0].rows/gridSize;

	for (int i = 0; i < 8; ++i){
        orientationFreq[i] = cv::Mat::zeros(window_height, window_width, CV_32S);
	}

	// �e�ۓ��摜�̗ݐ�
	for (int i = 0; i < (int)src.size(); ++i)
	{
		std::vector<cv::Mat_<int>> integ_angle;
		cv::Mat quant_angle;

		// ���z�����̌v�Z
		calcIntegralOrientation( src[i], integ_angle, mag_th, true, 7);

		// ���z�����̗ʎq��
		int freq_th = gridSize * gridSize * 0.3;
		quantizedIntegraOrientation(quant_angle, integ_angle, freq_th, gridSize);

		// ���z�����̗ݐ�
		cumulatedOrientation(quant_angle, orientationFreq);
	}


	// �e���v���[�g
	std::vector<Template>& templs = class_templates[class_id];

	Template templ;
	templ.oriMat = cv::Mat_<uchar>::zeros(window_height,window_width);
	templ.weightMat = cv::Mat_<int>::zeros(window_height,window_width);
	templ.width = src[0].cols;
	templ.height = src[0].rows;

	// �ۓ��摜��T��
	for (int r = 0; r < window_height; ++r) 
	{
		uchar* ori_r = templ.oriMat.ptr<uchar>(r);
		int* weight_r = templ.weightMat.ptr<int>(r);

		for (int c = 0; c < window_width; ++c) 
		{
			uchar ori = 0;			// ���z����
			int max_freq = 0;		// �ő�̕p�x

			// �������ɒT��
			for (int i = 0; i < 8; ++i) 
			{
				// �p�x��臒l�ȏ�ł���Βǉ�
				if (orientationFreq[i].at<int>(r,c) > freq_th)
				{
					ori += getBitNum(i);		// ���z�̒ǉ�

					if (max_freq < orientationFreq[i].at<int>(r,c)) {
						max_freq = orientationFreq[i].at<int>(r,c);
					}
				}
			}

			// �e���v���[�g�ɒǉ�
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
 * @brief   �e���v���[�g�摜�Ƃ̃}�b�`���O
 * 
 * @param   src[in]					���͉摜
 * @param   matches[in,out]			�}�b�`���O�̌���
 * @param   mag_th[in]				���z���x�ɂ��臒l
 * @param   similarity_rate[in]		�ގ��x�ɂ��臒l(0.0�`1.0)
 */
void P_COF::matchTemplate(const cv::Mat& src, std::vector<Match>& matches, float mag_th, float similarity_th)
{
	// ������
	matches.clear();

	// �ϐ��̗p��
	std::vector<cv::Mat_<int>> integ_angle;
	cv::Mat quant_angle;

	// ���z�����̌v�Z
	calcIntegralOrientation( src, integ_angle, mag_th, true, 7);

	// ���z�����̗ʎq��
	int freq_th = gridSize * gridSize * 0.3;
	quantizedIntegraOrientation(quant_angle, integ_angle, freq_th, gridSize);


	// �i�[���Ă���e���v���[�g�Ƃ̃}�b�`���O
	TemplatesMap::const_iterator it = class_templates.begin(), itend = class_templates.end();
	for ( ; it != itend; ++it) 
	{
		for (int i = 0; i < (int)it->second.size(); ++i)
		{
			cv::Mat similarity_map;		// �ގ��x�}�b�v

			// �ގ��x�}�b�v�̌v�Z
			calcSimilarity(quant_angle, it->second[i], similarity_map);

			// 臒l�ȏ�̓_��T��
			for (int r = 0; r < similarity_map.rows; ++r)
			{
				float* row = similarity_map.ptr<float>(r);

				for (int c = 0; c < similarity_map.cols; ++c)
				{
					float score = row[c];

					// 臒l�ȏ�ł���Ό��o
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
 * @brief  �ߖT�_�̓���
 * 
 * @param   matches[in,out]		�}�b�`���O����
 * @param   win_th[in]			��������̈�
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

			// ������臒l�ȓ����ގ��x���������e���v���[�g�ԍ��������ł���Γ���
			if (distance < dist_th && matches[n].similarity <= matches[n].similarity && matches[n].template_num == matches[m].template_num)
			{
				matches[n].x = -1.f;	// �폜�t���O
			}
		}
	}

	// �v�f�̍폜
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
 * @brief   �e���v���[�g�̕ۑ�
 * 
 * @param   savePath[in]	�ۑ���
 * @param   class_id[in]	�摜�̃N���X
 */
bool P_COF::saveTemplate( const std::string& savePath, const std::string& class_id)
{
	// �e���v���[�g
	std::vector<Template>& templs = class_templates[class_id];

	if (templs.size() == 0){
		std::cout << "�e���v���[�g��������܂���" << std::endl;
		return false;
	}


	// �e���v���[�g�̖���
	for (int i = 0; i < (int)templs.size(); ++i)
	{
		// �t�@�C����
		std::string fileName = savePath + std::to_string(i) + ".xml";
		cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);

		// �w�b�_
		cv::write(cvfs, "grid_size", gridSize);
		cv::write(cvfs, "template_width", templs[i].width);
		cv::write(cvfs, "template_height", templs[i].height);
		
		// �e���v���[�g
		cv::write(cvfs, "orientation", templs[i].oriMat);
		cv::write(cvfs, "weight", templs[i].weightMat);

		cvfs.release();
	}

	return true;
}


/**
 * @brief   �e���v���[�g�̓ǂݍ���
 * 
 * @param   savePath[in]	�ۑ���̃t�H���_��
 * @param   class_id[in]	�摜�̃N���X
 */
bool P_COF::loadTemplate( const std::string& savePath, const std::string& class_id)
{
	// �t�H���_����t�@�C�������擾
	std::vector<std::string> fileNames;
	WIN32_FIND_DATA ffd;
	HANDLE hF;

	std::string folderName = savePath + "/*.xml";
	hF = FindFirstFile( folderName.c_str(), &ffd);
	if (hF != INVALID_HANDLE_VALUE) {
		// �t�H���_���̃t�@�C���̒T��
		do {
			std::string fullpath = savePath + "/" + ffd.cFileName;

			// �t�H���_���̊i�[
			fileNames.emplace_back(fullpath);

		} while (FindNextFile(hF, &ffd ) != 0);
		FindClose(hF);
	}

	// �e���v���[�g
	std::vector<Template>& templs = class_templates[class_id];

	// �t�@�C�������ǂݍ���
	for (int f = 0; f < (int)fileNames.size(); ++f)
	{
		// �t�@�C���̓ǂݍ���
		cv::FileStorage cvfs(fileNames[f], cv::FileStorage::READ);

		// �w�b�_
		Template templ;
		cvfs["grid_size"] >> gridSize;
		cvfs["template_width"] >> templ.width;
		cvfs["template_height"] >> templ.height;

		// �e���v���[�g�̊i�[
		cvfs["orientation"] >> templ.oriMat;
		cvfs["weight"] >> templ.weightMat;

		templs.emplace_back(templ);
	}

	return true;
}

