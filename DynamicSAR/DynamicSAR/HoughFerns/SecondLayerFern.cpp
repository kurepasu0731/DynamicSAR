#include "SecondLayerFern.h"



/**
 * @brief   Fernの学習
 * 
 * @param   samples[in]		学習データ(前半はポジティブサンプル)
 * @param   posNum[in]		ポジティブサンプルの数
 * @param   il[in]			クラスの偏りに応じた重み付け
 */
void SecondLayerFern::Learn(const std::vector<std::shared_ptr<LearnSample>>& samples, int posNum, const std::vector<double>& il)
{
	// 初期化
	splitNode.clear();
	nodeData.clear();

	std::map<int, std::vector<int>> data_pool;		// 階層におけるノード番号に対応する学習データのID(一時保存用)
	{
		std::vector<int> data_id(samples.size(), 0);// 学習データのID(配列の番号)
		// 学習データにIDを付ける
		for (int i = 0; i < (int)samples.size(); ++i){
			data_id[i] = i;
		}
		data_pool[0] = data_id;						// ルートノードにおける学習データのID
	}

	std::vector<int> feature_indices;				// 特徴抽出用のインデックス番号
	

	// 乱数生成
	std::random_device rnd;							// 非決定的な乱数生成器を生成
    std::mt19937 mt(rnd());							// メルセンヌ・ツイスタ
	std::uniform_int_distribution<int> rand_f(0, posNum-1);		// 範囲の一様乱数(ポジティブサンプルの選択)
	std::uniform_real_distribution<float> rand_t(0.f, 1.f);		// 範囲の一様乱数(閾値の選択)


	// P-COF特徴量
	p_cof.gridSize = gridSize;

	// 最大の深さに達するまで
	for (int d = 0; d < depth; ++d)
	{
		// 現在の階層のノード数
		int node_num = pow(2, d);

		// 評価方法を変更
		bool gain_switch = false;
		if ((d%2 == 0 || d < 3) && d < (int)(depth*0.5))
			gain_switch = true;


		// 最良の分割を記憶する変数
		double best_gain = DBL_MAX;
		int best_feature = -1;			// 特徴選択用のインデックス番号
		double best_threshold = 0.0;	// 類似度の閾値


		///// 各特徴を試す /////
		for (int f = 0; f < featureTestNum; ++f)
		{
			// ポジティブサンプルのランダム選択
			int feature_num;
			while(1) 
			{
				feature_num = rand_f(mt);
				bool break_flag = true;

				// 過去に選択したサンプルは除外
				for( int i = 0; i < (int)feature_indices.size(); ++i){
					if ( feature_num == feature_indices[i]){
						break_flag = false;
					}
				}
				if (break_flag) {
					break;
				}
			}

			// 全てのサンプルに対して類似度を求める
			std::vector<float> similarity(samples.size());
			for (int i = 0; i < (int)samples.size(); ++i)
			{
				cv::Mat similarMat;
				p_cof.calcSimilarity(samples[i]->gradOri, samples[feature_num]->pcofOri, samples[feature_num]->weight, similarMat);		// 類似度

				similarity[i] = similarMat.at<float>(0);
			}


			///// 各閾値を試す /////
			for (int t = 0; t < thresholdTestNum; ++t)
			{
				// 閾値のランダム選択
				float threshold = rand_t(mt);

				double gain = 0;			// 情報利得

				///// 現在の階層のノード全てに対して評価 /////
				for (int n = 0; n < node_num; ++n)
				{
					// 現在のノードに割り当てられている学習サンプルのインデックス番号
					std::vector<int> sample_indices = data_pool[n];

					// 変数の初期化
					int leftCount = 0;		// 右の数
					int rightCount = 0;		// 左の数

					// 評価関数の切り替え
					if (gain_switch){
						// 情報利得の算出
						gain += splitInformationGain(samples, sample_indices, similarity, leftCount, rightCount, threshold, il);
					} else {
						// オフセットと姿勢の分散の算出
						gain += splitAllVariance(samples, sample_indices, similarity, leftCount, rightCount, threshold);
					}
				}

				// より良い分割であれば更新
				if (gain <= best_gain)
				{
					best_gain = gain;
					best_feature = feature_num;
					best_threshold = threshold;
				}
			}
		}

		///// 次の階層へ /////

		// 現在の結果を格納
		std::unique_ptr<Node> node(new Node);
		node->patch = samples[best_feature]->pcofOri.clone();
		node->weight = samples[best_feature]->weight.clone();
		node->threshold = best_threshold;
		splitNode.emplace_back(std::move(node));

		feature_indices.emplace_back(best_feature);
	

		// 選択された特徴のインデックス番号と閾値を用いて，もう一度分類
		std::vector<float> similarity(samples.size());
		for (int i = 0; i < (int)samples.size(); ++i)
		{
			cv::Mat similarMat;
			p_cof.calcSimilarity(samples[i]->gradOri, samples[best_feature]->pcofOri, samples[best_feature]->weight, similarMat);		// 類似度

			similarity[i] = similarMat.at<float>(0);
		}

		// 次の階層のデータを生成
		std::map<int, std::vector<int>> new_data_pool;		// 次の階層のノード番号と対応するサンプルのインデックスを格納

		// 現在の階層のノードに対して
		for (int n = 0; n < node_num; ++n)
		{
			// 現在のノードに割り当てられている学習サンプルのインデックス番号
			std::vector<int> sample_indices = data_pool[n];

			// 左右の一時格納データの作成(現在のデータIDを最適な分割をして格納)
			std::vector<int> data_id_l;		// 左のデータID
			std::vector<int> data_id_r;		// 右のデータID

			// サンプルを分岐
			for (int i = 0; i < (int)sample_indices.size(); ++i)
			{
				int index = sample_indices[i];
				if (similarity[index] < best_threshold) {
					data_id_l.emplace_back(index);
				} else {
					data_id_r.emplace_back(index);
				}
			}

			//std::cout << "深さ:" << d << ",ID:" <<  n << ",左:" << data_id_l.size() << ",右:" << data_id_r.size() << std::endl;

			// 次の階層のデータ
			new_data_pool[2*n] = data_id_l;
			new_data_pool[2*n+1] = data_id_r;
		}

		// 現在の階層のデータを削除し，次の階層のデータを格納
		data_pool.clear();
		data_pool.insert(new_data_pool.begin(), new_data_pool.end());
	}


	///// 末端ノードのデータをルックアップテーブルに格納 /////
	int node_num = pow(2, depth);

	// 末端ノードを探索
	for (int n = 0; n < node_num; ++n)
	{
		// 現在のノードに割り当てられている学習サンプルのインデックス番号
		std::vector<int> sample_indices = data_pool[n];

		// クラス確率密度の計算
		std::vector<float> distribution( numClass, 0.f);
		double total = 0.0;

		for (int i = 0; i < (int)sample_indices.size(); ++i)
		{
			int label = samples[sample_indices[i]]->label;
			distribution[label] += 1.f * il[label];
		}
		// 正規化
		for (int i = 0; i < numClass; ++i) {
			total += distribution[i];
		}
		for (int i = 0; i < numClass; ++i){
			if (total != 0.0) {
				distribution[i] /= total;
			} else { 
				distribution[i] = 0.0;
			}
		}

		// オフセットリストと姿勢リストの作成
		std::vector<cv::Point2f> offsetList;
		std::vector<glm::vec3> axisList;
		std::vector<glm::vec2> angleList;

		for (int i = 0; i < (int)sample_indices.size(); ++i)
		{
			int index = sample_indices[i];
			if( samples[index]->label == posLabel ){
				offsetList.emplace_back( samples[index]->offset);
				axisList.emplace_back( samples[index]->axis );
				angleList.emplace_back( samples[index]->rollAngle );
			}
		}

		std::unique_ptr<EndNodeData> node_data(new EndNodeData);
		node_data->distribution.assign( distribution.begin(), distribution.end());
		node_data->offsetList.assign( offsetList.begin(), offsetList.end());
		node_data->axisList.assign( axisList.begin(), axisList.end());
		node_data->angleList.assign( angleList.begin(), angleList.end());

		nodeData.emplace_back(std::move(node_data));		// 追加
	}
}




// 識別と投票処理
void SecondLayerFern::Predict(const cv::Mat& src, cv::Mat_<float>& likelihoodMap, std::vector<std::vector<glm::vec3>>& axisMap, std::vector<std::vector<glm::vec2>>& angleMap)
{
	// 学習時間の測定
	double t_start, t_finish, t_time, v_start, v_finish, v_time;
	t_start = static_cast<double>(cv::getTickCount());


	///// 全てのパッチに対してテンプレートマッチング /////
	int patch_heght = splitNode[0]->patch.rows;
	int patch_width = splitNode[0]->patch.cols;
	int height = src.rows - patch_heght + 1;
	int width = src.cols - patch_width + 1;
	cv::Mat findLUT = cv::Mat::zeros(height, width, CV_32F);		// Fernの末端ノードにアクセスするMat

	// 全てのパッチに対して
	for (int i = 0; i < (int)splitNode.size(); ++i)
	{
		// 類似度マップの生成
		cv::Mat similarMap;
		p_cof.calcSimilarity(src, splitNode[i]->patch, splitNode[i]->weight, similarMap);

		// 閾値処理で2値化
		cv::Mat binaryMap;
		double bit_pos = (1 << (depth-1-i));		// ビットを立てる位置
		cv::threshold (similarMap, binaryMap, splitNode[i]->threshold, bit_pos, cv::THRESH_BINARY);

		// ビットOR
		cv::add (findLUT, binaryMap, findLUT);
	}


	t_finish = static_cast<double>(cv::getTickCount());
	t_time = ( ( t_finish - t_start ) / cv::getTickFrequency() );
	//std::cout << "テンプレートマッチング処理 : " << t_time << "秒" << std::endl;

	v_start = static_cast<double>(cv::getTickCount());


	///// 投票 /////

	int mapWidth = likelihoodMap.cols;
	int mapHeight = likelihoodMap.rows;

	// 全ての画素を走査
	for (int r = 0; r < findLUT.rows; ++r)
	{
		float* LUT_r = findLUT.ptr<float>(r);

		for (int c = 0; c < findLUT.cols; ++c)
		{
			int index = nodeLUT[(int)LUT_r[c]];			// 参照するインデックス

			// オフセットベクトルが格納されていない場合は飛ばす
			if (index == -1)
			{
				continue;
			}
			int offset_num = nodeData[index]->offsetList.size();


			float vote = nodeData[index]->distribution[posLabel] / (float)offset_num;	// クラス確率による重み

			if (nodeData[index]->distribution[posLabel] > 0.65)
			{
				// 末端ノードのデータに対して
				for (int i = 0; i < offset_num; ++i)
				{
					// 投票座標
					int x = c + patch_width*0.5 + nodeData[index]->offsetList[i].x;
					int y = r + patch_heght*0.5 + nodeData[index]->offsetList[i].y;

					if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) {
						continue;
					}

					// 尤度マップに投票
					likelihoodMap(y, x) += vote;

					// 尤度マップの位置に対して姿勢の和
					axisMap[y][x] += nodeData[index]->axisList[i] * vote;
					angleMap[y][x] += nodeData[index]->angleList[i] * vote;
				}
			}
		}
	}


	v_finish = static_cast<double>(cv::getTickCount());
	v_time = ( ( v_finish - v_start ) / cv::getTickFrequency() );
	//std::cout << "投票処理 : " << v_time << "秒" << std::endl;
}



/**
 * @brief   分岐後の情報利得結果
 * 
 * @param	samples[in]				学習データ
 * @param	indices[in]				現在のノードにおけるサンプルのインデックス
 * @param	similarity[in]			類似度
 * @param   leftCount[in,out]		左の子ノード数
 * @param   rightCount[in,out]		右の子ノード数
 * @param	threshold[in]			選択された閾値
 * @param	il[in]					クラスの偏りに応じた重み付け
 * 
 * @returns		分岐後の情報利得
 */
inline double SecondLayerFern::splitInformationGain( const std::vector<std::shared_ptr<LearnSample>> &samples, const std::vector<int> &indices, const std::vector<float> &similarity, int &leftCount, int &rightCount, float threshold, const std::vector<double> &il)
{
	std::vector<double> leftDistribution(numClass, 0.0);	// 左のクラス確率
	std::vector<double> rightDistribution(numClass, 0.0);	// 右のクラス確率


	// サンプルを分岐
	for (int i = 0; i < (int)indices.size(); ++i)
	{
		int index = indices[i];
		if (similarity[index] < threshold) {
			leftCount++;
			leftDistribution[samples[index]->label] += 1.0 * il[samples[index]->label];
		} else {
			rightCount++;
			rightDistribution[samples[index]->label] += 1.0 * il[samples[index]->label];
		}
	}

	// 分岐が偏っていた場合、大きい値を返す
	if (leftCount == 0 || rightCount == 0 ) {
		return 1000.0;
	}
	// 偏っていない場合、情報利得の結果を返す
	else
	{
		double lsize = (double)leftCount;
		double rsize = (double)rightCount;
		double leftTotal = 0.0, rightTotal = 0.0;

		// 分岐後のクラス確率の総和
		for (int i = 0; i < numClass; ++i)
		{
			leftTotal += leftDistribution[i]; 
			rightTotal += rightDistribution[i]; 
		}
		// 正規化
		for (int i = 0; i < numClass; ++i)
		{
			leftDistribution[i] /= leftTotal;
			rightDistribution[i] /= rightTotal;
		}

		// 子ノードのエントロピーを計算
		return lsize / (lsize + rsize) * computeEntropy(leftDistribution) + rsize / (rsize + lsize) * computeEntropy(rightDistribution);
	}
}



/**
 * @brief   分岐後のオフセットと姿勢の分散結果
 * 
 * @param	samples[in]				学習データ
 * @param	indices[in]				現在のノードにおけるサンプルのインデックス
 * @param	similarity[in]			類似度
 * @param   leftCount[in,out]		左の子ノード数
 * @param   rightCount[in,out]		右の子ノード数
 * @param	threshold[in]			選択された閾値
 * 
 * @returns		分岐後の分散
 */
inline double SecondLayerFern::splitAllVariance( const std::vector<std::shared_ptr<LearnSample>> &samples, const std::vector<int> &indices, const std::vector<float> &similarity, int &leftCount, int &rightCount, float threshold)
{
	std::vector<cv::Point2f> leftOffsetList;				// 左のオフセットベクトルのリスト
	std::vector<cv::Point2f> rightOffsetList;				// 右のオフセットベクトルのリスト
	std::vector<glm::vec3> leftAxisList;					// 左の回転軸のリスト
	std::vector<glm::vec3> rightAxisList;					// 右の回転軸のリスト
	std::vector<glm::vec2> leftAngle;						// 左の回転ベクトル
	std::vector<glm::vec2> rightAngle;						// 右の回転ベクトル

	// サンプルを分岐
	for (int i = 0; i < (int)indices.size(); ++i)
	{
		int index = indices[i];

		if (similarity[index] < threshold) 
		{
			leftCount++;
			if (samples[index]->label == posLabel){
				leftOffsetList.push_back(samples[index]->offset);
				leftAxisList.emplace_back(samples[index]->axis);
				leftAngle.emplace_back(samples[index]->rollAngle);	
			}			
		} else 
		{
			rightCount++;
			if (samples[index]->label == posLabel){
				rightOffsetList.push_back(samples[index]->offset);
				rightAxisList.emplace_back(samples[index]->axis);
				rightAngle.emplace_back(samples[index]->rollAngle);	
			}
		}
	}


	// 分岐が偏っていた場合、大きい値を返す
	if (leftCount == 0 || rightCount == 0 ) {
		return 1000.0;
	}
	// 偏っていない場合、分散の結果を返す
	else
	{
		/***** オフセットの分散 *****/

		float tmpX, tmpY;
		float leftVariance = 0.0;							// 分散
		float rightVariance = 0.0;
		cv::Point2f leftCentroid = ( 0.0f, 0.0f);			// 平均値
		cv::Point2f rightCentroid = ( 0.0f, 0.0f);

		if (leftOffsetList.size() == 0){
			leftCentroid.x = 0.0f;
			leftCentroid.y = 0.0f;
		}else{
			// 左のサンプルのセントロイドの算出
			for (int i = 0; i < (int)leftOffsetList.size(); ++i)
			{
				leftCentroid.x += leftOffsetList[i].x;
				leftCentroid.y += leftOffsetList[i].y;
			}
			leftCentroid.x /= (float)leftOffsetList.size();
			leftCentroid.y /= (float)leftOffsetList.size();
		}
	
		if (rightOffsetList.size() == 0){
			rightCentroid.x = 0.0f;
			rightCentroid.y = 0.0f;
		}else{
			// 右のサンプルのセントロイドの算出
			for (int i = 0; i < (int)rightOffsetList.size(); ++i)
			{
				rightCentroid.x += rightOffsetList[ i ].x;
				rightCentroid.y += rightOffsetList[ i ].y;
			}
			rightCentroid.x /= (float)rightOffsetList.size();
			rightCentroid.y /= (float)rightOffsetList.size();
		}

		// 左の分散の算出
		for (int i = 0; i < (int)leftOffsetList.size(); ++i)
		{
			tmpX = leftCentroid.x - leftOffsetList[i].x;
			tmpX = tmpX * tmpX;
			tmpY = leftCentroid.y - leftOffsetList[i].y;
			tmpY = tmpY * tmpY;
			leftVariance += sqrt(tmpX + tmpY);
		}
		if (leftOffsetList.size() == 0){
			leftVariance = 0;
		}else{
			leftVariance /= (float)leftOffsetList.size();
		}

		//右の分散の算出
		for (int i = 0; i < (int)rightOffsetList.size(); ++i)
		{
			tmpX = rightCentroid.x - rightOffsetList[i].x;
			tmpX = tmpX * tmpX;
			tmpY = rightCentroid.y - rightOffsetList[i].y;
			tmpY = tmpY * tmpY;
			rightVariance += sqrt(tmpX + tmpY);
		}
		if (rightOffsetList.size() == 0){
			rightVariance = 0.0;
		}else{
			rightVariance /= (float)rightOffsetList.size();
		}

		// 子ノードの分散の和を返す
		double offsetVariance = (double)(leftVariance + rightVariance);



		/***** 姿勢の分散 *****/

		float tmpX2, tmpY2, tmpZ2;
		float leftAxisVariance = 0.0;										// 分散
		float rightAxisVariance = 0.0;
		glm::vec3 leftCentroidAxis = glm::vec3( 0.0f, 0.0f, 0.0f);	// 平均値
		glm::vec3 rightCentroidAxis = glm::vec3( 0.0f, 0.0f, 0.0f);

		// 左のサンプルのセントロイドの算出
		for (int i = 0; i < (int)leftAxisList.size(); ++i){
			leftCentroidAxis += leftAxisList[i];
		}
		// 正規化
		leftCentroidAxis = glm::normalize(leftCentroidAxis);
		
	
		// 右のサンプルのセントロイドの算出
		for (int i = 0; i < (int)rightAxisList.size(); ++i){
			rightCentroidAxis += rightAxisList[i];
		}
		// 正規化
		rightCentroidAxis = glm::normalize(rightCentroidAxis);
		

		// 左の分散の算出
		for (int i = 0; i < (int)leftAxisList.size(); ++i)
		{
			tmpX2 = leftCentroidAxis.x - leftAxisList[i].x;
			tmpX2 = tmpX2 * tmpX2;
			tmpY2 = leftCentroidAxis.y - leftAxisList[i].y;
			tmpY2 = tmpY2 * tmpY2;
			tmpZ2 = leftCentroidAxis.z - leftAxisList[i].z;
			tmpZ2 = tmpZ2 * tmpZ2;
			leftVariance += sqrt(tmpX2 + tmpY2 + tmpZ2);
		}
		if (leftAxisList.size() == 0){
			leftAxisVariance = 0;
		}else{
			leftAxisVariance /= (float)leftAxisList.size();
		}

		//右の分散の算出
		for (int i = 0; i < (int)rightAxisList.size(); ++i)
		{
			tmpX2 = rightCentroidAxis.x - rightAxisList[i].x;
			tmpX2 = tmpX2 * tmpX2;
			tmpY2 = rightCentroidAxis.y - rightAxisList[i].y;
			tmpY2 = tmpY2 * tmpY2;
			tmpZ2 = rightCentroidAxis.z - rightAxisList[i].z;
			tmpZ2 = tmpZ2 * tmpZ2;
			rightVariance += sqrt(tmpX2 + tmpY2 + tmpZ2);
		}
		if (rightAxisList.size() == 0){
			rightAxisVariance = 0.0;
		}else{
			rightAxisVariance /= (float)rightAxisList.size();
		}

		// 子ノードの分散の和を返す
		double axisVariance = (double)(leftAxisVariance + rightAxisVariance);


		// roll回転の分散
		float leftAngleVariance, rightAngleVariance;
		// 左の分散
		if (leftAngle.size() == 0) {
			leftAngleVariance = 0.0;
		} else {
			float cosTotal = 0.0;
			float sinTotal = 0.0;

			for ( int i = 0; i < (int)leftAngle.size(); ++i){
				cosTotal += leftAngle[i].x;
				sinTotal += leftAngle[i].y;
			}

			leftAngleVariance = 1.0 - (sqrt(cosTotal*cosTotal + sinTotal*sinTotal)/ (double)(leftAngle.size()) );
		}

		// 右の分散
		if (rightAngle.size() == 0) {
			rightAngleVariance = 0.0;
		} else {
			float cosTotal = 0.0;
			float sinTotal = 0.0;

			for ( int i = 0; i < (int)rightAngle.size(); ++i){
				cosTotal += rightAngle[i].x;
				sinTotal += rightAngle[i].y;
			}

			rightAngleVariance = 1.0 - (sqrt(cosTotal*cosTotal + sinTotal*sinTotal)/ (double)(rightAngle.size()) );
		}

		double angleVariance = (double)(leftAngleVariance + rightAngleVariance);

		// オフセットの分散と軸の分散とroll角の分散の和
		return offsetVariance + axisVariance + angleVariance;
	}
}




/**
 * @brief   シャノンエントロピー関数
 * 
 * @param   dist[in] 確率密度関数
 * 
 * @returns シャノンエントロピー   
 */
inline double SecondLayerFern::computeEntropy(const std::vector<double> &dist)
{
	double entropy = 0.0;

	for (int i = 0; i < numClass; ++i)
	{
		if ( dist[i] == 0.0){
			entropy += 0.0;
		} else {
			// log2が無いため、底の変換
			entropy += dist[i] * (double)(log10( dist[ i ] ) / log10( 2.0 ));
		}
	}

	return -entropy;
}





/**
 * @brief   Fernのパラメータを保存
 * 
 * @param   fileName[in]		保存するファイル名(バイナリファイル)
 * @param   fileName_txt[in]	保存するファイル名(テキストファイル)
 */
void SecondLayerFern::saveFern(const std::string &fileName, const std::string& fileName_txt)
{
	// ファイルオープン
	std::ofstream ofs(fileName, std::ios::out | std::ios::binary);
	std::ofstream ofs_t(fileName_txt);

	// ヘッダ
	int p_num = (int)splitNode.size();
	ofs.write((const char*)&p_num, sizeof(int));	// パッチ数
	ofs_t << p_num << std::endl;					// パッチ数

	int n_num = (int)nodeData.size();
	ofs.write((const char*)&n_num, sizeof(int));	// 末端ノード数
	ofs_t << n_num << std::endl;					// 末端ノード数

	int count=0;

	// 有効なノード数
	for (int i = 0; i < (int)nodeData.size(); ++i)
	{
		// ノードにポジティブサンプルが含まれている場合のみ
		if (nodeData[i]->offsetList.size() > 0)
		{
			count++;
		}
	}
	ofs.write((const char*)&count, sizeof(int));			// 有効なノード数
	ofs_t << count << std::endl;							// 有効なノード数

	if (splitNode.size() > 0) {
		int p_size = splitNode[0]->patch.cols;
		ofs.write((const char*)&p_size, sizeof(int));		// パッチサイズ
		ofs_t << p_size << std::endl;						// パッチサイズ
	} else {
		int p_size = 0;
		ofs.write((const char*)&p_size, sizeof(int));		// パッチサイズ
		ofs_t << 0 << std::endl;							// パッチサイズ
	}

	// 類似度評価の数
	for (int i = 0; i < (int)splitNode.size(); ++i)
	{
		std::vector<uchar> patch_vec;
		std::vector<int> weight_vec;

		// cv::Matからstd::vectorへ
		splitNode[i]->patch.reshape(0,1).copyTo(patch_vec);
		splitNode[i]->weight.reshape(0,1).copyTo(weight_vec);

		// パッチのpixel数
		int patch_pixel_num = patch_vec.size();
		ofs.write((const char*)&patch_pixel_num, sizeof(int));

		// パッチの累積勾配方向
		for (int j = 0; j < patch_pixel_num-1; ++j)
		{
			int p_vec = (int)patch_vec[j];
			ofs.write((const char*)&p_vec, sizeof(int));
			ofs_t << p_vec << ",";
		}
		int p_vec = (int)patch_vec[patch_pixel_num-1];
		ofs.write((const char*)&p_vec, sizeof(int));
		ofs_t << p_vec << std::endl;

		// パッチの重み
		for (int j = 0; j < patch_pixel_num-1; ++j)
		{
			int w_vec = weight_vec[j];
			ofs.write((const char*)&w_vec, sizeof(int));
			ofs_t << w_vec << ",";
		}
		int w_vec = weight_vec[patch_pixel_num-1] ;
		ofs.write((const char*)&w_vec, sizeof(int));
		ofs_t << w_vec << std::endl;
		
		// 類似度の閾値
		float split_th = splitNode[i]->threshold;
		ofs.write((const char*)&split_th, sizeof(float));
		ofs_t << split_th << std::endl;
	}


	// 末端ノード数
	for (int i = 0; i < (int)nodeData.size(); ++i)
	{
		// ノードにポジティブサンプルが含まれている場合のみ
		if (nodeData[i]->offsetList.size() > 0)
		{
			// ノード番号
			ofs.write((const char*)&i, sizeof(int));
			ofs_t << i << std::endl;

			// クラス確率
			int distribute_size = nodeData[i]->distribution.size();
			ofs.write((const char*)&distribute_size, sizeof(int));

			for (int j = 0; j < distribute_size-1; ++j)
			{
				float n_dist = nodeData[i]->distribution[j];
				ofs.write((const char*)&n_dist, sizeof(float));
				ofs_t << n_dist << ",";
			}
			float n_dist = nodeData[i]->distribution[distribute_size-1];
			ofs.write((const char*)&n_dist, sizeof(float));
			ofs_t << n_dist << std::endl;


			int size = nodeData[i]->offsetList.size();
			ofs.write((const char*)&size, sizeof(int));

			// オフセット
			for (int j = 0; j < size-1; ++j)
			{
				float offset_x = nodeData[i]->offsetList[j].x;
				float offset_y = nodeData[i]->offsetList[j].y;
				ofs.write((const char*)&offset_x, sizeof(float));
				ofs.write((const char*)&offset_y, sizeof(float));
				ofs_t << offset_x << "," << offset_y << ",";
			}
			if (size > 0) {
				float offset_x = nodeData[i]->offsetList[size-1].x;
				float offset_y = nodeData[i]->offsetList[size-1].y;
				ofs.write((const char*)&offset_x, sizeof(float));
				ofs.write((const char*)&offset_y, sizeof(float));
				ofs_t << offset_x << "," << offset_y << std::endl;
			}

			// 軸
			for (int j = 0; j < size-1; ++j)
			{
				float axis_x = nodeData[i]->axisList[j].x;
				float axis_y = nodeData[i]->axisList[j].y;
				float axis_z = nodeData[i]->axisList[j].z;
				ofs.write((const char*)&axis_x, sizeof(float));
				ofs.write((const char*)&axis_y, sizeof(float));
				ofs.write((const char*)&axis_z, sizeof(float));
				ofs_t << axis_x << "," << axis_y << "," << axis_z << ",";
			}
			if (size > 0) {
				float axis_x = nodeData[i]->axisList[size-1].x;
				float axis_y = nodeData[i]->axisList[size-1].y;
				float axis_z = nodeData[i]->axisList[size-1].z;
				ofs.write((const char*)&axis_x, sizeof(float));
				ofs.write((const char*)&axis_y, sizeof(float));
				ofs.write((const char*)&axis_z, sizeof(float));
				ofs_t << axis_x << "," << axis_y << "," << axis_z << std::endl;
			}

			// 角度
			for (int j = 0; j < size-1; ++j)
			{
				float angle_x = nodeData[i]->angleList[j].x;
				float angle_y = nodeData[i]->angleList[j].y;
				ofs.write((const char*)&angle_x, sizeof(float));
				ofs.write((const char*)&angle_y, sizeof(float));
				ofs_t << angle_x << "," << angle_y << ",";
			}
			if (size > 0) {
				float angle_x = nodeData[i]->angleList[size-1].x;
				float angle_y = nodeData[i]->angleList[size-1].y;
				ofs.write((const char*)&angle_x, sizeof(float));
				ofs.write((const char*)&angle_y, sizeof(float));
				ofs_t << angle_x << "," << angle_y << std::endl;
			}
		}
	}
}




/**
 * @brief   Fernを読み込む
 * 
 * @param   fileName[in]	決定木のファイル
 * 
 * @returns true or false  
 */
bool SecondLayerFern::loadFern(const std::string &fileName)
{
	// 初期化
	splitNode.clear();
	nodeData.clear();
	nodeLUT.clear();

	// ファイルの読み込み
	std::ifstream ifs(fileName, std::ios::in | std::ios::binary);

	if( !ifs ) {
		std::cerr << "ファイルを読み込めません" << std::endl;
		return false;
	}



	// ヘッダの読み込み
	int patch_num, node_num, valid_num, patch_size;
	ifs.read((char*)&patch_num, sizeof(int));
	ifs.read((char*)&node_num, sizeof(int));
	ifs.read((char*)&valid_num, sizeof(int));
	ifs.read((char*)&patch_size, sizeof(int));

	// 類似度評価の数
	for (int i = 0; i < patch_num; ++i)
	{
		std::unique_ptr<Node> node(new Node);
		std::vector<uchar> patch_vec;
		std::vector<int> weight_vec;

		// パッチのpixel数
		int patch_pixel_num;
		ifs.read((char*)&patch_pixel_num, sizeof(int));

		// パッチ
		for (int j = 0; j < patch_pixel_num; ++j)
		{
			int p_vec;
			ifs.read((char*)&p_vec, sizeof(int));
			patch_vec.emplace_back((uchar)(p_vec));
		}

		// 重み
		for (int j = 0; j < patch_pixel_num; ++j)
		{
			int w_vec;
			ifs.read((char*)&w_vec, sizeof(int));
			weight_vec.emplace_back(w_vec);
		}

		// std::vectorからcv::Matへ
		node->patch.create(patch_size, patch_size);
		node->weight.create(patch_size, patch_size);
		for (int r = 0; r < patch_size; ++r)
		{
			uchar* patch_r = node->patch.ptr<uchar>(r);
			int* weight_r = node->weight.ptr<int>(r);
			for (int c = 0; c < patch_size; ++c)
			{
				patch_r[c] = patch_vec[r*patch_size+c];
				weight_r[c] = weight_vec[r*patch_size+c];
			}
		}

		// 閾値
		float split_th;
		ifs.read((char*)&split_th, sizeof(float));
		node->threshold = split_th;

		splitNode.emplace_back(std::move(node));
	}


	// 有効な末端ノード数
	std::vector<int> indices;
	for (int i = 0; i < valid_num; ++i)
	{
		// ノード番号
		int valid_node;
		ifs.read((char*)&valid_node, sizeof(int));
		indices.emplace_back(valid_node);

		// データの格納
		std::unique_ptr<EndNodeData> data(new EndNodeData);

		// クラス確率
		int distribute_size;
		ifs.read((char*)&distribute_size, sizeof(int));

		for (int j = 0; j < distribute_size; ++j)
		{
			float n_dist;
			ifs.read((char*)&n_dist, sizeof(float));
			data->distribution.emplace_back(n_dist);
		}

		int size;
		ifs.read((char*)&size, sizeof(int));

		// オフセット
		for (int j = 0; j < size; ++j)
		{
			float offset_x, offset_y;
			ifs.read((char*)&offset_x, sizeof(float));
			ifs.read((char*)&offset_y, sizeof(float));
			data->offsetList.emplace_back(cv::Point2f(offset_x, offset_y));
		}


		// 軸
		for (int j = 0; j < size; ++j)
		{
			float axis_x, axis_y, axis_z;
			ifs.read((char*)&axis_x, sizeof(float));
			ifs.read((char*)&axis_y, sizeof(float));
			ifs.read((char*)&axis_z, sizeof(float));
			data->axisList.emplace_back(glm::vec3(axis_x, axis_y, axis_z));
		}


		// 角度
		for (int j = 0; j < size; ++j)
		{
			float angle_x, angle_y;
			ifs.read((char*)&angle_x, sizeof(float));
			ifs.read((char*)&angle_y, sizeof(float));
			data->angleList.emplace_back(glm::vec2(angle_x, angle_y));
		}

		nodeData.emplace_back(std::move(data));
	}


	// 末端ノードにアクセスするための連想配列
	nodeLUT.resize(node_num, -1);
	for (int i = 0; i < valid_num; ++i)
	{
		nodeLUT[indices[i]] = i;
	}

	return true;
}


/**
 * @brief   Fernを読み込む(テキスト形式)
 * 
 * @param   fileName[in]	決定木のファイル
 * 
 * @returns true or false  
 */
bool SecondLayerFern::loadFern_txt(const std::string &fileName)
{
	// 初期化
	splitNode.clear();
	nodeData.clear();
	nodeLUT.clear();

	// ファイルの読み込み
	std::ifstream ifs(fileName);

	if( !ifs ) {
		std::cerr << "ファイルを読み込めません" << std::endl;
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

	// ヘッダの読み込み
	int patch_num, node_num, valid_num, patch_size;
	patch_num = atoi(params[0][0].c_str());
	node_num = atoi(params[1][0].c_str());
	valid_num = atoi(params[2][0].c_str());
	patch_size = atoi(params[3][0].c_str());

	int count_num = 4;

	// 類似度評価の数
	for (int i = 0; i < patch_num; ++i)
	{
		std::unique_ptr<Node> node(new Node);
		std::vector<uchar> patch_vec;
		std::vector<int> weight_vec;

		// パッチ
		for (int j = 0; j < (int)params[count_num].size(); ++j)
		{
			patch_vec.emplace_back((uchar)atoi(params[count_num][j].c_str()));
		}
		count_num++;

		// 重み
		for (int j = 0; j < (int)params[count_num].size(); ++j)
		{
			weight_vec.emplace_back(atoi(params[count_num][j].c_str()));
		}
		count_num++;

		// std::vectorからcv::Matへ
		node->patch.create(patch_size, patch_size);
		node->weight.create(patch_size, patch_size);
		for (int r = 0; r < patch_size; ++r)
		{
			uchar* patch_r = node->patch.ptr<uchar>(r);
			int* weight_r = node->weight.ptr<int>(r);
			for (int c = 0; c < patch_size; ++c)
			{
				patch_r[c] = patch_vec[r*patch_size+c];
				weight_r[c] = weight_vec[r*patch_size+c];
			}
		}

		// 閾値
		node->threshold = atof(params[count_num][0].c_str());
		count_num++;

		splitNode.emplace_back(std::move(node));
	}


	// 有効な末端ノード数
	std::vector<int> indices;
	for (int i = 0; i < valid_num; ++i)
	{
		indices.emplace_back(atoi(params[count_num][0].c_str()));

		count_num++;

		// データの格納
		std::unique_ptr<EndNodeData> data(new EndNodeData);

		// クラス確率
		for (int j = 0; j < (int)params[count_num].size(); ++j)
		{
			data->distribution.emplace_back(atof(params[count_num][j].c_str()));
		}
		count_num++;

		// オフセット
		for (int j = 0; j < (int)params[count_num].size(); j+=2)
		{
			data->offsetList.emplace_back(cv::Point2f(atof(params[count_num][j].c_str()), atof(params[count_num][j+1].c_str())));
		}
		count_num++;

		// 軸
		for (int j = 0; j < (int)params[count_num].size(); j+=3)
		{
			data->axisList.emplace_back(glm::vec3(atof(params[count_num][j].c_str()), atof(params[count_num][j+1].c_str()), atof(params[count_num][j+2].c_str())));
		}
		count_num++;

		// 角度
		for (int j = 0; j < (int)params[count_num].size(); j+=2)
		{
			data->angleList.emplace_back(glm::vec2(atof(params[count_num][j].c_str()), atof(params[count_num][j+1].c_str())));
		}
		count_num++;

		nodeData.emplace_back(std::move(data));
	}


	// 末端ノードにアクセスするための連想配列
	nodeLUT.resize(node_num, -1);
	for (int i = 0; i < valid_num; ++i)
	{
		nodeLUT[indices[i]] = i;
	}

	return true;
}