# include "HoughFerns.h"


/**
 * @brief   全てのパラメータの初期化	
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
 * @brief   設定ファイルの読み込み
 * 
 * @param   fileName[in]	設定ファイル名
 * 
 * @returns true or false    
 */
bool HoughFerns::initFromFile(const std::string &fileName)
{
	// 設定ファイルの読み込み
	std::ifstream ifs(fileName);

	if( !ifs ) {
		std::cerr << "設定ファイルを読み込めません" << std::endl;
		return false;
	}

	std::vector<std::vector<std::string>> params;	// 文字の格納

	// 文字列の取得
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


	// Hough Forestsパラメータ
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
	// 学習サンプルパラメータ
	posLabel = atoi(params[13][1].c_str());
	negLabel = atoi(params[14][1].c_str());
	eyeVector.x = (float)atof(params[15][1].c_str());
	eyeVector.y = (float)atof(params[15][2].c_str());
	eyeVector.z = (float)atof(params[15][3].c_str());
	eyeUp.x = (float)atof(params[16][1].c_str());
	eyeUp.y = (float)atof(params[16][2].c_str());
	eyeUp.z = (float)atof(params[16][3].c_str());
	distance = (float)atof(params[17][1].c_str());
	// 特徴用パラメータ
	patchSize = atoi(params[18][1].c_str());
	gridSize = atoi(params[19][1].c_str());

	// グリッドサイズの設定
	p_cof.gridSize = gridSize;

	return true;
}


/**
 * @brief   Hough Fernsの学習を行う
 * 
 * @param   posFolder[in]	ポジティブ画像のあるフォルダ名
 * @param   negFolder[in]	ネガティブ画像のあるフォルダ名
 * @param   saveFolder[in]	Fernを保存するフォルダ名
 */
void HoughFerns::Learn(const std::string& posFolder, const std::string& negFolder, const std::string& saveFolder)
{
	// コンソールに出力
	std::cout << "------------------------------------" << std::endl;
	std::cout << " Hough Fernsによる学習" << std::endl;
	std::cout << "------------------------------------" << std::endl << std::endl;

	std::cout << "決定木の数(1層目)		: " << numFerns1 << std::endl;
	std::cout << "決定木の数(2層目)		: " << numFerns2 << std::endl;
	std::cout << "木の最大深さ(1層目)		: " << depth1 << std::endl;
	std::cout << "木の最大深さ(2層目)		: " << depth2 << std::endl;
	std::cout << "特徴選択回数(1層目)		: " << featureTestNum1 << std::endl;
	std::cout << "特徴選択回数(2層目)		: " << featureTestNum2 << std::endl;
	std::cout << "閾値選択回数(1層目)		: " << thresholdTestNum1 << std::endl;
	std::cout << "閾値選択回数(2層目)		: " << thresholdTestNum2 << std::endl;
	std::cout << "クラス数			: " << numClass << std::endl;
	std::cout << "ポジティブデータの割合(1層目)	: " << posDataPerTree1 << std::endl;
	std::cout << "ポジティブデータの割合(2層目)	: " << posDataPerTree2 << std::endl;
	std::cout << "ネガティブデータの割合(1層目)	: " << negDataPerTree1 << std::endl;
	std::cout << "ネガティブデータの割合(2層目)	: " << negDataPerTree2 << std::endl << std::endl;


	// 学習時間の測定
	double start, finish, time, f_start, f_finish, f_time;
	start = static_cast<double>(cv::getTickCount());
	f_start = static_cast<double>(cv::getTickCount());

	// 学習サンプルの抽出
	std::vector<std::shared_ptr<LearnSample>> samples;
	getLearnSamples(posFolder, negFolder, samples);


	///// 1層目のHough Fernsの学習 /////

	std::cout << "\n1層目のHough Fernsの学習中…" << std::endl;
	{
		std::vector<std::unique_ptr<FirstLayerFern>> ferns;

		for (int i = 0; i < numFerns1; ++i)
		{
			printf("%d / %d本目\r", i+1, numFerns1);

			// 学習データのブートストラップサンプリング
			std::vector<std::shared_ptr<LearnSample>> subSamples;
			int posSize, negSize;
			makeSubset (subSamples, samples, posDataPerTree1, negDataPerTree1, posSize, negSize);

			// InverseLabelの作成(クラス数の逆数)
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

			// 姿勢のInverseLabelの作成
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

			// Fernの作成
			std::unique_ptr<FirstLayerFern> fern( new FirstLayerFern(depth1, featureTestNum1, thresholdTestNum1, numClass, posLabel, negLabel, gridSize));

			// Fernの学習
			fern->Learn(subSamples, posSize, il, poseIl);

			// Fernの追加
			ferns.emplace_back(std::move(fern));
		}

		// Fernの保存
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

	std::cout << "1層目：" << numFerns1 << "本のFernを構築" << std::endl;
	std::cout << "1層目処理時間：" << f_time << "秒" << std::endl;



	///// 2層目のHough Fernsの学習 /////

	std::cout << "\n2層目のHough Fernsの学習中…" << std::endl;

	// 分割数だけ繰り返す
	for (int d = 0; d < 8; ++d)
	{
		std::cout << d+1 << "番目の姿勢方向の学習" << std::endl;

		// 学習時間の測定
		double s_start, s_finish, s_time;
		s_start = static_cast<double>(cv::getTickCount());

		std::vector<std::unique_ptr<SecondLayerFern>> ferns;

		// 姿勢方向の学習サンプルを取得
		int second_pos_num = 0;
		int second_neg_num = 0;
		std::vector<std::shared_ptr<LearnSample>> pose_samples;
		for (int s = 0; s < (int)samples.size(); ++s)
		{
			// ポジティブサンプルであれば
			if (samples[s]->label == posLabel)
			{
				// その姿勢方向であれば
				if (samples[s]->poseLabel == d)
				{
					pose_samples.emplace_back(samples[s]);
					second_pos_num++;
				}
			} 
			
		}
		for (int s = 0; s < (int)samples.size(); ++s)
		{
			// ネガティブサンプルは全て格納
			if (samples[s]->label == negLabel)
			{
				pose_samples.emplace_back(samples[s]);
				second_neg_num++;
			}
		}

		// Fernの学習
		for (int i = 0; i < numFerns2; ++i)
		{
			printf("%d / %d本目\r", i+1, numFerns2);

			// 学習データのブートストラップサンプリング
			std::vector<std::shared_ptr<LearnSample>> subSamples;
			int posSize, negSize;
			makeSubset (subSamples, pose_samples, second_pos_num, second_neg_num, posDataPerTree2, negDataPerTree2, posSize, negSize);

			// InverseLabelの作成(クラス数の逆数)
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

			// Fernの作成
			std::unique_ptr<SecondLayerFern> fern( new SecondLayerFern(depth2, featureTestNum2, thresholdTestNum2, numClass, posLabel, negLabel, gridSize));

			// Fernの学習
			fern->Learn(subSamples, posSize, il);

			// Fernの追加
			ferns.emplace_back(std::move(fern));
		}

		// Fernの保存
		char filename[ 256 ];
		char filename_txt[ 256 ];// Fernの追加
		for (int i = 0; i < numFerns2; ++i)
		{
			sprintf( filename, "%s/fern2_%d_%d.txt", saveFolder.c_str(), d, i );
			sprintf( filename_txt, "%s/fern2_%d_%d_t.txt", saveFolder.c_str(), d, i );
			ferns[i]->saveFern(filename, filename_txt);
		}

		s_finish = static_cast<double>(cv::getTickCount());
		s_time = ( ( s_finish - s_start ) / cv::getTickFrequency() );

		std::cout << "2層目 " << d << "番目：" << numFerns2 << "本のFernを構築" << std::endl;
		std::cout << "処理時間：" << s_time << "秒" << std::endl;
	}

	// Hough Fernsのパラメータを保存
	saveFerns(saveFolder);

	finish = static_cast<double>(cv::getTickCount());
	time = ( ( finish - start ) / cv::getTickFrequency() );
	std::cout << "全体処理時間 : " << time << "秒" << std::endl;
	std::cout << "学習終了" << std::endl << std::endl;
}


/**
 * @brief   Hough Fernsによる検出
 * 
 * @param   src[in]					入力画像
 * @param   min_distance1[in]		1層目の考慮する最小距離
 * @param   max_distance1[in]		1層目の考慮する最大距離
 * @param   distance_num1[in]		1層目の距離変化のステップ数
 * @param   min_distance2[in]		2層目の考慮する最小距離
 * @param   max_distance2[in]		2層目の考慮する最大距離
 * @param   distance_num2[in]		2層目の距離変化のステップ数
 * @param   detect_width[in]		姿勢推定に考慮する幅
 * @param   detect_height[in]		姿勢推定に考慮する高さ
 * @param   mag_th[in]				勾配強度の閾値
 * @param   meanshift_radius[in]	Mean Shiftのカーネル幅
 * @param   win_th[in]				Nearest Neighborで統合する範囲
 * @param   likelihood_th[in]		尤度の閾値
 */
void HoughFerns::detect(const cv::Mat& src, float min_distance, float max_distance, int distance_num, float min_scale, float max_scale, int scale_num,
						int detect_width, int detect_height, float mag_th, int meanshift_radius, int win_th, double likelihood_th)
{
	detectPoint.clear();								// 最終的な検出点
	pose_estimate.clear();								// 最終的な姿勢

	std::vector<cv::Point3f> detect_point;				// 1層目の検出位置
	std::vector<float> detect_scales;					// 1層目で検出したスケール

	// 1層目の位置検出及び大まかな姿勢推定
	detectFirst( src, detect_point, detect_scales, min_distance, max_distance, distance_num, mag_th, meanshift_radius, win_th, likelihood_th);

	// 2層目の正確な姿勢推定(検出回数分繰り返す)
	for (int i = 0; i < (int)detect_point.size(); ++i)
	{
		detectSecond( src, detect_point[i], detect_scales[i], i, min_scale, max_scale, scale_num, detect_width, detect_height, mag_th, meanshift_radius, win_th, likelihood_th);
	}
}



/**
 * @brief   Hough Fernsによる検出(1層目)
 * 
 * @param   src[in]					入力画像
 * @param   detect_point[in, out]	1層目で検出した座標
 * @param   detect_scale[in,out]	1層目で検出したスケール
 * @param   min_distance1[in]		1層目の考慮する最小距離
 * @param   max_distance1[in]		1層目の考慮する最大距離
 * @param   distance_num1[in]		1層目の距離変化のステップ数
 * @param   mag_th[in]				勾配強度の閾値
 * @param   meanshift_radius[in]	Mean Shiftのカーネル幅
 * @param   win_th[in]				Nearest Neighborで統合する範囲
 * @param   likelihood_th[in]		尤度の閾値
 */
void HoughFerns::detectFirst(const cv::Mat& src, std::vector<cv::Point3f> &detect_point, std::vector<float> &detect_scales, float min_distance, float max_distance, int distance_num, 
							 float mag_th, int meanshift_radius, int win_th, double likelihood_th)
{
	if (first_ferns.size() == 0){
		std::cerr << "学習されていません" << std::endl;
		return;
	}

	// 学習時間の測定
	double start, finish, time;
	double s_start, s_finish, s_time;
	double m_start, m_finish, m_time;
	double n_start, n_finish, n_time;
	double pose_start, pose_finish, pose_time;
	double total_start, total_finish, total_time;

	total_start = static_cast<double>(cv::getTickCount());

	// 最大・最小距離におけるサイズを推定(基準からのスケールの倍率)
	std::vector<float> scale_step;
	float dist_step;
	if(distance_num !=1){
		dist_step = (max_distance-min_distance) / (float)(distance_num-1);
	} else {
		dist_step = min_distance;
	}
	for (int i = 0; i < distance_num; ++i)
	{
		// スケール
		float new_scale = distance / (float)(dist_step*i+min_distance);
		scale_step.emplace_back(new_scale);
	}

	if (scale_step.size() == 0) {
		std::cerr << "処理する距離の範囲が適切でありません" << std::endl;
	}
	
	///// 入力画像の量子化勾配方向を推定 /////

	s_start = static_cast<double>(cv::getTickCount());

	// P-COF特徴量
	p_cof.gridSize = gridSize;

	// 入力画像の勾配方向
	std::vector<cv::Mat> quant_angles;	

	// スケールを変化
	for (int i = 0; i < (int)scale_step.size(); ++i)
	{
		cv::Mat resize_src;
		std::vector<cv::Mat_<int>> integ_angle;

		// スケール変化(パッチの大きさ：大→小＝入力画像：小→大)
		cv::resize(src, resize_src, cv::Size(), 1.f/scale_step[i], 1.f/scale_step[i]);

		// 勾配方向の計算
		p_cof.calcIntegralOrientation(resize_src, integ_angle, mag_th, true, 7);

		// 勾配方向の量子化
		cv::Mat quant_angle;
		int freq_th = gridSize * gridSize * 0.3;
		p_cof.quantizedIntegraOrientation(quant_angle, integ_angle, freq_th, gridSize);
		quant_angles.emplace_back(quant_angle);
	}


	// 尤度マップの初期化
	int end_index = quant_angles.size() - 1;
	likelihoodmap.resize(quant_angles.size());
	for( int i = 0; i < (int)(quant_angles.size()); ++i) {
		likelihoodmap[i] = cv::Mat_<float>( quant_angles[i].rows, quant_angles[i].cols, float( 0.f ) );
	}
	// 姿勢方向マップの初期化
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


	///// 各Fernにおいてテンプレートマッチングおよび投票処理 /////

	start = static_cast<double>(cv::getTickCount());

	// 尤度マップの総積(対数の総和)
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		for (int j = 0; j < numFerns1; ++j)
		{
			cv::Mat_<float> likelihood = cv::Mat_<float>( quant_angles[i].rows, quant_angles[i].cols, float( 0.f ) );

			std::vector<cv::Mat_<float>> pose_likelihood(8);
			for( int k = 0; k < 8; ++k) {
				pose_likelihood[k] = cv::Mat_<float>( quant_angles[i].rows, quant_angles[i].cols, float( 0.f ) );
			}

			// 尤度マップを取得
			first_ferns[j]->Predict( quant_angles[i], likelihood, pose_likelihood);

			// 尤度マップのサイズによって投票数が変化することを防ぐ
			likelihood *= (1.f * scale_step[i]);

			// 対数に変換
			cv::log(likelihood, likelihood);
			cv::add(likelihoodmap[i], likelihood, likelihoodmap[i]);

			// 姿勢クラス推定
			for (int k = 0; k < 8; ++k) {
				pose_map[i][k] = pose_map[i][k].mul(pose_likelihood[k]);
			}
		}
	}


	int kernel_size = 3;
	float kernel_sigma = 3.0f;
	
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		// 対数から元に戻す
		cv::exp(likelihoodmap[i], likelihoodmap[i]);

		// 小さくなり過ぎないようスケール倍
		likelihoodmap[i] *= 1000000.f;

		// 元の画像サイズに戻す(グリッド単位)
		cv::resize(likelihoodmap[i], likelihoodmap[i], cv::Size(), scale_step[i], scale_step[i]);

		// 同じ距離平面における平滑化
		cv::GaussianBlur(likelihoodmap[i], likelihoodmap[i], cv::Size(kernel_size,kernel_size), kernel_sigma, kernel_sigma);
	}

	// 距離方向の平滑化
	smoothMap(likelihoodmap);
	
	finish = static_cast<double>(cv::getTickCount());
	time = ( ( finish - start ) / cv::getTickFrequency() );


	///// 尤度マップの閾値処理 /////
	m_start = static_cast<double>(cv::getTickCount());

	// 最大値，最小値の算出
	double max_val = -DBL_MAX;
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		double minVal, maxVal;
		cv::minMaxLoc (likelihoodmap[i], &minVal, &maxVal, NULL, NULL);

		if (max_val < maxVal) {
			max_val = maxVal;
		}
	}

	// 尤度マップの可視化
	visualizeLikelihood(likelihoodmap, scale_step, max_val, false);


	// 閾値以上のみを残す
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		cv::threshold (likelihoodmap[i], likelihoodmap[i], max_val*likelihood_th, 1, cv::THRESH_TOZERO);
	}


	///// Mean Shiftで極大値を推定 /////

	std::vector<cv::Point3f> detectPointInit;
	std::vector<double> detectVote;
	meanShift(detectPointInit, detectVote, scale_step, meanshift_radius);

	m_finish = static_cast<double>(cv::getTickCount());
	m_time = ( ( m_finish - m_start ) / cv::getTickFrequency() );


	///// Nearest Neighborで近傍点を統合 /////

	n_start = static_cast<double>(cv::getTickCount());

	nearestNeighbor(detectPointInit, detectVote, win_th);

	n_finish = static_cast<double>(cv::getTickCount());
	n_time = ( ( n_finish - n_start ) / cv::getTickFrequency() );


	///// 姿勢の推定 /////

	pose_start = static_cast<double>(cv::getTickCount());


	// 3次元位置へ変換
	std::vector<cv::Point3f> detect3D;		// 検出点
	std::vector<int> scale_indexes;			// インデックス
	for (int i = 0; i < (int)detectPointInit.size(); ++i) {
		if (detectPointInit[i].x != -100.0) {

			// 2次元位置+距離
			detect3D.emplace_back(cv::Point3f((float)detectPointInit[i].x, (float)detectPointInit[i].y, (float)(distance / detectPointInit[i].z)));
			detect_point.emplace_back(cv::Point3f((float)detectPointInit[i].x*gridSize, (float)detectPointInit[i].y*gridSize, (float)(distance / detectPointInit[i].z)));
			
			// 最も近いスケールを取り出す
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

	// 姿勢方向の推定
	pose_distribute.clear();
	poseEstimation(pose_distribute, pose_map, detect3D, scale_indexes, scale_step);

	pose_finish = static_cast<double>(cv::getTickCount());
	pose_time = ( ( pose_finish - pose_start ) / cv::getTickFrequency() );

	total_finish = static_cast<double>(cv::getTickCount());
	total_time = ( ( total_finish - total_start ) / cv::getTickFrequency() );



	// 結果のコンソール出力
	for (int i = 0; i < (int)detect_point.size(); ++i) {
		std::cout << "検出位置(x,y,z): (" << detect_point[i].x << "," << detect_point[i].y << "," << detect_point[i].z << ")" << std::endl;
		std::cout << "パッチのスケール(index,s): (" << scale_indexes[i] << "," << scale_step[scale_indexes[i]] << ")" << std::endl;
		std::cout << "姿勢確率: " << std::endl;
		for (int j = 0; j < 8; ++j)
		{
			std::cout << j << ":" << pose_distribute[i][j] << std::endl;
		}
		std::cout << std::endl;
	}

	std::cout << "入力画像処理時間 : " << s_time << "秒" << std::endl;
	std::cout << "投票時間 : " << time << "秒" << std::endl;
	std::cout << "Mean Shift時間 : " << m_time << "秒" << std::endl;
	std::cout << "Nearest Neighbor時間 : " << n_time << "秒" << std::endl;
	std::cout << "姿勢推定時間 : " << pose_time << "秒" << std::endl;
	std::cout << "識別時間 : " << total_time << "秒" << std::endl << std::endl;
}


/**
 * @brief   Hough Fernsによる検出(2層目)
 * 
 * @param   src[in]					入力画像
 * @param   detect_point[in]		1層目で検出した座標
 * @param   detect_scale[in]		1層目で検出したスケール
 * @param   detect_num[in]			何番目の検出か
 * @param   min_scale[in]			1層目のスケールを基準に最小のスケール倍
 * @param   max_scale[in]			1層目のスケールを基準に最大のスケール倍
 * @param   scale_num[in]			スケール変化のステップ数
 * @param   detect_width[in]		姿勢推定に考慮する幅
 * @param   detect_height[in]		姿勢推定に考慮する高さ
 * @param   mag_th[in]				勾配強度の閾値
 * @param   meanshift_radius[in]	Mean Shiftのカーネル幅
 * @param   win_th[in]				Nearest Neighborで統合する範囲
 * @param   likelihood_th[in]		尤度の閾値
 */
void HoughFerns::detectSecond(const cv::Mat& src, const cv::Point3f &detect_point, float detect_scale, int detect_num, float min_scale, float max_scale, int scale_num, 
							  int detect_width, int detect_height, float mag_th, int meanshift_radius, int win_th, double likelihood_th)
{
	if (second_ferns.size() == 0){
		std::cerr << "学習されていません" << std::endl;
		return;
	}

	// 学習時間の測定
	double start, finish, time;
	double s_start, s_finish, s_time;
	double m_start, m_finish, m_time;
	double n_start, n_finish, n_time;
	double pose_start, pose_finish, pose_time;
	double total_start, total_finish, total_time;

	total_start = static_cast<double>(cv::getTickCount());


	// 1層目の検出点周りの領域を取得
	int center_x = detect_point.x;
	int center_y = detect_point.y;
	int range_x = detect_width*detect_scale / 2;
	int range_y = detect_height*detect_scale / 2;
	int up_left_x = center_x-range_x;
	int up_left_y = center_y-range_y;
	cv::Rect roi(up_left_x, up_left_y, 2*range_x, 2*range_y);	// 抽出領域
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

	// 1層目の検出領域の画像
	cv::Mat detect_src = src(roi).clone();

	// 最大・最小距離におけるサイズを推定(基準からのスケールの倍率)
	std::vector<float> scale_step;
	float scale_step_num=0;
	if (scale_num>1){
		scale_step_num = (max_scale-min_scale) / (float)(scale_num-1);
	}
	for (int i = 0; i < scale_num; ++i)
	{
		// スケール(1層目で検出したスケール倍)
		float new_scale = distance / detect_point.z * (max_scale - scale_step_num*i);
		scale_step.emplace_back(new_scale);
	}

	if (scale_step.size() == 0) {
		std::cerr << "処理する距離の範囲が適切でありません" << std::endl;
	}
	
	///// 入力画像の量子化勾配方向を推定 /////

	s_start = static_cast<double>(cv::getTickCount());

	// P-COF特徴量
	p_cof.gridSize = gridSize;

	// 入力画像の勾配方向
	std::vector<cv::Mat> quant_angles;	

	// スケールを変化
	for (int i = 0; i < (int)scale_step.size(); ++i)
	{
		cv::Mat resize_src;
		std::vector<cv::Mat_<int>> integ_angle;

		// スケール変化(パッチの大きさ：大→小＝入力画像：小→大)
		cv::resize(detect_src, resize_src, cv::Size(), 1.f/scale_step[i], 1.f/scale_step[i]);

		// 勾配方向の計算
		p_cof.calcIntegralOrientation(resize_src, integ_angle, mag_th, true, 7);

		// 勾配方向の量子化
		cv::Mat quant_angle;
		int freq_th = gridSize * gridSize * 0.3;
		p_cof.quantizedIntegraOrientation(quant_angle, integ_angle, freq_th, gridSize);
		quant_angles.emplace_back(quant_angle);
	}


	// 尤度マップの初期化
	int end_index = quant_angles.size() - 1;
	likelihoodmap.resize(quant_angles.size());
	for( int i = 0; i < (int)(quant_angles.size()); ++i) {
		likelihoodmap[i] = cv::Mat_<float>( quant_angles[i].rows, quant_angles[i].cols, float( 0.f ) );
	}
	// 軸のマップの初期化
	std::vector<std::vector<std::vector<glm::vec3>>> axis_map;
	std::vector<glm::vec3> axis_tmp(quant_angles[end_index].cols, glm::vec3(0.0f,0.0f,0.0f));
	std::vector<std::vector<glm::vec3>> axis_tmp2(quant_angles[end_index].rows, axis_tmp);
	axis_map.resize( quant_angles.size(), axis_tmp2);
	// 角度マップの初期化
	std::vector<std::vector<std::vector<glm::vec2>>> angle_map;
	std::vector<glm::vec2> angle_tmp(quant_angles[end_index].cols, glm::vec2(0.0f,0.0f));
	std::vector<std::vector<glm::vec2>> angle_tmp2(quant_angles[end_index].rows, angle_tmp);
	angle_map.resize( quant_angles.size(), angle_tmp2);

	s_finish = static_cast<double>(cv::getTickCount());
	s_time = ( (s_finish - s_start ) / cv::getTickFrequency() );


	///// 各Fernにおいてテンプレートマッチングおよび投票処理 /////

	start = static_cast<double>(cv::getTickCount());

	// 1層目の姿勢確率が高い方向を用いる
	int best_pose = 0;
	double max_distribution = 0.0;
	for (int i = 0; i < 8; ++i) {
		if (pose_distribute[detect_num][i] > max_distribution) {
			max_distribution = pose_distribute[detect_num][i];
			best_pose = i;
		}
	}

	// 尤度マップの総積(対数の総和)
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		for (int j = 0; j < numFerns2; ++j)
		{
			cv::Mat_<float> likelihood = cv::Mat_<float>( quant_angles[i].rows, quant_angles[i].cols, float( 0.f ) );

			// 尤度マップを取得
			second_ferns[best_pose][j]->Predict( quant_angles[i], likelihood, axis_map[i], angle_map[i]);

			// 尤度マップのサイズによって投票数が変化することを防ぐ
			likelihood *= (1.f * scale_step[i]);

			// 対数に変換
			cv::log(likelihood, likelihood);
			cv::add(likelihoodmap[i], likelihood, likelihoodmap[i]);
		}
	}


	int kernel_size = 3;
	float kernel_sigma = 3.0f;
	
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		// 対数から元に戻す
		cv::exp(likelihoodmap[i], likelihoodmap[i]);

		// 小さくなり過ぎないようスケール倍
		likelihoodmap[i] *= 1000000.f;

		// 元の画像サイズに戻す(グリッド単位)
		cv::resize(likelihoodmap[i], likelihoodmap[i], cv::Size(), scale_step[i], scale_step[i]);

		// 同じ距離平面における平滑化
		cv::GaussianBlur(likelihoodmap[i], likelihoodmap[i], cv::Size(kernel_size,kernel_size), kernel_sigma, kernel_sigma);
	}

	// 距離方向の平滑化
	smoothMap(likelihoodmap);
	
	finish = static_cast<double>(cv::getTickCount());
	time = ( ( finish - start ) / cv::getTickFrequency() );


	///// 尤度マップの閾値処理 /////
	m_start = static_cast<double>(cv::getTickCount());

	// 最大値，最小値の算出
	double max_val = -DBL_MAX;
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		double minVal, maxVal;
		cv::minMaxLoc (likelihoodmap[i], &minVal, &maxVal, NULL, NULL);

		if (max_val < maxVal) {
			max_val = maxVal;
		}
	}

	// 尤度マップの可視化
	visualizeLikelihood(likelihoodmap, scale_step, max_val, true);


	// 閾値以上のみを残す
	for (int i = 0; i < (int)quant_angles.size(); ++i)
	{
		cv::threshold (likelihoodmap[i], likelihoodmap[i], max_val*likelihood_th, 1, cv::THRESH_TOZERO);
	}

	///// Mean Shiftで極大値を推定 /////

	std::vector<cv::Point3f> detectPointInit;
	std::vector<double> detectVote;
	meanShift(detectPointInit, detectVote, scale_step, meanshift_radius);

	m_finish = static_cast<double>(cv::getTickCount());
	m_time = ( ( m_finish - m_start ) / cv::getTickFrequency() );


	///// Nearest Neighborで近傍点を統合 /////

	n_start = static_cast<double>(cv::getTickCount());

	nearestNeighbor(detectPointInit, detectVote, win_th);

	n_finish = static_cast<double>(cv::getTickCount());
	n_time = ( ( n_finish - n_start ) / cv::getTickFrequency() );


	///// 姿勢の推定 /////

	pose_start = static_cast<double>(cv::getTickCount());


	// 3次元位置へ変換
	std::vector<cv::Point3f> detect3D;		// 検出点
	std::vector<int> scale_indexes;			// インデックス
	for (int i = 0; i < (int)detectPointInit.size(); ++i) {
		if (detectPointInit[i].x != -100.0) {

			// 2次元位置+距離
			detect3D.emplace_back(cv::Point3f((float)detectPointInit[i].x, (float)detectPointInit[i].y, (float)(distance / detectPointInit[i].z)));
			detectPoint.emplace_back(cv::Point3f(up_left_x+(float)detectPointInit[i].x*gridSize, up_left_y+(float)detectPointInit[i].y*gridSize, (float)(distance / detectPointInit[i].z)));
			
			// 最も近いスケールを取り出す
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

	// 追加
	for (int i = 0; i < (int)pose_est.size(); ++i)
	{
		pose_estimate.emplace_back(pose_est[i]);
	}

	pose_finish = static_cast<double>(cv::getTickCount());
	pose_time = ( ( pose_finish - pose_start ) / cv::getTickFrequency() );

	total_finish = static_cast<double>(cv::getTickCount());
	total_time = ( ( total_finish - total_start ) / cv::getTickFrequency() );


	// 結果のコンソール出力
	for (int i = 0; i < (int)detectPoint.size(); ++i) {
		std::cout << "検出位置(x,y,z): (" << detectPoint[i].x << "," << detectPoint[i].y << "," << detectPoint[i].z << ")" << std::endl;
		std::cout << "パッチのスケール(index,s): (" << scale_indexes[i] << "," << scale_step[scale_indexes[i]] << ")" << std::endl;
		std::cout << "姿勢: " << glm::to_string(pose_est[0]) << std::endl;
		std::cout << std::endl;
	}

	std::cout << "入力画像処理時間 : " << s_time << "秒" << std::endl;
	std::cout << "投票時間 : " << time << "秒" << std::endl;
	std::cout << "Mean Shift時間 : " << m_time << "秒" << std::endl;
	std::cout << "Nearest Neighbor時間 : " << n_time << "秒" << std::endl;
	std::cout << "姿勢推定時間 : " << pose_time << "秒" << std::endl;
	std::cout << "識別時間 : " << total_time << "秒" << std::endl << std::endl;
}



/**
 * @brief   距離方向におけるガウシアンフィルタによるマップの平滑化
 */
void HoughFerns::smoothMap(std::vector<cv::Mat_<float>> &likelihood)
{

	// 最小の幅と高さ
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


	///// ガウシアンフィルタによる距離方向の平滑化 /////

	// マップのコピー
	std::vector<cv::Mat_<double>> copy_likelihood(likelihood.size());
	for (int i = 0; i < (int)copy_likelihood.size(); ++i) {
		copy_likelihood[i] = likelihood[i].clone();
	}
	int kernel_size = 3;
    float kernel_sigma = 0.5f;
	cv::Mat gaussKernel = cv::getGaussianKernel( 3, 3.0, CV_32F );

	// 一番小さいサイズのスケールに合わせて平滑化
	for (int y = 0; y < min_height; ++y)
	{
		for ( int x = 0; x < min_width; ++x) 
		{
			for( int s = 0; s < (int)likelihood.size(); ++s) 
			{
                float convSum = 0;
                int k = 0;

				int scBegin = s - kernel_size / 2;		// 先頭
                int scEnd = s + kernel_size / 2 + 1;	// 末尾

				// 先頭がはみ出る場合
				if (scBegin == -1) {
					scBegin = 0;
					convSum += (float)copy_likelihood[0](y, x) * 0.3f * gaussKernel.at<float>(k);	// 重み付きの最小スケール
					k++;
				}
				// 末尾がはみ出る場合
				if (scEnd > (int)likelihood.size()) {
					scEnd = (int)likelihood.size();
					convSum += (float)copy_likelihood[(int)likelihood.size()-1](y, x) * 0.3f * gaussKernel.at<float>(gaussKernel.cols-1);	// 重み付きの最大スケール
				}

				// カーネルの畳みこみ
                for( int td = scBegin; td < scEnd; td++, k++) {
					convSum += (float)copy_likelihood[td](y, x) * gaussKernel.at<float>(k);
				}

				likelihood[s](y, x) = convSum;
            }
		}
	}
}


/**
 * @brief   尤度マップの可視化
 * 
 * @param   likelihood[in]		尤度マップ
 * @param   scale_rate[in]		スケールの倍率
 * @param   max_value[in]		マップの最大値
 * @param   flag[in]			2層目かどうか
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
				// 色相マップ
				colorImage[s].data[ y * colorImage[s].step + x * channel + 0 ] = 120 - (int)( likelihood[s](y, x) * alpha * 120.0 / 255.0);
				colorImage[s].data[ y * colorImage[s].step + x * channel + 1 ] = 255;
				colorImage[s].data[ y * colorImage[s].step + x * channel + 2 ] = 255;
			}
		}
		cv::cvtColor(colorImage[s], colorImage[s], CV_HSV2BGR);
		// 表示
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
 * @brief   Mean Shiftで極大値推定
 * 
 * @param   meanshiftPoint[in,out]	Mean Shift後の検出点
 * @param   detectionVote[in,out]	検出の投票値を格納
 * @param   scales[in]				スケールの倍率
 * @param   radius[in]				カーネル幅
 */
void HoughFerns::meanShift(std::vector<cv::Point3f> &meanshiftPoint, std::vector<double> &detectionVote, const std::vector<float> scale_rate, int radius)
{
	double xi, xj, yi, yj, si, sj, sum_x, sum_y, sum_s, sum_w, Kernel, move_x, move_y, move_s;

	// 最小の幅と高さ
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

	std::vector<cv::Point3f> centerPoint;		// 検出座標

	// Mapから検出座標数だけ取り出し
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

	// 検出座標数だけ繰り返し
	for (int i = 0 ; i < (int)centerPoint.size() ; ++i)
	{
		xi = centerPoint[i].x;
		yi = centerPoint[i].y;
		si = centerPoint[i].z;

		// 収束するまで繰り返し
		while(1)
		{
			// 初期化
			sum_x = sum_y = sum_s = sum_w = 0.0;

			// 密度計算
			for (int j = 0 ; j < (int)centerPoint.size(); ++j)
			{
				xj = centerPoint[j].x;
				yj = centerPoint[j].y;
				sj = centerPoint[j].z; 

				// x-y-s空間のカーネル関数
				Kernel = - (((xi - xj) * (xi - xj) + (yi - yj) * (yi - yj) + (si - sj) * (si - sj)) / (radius * radius));
				Kernel = exp( Kernel );

				sum_x += xj * Kernel;
				sum_y += yj * Kernel;
				sum_s += sj * Kernel;
				sum_w += Kernel;
			}
			
			// 移動量の計算
			move_x = ( sum_x / sum_w ) - xi;
			move_y = ( sum_y / sum_w ) - yi;							
			move_s = ( sum_s / sum_w ) - si;							
			// 移動
			xi = (int)( xi + move_x );
			yi = (int)( yi + move_y );
			si =  si + move_s;

			// 閾値以下に収束するまで
			if (move_x > -1.0 && move_x < 1.0 && move_y > -1.0 && move_y < 1.0 && move_s > -1.0 && move_s < 1.0){
				break;
			}
		}

		// Mean Shift後位置保存
		meanshiftPoint[i].x = int(xi);
		meanshiftPoint[i].y = int(yi);
		meanshiftPoint[i].z = si;
	}
}


/**
 * @brief   Nearest Neighborで近傍点を統合
 * 
 * @param   detectionPoint[in,out]		検出点
 * @param   detectionVote[in,out]		投票値
 * @param   win_th[in]					統合する範囲 
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
					detectionVote[ n ] = -100.0;		// 削除フラグ
					detectionPoint[ n ] = cv::Point3f(-100.f, -100.f, -100.f);
				}
			}
		}
	}

	// 要素の削除
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
 * @brief   検出位置毎に姿勢推定
 * 
 * @param   pose_dist[in,out]			検出点に対応する姿勢確率
 * @param   poseMap[in,out]				姿勢マップ
 * @param   detectionPoint[in]			検出点
 * @param	scales[in]					検出時に最も近いスケールのインデックス
 * @param	scale_rate[in]				スケールの倍率
 */
void HoughFerns::poseEstimation(std::vector<std::vector<float>> &pose_dist, std::vector<std::vector<cv::Mat_<float>>> &poseMap, 
								const std::vector<cv::Point3f> &detectionPoint, const std::vector<int> scales, const std::vector<float> scale_rate)
{
	int x, y;
	pose_dist.clear();

	// 検出位置に対して姿勢推定
	for (int i = 0; i < (int)detectionPoint.size(); ++i)
	{
		// 投票時のスケールに戻す(重心位置の尤度マップのみ元のサイズになっている)
		x = detectionPoint[i].x / scale_rate[scales[i]];
		y = detectionPoint[i].y / scale_rate[scales[i]];

		std::vector<float> distribute;
		// 姿勢の平均値を格納
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
 * @brief   正確に姿勢推定
 * 
 * @param   pose_est[in,out]			検出点に対応する姿勢
 * @param   axisMap[in,out]				検出点に対応する回転軸
 * @param   angleMap[in]				検出点に対応する回転角
 * @param   detectionPoint[in]			検出点
 * @param	scales[in]					検出時に最も近いスケールのインデックス
 * @param	scale_rate[in]				スケールの倍率
 */
void HoughFerns::poseEstimation(std::vector<glm::mat4> &pose_est, std::vector<std::vector<std::vector<glm::vec3>>> &axisMap, std::vector<std::vector<std::vector<glm::vec2>>> &angleMap, 
								const std::vector<cv::Point3f> &detectionPoint, const std::vector<int> scales, const std::vector<float> scale_rate)
{
	int x, y;

	// 検出位置に対して姿勢推定
	for (int i = 0; i < (int)detectionPoint.size(); ++i)
	{
		// 投票時のスケールに戻す(重心位置の尤度マップのみ元のサイズになっている)
		x = detectionPoint[i].x / scale_rate[scales[i]];
		y = detectionPoint[i].y / scale_rate[scales[i]];
		
		// 軸の平均
		glm::vec3 meanAxis = glm::normalize(axisMap[scales[i]][y][x]);

		// roll角の平均
		glm::vec2 meanAngle = glm::normalize(angleMap[scales[i]][y][x]);
		float theta = std::atan2f(meanAngle.y, meanAngle.x);

		// 回転行列の生成
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


// 学習サンプルの取得
void HoughFerns::getLearnSamples(const std::string& posFolder, const std::string& negFolder, std::vector<std::shared_ptr<LearnSample>>& samples)
{

	///// ポジティブサンプルの取得 /////
	std::cout << "ポジティブサンプル抽出…" << std::endl;

	{
		// フォルダからファイル名を取得
		std::vector<std::string> fileNames;
		WIN32_FIND_DATA ffd;
		HANDLE hF;

		std::string folderName = posFolder + "/*.xml";
		hF = FindFirstFile( folderName.c_str(), &ffd);
		if (hF != INVALID_HANDLE_VALUE) {
			// フォルダ内のファイルの探索
			do {
				std::string fullpath = posFolder + "/" + ffd.cFileName;		// ファイルのパス

				// フォルダ名の格納
				fileNames.emplace_back(fullpath);

			} while (FindNextFile(hF, &ffd ) != 0);
			FindClose(hF);
		}

		bool init_load = true;

		// ファイル数分読み込む
		for (int f = 0; f < (int)fileNames.size(); ++f)
		{
			// ファイルの読み込み
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
				// データセットの環境が異なる場合に警告
				if (patchSize != patch_size || gridSize != grid_size || fabs(distance-dist) > 0.0001) {
					std::cerr << "データセットに環境が異なるものが含まれています" << std::endl;
					exit (0);
				}
			}

			// パッチの数だけ繰り返す
			for (int i = 0; i < patchNum; ++i)
			{
				cv::Mat gradMat, pcofMat, weightMat, offset;
				cvfs["offset"+std::to_string(i)] >> offset;
				cvfs["GradOri"+std::to_string(i)] >> gradMat;
				cvfs["PcofOri"+std::to_string(i)] >> pcofMat;
				cvfs["weight"+std::to_string(i)] >> weightMat;

				// 学習データ
				auto posSample = std::make_shared<LearnSample>();
				posSample->posInsert( gradMat, pcofMat, weightMat, 1, quadrant, cv::Point2f(offset.at<int>(0),offset.at<int>(1)), axis, rollAngle);

				// 追加
				samples.emplace_back(posSample);
				posSample_num++;
			}

			cvfs.release();		// 解放
		}
	}

	// グリッドサイズの設定
	p_cof.gridSize = gridSize;

	std::cout << "ポジティブサンプル数：" << posSample_num << std::endl;



	///// ネガティブサンプルの取得 /////
	std::cout << "ネガティブサンプル抽出…" << std::endl;

	{
		// フォルダからファイル名を取得
		std::vector<std::string> fileNames;
		WIN32_FIND_DATA ffd;
		HANDLE hF;

		std::string folderName = negFolder + "/*.*";
		hF = FindFirstFile( folderName.c_str(), &ffd);
		if (hF != INVALID_HANDLE_VALUE) {
			// フォルダ内のファイルの探索
			do {
				std::string fullpath = negFolder + "/" + ffd.cFileName;		// ファイルのパス

				// フォルダ名の格納
				fileNames.emplace_back(fullpath);

			} while (FindNextFile(hF, &ffd ) != 0);
			FindClose(hF);
		}

		// 特徴抽出
		P_COF p_cof(gridSize);

		// ファイル数分読み込む
		for (int f = 0; f < (int)fileNames.size(); ++f)
		{
			// グレイスケールで画像読み込み
			cv::Mat image = cv::imread(fileNames[f], CV_LOAD_IMAGE_GRAYSCALE);

			if (!image.empty())
			{
				std::vector<cv::Mat_<int>> integ_angle;
				cv::Mat quant_angle;

				// 勾配方向の計算
				p_cof.calcIntegralOrientation( image, integ_angle, 200.f, true, 7);

				// 勾配方向の量子化
				int freq_th = gridSize * gridSize * 0.3;
				p_cof.quantizedIntegraOrientation(quant_angle, integ_angle, freq_th, gridSize);

				// ステップ幅
				int step = patchSize / gridSize;

				// パッチサイズに切り出す
				for (int r = 0; r < quant_angle.rows - step; r+=step)
				{
					for (int c = 0; c < quant_angle.cols - step; c+=step)
					{
						cv::Rect roi(c, r, step, step);
						cv::Mat patch = quant_angle(roi);

						// 特徴数をカウント
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

						// 特徴がある場合のみ追加
						if (count > step*step*0.1)
						{
							// 学習データ
							auto negSample = std::make_shared<LearnSample>();
							negSample->negInsert(quant_angle(roi), 0);

							// 追加
							samples.emplace_back(negSample);
							negSample_num++;
						}
					}
				}
			}
		}
	}

	std::cout << "ネガティブサンプル数：" << negSample_num << std::endl;
}



/**
 * @brief   サブセットの作成(1層目用)
 * 
 * @param   subSamples[in,out]		サブセット
 * @param   samples[in]				全データ
 * @param   pos_rate[in]			ポジティブサンプルの割合 
 * @param   neg_rate[in]			ネガティブサンプルの割合 
 * @param   posSize[in,out]			ポジティブサンプルの数 
 * @param   negSize[in,out]			ネガティブサンプルの数
 */
void HoughFerns::makeSubset(std::vector<std::shared_ptr<LearnSample>> &subSamples, const std::vector<std::shared_ptr<LearnSample>> &samples, float pos_rate, float neg_rate, int &posSize, int &negSize)
{
	// 範囲確認
	if (pos_rate < 0.0){
		pos_rate = 1.0f;
	}
	if (neg_rate < 0.0){
		neg_rate = 1.0f;
	}
	posSize = posSample_num * pos_rate;
	negSize = negSample_num * neg_rate;
	int subsample_size = posSize+negSize;

	// 容量確保
	subSamples.clear();
	subSamples.resize(subsample_size, std::make_shared<LearnSample>());

	std::random_device rnd;							// 非決定的な乱数生成器を生成
    std::mt19937 mt(rnd());							// メルセンヌ・ツイスタ、引数は初期シード値
	std::uniform_int_distribution<int> rand_posSample(0, posSample_num-1);					// 一様乱数(ポジティブ)
	std::uniform_int_distribution<int> rand_negSample(posSample_num, samples.size()-1);		// 一様乱数(ネガティブ)

	// ランダムサンプリング(ポジティブ)
	for (int i = 0; i < posSize; ++i){
		subSamples[i] = samples[rand_posSample(mt)];
	}
	// ランダムサンプリング(ネガティブ)
	for (int i = posSize; i < subsample_size; ++i){
		subSamples[i] = samples[rand_negSample(mt)];
	}
}


/**
 * @brief   サブセットの作成
 * 
 * @param   subSamples[in,out]		サブセット
 * @param   samples[in]				全データ
 * @param   pos_num[in]				ポジティブサンプルの総数 
 * @param   neg_num[in]				ネガティブサンプルの総数
 * @param   pos_rate[in]			ポジティブサンプルの割合 
 * @param   neg_rate[in]			ネガティブサンプルの割合 
 * @param   posSize[in,out]			ポジティブサンプルの数 
 * @param   negSize[in,out]			ネガティブサンプルの数
 */
void HoughFerns::makeSubset(std::vector<std::shared_ptr<LearnSample>> &subSamples, const std::vector<std::shared_ptr<LearnSample>> &samples, int pos_num, int neg_num, 
							float pos_rate, float neg_rate, int &posSize, int &negSize)
{
	// 範囲確認
	if (pos_rate < 0.0){
		pos_rate = 1.0f;
	}
	if (neg_rate < 0.0){
		neg_rate = 1.0f;
	}
	posSize = pos_num * pos_rate;
	negSize = neg_num * neg_rate;
	int subsample_size = posSize+negSize;

	// 容量確保
	subSamples.clear();
	subSamples.resize(subsample_size, std::make_shared<LearnSample>());

	std::random_device rnd;							// 非決定的な乱数生成器を生成
    std::mt19937 mt(rnd());							// メルセンヌ・ツイスタ、引数は初期シード値
	std::uniform_int_distribution<int> rand_posSample(0, pos_num-1);					// 一様乱数(ポジティブ)
	std::uniform_int_distribution<int> rand_negSample(pos_num, samples.size()-1);		// 一様乱数(ネガティブ)

	// ランダムサンプリング(ポジティブ)
	for (int i = 0; i < posSize; ++i){
		subSamples[i] = samples[rand_posSample(mt)];
	}
	// ランダムサンプリング(ネガティブ)
	for (int i = posSize; i < subsample_size; ++i){
		subSamples[i] = samples[rand_negSample(mt)];
	}
}


/**
 * @brief   Hough Fernsを保存
 * 
 * @param   folderName[in]		Hough Fernsを保存するフォルダ
 */
void HoughFerns::saveFerns(const std::string &folderName)
{
	std::cout << "Hough Fernsの保存中…" << std::endl;

	std::ofstream ofs(folderName + "/ferns_param.csv");

	// Hough Fernsパラメータを保存
	ofs << "決定木の数(1層目)" << "," << numFerns1 << std::endl;
	ofs << "決定木の数(2層目)" << "," << numFerns2 << std::endl;
	ofs << "Fernの深さ(1層目)" << "," << depth1 << std::endl;
	ofs << "Fernの深さ(2層目)" << "," << depth2 << std::endl;
	ofs << "特徴選択回数(1層目)" << "," << featureTestNum1 << std::endl;
	ofs << "特徴選択回数(2層目)" << "," << featureTestNum2 << std::endl;
	ofs << "閾値選択回数(1層目)" << "," << thresholdTestNum1 << std::endl;
	ofs << "閾値選択回数(2層目)" << "," << thresholdTestNum2 << std::endl;
	ofs << "クラス数" << "," << numClass << std::endl;
	ofs << "ポジティブデータの割合(1層目)" << "," << posDataPerTree1 << std::endl;
	ofs << "ポジティブデータの割合(2層目)" << "," << posDataPerTree2 << std::endl;
	ofs << "ネガティブデータの割合(1層目)" << "," << negDataPerTree1 << std::endl;
	ofs << "ネガティブデータの割合(2層目)" << "," << negDataPerTree2 << std::endl;
	// 学習サンプル用パラメータを保存
	ofs << "ポジティブ画像のラベル" << "," << posLabel << std::endl;
	ofs << "ネガティブ画像のラベル" << "," << negLabel << std::endl;
	ofs << "レンダリング時のカメラ視線" << "," << eyeVector.x << "," << eyeVector.y << "," << eyeVector.z << std::endl;
	ofs << "レンダリング時のカメラ上方向" << "," << eyeUp.x << "," << eyeUp.y << "," << eyeUp.z << std::endl;
	ofs << "モデルとカメラとの距離" << "," << distance << std::endl;
	// 特徴用パラメータを保存
	ofs << "パッチサイズ" << "," << patchSize << std::endl;
	ofs << "グリッドサイズ" << "," << gridSize << std::endl;

	std::cout << "保存終了" << std::endl << std::endl;
}




/**
 * @brief   Hough Fernsの読み込み
 * 
 * @param   folderName[in]		Hough Forestsを読み込むフォルダ
 * 
 * @returns true or false
 */
bool HoughFerns::loadFerns(const std::string &folderName)
{
	std::cout << "Hough Fernsの読み込み中…" << std::endl;

	///// 1層目 /////

	// Fernsの準備
	first_ferns.clear();

	// Fernの読み込み
	char filename[ 256 ];
	for (int i = 0; i < numFerns1; ++i)
	{
		// Fernの作成
		std::unique_ptr<FirstLayerFern> fern(new FirstLayerFern(depth1, featureTestNum1, thresholdTestNum1, numClass, posLabel, negLabel, gridSize));

		double start, finish, time;
		start = static_cast<double>(cv::getTickCount());

		// Fernの読み込み
		sprintf( filename, "%s/fern1_%d.txt", folderName.c_str(), i );
		if ( fern->loadFern(filename) == false ){
			return false;
		}

		finish = static_cast<double>(cv::getTickCount());
		time = ( ( finish - start ) / cv::getTickFrequency() );
		std::cout << "Load First Layer:" << i << ", Time:" << time << std::endl; 

		// Fernの追加
		first_ferns.emplace_back(std::move(fern));
	}


	///// 2層目 /////

	// Fernsの準備
	second_ferns.clear();

	// Fernの読み込み
	char filename2[ 256 ];
	// 姿勢方向毎に読み込み
	for (int d = 0; d < 8; ++d)
	{
		std::vector<std::unique_ptr<SecondLayerFern>> ferns;

		for (int i = 0; i < numFerns2; ++i)
		{
			// Fernの作成
			std::unique_ptr<SecondLayerFern> fern(new SecondLayerFern(depth2, featureTestNum2, thresholdTestNum2, numClass, posLabel, negLabel, gridSize));

			double start, finish, time;
			start = static_cast<double>(cv::getTickCount());

			// Fernの読み込み
			sprintf( filename2, "%s/fern2_%d_%d.txt", folderName.c_str(), d, i );
			if ( fern->loadFern(filename2) == false ){
				return false;
			}

			finish = static_cast<double>(cv::getTickCount());
			time = ( ( finish - start ) / cv::getTickFrequency() );
			std::cout << "Load SecondLayer:" << d << "_" << i << ", Time:" << time << std::endl; 

			// Fernの追加
			ferns.emplace_back(std::move(fern));
		}
		// 追加
		second_ferns.emplace_back(std::move(ferns));
	}

	std::cout << "読み込み終了" << std::endl << std::endl;

	return true;
}