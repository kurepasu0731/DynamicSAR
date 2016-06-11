#include "Detection_thread.h"



/**
 * @brief   ��������
 * 
 * @param   paramFile[in]	�p�����[�^�t�@�C��	
 * @param   saveFolder[in]	Ferns�̃t�H���_
 */
void DetectionThread::init(const std::string &paramFile, const std::string &saveFolder)
{

	// �p�����[�^�̓ǂݍ���
	hough_ferns->initFromFile(paramFile);
	hough_ferns->loadFerns(saveFolder);

	critical_section->ready_detect_flag = true;
}


/**
 * @brief   ���o		
 */
void DetectionThread::detection()
{
	// ���͉摜
	cv::Mat cam_img;
	irCamDev->getImage(cam_img);

	// �c�ݕ␳
	if(critical_section->use_calib_flag)
	{
		cv::Mat map1, map2;
		critical_section->getUndistortMap(map1, map2);
		cv::remap(cam_img, cam_img, map1, map2, cv::INTER_LINEAR);
	}

	///// �ʒu�p�����菈�� /////
	detectPoseMat.clear();

	// ���o�p�����[�^�̎擾
	float min_distance1;				// 1�w�ڂ̍l������ŏ�����
	float max_distance1;				// 1�w�ڂ̍l������ő勗��
	int distance_step;					// 1�w�ڂ̋����ω��̃X�e�b�v��
	float min_scale2;					// 2�w�ڂ̍l������ŏ��T�C�Y
	float max_scale2;					// 2�w�ڂ̍l������ő�T�C�Y
	int scale_step;						// 2�w�ڂ̃X�P�[���̃X�e�b�v��
	int detect_width;					// 2�w�ڂ̌��o�����`�̈�
	int detect_height;					// 2�w�ڂ̌��o�����`�̈�
	float grad_th;						// �P�x���z��臒l
	int meanshift_radius;				// Mean Shift�̃J�[�l����
	int win_th;							// Nearest Neighbor�œ�������T�C�Y
	double likelihood_th;				// ���[�ɂ��ޓx��臒l

	critical_section->getDetectParams(min_distance1, max_distance1, distance_step, min_scale2, max_scale2, scale_step,
									detect_width, detect_height, grad_th, meanshift_radius, win_th, likelihood_th);

	// Hough Ferns�ɂ�錟�o
	hough_ferns->detect(cam_img, min_distance1, max_distance1, distance_step, min_scale2, max_scale2, scale_step,
									detect_width, detect_height, grad_th, meanshift_radius, win_th, likelihood_th);

	// �ʒu�p������
	resultPose( hough_ferns->detectPoint, hough_ferns->pose_estimate,  cam_img.cols, cam_img.rows);

	// ���ʂ̃Z�b�g
	critical_section->setDetectPoseMatrix(detectPoseMat);

	// �g���b�L���O�J�n
	if(critical_section->detect_tracking_flag)
	{
		critical_section->tracking_flag = true;
	}
}



/**
 * @brief   �ʒu�p�����o���ʂ̕`��
 * 
 * @param   detectPoint[in]		���o���W
 * @param   detectPose[in]		���o�����ʒu�ɑ΂���p��
 * @param   width[in]			�E�B���h�E�̕�
 * @param   height[in]			�E�B���h�E�̍���	
 */
void DetectionThread::resultPose(const std::vector<cv::Point3f> &detectPoint, const std::vector<glm::mat4> &detectPose, int width, int height)
{
	// �ʒu�p�����茋�ʂ�`��
	for (int i = 0; i < (int)detectPoint.size(); ++i)
	{
		// �p��
		glm::mat4 estimate = detectPose[i];

		// xy���W�����[���h���W�ɕϊ�
		glm::mat4 projection = critical_section->getCameraIntrinsicMatrix();
		glm::mat4 view(1.0f);

		// �r���[�|�[�g�t�ϊ�
		glm::vec4 device_vec;
		device_vec.w = detectPoint[i].z;
		device_vec.x = device_vec.w * (float)( (2.0 * detectPoint[i].x / width) - 1.0);
		device_vec.y = device_vec.w * (float)( (2.0 * (height-detectPoint[i].y) / height) - 1.0);
		device_vec.z = (float)( (camera.getNear()+camera.getFar())*device_vec.w - 2*camera.getNear()*camera.getFar()) / (camera.getNear() - camera.getFar());

		// ���K���f�o�C�X���W�n���烏�[���h���W�n�֕ϊ�
		glm::vec4 world_vec = glm::inverse(view) * glm::inverse(projection) * device_vec;

		estimate[3][0] = world_vec.x;
		estimate[3][1] = world_vec.y;
		estimate[3][2] = world_vec.z;


		// �ǉ�
		detectPoseMat.emplace_back(estimate);
	}
}