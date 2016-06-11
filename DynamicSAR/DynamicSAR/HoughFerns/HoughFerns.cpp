# include "HoughFerns.h"


/**
 * @brief   �S�Ẵp�����[�^�̏�����	
 */
void HoughFerns::init(int _numFerns1, int _numFerns2, int _depth1, int _depth2, int _featureTestNum1, int _featureTestNum2, int _thresholdTestNum1, int _thresholdTestNum2, 
				int _numClass, int _posLabel, int _negLabel, float _posDataPerTree1, float _posDataPerTree2, float _negDataPerTree1, float _negDataPerTree2)
{
	numFerns1 =_numFerns1;
	numFerns2 =_numFerns2;
	depth1 = _depth1;
	depth2 = _depth2;
	featureTestNum1 = _featureTestNum1;
	featureTestNum2 = _featureTestNum2;
	thresholdTestNum1 = _thresholdTestNum1;
	thresholdTestNum2 = _thresholdTestNum2;
	numClass = _numClass;
	posLabel = _posLabel;
	negLabel = _negLabel;
	posDataPerTree1 = _posDataPerTree1;
	posDataPerTree2 = _posDataPerTree2;
	negDataPerTree1 = _negDataPerTree1;
	negDataPerTree2 = _negDataPerTree2;
}


/**
 * @brief   �ݒ�t�@�C���̓ǂݍ���
 * 
 * @param   fileName[in]	�ݒ�t�@�C����
 * 
 * @returns true or false    
 */
bool HoughFerns::initFromFile(const std::string &fileName)
{
	// �ݒ�t�@�C���̓ǂݍ���
	std::ifstream ifs(fileName);

	if( !ifs ) {
		std::cerr << "�ݒ�t�@�C����ǂݍ��߂܂���" << std::endl;
		return false;
	}

	std::vector<std::vector<std::string>> params;	// �����̊i�[

	// ������̎擾
	std::string str;
	while (std::getline(ifs, str))
	{
		std::vector<std::string> param;

		std::string tmp;
		std::istringstream stream(str);
		while (std::getline(stream, tmp, ','))
		{
			param.emplace_back(tmp);
		}
		params.emplace_back(param);
	}


	// Hough Forests�p�����[�^
	numFerns1 = atoi(params[0][1].c_str());
	numFerns2 = atoi(params[1][1].c_str());
	depth1 = atoi(params[2][1].c_str());
	depth2 = atoi(params[3][1].c_str());
	featureTestNum1 = atoi(params[4][1].c_str());
	featureTestNum2 = atoi(params[5][1].c_str());
	thresholdTestNum1 = atoi(params[6][1].c_str());
	thresholdTestNum2 = atoi(params[7][1].c_str());
	numClass = atoi(params[8][1].c_str());
	posDataPerTree1 = atof(params[9][1].c_str());
	posDataPerTree2 = atof(params[10][1].c_str());
	negDataPerTree1 = atof(params[11][1].c_str());
	negDataPerTree2 = atof(params[12][1].c_str());
	// �w�K�T���v���p�����[�^
	posLabel = atoi(params[13][1].c_str());
	negLabel = atoi(params[14][1].c_str());
	eyeVector.x = (float)atof(params[15][1].c_str());
	eyeVector.y = (float)atof(params[15][2].c_str());
	eyeVector.z = (float)atof(params[15][3].c_str());
	eyeUp.x = (float)atof(params[16][1].c_str());
	eyeUp.y = (float)atof(params[16][2].c_str());
	eyeUp.z = (float)atof(params[16][3].c_str());
	distance = (float)atof(params[17][1].c_str());
	// �����p�p�����[�^
	patchSize = atoi(params[18][1].c_str());
	gridSize = atoi(params[19][1].c_str());

	// �O���b�h�T�C�Y�̐ݒ�
	p_cof.gridSize = gridSize;

	return true;
}


/**
 * @brief   Hough Ferns�̊w�K���s��
 * 
 * @param   posFolder[in]	�|�W�e�B�u�摜�̂���t�H���_��
 * @param   negFolder[in]	�l�K�e�B�u�摜�̂���t�H���_��
 * @param   saveFolder[in]	Fern��ۑ�����t�H���_��
 */
void HoughFerns::Learn(const std::string& posFolder, const std::string& negFolder, const std::string& saveFolder)
{
	// �R���\�[���ɏo��
	std::cout << "------------------------------------" << std::endl;
	std::cout << " Hough Ferns�ɂ��w�K" << std::endl;
	std::cout << "------------------------------------" << std::endl << std::endl;

	std::cout << "����؂̐�(1�w��)		: " << numFerns1 << std::endl;
	std::cout << "����؂̐�(2�w��)		: " << numFerns2 << std::endl;
	std::cout << "�؂̍ő�[��(1�w��)		: " << depth1 << std::endl;
	std::cout << "�؂̍ő�[��(2�w��)		: " << depth2 << std::endl;
	std::cout << "�����I����(1�w��)		: " << featureTestNum1 << std::endl;
	std::cout << "�����I����(2�w��)		: " << featureTestNum2 << std::endl;
	std::cout << "臒l�I����(1�w��)		: " << thresholdTestNum1 << std::endl;
	std::cout << "臒l�I����(2�w��)		: " << thresholdTestNum2 << std::endl;
	std::cout << "�N���X��			: " << numClass << std::endl;
	std::cout << "�|�W�e�B�u�f�[�^�̊���(1�w��)	: " << posDataPerTree1 << std::endl;
	std::cout << "�|�W�e�B�u�f�[�^�̊���(2�w��)	: " << posDataPerTree2 << std::endl;
	std::cout << "�l�K�e�B�u�f�[�^�̊���(1�w��)	: " << negDataPerTree1 << std::endl;
	std::cout << "�l�K�e�B�u�f�[�^�̊���(2�w��)	: " << negDataPerTree2 << std::endl << std::endl;


	// �w�K���Ԃ̑���
	double start, finish, time, f_start, f_finish, f_time;
	start = static_cast<double>(cv::getTickCount());
	f_start = static_cast<double>(cv::getTickCount());

	// �w�K�T���v���̒��o
	std::vector<std::shared_ptr<LearnSample>> samples;
	getLearnSamples(posFolder, negFolder, samples);


	///// 1�w�ڂ�Hough Ferns�̊w�K /////

	std::cout << "\n1�w�ڂ�Hough Ferns�̊w�K���c" << std::endl;
	{
		std::vector<std::unique_ptr<FirstLayerFern>> ferns;

		for (int i = 0; i < numFerns1; ++i)
		{
			printf("%d / %d�{��\r", i+1, numFerns1);

			// �w�K�f�[�^�̃u�[�g�X�g���b�v�T���v�����O
			std::vector<std::shared_ptr<LearnSample>> subSamples;
			int posSize, negSize;
			makeSubset (subSamples, samples, posDataPerTree1, negDataPerTree1, posSize, negSize);

			// InverseLabel�̍쐬(�N���X���̋t��)
			std::vector<double> il( numClass, 0.0);

			for (int j = 0; j < (int)subSamples.size(); ++j){
				il[ subSamples[j]->label ]++;
			}
			for (int j = 0; j < il.size(); ++j){
				if (il[j] != 0.0){
					il[j] = (double)1.0 / il[j];
				}else{
					il[j] = 0.0;
				}
			}

			// �p����InverseLabel�̍쐬
			std::vector<double> poseIl(8, 0.0);
			for (int j = 0; j < (int)subSamples.size(); ++j){
				if (subSamples[j]->label != negLabel) {
					poseIl[ subSamples[j]->poseLabel ]++;
				}
			}
			for (int j = 0; j < (int)poseIl.size(); ++j){
				if (poseIl[j] != 0.0){
					poseIl[j] = (double)1.0 / poseIl[j];
				}else{
					poseIl[j] = 0.0;
				}
			}

			// Fern�̍쐬
			std::unique_ptr<FirstLayerFern> fern( new FirstLayerFern(depth1, featureTestNum1, thresholdTestNum1, numClass, posLabel, negLabel, gridSize));

			// Fern�̊w�K
			fern->Learn(subSamples, posSize, il, poseIl);

			// Fern�̒ǉ�
			ferns.emplace_back(std::move(fern));
		}

		// Fern�̕ۑ�
		char filename[ 256 ];
		char filename_t[ 256 ];
		for (int i = 0; i < numFerns1; ++i)
		{
			sprintf( filename, "%s/fern1_%d.txt", saveFolder.c_str(), i );
			sprintf( filename_t, "%s/fern1_%d_t.txt", saveFolder.c_str(), i );
			ferns[i]->saveFern(filename, filename_t);
		}
	}

	f_finish = static_cast<double>(cv::getTickCount());
	f_time = ( ( f_finish - f_start ) / cv::getTickFrequency() );

	std::cout << "1�w�ځF" << numFerns1 << "�{��Fern���\�z" << std::endl;
	std::cout << "1�w�ڏ������ԁF" << f_time << "�b" << std::endl;



	///// 2�w�ڂ�Hough Ferns�̊w�K /////

	std::cout << "\n2�w�ڂ�Hough Ferns�̊w�K���c" << std::endl;

	// �����������J��Ԃ�
	for (int d = 0; d < 8; ++d)
	{
		std::cout << d+1 << "�Ԗڂ̎p�������̊w�K" << std::endl;

		// �w�K���Ԃ̑���
		double s_start, s_finish, s_time;
		s_start = static_cast<double>(cv::getTickCount());

		std::vector<std::unique_ptr<SecondLayerFern>> ferns;

		// �p�������̊w�K�T���v�����擾
		int second_pos_num = 0;
		int second_neg_num = 0;
		std::vector<std::shared_ptr<LearnSample>> pose_samples;
		for (int s = 0; s < (int)samples.size(); ++s)
		{
			// �|�W�e�B�u�T���v���ł����
			if (samples[s]->label == posLabel)
			{
				// ���̎p�������ł����
				if (samples[s]->poseLabel == d)
				{
					pose_samples.emplace_back(samples[s]);
					second_pos_num++;
				}
			} 
			
		}
		for (int s = 0; s < (int)samples.size(); ++s)
		{
			// �l�K�e�B�u�T���v���͑S�Ċi�[
			if (samples[s]->label == negLabel)
			{
				pose_samples.emplace_back(samples[s]);
				second_neg_num++;
			}
		}

		// Fern�̊w�K
		for (int i = 0; i < numFerns2; ++i)
		{
			printf("%d / %d�{��\r", i+1, numFerns2);

			// �w�K�f�[�^�̃u�[�g�X�g���b�v�T���v�����O
			std::vector<std::shared_ptr<LearnSample>> subSamples;
			int posSize, negSize;
			makeSubset (subSamples, pose_samples, second_pos_num, second_neg_num, posDataPerTree2, negDataPerTree2, posSize, negSize);

			// InverseLabel�̍쐬(�N���X���̋t��)
			std::vector<double> il( numClass, 0.0);

			for (int j = 0; j < (int)subSamples.size(); ++j){
				il[ subSamples[j]->label ]++;
			}
			for (int j = 0; j < il.size(); ++j){
				if (il[j] != 0.0){
					il[j] = (double)1.0 / il[j];
				}else{
					il[j] = 0.0;
				}
			}

			// Fern�̍쐬
			std::unique_ptr<SecondLayerFern> fern( new SecondLayerFern(depth2, featureTestNum2, thresholdTestNum2, numClass, posLabel, negLabel, gridSize));

			// Fern�̊w�K
			fern->Learn(subSamples, posSize, il);

			// Fern�̒ǉ�
			ferns.emplace_back(std::move(fern));
		}

		// Fern�̕ۑ�
		char filename[ 256 ];
		char filename_txt[ 256 ];// Fern�̒ǉ�
		for (int i = 0; i < numFerns2; ++i)
		{
			sprintf( filename, "%s/fern2_%d_%d.txt", saveFolder.c_str(), d, i );
			sprintf( filename_txt, "%s/fern2_%d_%d_t.txt", saveFolder.c_str(), d, i );
			ferns[i]->saveFern(filename, filename_txt);
		}

		s_finish = static_cast<double>(cv::getTickCount());
		s_time = ( ( s_finish - s_start ) / cv::getTickFrequency() );

		std::cout << "2�w�� " << d << "�ԖځF" << numFerns2 << "�{��Fern���\�z" << std::endl;
		std::cout << "�������ԁF" << s_time << "�b" << std::endl;
	}

	// Hough Ferns�̃p�����[�^��ۑ�
	saveFerns(saveFolder);

	finish = static_cast<double>(cv::getTickCount());
	time = ( ( finish - start ) / cv::getTickFrequency() );
	std::cout << "�S�̏������� : " << time << "�b" << std::endl;
	std::cout << "�w�K�I��" << std::endl << std::endl;
}


/**
 * @brief   Hough Ferns�ɂ�錟�o
 * 
 * @param   src[in]					���͉摜
 * @param   min_distance1[in]		1�w�ڂ̍l������ŏ�����
 * @param   max_distance1[in]		1�w�ڂ̍l������ő勗��
 * @param   distance_num1[in]		1�w�ڂ̋����ω��̃X�e�b�v��
 * @param   min_distance2[in]		2�w�ڂ̍l������ŏ�����
 * @param   max_distance2[in]		2�w�ڂ̍l������ő勗��
 * @param   distance_num2[in]		2�w�ڂ̋����ω��̃X�e�b�v��
 * @param   detect_width[in]		�p������ɍl�����镝
 * @param   detect_height[in]		�p������ɍl�����鍂��
 * @param   mag_th[in]				���z���x��臒l
 * @param   meanshift_radius[in]	Mean Shift�̃J�[�l����
 * @param   win_th[in]				Nearest Neighbor�œ�������͈�
 * @param   likelihood_th[in]		�ޓx��臒l
 */
void HoughFerns::detect(const cv::Mat& src, float min_distance, float max_distance, int distance_num, float min_scale, float max_scale, int scale_num,
						int detect_width, int detect_height, float mag_th, int meanshift_radius, int win_th, double likelihood_th)
{
	detectPoint.clear();								// �ŏI�I�Ȍ��o�_
	pose_estimate.clear();								// �ŏI�I�Ȏp��

	std::vector<cv::Point3f> detect_point;				// 1�w�ڂ̌��o�ʒu
	std::vector<float> detect_scales;					// 1�w�ڂŌ��o�����X�P�[��

	// 1�w�ڂ̈ʒu���o�y�ё�܂��Ȏp������
	detectFirst( src, detect_point, detect_scales, min_distance, max_distance, distance_num, mag_th, meanshift_radius, win_th, likelihood_th);

	// 2�w�ڂ̐��m�Ȏp������(���o�񐔕��J��Ԃ�)
	for (int i = 0; i < (int)detect_point.size(); ++i)
	{
		detectSecond( src, detect_point[i], detect_scales[i], i, min_scale, max_scale, scale_num, detect_width, detect_height, mag_th, meanshift_radius, win_th, likelihood_th);
	}
}



/**
 * @brief   Hough Ferns�ɂ�錟�o(1�w��)
 * 
 * @param   src[in]					���͉摜
 * @param   detect_point[in, out]	1�w�ڂŌ��o�������W
 * @param   detect_scale[in,out]	1�w�ڂŌ��o�����X�P�[��
 * @param   min_distance1[in]		1�w�ڂ̍l������ŏ�����
 * @param   max_distance1[in]		1�w�ڂ̍l������ő勗��
 * @param   distance_num1[in]		1�w�ڂ̋����ω��̃X�e�b�v��
 * @param   mag_th[in]				���z���x��臒l
 * @param   meanshift_radius[in]	Mean Shift�̃J�[�l����
 * @param   win_th[in]				Nearest Neighbor�œ�������͈�
 * @param   likelihood_th[in]		�ޓx��臒l
 */
void HoughFerns::detectFirst(const cv::Mat& src, std::vector<cv::Point3f> &detect_point, std::vector<float> &detect_scales, float min_distance, float max_distance, int distance_num, 
							 float mag_th, int meanshift_radius, int win_th, double likelihood_th)
{
	if (first_ferns.size() == 0){
		std::cerr << "�w�K����Ă��܂���" << std::endl;
		return;
	}

	// �w�K���Ԃ̑���
	double start, finish, time;
	double s_start, s_finish, s_time;
	double m_start, m_finish, m_time;
	double n_start, n_finish, n_time;
	double pose_start, pose_finish, pose_time;
	double total_start, total_finish, total_time;

	total_start = static_cast<double>(cv::getTickCount());

	// �ő�E�ŏ������ɂ�����T�C�Y�𐄒�(�����̃X�P�[���̔{��)
	std::vector<float> scale_step;
	float dist_step;
	if(distance_num !=1){
		dist_step = (max_distance-min_distance) / (float)(distance_num-1);
	} else {
		dist_step = min_distance;
	}
	for (int i = 0; i < distance_num; ++i)
	{
		// �X�P�[��
		float new_scale = distance / (float)(dist_step*i+min_distance);
		scale_step.emplace_back(new_scale);
	}

	if (scale_step.size() == 0) {
		std::cerr << "�������鋗���͈̔͂��K�؂ł���܂���" << std::endl;
	}
	
	///// ���͉摜�̗ʎq�����z�����𐄒� /////

	s_start = static_cast<double>(cv::getTickCount());

	// P-COF������
	p_cof.gridSize = gridSize;

	// ���͉摜�̌��z����
	std::vector<cv::Mat> quant_angles;	

	// �X�P�[����ω�
	for (int i = 0; i < (int)scale_step.size(); ++i)
	{
		cv::Mat resize_src;
		std::vector<cv::Mat_<int>> integ_angle;

		// �X�P�[���ω�(�p�b�`�̑傫���F�偨�������͉摜�F������)
		cv::resize(src, resize_src, cv::Size(), 1.f/scale_step[i], 1.f/scale_step[i]);

		// ���z�����̌v�Z
		p_cof.calcIntegralOrientation(resize_src, integ_angle, mag_th, true, 7);

		// ���z�����̗ʎq��
		cv::Mat quant_angle;
		int freq_th = gridSize * gridSize * 0.3;
		p_cof.quantizedIntegraOrientation(quant_angle, integ_angle, freq_th, gridSize);
		quant_angles.emplace_back(quant_angle);
	}


	// �ޓx�}�b�v�̏�����
	int end_index = quant_angles.size() - 1;
	likelihoodmap.resize(quant_angles.size());
	for( int i = 0; i < (int)(quant_angles.size()); ++i) {
		likelihoodmap[i] = cv::Mat_<float>( quant_angles[i].rows, quant_angles[i].cols, float( 0.f ) );
	}
	// �p�������}�b�v�̏�����
	std::vector<std::vector<cv::Mat_<float>>> pose_map;
	pose_map.resize(quant_angles.size());
	for( int i = 0; i < (int)(quant_angles.size()); ++i) {
		pose_map[i].resize(8);
		for (int j = 0; j < 8; ++j) {
			pose_map[i][j] = cv::Mat_<float>( quant_angles[i].rows, quant_angles[i].cols, float( 1.f ) );
		}
	}


	s_finish = static_cast<double>(cv::getTickCount());
	s_time = ( (s_finish - s_start ) / cv::getTickFrequency() );


	///// �eFern�ɂ����ăe���v���[�g�}�b�`���O����ѓ��[���� /////

	start = static_cast<double>(cv::getTickCount());

	// �ޓx�}�b�v�̑���(�ΐ��̑��a)
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		for (int j = 0; j < numFerns1; ++j)
		{
			cv::Mat_<float> likelihood = cv::Mat_<float>( quant_angles[i].rows, quant_angles[i].cols, float( 0.f ) );

			std::vector<cv::Mat_<float>> pose_likelihood(8);
			for( int k = 0; k < 8; ++k) {
				pose_likelihood[k] = cv::Mat_<float>( quant_angles[i].rows, quant_angles[i].cols, float( 0.f ) );
			}

			// �ޓx�}�b�v���擾
			first_ferns[j]->Predict( quant_angles[i], likelihood, pose_likelihood);

			// �ޓx�}�b�v�̃T�C�Y�ɂ���ē��[�����ω����邱�Ƃ�h��
			likelihood *= (1.f * scale_step[i]);

			// �ΐ��ɕϊ�
			cv::log(likelihood, likelihood);
			cv::add(likelihoodmap[i], likelihood, likelihoodmap[i]);

			// �p���N���X����
			for (int k = 0; k < 8; ++k) {
				pose_map[i][k] = pose_map[i][k].mul(pose_likelihood[k]);
			}
		}
	}


	int kernel_size = 3;
	float kernel_sigma = 3.0f;
	
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		// �ΐ����猳�ɖ߂�
		cv::exp(likelihoodmap[i], likelihoodmap[i]);

		// �������Ȃ�߂��Ȃ��悤�X�P�[���{
		likelihoodmap[i] *= 1000000.f;

		// ���̉摜�T�C�Y�ɖ߂�(�O���b�h�P��)
		cv::resize(likelihoodmap[i], likelihoodmap[i], cv::Size(), scale_step[i], scale_step[i]);

		// �����������ʂɂ����镽����
		cv::GaussianBlur(likelihoodmap[i], likelihoodmap[i], cv::Size(kernel_size,kernel_size), kernel_sigma, kernel_sigma);
	}

	// ���������̕�����
	smoothMap(likelihoodmap);
	
	finish = static_cast<double>(cv::getTickCount());
	time = ( ( finish - start ) / cv::getTickFrequency() );


	///// �ޓx�}�b�v��臒l���� /////
	m_start = static_cast<double>(cv::getTickCount());

	// �ő�l�C�ŏ��l�̎Z�o
	double max_val = -DBL_MAX;
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		double minVal, maxVal;
		cv::minMaxLoc (likelihoodmap[i], &minVal, &maxVal, NULL, NULL);

		if (max_val < maxVal) {
			max_val = maxVal;
		}
	}

	// �ޓx�}�b�v�̉���
	visualizeLikelihood(likelihoodmap, scale_step, max_val, false);


	// 臒l�ȏ�݂̂��c��
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		cv::threshold (likelihoodmap[i], likelihoodmap[i], max_val*likelihood_th, 1, cv::THRESH_TOZERO);
	}


	///// Mean Shift�ŋɑ�l�𐄒� /////

	std::vector<cv::Point3f> detectPointInit;
	std::vector<double> detectVote;
	meanShift(detectPointInit, detectVote, scale_step, meanshift_radius);

	m_finish = static_cast<double>(cv::getTickCount());
	m_time = ( ( m_finish - m_start ) / cv::getTickFrequency() );


	///// Nearest Neighbor�ŋߖT�_�𓝍� /////

	n_start = static_cast<double>(cv::getTickCount());

	nearestNeighbor(detectPointInit, detectVote, win_th);

	n_finish = static_cast<double>(cv::getTickCount());
	n_time = ( ( n_finish - n_start ) / cv::getTickFrequency() );


	///// �p���̐��� /////

	pose_start = static_cast<double>(cv::getTickCount());


	// 3�����ʒu�֕ϊ�
	std::vector<cv::Point3f> detect3D;		// ���o�_
	std::vector<int> scale_indexes;			// �C���f�b�N�X
	for (int i = 0; i < (int)detectPointInit.size(); ++i) {
		if (detectPointInit[i].x != -100.0) {

			// 2�����ʒu+����
			detect3D.emplace_back(cv::Point3f((float)detectPointInit[i].x, (float)detectPointInit[i].y, (float)(distance / detectPointInit[i].z)));
			detect_point.emplace_back(cv::Point3f((float)detectPointInit[i].x*gridSize, (float)detectPointInit[i].y*gridSize, (float)(distance / detectPointInit[i].z)));
			
			// �ł��߂��X�P�[�������o��
			float d_scale=100.f;
			int scale_index = 0;
			for(int j = 0; j < (int)(scale_step.size()); ++j) {
				float div_scale = abs(detectPointInit[i].z - scale_step[j]);
				if( div_scale < d_scale) {
					d_scale = div_scale;
					scale_index = j;
				}
			}
			detect_scales.emplace_back(scale_step[scale_index]);
			scale_indexes.emplace_back(scale_index);
		}
	}

	// �p�������̐���
	pose_distribute.clear();
	poseEstimation(pose_distribute, pose_map, detect3D, scale_indexes, scale_step);

	pose_finish = static_cast<double>(cv::getTickCount());
	pose_time = ( ( pose_finish - pose_start ) / cv::getTickFrequency() );

	total_finish = static_cast<double>(cv::getTickCount());
	total_time = ( ( total_finish - total_start ) / cv::getTickFrequency() );



	// ���ʂ̃R���\�[���o��
	for (int i = 0; i < (int)detect_point.size(); ++i) {
		std::cout << "���o�ʒu(x,y,z): (" << detect_point[i].x << "," << detect_point[i].y << "," << detect_point[i].z << ")" << std::endl;
		std::cout << "�p�b�`�̃X�P�[��(index,s): (" << scale_indexes[i] << "," << scale_step[scale_indexes[i]] << ")" << std::endl;
		std::cout << "�p���m��: " << std::endl;
		for (int j = 0; j < 8; ++j)
		{
			std::cout << j << ":" << pose_distribute[i][j] << std::endl;
		}
		std::cout << std::endl;
	}

	std::cout << "���͉摜�������� : " << s_time << "�b" << std::endl;
	std::cout << "���[���� : " << time << "�b" << std::endl;
	std::cout << "Mean Shift���� : " << m_time << "�b" << std::endl;
	std::cout << "Nearest Neighbor���� : " << n_time << "�b" << std::endl;
	std::cout << "�p�����莞�� : " << pose_time << "�b" << std::endl;
	std::cout << "���ʎ��� : " << total_time << "�b" << std::endl << std::endl;
}


/**
 * @brief   Hough Ferns�ɂ�錟�o(2�w��)
 * 
 * @param   src[in]					���͉摜
 * @param   detect_point[in]		1�w�ڂŌ��o�������W
 * @param   detect_scale[in]		1�w�ڂŌ��o�����X�P�[��
 * @param   detect_num[in]			���Ԗڂ̌��o��
 * @param   min_scale[in]			1�w�ڂ̃X�P�[������ɍŏ��̃X�P�[���{
 * @param   max_scale[in]			1�w�ڂ̃X�P�[������ɍő�̃X�P�[���{
 * @param   scale_num[in]			�X�P�[���ω��̃X�e�b�v��
 * @param   detect_width[in]		�p������ɍl�����镝
 * @param   detect_height[in]		�p������ɍl�����鍂��
 * @param   mag_th[in]				���z���x��臒l
 * @param   meanshift_radius[in]	Mean Shift�̃J�[�l����
 * @param   win_th[in]				Nearest Neighbor�œ�������͈�
 * @param   likelihood_th[in]		�ޓx��臒l
 */
void HoughFerns::detectSecond(const cv::Mat& src, const cv::Point3f &detect_point, float detect_scale, int detect_num, float min_scale, float max_scale, int scale_num, 
							  int detect_width, int detect_height, float mag_th, int meanshift_radius, int win_th, double likelihood_th)
{
	if (second_ferns.size() == 0){
		std::cerr << "�w�K����Ă��܂���" << std::endl;
		return;
	}

	// �w�K���Ԃ̑���
	double start, finish, time;
	double s_start, s_finish, s_time;
	double m_start, m_finish, m_time;
	double n_start, n_finish, n_time;
	double pose_start, pose_finish, pose_time;
	double total_start, total_finish, total_time;

	total_start = static_cast<double>(cv::getTickCount());


	// 1�w�ڂ̌��o�_����̗̈���擾
	int center_x = detect_point.x;
	int center_y = detect_point.y;
	int range_x = detect_width*detect_scale / 2;
	int range_y = detect_height*detect_scale / 2;
	int up_left_x = center_x-range_x;
	int up_left_y = center_y-range_y;
	cv::Rect roi(up_left_x, up_left_y, 2*range_x, 2*range_y);	// ���o�̈�
	if(roi.x < 0){
		roi.x = 0;
	}
	if(roi.x+roi.width > src.cols){
		roi.width = src.cols - roi.x;
	}
	if(roi.y < 0){
		roi.y = 0;
	}
	if(roi.y+roi.height > src.rows){
		roi.height = src.rows - roi.y;
	}

	// 1�w�ڂ̌��o�̈�̉摜
	cv::Mat detect_src = src(roi).clone();

	// �ő�E�ŏ������ɂ�����T�C�Y�𐄒�(�����̃X�P�[���̔{��)
	std::vector<float> scale_step;
	float scale_step_num=0;
	if (scale_num>1){
		scale_step_num = (max_scale-min_scale) / (float)(scale_num-1);
	}
	for (int i = 0; i < scale_num; ++i)
	{
		// �X�P�[��(1�w�ڂŌ��o�����X�P�[���{)
		float new_scale = distance / detect_point.z * (max_scale - scale_step_num*i);
		scale_step.emplace_back(new_scale);
	}

	if (scale_step.size() == 0) {
		std::cerr << "�������鋗���͈̔͂��K�؂ł���܂���" << std::endl;
	}
	
	///// ���͉摜�̗ʎq�����z�����𐄒� /////

	s_start = static_cast<double>(cv::getTickCount());

	// P-COF������
	p_cof.gridSize = gridSize;

	// ���͉摜�̌��z����
	std::vector<cv::Mat> quant_angles;	

	// �X�P�[����ω�
	for (int i = 0; i < (int)scale_step.size(); ++i)
	{
		cv::Mat resize_src;
		std::vector<cv::Mat_<int>> integ_angle;

		// �X�P�[���ω�(�p�b�`�̑傫���F�偨�������͉摜�F������)
		cv::resize(detect_src, resize_src, cv::Size(), 1.f/scale_step[i], 1.f/scale_step[i]);

		// ���z�����̌v�Z
		p_cof.calcIntegralOrientation(resize_src, integ_angle, mag_th, true, 7);

		// ���z�����̗ʎq��
		cv::Mat quant_angle;
		int freq_th = gridSize * gridSize * 0.3;
		p_cof.quantizedIntegraOrientation(quant_angle, integ_angle, freq_th, gridSize);
		quant_angles.emplace_back(quant_angle);
	}


	// �ޓx�}�b�v�̏�����
	int end_index = quant_angles.size() - 1;
	likelihoodmap.resize(quant_angles.size());
	for( int i = 0; i < (int)(quant_angles.size()); ++i) {
		likelihoodmap[i] = cv::Mat_<float>( quant_angles[i].rows, quant_angles[i].cols, float( 0.f ) );
	}
	// ���̃}�b�v�̏�����
	std::vector<std::vector<std::vector<glm::vec3>>> axis_map;
	std::vector<glm::vec3> axis_tmp(quant_angles[end_index].cols, glm::vec3(0.0f,0.0f,0.0f));
	std::vector<std::vector<glm::vec3>> axis_tmp2(quant_angles[end_index].rows, axis_tmp);
	axis_map.resize( quant_angles.size(), axis_tmp2);
	// �p�x�}�b�v�̏�����
	std::vector<std::vector<std::vector<glm::vec2>>> angle_map;
	std::vector<glm::vec2> angle_tmp(quant_angles[end_index].cols, glm::vec2(0.0f,0.0f));
	std::vector<std::vector<glm::vec2>> angle_tmp2(quant_angles[end_index].rows, angle_tmp);
	angle_map.resize( quant_angles.size(), angle_tmp2);

	s_finish = static_cast<double>(cv::getTickCount());
	s_time = ( (s_finish - s_start ) / cv::getTickFrequency() );


	///// �eFern�ɂ����ăe���v���[�g�}�b�`���O����ѓ��[���� /////

	start = static_cast<double>(cv::getTickCount());

	// 1�w�ڂ̎p���m��������������p����
	int best_pose = 0;
	double max_distribution = 0.0;
	for (int i = 0; i < 8; ++i) {
		if (pose_distribute[detect_num][i] > max_distribution) {
			max_distribution = pose_distribute[detect_num][i];
			best_pose = i;
		}
	}

	// �ޓx�}�b�v�̑���(�ΐ��̑��a)
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		for (int j = 0; j < numFerns2; ++j)
		{
			cv::Mat_<float> likelihood = cv::Mat_<float>( quant_angles[i].rows, quant_angles[i].cols, float( 0.f ) );

			// �ޓx�}�b�v���擾
			second_ferns[best_pose][j]->Predict( quant_angles[i], likelihood, axis_map[i], angle_map[i]);

			// �ޓx�}�b�v�̃T�C�Y�ɂ���ē��[�����ω����邱�Ƃ�h��
			likelihood *= (1.f * scale_step[i]);

			// �ΐ��ɕϊ�
			cv::log(likelihood, likelihood);
			cv::add(likelihoodmap[i], likelihood, likelihoodmap[i]);
		}
	}


	int kernel_size = 3;
	float kernel_sigma = 3.0f;
	
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		// �ΐ����猳�ɖ߂�
		cv::exp(likelihoodmap[i], likelihoodmap[i]);

		// �������Ȃ�߂��Ȃ��悤�X�P�[���{
		likelihoodmap[i] *= 1000000.f;

		// ���̉摜�T�C�Y�ɖ߂�(�O���b�h�P��)
		cv::resize(likelihoodmap[i], likelihoodmap[i], cv::Size(), scale_step[i], scale_step[i]);

		// �����������ʂɂ����镽����
		cv::GaussianBlur(likelihoodmap[i], likelihoodmap[i], cv::Size(kernel_size,kernel_size), kernel_sigma, kernel_sigma);
	}

	// ���������̕�����
	smoothMap(likelihoodmap);
	
	finish = static_cast<double>(cv::getTickCount());
	time = ( ( finish - start ) / cv::getTickFrequency() );


	///// �ޓx�}�b�v��臒l���� /////
	m_start = static_cast<double>(cv::getTickCount());

	// �ő�l�C�ŏ��l�̎Z�o
	double max_val = -DBL_MAX;
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		double minVal, maxVal;
		cv::minMaxLoc (likelihoodmap[i], &minVal, &maxVal, NULL, NULL);

		if (max_val < maxVal) {
			max_val = maxVal;
		}
	}

	// �ޓx�}�b�v�̉���
	visualizeLikelihood(likelihoodmap, scale_step, max_val, true);


	// 臒l�ȏ�݂̂��c��
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		cv::threshold (likelihoodmap[i], likelihoodmap[i], max_val*likelihood_th, 1, cv::THRESH_TOZERO);
	}

	///// Mean Shift�ŋɑ�l�𐄒� /////

	std::vector<cv::Point3f> detectPointInit;
	std::vector<double> detectVote;
	meanShift(detectPointInit, detectVote, scale_step, meanshift_radius);

	m_finish = static_cast<double>(cv::getTickCount());
	m_time = ( ( m_finish - m_start ) / cv::getTickFrequency() );


	///// Nearest Neighbor�ŋߖT�_�𓝍� /////

	n_start = static_cast<double>(cv::getTickCount());

	nearestNeighbor(detectPointInit, detectVote, win_th);

	n_finish = static_cast<double>(cv::getTickCount());
	n_time = ( ( n_finish - n_start ) / cv::getTickFrequency() );


	///// �p���̐��� /////

	pose_start = static_cast<double>(cv::getTickCount());


	// 3�����ʒu�֕ϊ�
	std::vector<cv::Point3f> detect3D;		// ���o�_
	std::vector<int> scale_indexes;			// �C���f�b�N�X
	for (int i = 0; i < (int)detectPointInit.size(); ++i) {
		if (detectPointInit[i].x != -100.0) {

			// 2�����ʒu+����
			detect3D.emplace_back(cv::Point3f((float)detectPointInit[i].x, (float)detectPointInit[i].y, (float)(distance / detectPointInit[i].z)));
			detectPoint.emplace_back(cv::Point3f(up_left_x+(float)detectPointInit[i].x*gridSize, up_left_y+(float)detectPointInit[i].y*gridSize, (float)(distance / detectPointInit[i].z)));
			
			// �ł��߂��X�P�[�������o��
			float d_scale=100.f;
			int scale_index = 0;
			for(int j = 0; j < (int)(scale_step.size()); ++j) {
				float div_scale = abs(detectPointInit[i].z - scale_step[j]);
				if( div_scale < d_scale) {
					d_scale = div_scale;
					scale_index = j;
				}
			}
			scale_indexes.emplace_back(scale_index);
		}
	}


	std::vector<glm::mat4> pose_est;
	poseEstimation(pose_est, axis_map, angle_map, detect3D, scale_indexes, scale_step);

	// �ǉ�
	for (int i = 0; i < (int)pose_est.size(); ++i)
	{
		pose_estimate.emplace_back(pose_est[i]);
	}

	pose_finish = static_cast<double>(cv::getTickCount());
	pose_time = ( ( pose_finish - pose_start ) / cv::getTickFrequency() );

	total_finish = static_cast<double>(cv::getTickCount());
	total_time = ( ( total_finish - total_start ) / cv::getTickFrequency() );


	// ���ʂ̃R���\�[���o��
	for (int i = 0; i < (int)detectPoint.size(); ++i) {
		std::cout << "���o�ʒu(x,y,z): (" << detectPoint[i].x << "," << detectPoint[i].y << "," << detectPoint[i].z << ")" << std::endl;
		std::cout << "�p�b�`�̃X�P�[��(index,s): (" << scale_indexes[i] << "," << scale_step[scale_indexes[i]] << ")" << std::endl;
		std::cout << "�p��: " << glm::to_string(pose_est[0]) << std::endl;
		std::cout << std::endl;
	}

	std::cout << "���͉摜�������� : " << s_time << "�b" << std::endl;
	std::cout << "���[���� : " << time << "�b" << std::endl;
	std::cout << "Mean Shift���� : " << m_time << "�b" << std::endl;
	std::cout << "Nearest Neighbor���� : " << n_time << "�b" << std::endl;
	std::cout << "�p�����莞�� : " << pose_time << "�b" << std::endl;
	std::cout << "���ʎ��� : " << total_time << "�b" << std::endl << std::endl;
}



/**
 * @brief   ���������ɂ�����K�E�V�A���t�B���^�ɂ��}�b�v�̕�����
 */
void HoughFerns::smoothMap(std::vector<cv::Mat_<float>> &likelihood)
{

	// �ŏ��̕��ƍ���
	int min_width = INT_MAX;
	int min_height = INT_MAX;
	for (int i = 0; i < (int)likelihood.size(); ++i)
	{
		if(min_width > likelihood[i].cols) {
			min_width = likelihood[i].cols;
		}

		if(min_height > likelihood[i].rows) {
			min_height = likelihood[i].rows;
		}
	}


	///// �K�E�V�A���t�B���^�ɂ�鋗�������̕����� /////

	// �}�b�v�̃R�s�[
	std::vector<cv::Mat_<double>> copy_likelihood(likelihood.size());
	for (int i = 0; i < (int)copy_likelihood.size(); ++i) {
		copy_likelihood[i] = likelihood[i].clone();
	}
	int kernel_size = 3;
    float kernel_sigma = 0.5f;
	cv::Mat gaussKernel = cv::getGaussianKernel( 3, 3.0, CV_32F );

	// ��ԏ������T�C�Y�̃X�P�[���ɍ��킹�ĕ�����
	for (int y = 0; y < min_height; ++y)
	{
		for ( int x = 0; x < min_width; ++x) 
		{
			for( int s = 0; s < (int)likelihood.size(); ++s) 
			{
                float convSum = 0;
                int k = 0;

				int scBegin = s - kernel_size / 2;		// �擪
                int scEnd = s + kernel_size / 2 + 1;	// ����

				// �擪���͂ݏo��ꍇ
				if (scBegin == -1) {
					scBegin = 0;
					convSum += (float)copy_likelihood[0](y, x) * 0.3f * gaussKernel.at<float>(k);	// �d�ݕt���̍ŏ��X�P�[��
					k++;
				}
				// �������͂ݏo��ꍇ
				if (scEnd > (int)likelihood.size()) {
					scEnd = (int)likelihood.size();
					convSum += (float)copy_likelihood[(int)likelihood.size()-1](y, x) * 0.3f * gaussKernel.at<float>(gaussKernel.cols-1);	// �d�ݕt���̍ő�X�P�[��
				}

				// �J�[�l���̏�݂���
                for( int td = scBegin; td < scEnd; td++, k++) {
					convSum += (float)copy_likelihood[td](y, x) * gaussKernel.at<float>(k);
				}

				likelihood[s](y, x) = convSum;
            }
		}
	}
}


/**
 * @brief   �ޓx�}�b�v�̉���
 * 
 * @param   likelihood[in]		�ޓx�}�b�v
 * @param   scale_rate[in]		�X�P�[���̔{��
 * @param   max_value[in]		�}�b�v�̍ő�l
 * @param   flag[in]			2�w�ڂ��ǂ���
 */
void HoughFerns::visualizeLikelihood(const std::vector<cv::Mat_<float>> &likelihood, const std::vector<float> scale_rate, float max_value, bool flag)
{
	std::vector<cv::Mat> colorImage(likelihood.size());
	for (int i = 0; i < (int)likelihood.size(); ++i) {
		colorImage[i] = cv::Mat::zeros( likelihood[i].rows, likelihood[i].cols, CV_8UC3);
	}

	int channel = colorImage[0].channels();
	double alpha = 255.0 / ( max_value );

	for (int s = 0; s < (int)likelihood.size(); ++s) {
		for (int y = 0; y < (int)likelihood[s].rows; ++y){
			for (int x = 0; x < (int)likelihood[s].cols; ++x){
				// �F���}�b�v
				colorImage[s].data[ y * colorImage[s].step + x * channel + 0 ] = 120 - (int)( likelihood[s](y, x) * alpha * 120.0 / 255.0);
				colorImage[s].data[ y * colorImage[s].step + x * channel + 1 ] = 255;
				colorImage[s].data[ y * colorImage[s].step + x * channel + 2 ] = 255;
			}
		}
		cv::cvtColor(colorImage[s], colorImage[s], CV_HSV2BGR);
		// �\��
		if(!flag) {
			cv::namedWindow("1st"+std::to_string(s), cv::WINDOW_NORMAL);
			cv::imshow("1st"+std::to_string(s), colorImage[s]);
		} else {
			cv::namedWindow("2nd"+std::to_string(s), cv::WINDOW_NORMAL);
			cv::imshow("2nd"+std::to_string(s), colorImage[s]);
		}
	}
}



/**
 * @brief   Mean Shift�ŋɑ�l����
 * 
 * @param   meanshiftPoint[in,out]	Mean Shift��̌��o�_
 * @param   detectionVote[in,out]	���o�̓��[�l���i�[
 * @param   scales[in]				�X�P�[���̔{��
 * @param   radius[in]				�J�[�l����
 */
void HoughFerns::meanShift(std::vector<cv::Point3f> &meanshiftPoint, std::vector<double> &detectionVote, const std::vector<float> scale_rate, int radius)
{
	double xi, xj, yi, yj, si, sj, sum_x, sum_y, sum_s, sum_w, Kernel, move_x, move_y, move_s;

	// �ŏ��̕��ƍ���
	int min_width = INT_MAX;
	int min_height = INT_MAX;
	for (int i = 0; i < (int)likelihoodmap.size(); ++i)
	{
		if(min_width > likelihoodmap[i].cols) {
			min_width = likelihoodmap[i].cols;
		}

		if(min_height > likelihoodmap[i].rows) {
			min_height = likelihoodmap[i].rows;
		}
	}

	std::vector<cv::Point3f> centerPoint;		// ���o���W

	// Map���猟�o���W���������o��
	for (int s = 0; s < (int)likelihoodmap.size(); ++s) {
		for (int y = 0; y < min_height; ++y){
			for (int x = 0; x < min_width; ++x){

				if( likelihoodmap[s](y, x) != 0.0){
					centerPoint.push_back(cv::Point3f(x, y, scale_rate[s]));
					detectionVote.push_back(likelihoodmap[s](y, x));
				}
			}
		}
	}

	meanshiftPoint.resize(centerPoint.size());

	// ���o���W�������J��Ԃ�
	for (int i = 0 ; i < (int)centerPoint.size() ; ++i)
	{
		xi = centerPoint[i].x;
		yi = centerPoint[i].y;
		si = centerPoint[i].z;

		// ��������܂ŌJ��Ԃ�
		while(1)
		{
			// ������
			sum_x = sum_y = sum_s = sum_w = 0.0;

			// ���x�v�Z
			for (int j = 0 ; j < (int)centerPoint.size(); ++j)
			{
				xj = centerPoint[j].x;
				yj = centerPoint[j].y;
				sj = centerPoint[j].z; 

				// x-y-s��Ԃ̃J�[�l���֐�
				Kernel = - (((xi - xj) * (xi - xj) + (yi - yj) * (yi - yj) + (si - sj) * (si - sj)) / (radius * radius));
				Kernel = exp( Kernel );

				sum_x += xj * Kernel;
				sum_y += yj * Kernel;
				sum_s += sj * Kernel;
				sum_w += Kernel;
			}
			
			// �ړ��ʂ̌v�Z
			move_x = ( sum_x / sum_w ) - xi;
			move_y = ( sum_y / sum_w ) - yi;							
			move_s = ( sum_s / sum_w ) - si;							
			// �ړ�
			xi = (int)( xi + move_x );
			yi = (int)( yi + move_y );
			si =  si + move_s;

			// 臒l�ȉ��Ɏ�������܂�
			if (move_x > -1.0 && move_x < 1.0 && move_y > -1.0 && move_y < 1.0 && move_s > -1.0 && move_s < 1.0){
				break;
			}
		}

		// Mean Shift��ʒu�ۑ�
		meanshiftPoint[i].x = int(xi);
		meanshiftPoint[i].y = int(yi);
		meanshiftPoint[i].z = si;
	}
}


/**
 * @brief   Nearest Neighbor�ŋߖT�_�𓝍�
 * 
 * @param   detectionPoint[in,out]		���o�_
 * @param   detectionVote[in,out]		���[�l
 * @param   win_th[in]					��������͈� 
 */
void HoughFerns::nearestNeighbor(std::vector<cv::Point3f> &detectionPoint, std::vector<double> &detectionVote, int win_th)
{
	double distance;
	int x, y, s;

	for (int n = 0; n < (int)detectionPoint.size()-1; ++n)
	{
		for (int m = n+1; m < (int)detectionPoint.size(); ++m)
		{	
			if (detectionPoint[ n ].x != -100)
			{
				x = ( detectionPoint[ n ].x - detectionPoint[ m ].x );
				y = ( detectionPoint[ n ].y - detectionPoint[ m ].y );
				s = (int)( detectionPoint[ n ].z - detectionPoint[ m ].z );
				distance = ( ( x * x ) + ( y * y ) + ( s * s ));
				if (distance < (win_th*win_th) )
				{
					detectionVote[ n ] = -100.0;		// �폜�t���O
					detectionPoint[ n ] = cv::Point3f(-100.f, -100.f, -100.f);
				}
			}
		}
	}

	// �v�f�̍폜
	auto itr = detectionPoint.begin();
	while (itr != detectionPoint.end())
	{
		if (itr->x == -100.f)
		{
			itr = detectionPoint.erase(itr);
		} else {
			itr++;
		}
	}

	auto itr2 = detectionVote.begin();
	while (itr2 != detectionVote.end())
	{
		if (*itr2 == -100.0)
		{
			itr2 = detectionVote.erase(itr2);
		} else {
			itr2++;
		}
	}
}


/**
 * @brief   ���o�ʒu���Ɏp������
 * 
 * @param   pose_dist[in,out]			���o�_�ɑΉ�����p���m��
 * @param   poseMap[in,out]				�p���}�b�v
 * @param   detectionPoint[in]			���o�_
 * @param	scales[in]					���o���ɍł��߂��X�P�[���̃C���f�b�N�X
 * @param	scale_rate[in]				�X�P�[���̔{��
 */
void HoughFerns::poseEstimation(std::vector<std::vector<float>> &pose_dist, std::vector<std::vector<cv::Mat_<float>>> &poseMap, 
								const std::vector<cv::Point3f> &detectionPoint, const std::vector<int> scales, const std::vector<float> scale_rate)
{
	int x, y;
	pose_dist.clear();

	// ���o�ʒu�ɑ΂��Ďp������
	for (int i = 0; i < (int)detectionPoint.size(); ++i)
	{
		// ���[���̃X�P�[���ɖ߂�(�d�S�ʒu�̖ޓx�}�b�v�̂݌��̃T�C�Y�ɂȂ��Ă���)
		x = detectionPoint[i].x / scale_rate[scales[i]];
		y = detectionPoint[i].y / scale_rate[scales[i]];

		std::vector<float> distribute;
		// �p���̕��ϒl���i�[
		double total_pose = 0.0;
		for (int j = 0; j < 8; ++j) {
			total_pose += poseMap[scales[i]][j](y,x);
		}
		for (int j = 0; j < 8; ++j) {
			float mean = 0.0;
			if (total_pose != 0.0) {
				mean = poseMap[scales[i]][j](y,x) / total_pose;
			}
			distribute.emplace_back(mean);
		}
		pose_dist.emplace_back(distribute);
	}
}


/**
 * @brief   ���m�Ɏp������
 * 
 * @param   pose_est[in,out]			���o�_�ɑΉ�����p��
 * @param   axisMap[in,out]				���o�_�ɑΉ������]��
 * @param   angleMap[in]				���o�_�ɑΉ������]�p
 * @param   detectionPoint[in]			���o�_
 * @param	scales[in]					���o���ɍł��߂��X�P�[���̃C���f�b�N�X
 * @param	scale_rate[in]				�X�P�[���̔{��
 */
void HoughFerns::poseEstimation(std::vector<glm::mat4> &pose_est, std::vector<std::vector<std::vector<glm::vec3>>> &axisMap, std::vector<std::vector<std::vector<glm::vec2>>> &angleMap, 
								const std::vector<cv::Point3f> &detectionPoint, const std::vector<int> scales, const std::vector<float> scale_rate)
{
	int x, y;

	// ���o�ʒu�ɑ΂��Ďp������
	for (int i = 0; i < (int)detectionPoint.size(); ++i)
	{
		// ���[���̃X�P�[���ɖ߂�(�d�S�ʒu�̖ޓx�}�b�v�̂݌��̃T�C�Y�ɂȂ��Ă���)
		x = detectionPoint[i].x / scale_rate[scales[i]];
		y = detectionPoint[i].y / scale_rate[scales[i]];
		
		// ���̕���
		glm::vec3 meanAxis = glm::normalize(axisMap[scales[i]][y][x]);

		// roll�p�̕���
		glm::vec2 meanAngle = glm::normalize(angleMap[scales[i]][y][x]);
		float theta = std::atan2f(meanAngle.y, meanAngle.x);

		// ��]�s��̐���
		glm::vec3 eyePosition = glm::normalize(eyeVector - meanAxis);
		glm::mat4 ViewMatrix;
		glm::vec3 const f(glm::normalize(eyeVector - eyePosition));
		glm::vec3 s0(glm::cross(f, eyeUp));
		if (s0 == glm::vec3(0.0f,0.0f,0.0f))
		{
			s0 = glm::vec3(0.0f,0.0f,-1.0f);
		}
		glm::vec3 const s(glm::normalize(s0));
		glm::vec3 const u(glm::cross(s, f));

		ViewMatrix[0][0] = s.x;
		ViewMatrix[1][0] = s.y;
		ViewMatrix[2][0] = s.z;
		ViewMatrix[0][1] = u.x;
		ViewMatrix[1][1] = u.y;
		ViewMatrix[2][1] = u.z;
		ViewMatrix[0][2] =-f.x;
		ViewMatrix[1][2] =-f.y;
		ViewMatrix[2][2] =-f.z;
		ViewMatrix[3][0] =-glm::dot(s, eyePosition);
		ViewMatrix[3][1] =-glm::dot(u, eyePosition);
		ViewMatrix[3][2] = glm::dot(f, eyePosition);

		glm::quat quat(std::cos(theta/2.0), meanAxis.x*std::sin(theta/2.0), meanAxis.y*std::sin(theta/2.0), meanAxis.z*std::sin(theta/2.0) );
		glm::mat4 modelMatrix = ViewMatrix * glm::mat4_cast(quat);

		pose_est.emplace_back(modelMatrix);

		/*std::cout << detectionPoint[i] << std::endl;
		std::cout << glm::to_string(meanAxis) << "," << theta << std::endl;
		std::cout << glm::to_string(modelMatrix) << "," << std::endl;*/
	}
}


// �w�K�T���v���̎擾
void HoughFerns::getLearnSamples(const std::string& posFolder, const std::string& negFolder, std::vector<std::shared_ptr<LearnSample>>& samples)
{

	///// �|�W�e�B�u�T���v���̎擾 /////
	std::cout << "�|�W�e�B�u�T���v�����o�c" << std::endl;

	{
		// �t�H���_����t�@�C�������擾
		std::vector<std::string> fileNames;
		WIN32_FIND_DATA ffd;
		HANDLE hF;

		std::string folderName = posFolder + "/*.xml";
		hF = FindFirstFile( folderName.c_str(), &ffd);
		if (hF != INVALID_HANDLE_VALUE) {
			// �t�H���_���̃t�@�C���̒T��
			do {
				std::string fullpath = posFolder + "/" + ffd.cFileName;		// �t�@�C���̃p�X

				// �t�H���_���̊i�[
				fileNames.emplace_back(fullpath);

			} while (FindNextFile(hF, &ffd ) != 0);
			FindClose(hF);
		}

		bool init_load = true;

		// �t�@�C�������ǂݍ���
		for (int f = 0; f < (int)fileNames.size(); ++f)
		{
			// �t�@�C���̓ǂݍ���
			cv::FileStorage cvfs(fileNames[f], cv::FileStorage::READ);

			int patchNum, patch_size, grid_size, quadrant;
			float angle, dist;
			cv::Mat axis_cv, eyeVec_cv, eyeUp_cv;
			cvfs["patchNum"] >> patchNum;
			cvfs["patchSize"] >> patch_size;
			cvfs["gridSize"] >> grid_size;
			cvfs["axis"] >> axis_cv;
			cvfs["eyeVector"] >> eyeVec_cv;
			cvfs["eyeUp"] >> eyeUp_cv;
			cvfs["angle"] >> angle;
			cvfs["distance"] >> dist;
			cvfs["quadrant"] >> quadrant;

			glm::vec3 axis(axis_cv.at<float>(0), axis_cv.at<float>(1), axis_cv.at<float>(2));
			glm::vec2 rollAngle(std::cos(angle), std::sin(angle));

			if (init_load){
				patchSize = patch_size;
				gridSize = grid_size;
				eyeVector = glm::vec3(eyeVec_cv.at<float>(0), eyeVec_cv.at<float>(1), eyeVec_cv.at<float>(2));
				eyeUp = glm::vec3(eyeUp_cv.at<float>(0), eyeUp_cv.at<float>(1), eyeUp_cv.at<float>(2));
				distance = dist;
				init_load = false;
			} else {
				// �f�[�^�Z�b�g�̊����قȂ�ꍇ�Ɍx��
				if (patchSize != patch_size || gridSize != grid_size || fabs(distance-dist) > 0.0001) {
					std::cerr << "�f�[�^�Z�b�g�Ɋ����قȂ���̂��܂܂�Ă��܂�" << std::endl;
					exit (0);
				}
			}

			// �p�b�`�̐������J��Ԃ�
			for (int i = 0; i < patchNum; ++i)
			{
				cv::Mat gradMat, pcofMat, weightMat, offset;
				cvfs["offset"+std::to_string(i)] >> offset;
				cvfs["GradOri"+std::to_string(i)] >> gradMat;
				cvfs["PcofOri"+std::to_string(i)] >> pcofMat;
				cvfs["weight"+std::to_string(i)] >> weightMat;

				// �w�K�f�[�^
				auto posSample = std::make_shared<LearnSample>();
				posSample->posInsert( gradMat, pcofMat, weightMat, 1, quadrant, cv::Point2f(offset.at<int>(0),offset.at<int>(1)), axis, rollAngle);

				// �ǉ�
				samples.emplace_back(posSample);
				posSample_num++;
			}

			cvfs.release();		// ���
		}
	}

	// �O���b�h�T�C�Y�̐ݒ�
	p_cof.gridSize = gridSize;

	std::cout << "�|�W�e�B�u�T���v�����F" << posSample_num << std::endl;



	///// �l�K�e�B�u�T���v���̎擾 /////
	std::cout << "�l�K�e�B�u�T���v�����o�c" << std::endl;

	{
		// �t�H���_����t�@�C�������擾
		std::vector<std::string> fileNames;
		WIN32_FIND_DATA ffd;
		HANDLE hF;

		std::string folderName = negFolder + "/*.*";
		hF = FindFirstFile( folderName.c_str(), &ffd);
		if (hF != INVALID_HANDLE_VALUE) {
			// �t�H���_���̃t�@�C���̒T��
			do {
				std::string fullpath = negFolder + "/" + ffd.cFileName;		// �t�@�C���̃p�X

				// �t�H���_���̊i�[
				fileNames.emplace_back(fullpath);

			} while (FindNextFile(hF, &ffd ) != 0);
			FindClose(hF);
		}

		// �������o
		P_COF p_cof(gridSize);

		// �t�@�C�������ǂݍ���
		for (int f = 0; f < (int)fileNames.size(); ++f)
		{
			// �O���C�X�P�[���ŉ摜�ǂݍ���
			cv::Mat image = cv::imread(fileNames[f], CV_LOAD_IMAGE_GRAYSCALE);

			if (!image.empty())
			{
				std::vector<cv::Mat_<int>> integ_angle;
				cv::Mat quant_angle;

				// ���z�����̌v�Z
				p_cof.calcIntegralOrientation( image, integ_angle, 200.f, true, 7);

				// ���z�����̗ʎq��
				int freq_th = gridSize * gridSize * 0.3;
				p_cof.quantizedIntegraOrientation(quant_angle, integ_angle, freq_th, gridSize);

				// �X�e�b�v��
				int step = patchSize / gridSize;

				// �p�b�`�T�C�Y�ɐ؂�o��
				for (int r = 0; r < quant_angle.rows - step; r+=step)
				{
					for (int c = 0; c < quant_angle.cols - step; c+=step)
					{
						cv::Rect roi(c, r, step, step);
						cv::Mat patch = quant_angle(roi);

						// ���������J�E���g
						int count = 0;
						for (int r = 0; r < step; ++r)
						{
							uchar* ori_r = patch.ptr<uchar>(r);
							for (int c = 0; c < step; ++c)
							{
								if (ori_r[c] > 0) {
									count++;
								}
							}
						}

						// ����������ꍇ�̂ݒǉ�
						if (count > step*step*0.1)
						{
							// �w�K�f�[�^
							auto negSample = std::make_shared<LearnSample>();
							negSample->negInsert(quant_angle(roi), 0);

							// �ǉ�
							samples.emplace_back(negSample);
							negSample_num++;
						}
					}
				}
			}
		}
	}

	std::cout << "�l�K�e�B�u�T���v�����F" << negSample_num << std::endl;
}



/**
 * @brief   �T�u�Z�b�g�̍쐬(1�w�ڗp)
 * 
 * @param   subSamples[in,out]		�T�u�Z�b�g
 * @param   samples[in]				�S�f�[�^
 * @param   pos_rate[in]			�|�W�e�B�u�T���v���̊��� 
 * @param   neg_rate[in]			�l�K�e�B�u�T���v���̊��� 
 * @param   posSize[in,out]			�|�W�e�B�u�T���v���̐� 
 * @param   negSize[in,out]			�l�K�e�B�u�T���v���̐�
 */
void HoughFerns::makeSubset(std::vector<std::shared_ptr<LearnSample>> &subSamples, const std::vector<std::shared_ptr<LearnSample>> &samples, float pos_rate, float neg_rate, int &posSize, int &negSize)
{
	// �͈͊m�F
	if (pos_rate < 0.0){
		pos_rate = 1.0f;
	}
	if (neg_rate < 0.0){
		neg_rate = 1.0f;
	}
	posSize = posSample_num * pos_rate;
	negSize = negSample_num * neg_rate;
	int subsample_size = posSize+negSize;

	// �e�ʊm��
	subSamples.clear();
	subSamples.resize(subsample_size, std::make_shared<LearnSample>());

	std::random_device rnd;							// �񌈒�I�ȗ���������𐶐�
    std::mt19937 mt(rnd());							// �����Z���k�E�c�C�X�^�A�����͏����V�[�h�l
	std::uniform_int_distribution<int> rand_posSample(0, posSample_num-1);					// ��l����(�|�W�e�B�u)
	std::uniform_int_distribution<int> rand_negSample(posSample_num, samples.size()-1);		// ��l����(�l�K�e�B�u)

	// �����_���T���v�����O(�|�W�e�B�u)
	for (int i = 0; i < posSize; ++i){
		subSamples[i] = samples[rand_posSample(mt)];
	}
	// �����_���T���v�����O(�l�K�e�B�u)
	for (int i = posSize; i < subsample_size; ++i){
		subSamples[i] = samples[rand_negSample(mt)];
	}
}


/**
 * @brief   �T�u�Z�b�g�̍쐬
 * 
 * @param   subSamples[in,out]		�T�u�Z�b�g
 * @param   samples[in]				�S�f�[�^
 * @param   pos_num[in]				�|�W�e�B�u�T���v���̑��� 
 * @param   neg_num[in]				�l�K�e�B�u�T���v���̑���
 * @param   pos_rate[in]			�|�W�e�B�u�T���v���̊��� 
 * @param   neg_rate[in]			�l�K�e�B�u�T���v���̊��� 
 * @param   posSize[in,out]			�|�W�e�B�u�T���v���̐� 
 * @param   negSize[in,out]			�l�K�e�B�u�T���v���̐�
 */
void HoughFerns::makeSubset(std::vector<std::shared_ptr<LearnSample>> &subSamples, const std::vector<std::shared_ptr<LearnSample>> &samples, int pos_num, int neg_num, 
							float pos_rate, float neg_rate, int &posSize, int &negSize)
{
	// �͈͊m�F
	if (pos_rate < 0.0){
		pos_rate = 1.0f;
	}
	if (neg_rate < 0.0){
		neg_rate = 1.0f;
	}
	posSize = pos_num * pos_rate;
	negSize = neg_num * neg_rate;
	int subsample_size = posSize+negSize;

	// �e�ʊm��
	subSamples.clear();
	subSamples.resize(subsample_size, std::make_shared<LearnSample>());

	std::random_device rnd;							// �񌈒�I�ȗ���������𐶐�
    std::mt19937 mt(rnd());							// �����Z���k�E�c�C�X�^�A�����͏����V�[�h�l
	std::uniform_int_distribution<int> rand_posSample(0, pos_num-1);					// ��l����(�|�W�e�B�u)
	std::uniform_int_distribution<int> rand_negSample(pos_num, samples.size()-1);		// ��l����(�l�K�e�B�u)

	// �����_���T���v�����O(�|�W�e�B�u)
	for (int i = 0; i < posSize; ++i){
		subSamples[i] = samples[rand_posSample(mt)];
	}
	// �����_���T���v�����O(�l�K�e�B�u)
	for (int i = posSize; i < subsample_size; ++i){
		subSamples[i] = samples[rand_negSample(mt)];
	}
}


/**
 * @brief   Hough Ferns��ۑ�
 * 
 * @param   folderName[in]		Hough Ferns��ۑ�����t�H���_
 */
void HoughFerns::saveFerns(const std::string &folderName)
{
	std::cout << "Hough Ferns�̕ۑ����c" << std::endl;

	std::ofstream ofs(folderName + "/ferns_param.csv");

	// Hough Ferns�p�����[�^��ۑ�
	ofs << "����؂̐�(1�w��)" << "," << numFerns1 << std::endl;
	ofs << "����؂̐�(2�w��)" << "," << numFerns2 << std::endl;
	ofs << "Fern�̐[��(1�w��)" << "," << depth1 << std::endl;
	ofs << "Fern�̐[��(2�w��)" << "," << depth2 << std::endl;
	ofs << "�����I����(1�w��)" << "," << featureTestNum1 << std::endl;
	ofs << "�����I����(2�w��)" << "," << featureTestNum2 << std::endl;
	ofs << "臒l�I����(1�w��)" << "," << thresholdTestNum1 << std::endl;
	ofs << "臒l�I����(2�w��)" << "," << thresholdTestNum2 << std::endl;
	ofs << "�N���X��" << "," << numClass << std::endl;
	ofs << "�|�W�e�B�u�f�[�^�̊���(1�w��)" << "," << posDataPerTree1 << std::endl;
	ofs << "�|�W�e�B�u�f�[�^�̊���(2�w��)" << "," << posDataPerTree2 << std::endl;
	ofs << "�l�K�e�B�u�f�[�^�̊���(1�w��)" << "," << negDataPerTree1 << std::endl;
	ofs << "�l�K�e�B�u�f�[�^�̊���(2�w��)" << "," << negDataPerTree2 << std::endl;
	// �w�K�T���v���p�p�����[�^��ۑ�
	ofs << "�|�W�e�B�u�摜�̃��x��" << "," << posLabel << std::endl;
	ofs << "�l�K�e�B�u�摜�̃��x��" << "," << negLabel << std::endl;
	ofs << "�����_�����O���̃J��������" << "," << eyeVector.x << "," << eyeVector.y << "," << eyeVector.z << std::endl;
	ofs << "�����_�����O���̃J���������" << "," << eyeUp.x << "," << eyeUp.y << "," << eyeUp.z << std::endl;
	ofs << "���f���ƃJ�����Ƃ̋���" << "," << distance << std::endl;
	// �����p�p�����[�^��ۑ�
	ofs << "�p�b�`�T�C�Y" << "," << patchSize << std::endl;
	ofs << "�O���b�h�T�C�Y" << "," << gridSize << std::endl;

	std::cout << "�ۑ��I��" << std::endl << std::endl;
}




/**
 * @brief   Hough Ferns�̓ǂݍ���
 * 
 * @param   folderName[in]		Hough Forests��ǂݍ��ރt�H���_
 * 
 * @returns true or false
 */
bool HoughFerns::loadFerns(const std::string &folderName)
{
	std::cout << "Hough Ferns�̓ǂݍ��ݒ��c" << std::endl;

	///// 1�w�� /////

	// Ferns�̏���
	first_ferns.clear();

	// Fern�̓ǂݍ���
	char filename[ 256 ];
	for (int i = 0; i < numFerns1; ++i)
	{
		// Fern�̍쐬
		std::unique_ptr<FirstLayerFern> fern(new FirstLayerFern(depth1, featureTestNum1, thresholdTestNum1, numClass, posLabel, negLabel, gridSize));

		double start, finish, time;
		start = static_cast<double>(cv::getTickCount());

		// Fern�̓ǂݍ���
		sprintf( filename, "%s/fern1_%d.txt", folderName.c_str(), i );
		if ( fern->loadFern(filename) == false ){
			return false;
		}

		finish = static_cast<double>(cv::getTickCount());
		time = ( ( finish - start ) / cv::getTickFrequency() );
		std::cout << "Load First Layer:" << i << ", Time:" << time << std::endl; 

		// Fern�̒ǉ�
		first_ferns.emplace_back(std::move(fern));
	}


	///// 2�w�� /////

	// Ferns�̏���
	second_ferns.clear();

	// Fern�̓ǂݍ���
	char filename2[ 256 ];
	// �p���������ɓǂݍ���
	for (int d = 0; d < 8; ++d)
	{
		std::vector<std::unique_ptr<SecondLayerFern>> ferns;

		for (int i = 0; i < numFerns2; ++i)
		{
			// Fern�̍쐬
			std::unique_ptr<SecondLayerFern> fern(new SecondLayerFern(depth2, featureTestNum2, thresholdTestNum2, numClass, posLabel, negLabel, gridSize));

			double start, finish, time;
			start = static_cast<double>(cv::getTickCount());

			// Fern�̓ǂݍ���
			sprintf( filename2, "%s/fern2_%d_%d.txt", folderName.c_str(), d, i );
			if ( fern->loadFern(filename2) == false ){
				return false;
			}

			finish = static_cast<double>(cv::getTickCount());
			time = ( ( finish - start ) / cv::getTickFrequency() );
			std::cout << "Load SecondLayer:" << d << "_" << i << ", Time:" << time << std::endl; 

			// Fern�̒ǉ�
			ferns.emplace_back(std::move(fern));
		}
		// �ǉ�
		second_ferns.emplace_back(std::move(ferns));
	}

	std::cout << "�ǂݍ��ݏI��" << std::endl << std::endl;

	return true;
}