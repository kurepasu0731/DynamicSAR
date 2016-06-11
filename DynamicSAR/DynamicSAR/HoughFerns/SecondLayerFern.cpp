#include "SecondLayerFern.h"



/**
 * @brief   Fern�̊w�K
 * 
 * @param   samples[in]		�w�K�f�[�^(�O���̓|�W�e�B�u�T���v��)
 * @param   posNum[in]		�|�W�e�B�u�T���v���̐�
 * @param   il[in]			�N���X�̕΂�ɉ������d�ݕt��
 */
void SecondLayerFern::Learn(const std::vector<std::shared_ptr<LearnSample>>& samples, int posNum, const std::vector<double>& il)
{
	// ������
	splitNode.clear();
	nodeData.clear();

	std::map<int, std::vector<int>> data_pool;		// �K�w�ɂ�����m�[�h�ԍ��ɑΉ�����w�K�f�[�^��ID(�ꎞ�ۑ��p)
	{
		std::vector<int> data_id(samples.size(), 0);// �w�K�f�[�^��ID(�z��̔ԍ�)
		// �w�K�f�[�^��ID��t����
		for (int i = 0; i < (int)samples.size(); ++i){
			data_id[i] = i;
		}
		data_pool[0] = data_id;						// ���[�g�m�[�h�ɂ�����w�K�f�[�^��ID
	}

	std::vector<int> feature_indices;				// �������o�p�̃C���f�b�N�X�ԍ�
	

	// ��������
	std::random_device rnd;							// �񌈒�I�ȗ���������𐶐�
    std::mt19937 mt(rnd());							// �����Z���k�E�c�C�X�^
	std::uniform_int_distribution<int> rand_f(0, posNum-1);		// �͈͂̈�l����(�|�W�e�B�u�T���v���̑I��)
	std::uniform_real_distribution<float> rand_t(0.f, 1.f);		// �͈͂̈�l����(臒l�̑I��)


	// P-COF������
	p_cof.gridSize = gridSize;

	// �ő�̐[���ɒB����܂�
	for (int d = 0; d < depth; ++d)
	{
		// ���݂̊K�w�̃m�[�h��
		int node_num = pow(2, d);

		// �]�����@��ύX
		bool gain_switch = false;
		if ((d%2 == 0 || d < 3) && d < (int)(depth*0.5))
			gain_switch = true;


		// �ŗǂ̕������L������ϐ�
		double best_gain = DBL_MAX;
		int best_feature = -1;			// �����I��p�̃C���f�b�N�X�ԍ�
		double best_threshold = 0.0;	// �ގ��x��臒l


		///// �e���������� /////
		for (int f = 0; f < featureTestNum; ++f)
		{
			// �|�W�e�B�u�T���v���̃����_���I��
			int feature_num;
			while(1) 
			{
				feature_num = rand_f(mt);
				bool break_flag = true;

				// �ߋ��ɑI�������T���v���͏��O
				for( int i = 0; i < (int)feature_indices.size(); ++i){
					if ( feature_num == feature_indices[i]){
						break_flag = false;
					}
				}
				if (break_flag) {
					break;
				}
			}

			// �S�ẴT���v���ɑ΂��ėގ��x�����߂�
			std::vector<float> similarity(samples.size());
			for (int i = 0; i < (int)samples.size(); ++i)
			{
				cv::Mat similarMat;
				p_cof.calcSimilarity(samples[i]->gradOri, samples[feature_num]->pcofOri, samples[feature_num]->weight, similarMat);		// �ގ��x

				similarity[i] = similarMat.at<float>(0);
			}


			///// �e臒l������ /////
			for (int t = 0; t < thresholdTestNum; ++t)
			{
				// 臒l�̃����_���I��
				float threshold = rand_t(mt);

				double gain = 0;			// ��񗘓�

				///// ���݂̊K�w�̃m�[�h�S�Ăɑ΂��ĕ]�� /////
				for (int n = 0; n < node_num; ++n)
				{
					// ���݂̃m�[�h�Ɋ��蓖�Ă��Ă���w�K�T���v���̃C���f�b�N�X�ԍ�
					std::vector<int> sample_indices = data_pool[n];

					// �ϐ��̏�����
					int leftCount = 0;		// �E�̐�
					int rightCount = 0;		// ���̐�

					// �]���֐��̐؂�ւ�
					if (gain_switch){
						// ��񗘓��̎Z�o
						gain += splitInformationGain(samples, sample_indices, similarity, leftCount, rightCount, threshold, il);
					} else {
						// �I�t�Z�b�g�Ǝp���̕��U�̎Z�o
						gain += splitAllVariance(samples, sample_indices, similarity, leftCount, rightCount, threshold);
					}
				}

				// ���ǂ������ł���΍X�V
				if (gain <= best_gain)
				{
					best_gain = gain;
					best_feature = feature_num;
					best_threshold = threshold;
				}
			}
		}

		///// ���̊K�w�� /////

		// ���݂̌��ʂ��i�[
		std::unique_ptr<Node> node(new Node);
		node->patch = samples[best_feature]->pcofOri.clone();
		node->weight = samples[best_feature]->weight.clone();
		node->threshold = best_threshold;
		splitNode.emplace_back(std::move(node));

		feature_indices.emplace_back(best_feature);
	

		// �I�����ꂽ�����̃C���f�b�N�X�ԍ���臒l��p���āC������x����
		std::vector<float> similarity(samples.size());
		for (int i = 0; i < (int)samples.size(); ++i)
		{
			cv::Mat similarMat;
			p_cof.calcSimilarity(samples[i]->gradOri, samples[best_feature]->pcofOri, samples[best_feature]->weight, similarMat);		// �ގ��x

			similarity[i] = similarMat.at<float>(0);
		}

		// ���̊K�w�̃f�[�^�𐶐�
		std::map<int, std::vector<int>> new_data_pool;		// ���̊K�w�̃m�[�h�ԍ��ƑΉ�����T���v���̃C���f�b�N�X���i�[

		// ���݂̊K�w�̃m�[�h�ɑ΂���
		for (int n = 0; n < node_num; ++n)
		{
			// ���݂̃m�[�h�Ɋ��蓖�Ă��Ă���w�K�T���v���̃C���f�b�N�X�ԍ�
			std::vector<int> sample_indices = data_pool[n];

			// ���E�̈ꎞ�i�[�f�[�^�̍쐬(���݂̃f�[�^ID���œK�ȕ��������Ċi�[)
			std::vector<int> data_id_l;		// ���̃f�[�^ID
			std::vector<int> data_id_r;		// �E�̃f�[�^ID

			// �T���v���𕪊�
			for (int i = 0; i < (int)sample_indices.size(); ++i)
			{
				int index = sample_indices[i];
				if (similarity[index] < best_threshold) {
					data_id_l.emplace_back(index);
				} else {
					data_id_r.emplace_back(index);
				}
			}

			//std::cout << "�[��:" << d << ",ID:" <<  n << ",��:" << data_id_l.size() << ",�E:" << data_id_r.size() << std::endl;

			// ���̊K�w�̃f�[�^
			new_data_pool[2*n] = data_id_l;
			new_data_pool[2*n+1] = data_id_r;
		}

		// ���݂̊K�w�̃f�[�^���폜���C���̊K�w�̃f�[�^���i�[
		data_pool.clear();
		data_pool.insert(new_data_pool.begin(), new_data_pool.end());
	}


	///// ���[�m�[�h�̃f�[�^�����b�N�A�b�v�e�[�u���Ɋi�[ /////
	int node_num = pow(2, depth);

	// ���[�m�[�h��T��
	for (int n = 0; n < node_num; ++n)
	{
		// ���݂̃m�[�h�Ɋ��蓖�Ă��Ă���w�K�T���v���̃C���f�b�N�X�ԍ�
		std::vector<int> sample_indices = data_pool[n];

		// �N���X�m�����x�̌v�Z
		std::vector<float> distribution( numClass, 0.f);
		double total = 0.0;

		for (int i = 0; i < (int)sample_indices.size(); ++i)
		{
			int label = samples[sample_indices[i]]->label;
			distribution[label] += 1.f * il[label];
		}
		// ���K��
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

		// �I�t�Z�b�g���X�g�Ǝp�����X�g�̍쐬
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

		nodeData.emplace_back(std::move(node_data));		// �ǉ�
	}
}




// ���ʂƓ��[����
void SecondLayerFern::Predict(const cv::Mat& src, cv::Mat_<float>& likelihoodMap, std::vector<std::vector<glm::vec3>>& axisMap, std::vector<std::vector<glm::vec2>>& angleMap)
{
	// �w�K���Ԃ̑���
	double t_start, t_finish, t_time, v_start, v_finish, v_time;
	t_start = static_cast<double>(cv::getTickCount());


	///// �S�Ẵp�b�`�ɑ΂��ăe���v���[�g�}�b�`���O /////
	int patch_heght = splitNode[0]->patch.rows;
	int patch_width = splitNode[0]->patch.cols;
	int height = src.rows - patch_heght + 1;
	int width = src.cols - patch_width + 1;
	cv::Mat findLUT = cv::Mat::zeros(height, width, CV_32F);		// Fern�̖��[�m�[�h�ɃA�N�Z�X����Mat

	// �S�Ẵp�b�`�ɑ΂���
	for (int i = 0; i < (int)splitNode.size(); ++i)
	{
		// �ގ��x�}�b�v�̐���
		cv::Mat similarMap;
		p_cof.calcSimilarity(src, splitNode[i]->patch, splitNode[i]->weight, similarMap);

		// 臒l������2�l��
		cv::Mat binaryMap;
		double bit_pos = (1 << (depth-1-i));		// �r�b�g�𗧂Ă�ʒu
		cv::threshold (similarMap, binaryMap, splitNode[i]->threshold, bit_pos, cv::THRESH_BINARY);

		// �r�b�gOR
		cv::add (findLUT, binaryMap, findLUT);
	}


	t_finish = static_cast<double>(cv::getTickCount());
	t_time = ( ( t_finish - t_start ) / cv::getTickFrequency() );
	//std::cout << "�e���v���[�g�}�b�`���O���� : " << t_time << "�b" << std::endl;

	v_start = static_cast<double>(cv::getTickCount());


	///// ���[ /////

	int mapWidth = likelihoodMap.cols;
	int mapHeight = likelihoodMap.rows;

	// �S�Ẳ�f�𑖍�
	for (int r = 0; r < findLUT.rows; ++r)
	{
		float* LUT_r = findLUT.ptr<float>(r);

		for (int c = 0; c < findLUT.cols; ++c)
		{
			int index = nodeLUT[(int)LUT_r[c]];			// �Q�Ƃ���C���f�b�N�X

			// �I�t�Z�b�g�x�N�g�����i�[����Ă��Ȃ��ꍇ�͔�΂�
			if (index == -1)
			{
				continue;
			}
			int offset_num = nodeData[index]->offsetList.size();


			float vote = nodeData[index]->distribution[posLabel] / (float)offset_num;	// �N���X�m���ɂ��d��

			if (nodeData[index]->distribution[posLabel] > 0.65)
			{
				// ���[�m�[�h�̃f�[�^�ɑ΂���
				for (int i = 0; i < offset_num; ++i)
				{
					// ���[���W
					int x = c + patch_width*0.5 + nodeData[index]->offsetList[i].x;
					int y = r + patch_heght*0.5 + nodeData[index]->offsetList[i].y;

					if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) {
						continue;
					}

					// �ޓx�}�b�v�ɓ��[
					likelihoodMap(y, x) += vote;

					// �ޓx�}�b�v�̈ʒu�ɑ΂��Ďp���̘a
					axisMap[y][x] += nodeData[index]->axisList[i] * vote;
					angleMap[y][x] += nodeData[index]->angleList[i] * vote;
				}
			}
		}
	}


	v_finish = static_cast<double>(cv::getTickCount());
	v_time = ( ( v_finish - v_start ) / cv::getTickFrequency() );
	//std::cout << "���[���� : " << v_time << "�b" << std::endl;
}



/**
 * @brief   �����̏�񗘓�����
 * 
 * @param	samples[in]				�w�K�f�[�^
 * @param	indices[in]				���݂̃m�[�h�ɂ�����T���v���̃C���f�b�N�X
 * @param	similarity[in]			�ގ��x
 * @param   leftCount[in,out]		���̎q�m�[�h��
 * @param   rightCount[in,out]		�E�̎q�m�[�h��
 * @param	threshold[in]			�I�����ꂽ臒l
 * @param	il[in]					�N���X�̕΂�ɉ������d�ݕt��
 * 
 * @returns		�����̏�񗘓�
 */
inline double SecondLayerFern::splitInformationGain( const std::vector<std::shared_ptr<LearnSample>> &samples, const std::vector<int> &indices, const std::vector<float> &similarity, int &leftCount, int &rightCount, float threshold, const std::vector<double> &il)
{
	std::vector<double> leftDistribution(numClass, 0.0);	// ���̃N���X�m��
	std::vector<double> rightDistribution(numClass, 0.0);	// �E�̃N���X�m��


	// �T���v���𕪊�
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

	// ���򂪕΂��Ă����ꍇ�A�傫���l��Ԃ�
	if (leftCount == 0 || rightCount == 0 ) {
		return 1000.0;
	}
	// �΂��Ă��Ȃ��ꍇ�A��񗘓��̌��ʂ�Ԃ�
	else
	{
		double lsize = (double)leftCount;
		double rsize = (double)rightCount;
		double leftTotal = 0.0, rightTotal = 0.0;

		// �����̃N���X�m���̑��a
		for (int i = 0; i < numClass; ++i)
		{
			leftTotal += leftDistribution[i]; 
			rightTotal += rightDistribution[i]; 
		}
		// ���K��
		for (int i = 0; i < numClass; ++i)
		{
			leftDistribution[i] /= leftTotal;
			rightDistribution[i] /= rightTotal;
		}

		// �q�m�[�h�̃G���g���s�[���v�Z
		return lsize / (lsize + rsize) * computeEntropy(leftDistribution) + rsize / (rsize + lsize) * computeEntropy(rightDistribution);
	}
}



/**
 * @brief   �����̃I�t�Z�b�g�Ǝp���̕��U����
 * 
 * @param	samples[in]				�w�K�f�[�^
 * @param	indices[in]				���݂̃m�[�h�ɂ�����T���v���̃C���f�b�N�X
 * @param	similarity[in]			�ގ��x
 * @param   leftCount[in,out]		���̎q�m�[�h��
 * @param   rightCount[in,out]		�E�̎q�m�[�h��
 * @param	threshold[in]			�I�����ꂽ臒l
 * 
 * @returns		�����̕��U
 */
inline double SecondLayerFern::splitAllVariance( const std::vector<std::shared_ptr<LearnSample>> &samples, const std::vector<int> &indices, const std::vector<float> &similarity, int &leftCount, int &rightCount, float threshold)
{
	std::vector<cv::Point2f> leftOffsetList;				// ���̃I�t�Z�b�g�x�N�g���̃��X�g
	std::vector<cv::Point2f> rightOffsetList;				// �E�̃I�t�Z�b�g�x�N�g���̃��X�g
	std::vector<glm::vec3> leftAxisList;					// ���̉�]���̃��X�g
	std::vector<glm::vec3> rightAxisList;					// �E�̉�]���̃��X�g
	std::vector<glm::vec2> leftAngle;						// ���̉�]�x�N�g��
	std::vector<glm::vec2> rightAngle;						// �E�̉�]�x�N�g��

	// �T���v���𕪊�
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


	// ���򂪕΂��Ă����ꍇ�A�傫���l��Ԃ�
	if (leftCount == 0 || rightCount == 0 ) {
		return 1000.0;
	}
	// �΂��Ă��Ȃ��ꍇ�A���U�̌��ʂ�Ԃ�
	else
	{
		/***** �I�t�Z�b�g�̕��U *****/

		float tmpX, tmpY;
		float leftVariance = 0.0;							// ���U
		float rightVariance = 0.0;
		cv::Point2f leftCentroid = ( 0.0f, 0.0f);			// ���ϒl
		cv::Point2f rightCentroid = ( 0.0f, 0.0f);

		if (leftOffsetList.size() == 0){
			leftCentroid.x = 0.0f;
			leftCentroid.y = 0.0f;
		}else{
			// ���̃T���v���̃Z���g���C�h�̎Z�o
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
			// �E�̃T���v���̃Z���g���C�h�̎Z�o
			for (int i = 0; i < (int)rightOffsetList.size(); ++i)
			{
				rightCentroid.x += rightOffsetList[ i ].x;
				rightCentroid.y += rightOffsetList[ i ].y;
			}
			rightCentroid.x /= (float)rightOffsetList.size();
			rightCentroid.y /= (float)rightOffsetList.size();
		}

		// ���̕��U�̎Z�o
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

		//�E�̕��U�̎Z�o
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

		// �q�m�[�h�̕��U�̘a��Ԃ�
		double offsetVariance = (double)(leftVariance + rightVariance);



		/***** �p���̕��U *****/

		float tmpX2, tmpY2, tmpZ2;
		float leftAxisVariance = 0.0;										// ���U
		float rightAxisVariance = 0.0;
		glm::vec3 leftCentroidAxis = glm::vec3( 0.0f, 0.0f, 0.0f);	// ���ϒl
		glm::vec3 rightCentroidAxis = glm::vec3( 0.0f, 0.0f, 0.0f);

		// ���̃T���v���̃Z���g���C�h�̎Z�o
		for (int i = 0; i < (int)leftAxisList.size(); ++i){
			leftCentroidAxis += leftAxisList[i];
		}
		// ���K��
		leftCentroidAxis = glm::normalize(leftCentroidAxis);
		
	
		// �E�̃T���v���̃Z���g���C�h�̎Z�o
		for (int i = 0; i < (int)rightAxisList.size(); ++i){
			rightCentroidAxis += rightAxisList[i];
		}
		// ���K��
		rightCentroidAxis = glm::normalize(rightCentroidAxis);
		

		// ���̕��U�̎Z�o
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

		//�E�̕��U�̎Z�o
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

		// �q�m�[�h�̕��U�̘a��Ԃ�
		double axisVariance = (double)(leftAxisVariance + rightAxisVariance);


		// roll��]�̕��U
		float leftAngleVariance, rightAngleVariance;
		// ���̕��U
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

		// �E�̕��U
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

		// �I�t�Z�b�g�̕��U�Ǝ��̕��U��roll�p�̕��U�̘a
		return offsetVariance + axisVariance + angleVariance;
	}
}




/**
 * @brief   �V���m���G���g���s�[�֐�
 * 
 * @param   dist[in] �m�����x�֐�
 * 
 * @returns �V���m���G���g���s�[   
 */
inline double SecondLayerFern::computeEntropy(const std::vector<double> &dist)
{
	double entropy = 0.0;

	for (int i = 0; i < numClass; ++i)
	{
		if ( dist[i] == 0.0){
			entropy += 0.0;
		} else {
			// log2���������߁A��̕ϊ�
			entropy += dist[i] * (double)(log10( dist[ i ] ) / log10( 2.0 ));
		}
	}

	return -entropy;
}





/**
 * @brief   Fern�̃p�����[�^��ۑ�
 * 
 * @param   fileName[in]		�ۑ�����t�@�C����(�o�C�i���t�@�C��)
 * @param   fileName_txt[in]	�ۑ�����t�@�C����(�e�L�X�g�t�@�C��)
 */
void SecondLayerFern::saveFern(const std::string &fileName, const std::string& fileName_txt)
{
	// �t�@�C���I�[�v��
	std::ofstream ofs(fileName, std::ios::out | std::ios::binary);
	std::ofstream ofs_t(fileName_txt);

	// �w�b�_
	int p_num = (int)splitNode.size();
	ofs.write((const char*)&p_num, sizeof(int));	// �p�b�`��
	ofs_t << p_num << std::endl;					// �p�b�`��

	int n_num = (int)nodeData.size();
	ofs.write((const char*)&n_num, sizeof(int));	// ���[�m�[�h��
	ofs_t << n_num << std::endl;					// ���[�m�[�h��

	int count=0;

	// �L���ȃm�[�h��
	for (int i = 0; i < (int)nodeData.size(); ++i)
	{
		// �m�[�h�Ƀ|�W�e�B�u�T���v�����܂܂�Ă���ꍇ�̂�
		if (nodeData[i]->offsetList.size() > 0)
		{
			count++;
		}
	}
	ofs.write((const char*)&count, sizeof(int));			// �L���ȃm�[�h��
	ofs_t << count << std::endl;							// �L���ȃm�[�h��

	if (splitNode.size() > 0) {
		int p_size = splitNode[0]->patch.cols;
		ofs.write((const char*)&p_size, sizeof(int));		// �p�b�`�T�C�Y
		ofs_t << p_size << std::endl;						// �p�b�`�T�C�Y
	} else {
		int p_size = 0;
		ofs.write((const char*)&p_size, sizeof(int));		// �p�b�`�T�C�Y
		ofs_t << 0 << std::endl;							// �p�b�`�T�C�Y
	}

	// �ގ��x�]���̐�
	for (int i = 0; i < (int)splitNode.size(); ++i)
	{
		std::vector<uchar> patch_vec;
		std::vector<int> weight_vec;

		// cv::Mat����std::vector��
		splitNode[i]->patch.reshape(0,1).copyTo(patch_vec);
		splitNode[i]->weight.reshape(0,1).copyTo(weight_vec);

		// �p�b�`��pixel��
		int patch_pixel_num = patch_vec.size();
		ofs.write((const char*)&patch_pixel_num, sizeof(int));

		// �p�b�`�̗ݐό��z����
		for (int j = 0; j < patch_pixel_num-1; ++j)
		{
			int p_vec = (int)patch_vec[j];
			ofs.write((const char*)&p_vec, sizeof(int));
			ofs_t << p_vec << ",";
		}
		int p_vec = (int)patch_vec[patch_pixel_num-1];
		ofs.write((const char*)&p_vec, sizeof(int));
		ofs_t << p_vec << std::endl;

		// �p�b�`�̏d��
		for (int j = 0; j < patch_pixel_num-1; ++j)
		{
			int w_vec = weight_vec[j];
			ofs.write((const char*)&w_vec, sizeof(int));
			ofs_t << w_vec << ",";
		}
		int w_vec = weight_vec[patch_pixel_num-1] ;
		ofs.write((const char*)&w_vec, sizeof(int));
		ofs_t << w_vec << std::endl;
		
		// �ގ��x��臒l
		float split_th = splitNode[i]->threshold;
		ofs.write((const char*)&split_th, sizeof(float));
		ofs_t << split_th << std::endl;
	}


	// ���[�m�[�h��
	for (int i = 0; i < (int)nodeData.size(); ++i)
	{
		// �m�[�h�Ƀ|�W�e�B�u�T���v�����܂܂�Ă���ꍇ�̂�
		if (nodeData[i]->offsetList.size() > 0)
		{
			// �m�[�h�ԍ�
			ofs.write((const char*)&i, sizeof(int));
			ofs_t << i << std::endl;

			// �N���X�m��
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

			// �I�t�Z�b�g
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

			// ��
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

			// �p�x
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
 * @brief   Fern��ǂݍ���
 * 
 * @param   fileName[in]	����؂̃t�@�C��
 * 
 * @returns true or false  
 */
bool SecondLayerFern::loadFern(const std::string &fileName)
{
	// ������
	splitNode.clear();
	nodeData.clear();
	nodeLUT.clear();

	// �t�@�C���̓ǂݍ���
	std::ifstream ifs(fileName, std::ios::in | std::ios::binary);

	if( !ifs ) {
		std::cerr << "�t�@�C����ǂݍ��߂܂���" << std::endl;
		return false;
	}



	// �w�b�_�̓ǂݍ���
	int patch_num, node_num, valid_num, patch_size;
	ifs.read((char*)&patch_num, sizeof(int));
	ifs.read((char*)&node_num, sizeof(int));
	ifs.read((char*)&valid_num, sizeof(int));
	ifs.read((char*)&patch_size, sizeof(int));

	// �ގ��x�]���̐�
	for (int i = 0; i < patch_num; ++i)
	{
		std::unique_ptr<Node> node(new Node);
		std::vector<uchar> patch_vec;
		std::vector<int> weight_vec;

		// �p�b�`��pixel��
		int patch_pixel_num;
		ifs.read((char*)&patch_pixel_num, sizeof(int));

		// �p�b�`
		for (int j = 0; j < patch_pixel_num; ++j)
		{
			int p_vec;
			ifs.read((char*)&p_vec, sizeof(int));
			patch_vec.emplace_back((uchar)(p_vec));
		}

		// �d��
		for (int j = 0; j < patch_pixel_num; ++j)
		{
			int w_vec;
			ifs.read((char*)&w_vec, sizeof(int));
			weight_vec.emplace_back(w_vec);
		}

		// std::vector����cv::Mat��
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

		// 臒l
		float split_th;
		ifs.read((char*)&split_th, sizeof(float));
		node->threshold = split_th;

		splitNode.emplace_back(std::move(node));
	}


	// �L���Ȗ��[�m�[�h��
	std::vector<int> indices;
	for (int i = 0; i < valid_num; ++i)
	{
		// �m�[�h�ԍ�
		int valid_node;
		ifs.read((char*)&valid_node, sizeof(int));
		indices.emplace_back(valid_node);

		// �f�[�^�̊i�[
		std::unique_ptr<EndNodeData> data(new EndNodeData);

		// �N���X�m��
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

		// �I�t�Z�b�g
		for (int j = 0; j < size; ++j)
		{
			float offset_x, offset_y;
			ifs.read((char*)&offset_x, sizeof(float));
			ifs.read((char*)&offset_y, sizeof(float));
			data->offsetList.emplace_back(cv::Point2f(offset_x, offset_y));
		}


		// ��
		for (int j = 0; j < size; ++j)
		{
			float axis_x, axis_y, axis_z;
			ifs.read((char*)&axis_x, sizeof(float));
			ifs.read((char*)&axis_y, sizeof(float));
			ifs.read((char*)&axis_z, sizeof(float));
			data->axisList.emplace_back(glm::vec3(axis_x, axis_y, axis_z));
		}


		// �p�x
		for (int j = 0; j < size; ++j)
		{
			float angle_x, angle_y;
			ifs.read((char*)&angle_x, sizeof(float));
			ifs.read((char*)&angle_y, sizeof(float));
			data->angleList.emplace_back(glm::vec2(angle_x, angle_y));
		}

		nodeData.emplace_back(std::move(data));
	}


	// ���[�m�[�h�ɃA�N�Z�X���邽�߂̘A�z�z��
	nodeLUT.resize(node_num, -1);
	for (int i = 0; i < valid_num; ++i)
	{
		nodeLUT[indices[i]] = i;
	}

	return true;
}


/**
 * @brief   Fern��ǂݍ���(�e�L�X�g�`��)
 * 
 * @param   fileName[in]	����؂̃t�@�C��
 * 
 * @returns true or false  
 */
bool SecondLayerFern::loadFern_txt(const std::string &fileName)
{
	// ������
	splitNode.clear();
	nodeData.clear();
	nodeLUT.clear();

	// �t�@�C���̓ǂݍ���
	std::ifstream ifs(fileName);

	if( !ifs ) {
		std::cerr << "�t�@�C����ǂݍ��߂܂���" << std::endl;
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

	// �w�b�_�̓ǂݍ���
	int patch_num, node_num, valid_num, patch_size;
	patch_num = atoi(params[0][0].c_str());
	node_num = atoi(params[1][0].c_str());
	valid_num = atoi(params[2][0].c_str());
	patch_size = atoi(params[3][0].c_str());

	int count_num = 4;

	// �ގ��x�]���̐�
	for (int i = 0; i < patch_num; ++i)
	{
		std::unique_ptr<Node> node(new Node);
		std::vector<uchar> patch_vec;
		std::vector<int> weight_vec;

		// �p�b�`
		for (int j = 0; j < (int)params[count_num].size(); ++j)
		{
			patch_vec.emplace_back((uchar)atoi(params[count_num][j].c_str()));
		}
		count_num++;

		// �d��
		for (int j = 0; j < (int)params[count_num].size(); ++j)
		{
			weight_vec.emplace_back(atoi(params[count_num][j].c_str()));
		}
		count_num++;

		// std::vector����cv::Mat��
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

		// 臒l
		node->threshold = atof(params[count_num][0].c_str());
		count_num++;

		splitNode.emplace_back(std::move(node));
	}


	// �L���Ȗ��[�m�[�h��
	std::vector<int> indices;
	for (int i = 0; i < valid_num; ++i)
	{
		indices.emplace_back(atoi(params[count_num][0].c_str()));

		count_num++;

		// �f�[�^�̊i�[
		std::unique_ptr<EndNodeData> data(new EndNodeData);

		// �N���X�m��
		for (int j = 0; j < (int)params[count_num].size(); ++j)
		{
			data->distribution.emplace_back(atof(params[count_num][j].c_str()));
		}
		count_num++;

		// �I�t�Z�b�g
		for (int j = 0; j < (int)params[count_num].size(); j+=2)
		{
			data->offsetList.emplace_back(cv::Point2f(atof(params[count_num][j].c_str()), atof(params[count_num][j+1].c_str())));
		}
		count_num++;

		// ��
		for (int j = 0; j < (int)params[count_num].size(); j+=3)
		{
			data->axisList.emplace_back(glm::vec3(atof(params[count_num][j].c_str()), atof(params[count_num][j+1].c_str()), atof(params[count_num][j+2].c_str())));
		}
		count_num++;

		// �p�x
		for (int j = 0; j < (int)params[count_num].size(); j+=2)
		{
			data->angleList.emplace_back(glm::vec2(atof(params[count_num][j].c_str()), atof(params[count_num][j+1].c_str())));
		}
		count_num++;

		nodeData.emplace_back(std::move(data));
	}


	// ���[�m�[�h�ɃA�N�Z�X���邽�߂̘A�z�z��
	nodeLUT.resize(node_num, -1);
	for (int i = 0; i < valid_num; ++i)
	{
		nodeLUT[indices[i]] = i;
	}

	return true;
}