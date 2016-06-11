#include "Tracking_thread.h"


void TrackingThread::init(const std::string& modelFile, const std::string& trackingFile, const cv::Matx33f& _camMat, const glm::mat4& _cameraMat)
{
	// �w�i�F
	glClearColor(1.f, 1.f, 1.f, 1.f);

	// �f�v�X�e�X�g
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// �J�����O
	glEnable(GL_CULL_FACE);

	// �|�C���g�X�v���C�g�̐ݒ�
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);

	// �I�t�X�N���[�������_�����O�̏�����
	offscreen1.init(offscreen1.offscreen_width, offscreen1.offscreen_height);
	offscreen2.init(offscreen2.offscreen_width, offscreen2.offscreen_height);
	offscreen3.init(offscreen3.offscreen_width, offscreen3.offscreen_height);
	offscreen4.init(offscreen4.offscreen_width, offscreen4.offscreen_height);

	
	// �����p�����[�^�̊i�[
	cameraMat = _cameraMat;
	camMat = _camMat;


	///// ���̃��f���p /////

	// �v���O�����I�u�W�F�N�g���쐬����
	init_GLSL( &program_orig, "../../Shader/shading.vert", "../../Shader/shading.frag");
	glUseProgram(program_orig);

	// uniform�ϐ��̏ꏊ���擾
	MatrixID = glGetUniformLocation(program_orig, "MVP");
	ViewMatrixID = glGetUniformLocation(program_orig, "V");
	ModelMatrixID = glGetUniformLocation(program_orig, "M");

	// �����ʒu�̎擾
	LightID = glGetUniformLocation(program_orig, "LightPosition_worldspace");
	LightPowerID = glGetUniformLocation(program_orig, "LightPower");

	// ���f���̓ǂݍ���
	orig_mesh.loadMesh(modelFile);


	///// CAD���f���G�b�W���o�p /////
	init_GLSL( &program_CADedge, "../../Shader/diffgauss.vert", "../../Shader/diffgauss.frag");
	glUseProgram(program_CADedge);

	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	edge_texID = glGetUniformLocation(program_CADedge, "inputImage");
	edge_MatrixID = glGetUniformLocation(program_CADedge, "MVP");
	edge_locW = glGetUniformLocation(program_CADedge,"imageWidth");
	edge_locH = glGetUniformLocation(program_CADedge,"imageHeight");

	// uniform�ϐ��ɒl��n��
	glUniform1i(edge_locW, windowWidth);
	glUniform1i(edge_locH, windowHeight); 

	// ���b�V���̓ǂݍ���
	edge_rect.init();
	edge_rect.resizeRectangle( windowWidth, windowHeight);


	///// �g���b�L���O���f���p /////

	//	�v���O�����I�u�W�F�N�g���쐬����
	init_GLSL( &program_point, "../../Shader/edge.vert", "../../Shader/edge.frag");
	glUseProgram(program_point);

	// uniform�ϐ��̏ꏊ���擾
	pointMatrixID = glGetUniformLocation(program_point, "MVP");
	pointViewMatrixID = glGetUniformLocation(program_point, "V");
	pointModelMatrixID = glGetUniformLocation(program_point, "M");
	
	// �_�̑傫��
	GLuint pointSizeID = glGetUniformLocation(program_point, "point_size");
	GLfloat p_size = 0.5f;
	glUniform1f(pointSizeID, p_size);

	// �g���b�L���O�p�̃t�@�C���̓ǂݍ���
	pointCloud.clear();
	pointNormal.clear();
	loadPointCloudPly(trackingFile, pointCloud, pointNormal);

	// �C���f�b�N�X�͓��͔z��̏��Ԃ����̂܂ܗp����
	std::vector<unsigned int> pointIndex(pointCloud.size());
	std::vector<glm::vec3> pointIndexF(pointCloud.size());		// ������C���f�b�N�X��256*256*256�܂�

	// �����_�[�o�b�t�@��RGB���C���f�b�N�X�Ɋ��蓖�Ă�
	for (int i = 0; i < pointCloud.size(); ++i){
		pointIndex[i] = i;
		int i1 = i / (256*256);
		int i2 = i / 256;
		int i3 = i % 256;
		pointIndexF[i] = glm::vec3((float)i1/255.f,(float)i2/255.f,(float)i3/255.f);
	}

	// ���f���̓ǂݍ���
	// VBO�̐���
    glGenBuffers(3, pointVBO);
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*pointCloud.size(), &pointCloud[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, pointVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*pointNormal.size(), &pointNormal[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, pointVBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*pointIndexF.size(), &pointIndexF[0], GL_STATIC_DRAW);

	// IBO�̐���
    glGenBuffers(1, &pointIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pointIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*pointIndex.size(), &pointIndex[0], GL_STATIC_DRAW);

	//�@�o�C���h�������̂����ǂ�
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	///// �V�F�[�_�G�t�F�N�g�p /////

	//	�v���O�����I�u�W�F�N�g���쐬����
	init_GLSL( &program_image, "../../Shader/image_rect.vert", "../../Shader/image_rect.frag");
	glUseProgram(program_image);

	// �V�F�[�_��uniform�ϐ��̈ʒu���擾
	tex0ID = glGetUniformLocation(program_image, "inputImage0");
	tex1ID = glGetUniformLocation(program_image, "inputImage1");
	tex2ID = glGetUniformLocation(program_image, "inputImage2");
	tex3ID = glGetUniformLocation(program_image, "inputImage3");
	imageMatrixID = glGetUniformLocation(program_image, "MVP");
	locW = glGetUniformLocation(program_image,"imageWidth");
	locH = glGetUniformLocation(program_image,"imageHeight");

	// uniform�ϐ��ɒl��n��
	glUniform1i(locW, windowWidth);
	glUniform1i(locH, windowHeight); 

	// ���b�V���̓ǂݍ���
	image_rect.init();
	image_rect.resizeRectangle( windowWidth, windowHeight);

	///// ���͉摜�p /////

	// �w�i���b�V���̐���
	input_mesh.init(inputImg, "../../Shader/background.vert", "../../Shader/background.frag", windowWidth, windowHeight);


	///// �G�b�W�`��p /////
	//	�v���O�����I�u�W�F�N�g���쐬����
	init_GLSL( &program_edge, "../../Shader/point.vert", "../../Shader/point.frag");
	glUseProgram(program_edge);

	// uniform�ϐ��̏ꏊ���擾
	pMatrixID = glGetUniformLocation(program_edge, "MVP");
	
	// �_�̑傫��
	glUseProgram(program_edge);
	GLuint psizeID = glGetUniformLocation(program_edge, "point_size");
	GLfloat psize = 0.5f;
	glUniform1f(psizeID, psize);

	// VBO�̐���
    glGenBuffers(1, &pVBO);
    glGenBuffers(1, &pIBO);
}


// ���t���[���s������
void TrackingThread::display()
{
	// �p�����[�^�̎擾
	critical_section->getTrackingParams(find_distance, error_th, edge_th1, edge_th2, trackingTime, delayTime);

	///// �g���b�L���O /////
	tracking();

	///// �V�[���̕`�� /////

	// �E�B���h�E����������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, windowWidth, windowHeight);	
	// �w�i�F
	glClearColor(1.f, 1.f, 1.f, 1.f);

	if (pointEdge.size() > 0) {
		//edgeTracker->scene_orig(windowWidth, windowHeight);
		draw_point(pointEdge, windowWidth, windowHeight);
	}

	// �w�i
	scene_background(windowWidth, windowHeight);

	// �J���[�o�b�t�@�����ւ�,�C�x���g���擾
	swapBuffers();
}


// ���f���x�[�X�̃g���b�L���O
void TrackingThread::tracking()
{
	// ���͉摜�̍X�V
	cv::Mat cap;
	irCamDev->captureImage(cap);

	// �c�ݕ␳
	if(critical_section->use_calib_flag)
	{
		cv::Mat map1, map2;
		critical_section->getUndistortMap(map1, map2);
		cv::remap(cap, cap, map1, map2, cv::INTER_LINEAR);
	}

	// ������Ԃł����
	if (!critical_section->tracking_flag) {
		setInitPose();
	}
	
	/*double start, finish, time;
	start = static_cast<double>(cv::getTickCount());*/

	for (int i = 0; i < 1; ++i)
	{
		inputImg = cap.clone();
		
		// �I�t�X�N���[����3D���f����3�����_����ѓ��e���2�����_�̎擾
		offscreen_render(windowWidth, windowHeight);
		
		// 3D���f���̗֊s���̒��o
		extractModelEdge();

		// ���͉摜�̃G�b�W�̒��o
		extractInputEdge();

		// ���͉摜�Ƃ̑Ή������߂�
		estimateCorrespondence();

		// �p���̍X�V
		if (critical_section->tracking_flag) {	

			// IRLS�ɂ��p������
			estimatePoseIRLS();
		}
	}

	/*finish = static_cast<double>(cv::getTickCount());
	time = ( ( finish - start ) / cv::getTickFrequency() );
	std::cout << "�������� : " << time*1000 << "ms" << std::endl;*/

	// �G���[���̌v�Z
	calcError();
}


// �I�t�X�N���[���ł̕`��
void TrackingThread::offscreen_render(int width, int height)
{
	///// 1�p�X�� /////
	offscreen1.startRender();

	// �E�B���h�E����������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, width, height);
	glClearColor(1.f, 1.f, 1.f, 1.f);	// �w�i�F

	// ���̃��f���̕`��
	scene_orig(width, height);

	offscreen1.endRender();


	///// 2�p�X�� /////
	offscreen2.startRender();

	// �E�B���h�E����������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, width, height);
	glClearColor(1.f, 1.f, 1.f, 1.f);	// �w�i�F

	glUseProgram(program_CADedge);

	// ���s���e
	glm::mat4 OrthoMatrix0 = glm::ortho(0.f, (float)width, (float)height, 0.f, -1.f, 1.f);

	// Uniform�ϐ��ɍs��𑗂�
	glUniformMatrix4fv(edge_MatrixID, 1, GL_FALSE, &OrthoMatrix0[0][0]);
	glUniform1i(edge_texID, 0);
	glUniform1i(edge_locW, width);
	glUniform1i(edge_locH, height);

	// ����
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, offscreen1.texID[0]);		// �����f���̉摜

	// �G�b�W�`��
	edge_rect.draw();

	glBindTexture(GL_TEXTURE_2D, 0);

	offscreen2.endRender();


	///// 3�p�X�� /////
	offscreen3.startRender();

	// �E�B���h�E����������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, width, height);
	glClearColor(1.f, 1.f, 1.f, 1.f);	// �w�i�F

	// �g���b�L���O���f���̕`��
	scene_point(width, height);

	offscreen3.endRender();


	///// 4�p�X�� /////
	offscreen4.startRender();

	// �E�B���h�E����������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, width, height);
	glClearColor(1.f, 1.f, 1.f, 1.f);	// �w�i�F

	glUseProgram(program_image);

	// ���s���e
	glm::mat4 OrthoMatrix = glm::ortho(0.f, (float)width, (float)height, 0.f, -1.f, 1.f);

	// Uniform�ϐ��ɍs��𑗂�
	glUniformMatrix4fv(imageMatrixID, 1, GL_FALSE, &OrthoMatrix[0][0]);
	glUniform1i(tex0ID, 0);
	glUniform1i(tex1ID, 1);
	glUniform1i(tex2ID, 2);
	glUniform1i(tex3ID, 3);
	glUniform1i(locW, width);
	glUniform1i(locH, height);

	// ����
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, offscreen1.texID[1]);		// �����f���̃f�v�X
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, offscreen2.texID[0]);		// �G�b�W�摜
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, offscreen3.texID[0]);		// �g���b�L���O���f���̒��_�C���f�b�N�X
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, offscreen3.texID[1]);		// �g���b�L���O���f���̃f�v�X

	image_rect.draw();

	glBindTexture(GL_TEXTURE_2D, 0);

	// �G�b�W�_�̃����_�����O	
	offscreen4.getRenderRGB(indexImg);

	offscreen4.endRender();
}


// 3D���f���̗֊s���̌��o
void TrackingThread::extractModelEdge()
{
	// ������
	pointEdge.clear();
	correspondPoints.clear();

	// 3�����_�Ɠ��e���2�����_�̎擾
	for (int r = 0; r < indexImg.rows; ++r) 
	{
		cv::Vec3b* index_p = indexImg.ptr<cv::Vec3b>(r);
		for (int c = 0; c < indexImg.cols; ++c) 
		{
			int B = index_p[c][0];
			int G = index_p[c][1];
			int R = index_p[c][2];

			// ���ȊO�̃s�N�Z�������o��
			if ( R != 255 && G != 255 && B != 255) {
				int index = R*256*256 + G*256 + B;
				Correspond model_point;

				// 3�������W�l
				model_point.model3D = cv::Point3f(pointCloud[index].x, pointCloud[index].y, pointCloud[index].z);

				// 2�����ւ̓��e
				cv::Vec3f normal3d = cv::Point3f(pointNormal[index].x, pointNormal[index].y, pointNormal[index].z);
				cv::Vec2f point2d;
				cv::Vec2f normal2d;
				projection3Dto2D( model_point.model3D, normal3d, point2d, normal2d);

				model_point.model2D = point2d;
				model_point.normal2D = normal2d;

				// ���e�_���͈͓��ł���Βǉ�
				if (point2d[0] >= 0 && point2d[0] < windowWidth && point2d[1] >= 0 && point2d[1] < windowHeight &&
					normal2d[0] != std::numeric_limits<float>::infinity() && normal2d[0] != -std::numeric_limits<float>::infinity()) 
				{
					correspondPoints.emplace_back (model_point);
					pointEdge.emplace_back(pointCloud[index]);
				}
			}
		}

	}
}


// ���͉摜�̃G�b�W���o
void TrackingThread::extractInputEdge()
{
	// �O���C�X�P�[��
	cv::Mat grayImg;
	if (inputImg.channels() == 3){
		cv::cvtColor(inputImg, grayImg, CV_RGB2GRAY);
	}else {
		grayImg = inputImg.clone();
	}

	// �K�E�V�A���t�B���^
	//cv::GaussianBlur(grayImg, grayImg, cv::Size(3,3), 3, 3);

	// �G�b�W���o
	cv::Canny (grayImg, cannyImg, edge_th1, edge_th2);

	// Sobel�t�B���^
	cv::Sobel(grayImg, sobelImgX, CV_32F, 1, 0, 3, 1.0, 0.0, cv::BORDER_REPLICATE);
	cv::Sobel(grayImg, sobelImgY, CV_32F, 0, 1, 3, 1.0, 0.0, cv::BORDER_REPLICATE);

	//cv::imshow("j", cannyImg);
}


// ���͉摜�Ƃ̑Ή��𐄒�
void TrackingThread::estimateCorrespondence()
{
	edge_img = cannyImg.clone();
	cv::cvtColor(edge_img, edge_img, CV_GRAY2BGR);

	// cm����s�N�Z���P�ʂւ̕ϊ�
	max_distance = std::abs(camMat(0,0) * find_distance / estimatedPose(2,3));

	// 3D���f���̃G�b�W�_�Ƃ̑Ή��_�̒T��
	for (int i = 0; i < static_cast<int>(correspondPoints.size()); ++i)
	{
		float model_normal_x = correspondPoints[i].normal2D.x;
		float model_normal_y = correspondPoints[i].normal2D.y;

		// �T������(3D���f���̃G�b�W�̖@������)
		float find_vec_rad = std::atan2f(model_normal_y, model_normal_x);
		float dx = cos(find_vec_rad);
		float dy = sin(find_vec_rad);

		// 3D���f����2�����_�̈ʒu
		int x = static_cast<int>(correspondPoints[i].model2D.x);
		int y = static_cast<int>(correspondPoints[i].model2D.y);

		correspondPoints[i].normalUnit = cv::Point2f(dx, dy);
		correspondPoints[i].dist = max_distance;

		// �@��������T��
		for (int j = 0; j < static_cast<int>(max_distance); ++j)
		{
			///// �������ւ̒T�� /////
			float point_x = correspondPoints[i].model2D.x + dx * (float)j;
			float point_y = correspondPoints[i].model2D.y + dy * (float)j;
			x = static_cast<int>(point_x + 0.5);
			y = static_cast<int>(point_y + 0.5);

			// �͈͓������͉摜�̃G�b�W�_�ł����
			if ( x >= 0 && x < windowWidth && y >= 0 && y < windowHeight && cannyImg.at<uchar>(y,x) > uchar(50) )
			{
				float input_edge_x = sobelImgX.at<float>(y,x);
				float input_edge_y = sobelImgY.at<float>(y,x);

				// ���͉摜�̃G�b�W�̌��z������3D���f���̖@���̓���
				float edge_dot = input_edge_x * model_normal_x + input_edge_y * model_normal_y;
				// �m�����̐�
				float edge_norm = std::sqrtf((input_edge_x*input_edge_x + input_edge_y*input_edge_y) * (model_normal_x*model_normal_x + model_normal_y*model_normal_y));
				// �R�T�C���ގ��x
				float cos_theta = edge_dot / edge_norm;

				// ���z������90���ȓ��Ɏ��܂�Ό��o
				if ( cos_theta >= 0.7071 || cos_theta <= -0.7071)
				{
					correspondPoints[i].dist = std::sqrt( (double)j*dx*(double)j*dx + (double)j*dy*(double)j*dy );
					correspondPoints[i].imageEdge2D = cv::Point2f(point_x,point_y);
					break;
				}
			}

			// 0�̎��͈ȉ��̏������X�L�b�v
			if(j == 0) {
				continue;
			}

			///// �������ւ̒T�� /////
			point_x = correspondPoints[i].model2D.x - dx * (float)j;
			point_y = correspondPoints[i].model2D.y - dy * (float)j;
			x = static_cast<int>(point_x + 0.5);
			y = static_cast<int>(point_y + 0.5);

			// �͈͓������͉摜�̃G�b�W�_�ł����
			if ( x >= 0 && x < windowWidth && y >= 0 && y < windowHeight && cannyImg.at<uchar>(y,x) > uchar(50) )
			{
				float input_edge_x = sobelImgX.at<float>(y,x);
				float input_edge_y = sobelImgY.at<float>(y,x);

				// ���͉摜�̃G�b�W�̌��z������3D���f���̖@���̓���
				float edge_dot = input_edge_x * model_normal_x + input_edge_y * model_normal_y;
				// �m�����̐�
				float edge_norm = std::sqrtf((input_edge_x*input_edge_x + input_edge_y*input_edge_y) * (model_normal_x*model_normal_x + model_normal_y*model_normal_y));
				// �R�T�C���ގ��x
				float cos_theta = edge_dot / edge_norm;			

				// ���z������90���ȓ��Ɏ��܂�Ό��o
				if ( cos_theta >= 0.7071 || cos_theta <= -0.7071)
				{
					correspondPoints[i].dist = std::sqrt( (double)j*dx*(double)j*dx + (double)j*dy*(double)j*dy );
					correspondPoints[i].imageEdge2D = cv::Point2f(point_x,point_y);

					// �@���̒P�ʃx�N�g���̔��]
					correspondPoints[i].normalUnit = -correspondPoints[i].normalUnit;
					break;
				}
			}
		}

		// �m�F�p
		cv::circle(edge_img, correspondPoints[i].model2D, 3, cv::Scalar(0,0,200), -1, CV_AA);

		if (correspondPoints[i].dist < max_distance) {
			cv::line(inputImg, correspondPoints[i].model2D, correspondPoints[i].imageEdge2D, cv::Scalar(200,0,0), 2, CV_AA);

			// �m�F�p
			cv::line(edge_img, correspondPoints[i].model2D, correspondPoints[i].imageEdge2D, cv::Scalar(200,0,0), 2, CV_AA);
		}
	}
}


// ���R�r�A���̌v�Z
TooN::Vector<6> TrackingThread::calcJacobian(const cv::Point3f& pts3, const cv::Point2f& pts2, const cv::Point2f& ptsnv, double ptsd, const TooN::SE3& E)
{
	TooN::Vector<4> vpts3;	// 3�����_
	TooN::Vector<3> vpts2;	// 2�����_
	TooN::Vector<2> vptsn;	// �@���̒P�ʃx�N�g��
	TooN::Vector<6> J;
	TooN::Matrix<2,2> ja_;

	// ������
	vpts3 = pts3.x, pts3.y, pts3.z, 1.0;
	vpts2 = pts2.x, pts2.y, 1.0;
	vptsn = ptsnv.x, ptsnv.y;

	ja_[0][0] = -static_cast<double>(camMat(0,0)); ja_[0][1] = 0.0;
	ja_[1][1] = static_cast<double>(camMat(1,1)); ja_[1][0] = 0.0;

	for(int i = 0; i < 6; i++)
	{
		TooN::Vector<4> cam_coord = E * vpts3;
		TooN::Vector<4> temp = E * TooN::SE3::generator_field(i, vpts3);
		TooN::Vector<2> temp2 = temp.slice<0,2>() / cam_coord[2] - cam_coord.slice<0,2>() * (temp[2]/cam_coord[2]/cam_coord[2]);
		J[i] = vptsn*ja_*temp2;
	}

	return J;
}


// IRLS�ɂ��p������
void TrackingThread::estimatePoseIRLS()
{
	double m_prev[4][4];
	for(int r = 0; r < 4; ++r) {
		for(int c = 0; c < 4; ++c) {
			m_prev[r][c] = estimatedPose(r,c);
		}
	}

	TooN::Matrix<4> M(m_prev);
	TooN::SE3 se3_prev;
	se3_prev = M;

	TooN::WLS<6> wls;
	for (int i = 0; i< (int)correspondPoints.size(); ++i)
	{
		if(correspondPoints[i].dist < max_distance)
		{
			wls.add_df( correspondPoints[i].dist,
						calcJacobian(correspondPoints[i].model3D, correspondPoints[i].model2D, correspondPoints[i].normalUnit, correspondPoints[i].dist, se3_prev),
						1/((1.0 + abs(correspondPoints[i].dist))));
		}
	}

	wls.compute();

	TooN::Vector<6> mu = wls.get_mu();
	TooN::Matrix<6> inv_cov = (wls.get_C_inv()) ;

	// ������
	cv::Mat cov = cv::Mat::eye(6, 6, CV_32F)*0.1;
	for(int i = 0; i < 6; ++i) {
		for(int j = 0; j < 6; ++j){
			covariance(i,j) = inv_cov(i,j);
		}
	}

	// �t�s��
	covariance = covariance.inv();
	cv::add(covariance, cov, covariance);

	TooN::SE3 se3_cur;
	se3_cur = se3_prev * TooN::SE3::exp(mu);

	TooN::Matrix<3> rot = se3_cur.getRot();
	TooN::Vector<3> trans = se3_cur.getTrans();


	// ��]
	glm::mat4 rotation( rot(0,0), rot(1,0), rot(2,0), 0.0f,
						rot(0,1), rot(1,1), rot(2,1), 0.0f,
						rot(0,2), rot(1,2), rot(2,2), 0.0f,
						0.f, 0.f, 0.f, 1.f);


	///// �\���t�B���^ //////
	// �V�����ʒu�p���̊i�[
	timer.stop();
	predict_point->addData(timer.MSec(), cv::Point3f(trans[0],trans[1],trans[2]));
	predict_quat->addData(timer.MSec(), glm::quat_cast(rotation));

	// ���e�\���ʒu
	cv::Point3f predict_point2 = predict_point->calcYValue(timer.MSec()+trackingTime);

	// �x���⏞�����ʒu�p��
	glm::mat4 predictPose = glm::mat4_cast(predict_quat->calcYValue(timer.MSec()+trackingTime));
	predictPose[3][0] = predict_point2.x;
	predictPose[3][1] = predict_point2.y;
	predictPose[3][2] = predict_point2.z;

	// �\���t�B���^�g�p��
	if (critical_section->predict_flag && !firstTime)
	{
		// �p���̍X�V
		estimatedPose(0, 0) = predictPose[0][0];
		estimatedPose(0, 1) = predictPose[1][0];
		estimatedPose(0, 2) = predictPose[2][0];
		estimatedPose(0, 3) = predictPose[3][0];
		estimatedPose(1, 0) = predictPose[0][1];
		estimatedPose(1, 1) = predictPose[1][1];
		estimatedPose(1, 2) = predictPose[2][1];
		estimatedPose(1, 3) = predictPose[3][1];
		estimatedPose(2, 0) = predictPose[0][2];
		estimatedPose(2, 1) = predictPose[1][2];
		estimatedPose(2, 2) = predictPose[2][2];
		estimatedPose(2, 3) = predictPose[3][2];
		estimatedPose(3, 0) = 0.0f;
		estimatedPose(3, 1) = 0.0f;
		estimatedPose(3, 2) = 0.0f;
		estimatedPose(3, 3) = 1.0f;
	}
	else
	{
		// �p���̍X�V
		estimatedPose(0, 0) = static_cast<float>(rot(0,0));
		estimatedPose(0, 1) = static_cast<float>(rot(0,1));
		estimatedPose(0, 2) = static_cast<float>(rot(0,2));
		estimatedPose(0, 3) = static_cast<float>(trans[0]);
		estimatedPose(1, 0) = static_cast<float>(rot(1,0));
		estimatedPose(1, 1) = static_cast<float>(rot(1,1));
		estimatedPose(1, 2) = static_cast<float>(rot(1,2));
		estimatedPose(1, 3) = static_cast<float>(trans[1]);
		estimatedPose(2, 0) = static_cast<float>(rot(2,0));
		estimatedPose(2, 1) = static_cast<float>(rot(2,1));
		estimatedPose(2, 2) = static_cast<float>(rot(2,2));
		estimatedPose(2, 3) = static_cast<float>(trans[2]);
		estimatedPose(3, 0) = 0.0f;
		estimatedPose(3, 1) = 0.0f;
		estimatedPose(3, 2) = 0.0f;
		estimatedPose(3, 3) = 1.0f;
	}

	firstTime = true;

	///// �ʒu�p���X�V /////
	glm::mat4 trackingPose( estimatedPose(0,0), estimatedPose(1,0), estimatedPose(2,0), 0,
							estimatedPose(0,1), estimatedPose(1,1), estimatedPose(2,1), 0, 
							estimatedPose(0,2), estimatedPose(1,2), estimatedPose(2,2), 0, 
							estimatedPose(0,3), estimatedPose(1,3), estimatedPose(2,3), 1);

	
	critical_section->setTrackingPoseMatrix(trackingPose);


	///// �x���⏞ /////	

	// �V�����ʒu�p���̊i�[
	timer.stop();
	compensate_point->addData(timer.MSec(), cv::Point3f(trans[0],trans[1],trans[2]));
	compensate_quat->addData(timer.MSec(), glm::quat_cast(rotation));

	// ���e�\���ʒu
	cv::Point3f compensate_point2 = compensate_point->calcYValue(timer.MSec()+delayTime);

	// �x���⏞�����ʒu�p��
	glm::mat4 compensatePose = glm::mat4_cast(compensate_quat->calcYValue(timer.MSec()+delayTime));
	compensatePose[3][0] = compensate_point2.x;
	compensatePose[3][1] = compensate_point2.y;
	compensatePose[3][2] = compensate_point2.z;

	critical_section->setCompensatePoseMatrix(compensatePose);
}


// �G���[���̐���
void TrackingThread::calcError()
{
	float sum = 0.f;
	float max_error = max_distance * (int)correspondPoints.size();

	// ���a
	for (int i = 0; i < (int)correspondPoints.size(); ++i){
		sum += correspondPoints[i].dist;
	}

	// �G���[����臒l�ȏ�ɂȂ�����g���b�L���O�I��
	if ( sum / max_error > error_th || sum < 0.00001) {
		critical_section->tracking_flag = false;
		critical_section->tracking_success_flag = false;
	}
	else if(critical_section->detect_tracking_flag){
		critical_section->tracking_flag = true;
		critical_section->tracking_success_flag = true;
	}
}

// �`��V�[��(�����f��)
void TrackingThread::scene_orig(int width, int height)
{	
	///// ���̃��f���p /////
	glEnable(GL_POLYGON_OFFSET_FILL);	// �͂��ɉ��ɂ��炷
	glPolygonOffset(1.f, 1.f);

	glUseProgram(program_orig);

	// �J�������W�n�֕ϊ�
	glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);
	glm::mat4 ViewMatrix = camera.updateViewMatrix(camera.eyePosition, camera.eyeVector, camera.eyeUp);

	// ���f���̍��W�ϊ�
	glm::mat4 ModelMatrix = critical_section->getModelMatrix();

	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	// �����p�����[�^���g���ꍇ
	if(critical_section->use_calib_flag)
	{
		MVP = cameraMat * ViewMatrix * ModelMatrix;
	}

	// Uniform�ϐ��ɍs��𑗂�
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(LightPowerID, critical_section->getLightPower());

	// ���f���̃����_�����O
	orig_mesh.render();
	glDisable(GL_POLYGON_OFFSET_FILL);
}


// �`��V�[��(�g���b�L���O���f��)
void TrackingThread::scene_point(int width, int height)
{	
	///// �g���b�L���O���f���p /////
	glUseProgram(program_point);

	// �J�������W�n�֕ϊ�
	glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);
	glm::mat4 ViewMatrix = camera.updateViewMatrix(camera.eyePosition, camera.eyeVector, camera.eyeUp);

	// ���f���̍��W�ϊ�
	glm::mat4 ModelMatrix = critical_section->getModelMatrix();

	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	// �����p�����[�^���g���ꍇ
	if(critical_section->use_calib_flag)
	{
		MVP = cameraMat * ViewMatrix * ModelMatrix;
	}

	glUniformMatrix4fv(pointMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(pointModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(pointViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);


	// �|�C���g�X�v���C�g�ɂ�郌���_�����O
	// �V�F�[�_�̕ϐ��ɒ��_����Ή��t����
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// VBO�̎w��
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO[0]);
	// ���_���̊i�[�ꏊ�Ə����̎w��
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);			// ���_

	glBindBuffer(GL_ARRAY_BUFFER, pointVBO[1]);
	// ���_���̊i�[�ꏊ�Ə����̎w��
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);			// �@��

	glBindBuffer(GL_ARRAY_BUFFER, pointVBO[2]);
	// ���_���̊i�[�ꏊ�Ə����̎w��
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);			// �C���f�b�N�X

	// IBO�̎w��
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pointIBO);

	glDrawElements(GL_POINTS, pointCloud.size(), GL_UNSIGNED_INT, (void*)0);

	// ���_�̑Ή��t���̉���
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

	//�@�o�C���h�������̂����ǂ�
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



// �`��V�[��(���͉摜)
void TrackingThread::scene_background(int width, int height)
{
	// �e�N�X�`���̍X�V
	input_mesh.textureUpdate(edge_img);
	glm::mat4 OrthoMatrix = glm::ortho(0.f, (float)windowWidth, (float)windowHeight, 0.f, -100.f, 1.f);		// far�N���b�v�ʋ߂��ŕ`��
	input_mesh.changeMatrix(OrthoMatrix);

	// �`��
	input_mesh.draw();
}




// ���_�̕`��
void TrackingThread::draw_point(const std::vector<glm::vec3>& point_cloud, int width, int height)
{
	glUseProgram(program_edge);

	// �J�������W�n�֕ϊ�
	glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), width, height);
	glm::mat4 ViewMatrix = camera.updateViewMatrix(camera.eyePosition, camera.eyeVector, camera.eyeUp);

	// ���f���̍��W�ϊ�
	glm::mat4 ModelMatrix = critical_section->getModelMatrix();

	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	// �����p�����[�^���g���ꍇ
	if(critical_section->use_calib_flag)
	{
		MVP = cameraMat * ViewMatrix * ModelMatrix;
	}

	glUniformMatrix4fv(pMatrixID, 1, GL_FALSE, &MVP[0][0]);


	std::vector<unsigned int> index_vect(point_cloud.size());
	for (int i = 0; i < point_cloud.size(); ++i){
		index_vect[i] = i;
	}

	// VBO�̐���
    glBindBuffer(GL_ARRAY_BUFFER, pVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*point_cloud.size(), &point_cloud[0], GL_DYNAMIC_DRAW);

	// IBO�̐���
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*index_vect.size(), &index_vect[0], GL_STATIC_DRAW);

	// �V�F�[�_�̕ϐ��ɒ��_����Ή��t����
	glEnableVertexAttribArray(0);

	// ���_���̊i�[�ꏊ�Ə����̎w��
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);			// ���_

	// IBO�̎w��
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIBO);

	glDrawElements(GL_POINTS, point_cloud.size(), GL_UNSIGNED_INT, 0);

	// ���_�̑Ή��t���̉���
    glDisableVertexAttribArray(0);

	//�@�o�C���h�������̂����ǂ�
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// �����p���̃Z�b�g
void TrackingThread::setInitPose()
{
	// ���f���̍��W�ϊ�
	glm::mat4 ModelMatrix = critical_section->getModelMatrix();

	// GLM����CV�ւ̕ϊ�
	estimatedPose = cv::Matx44f(  ModelMatrix[0][0], ModelMatrix[1][0], ModelMatrix[2][0], ModelMatrix[3][0]
								, ModelMatrix[0][1], ModelMatrix[1][1], ModelMatrix[2][1], ModelMatrix[3][1]
								, ModelMatrix[0][2], ModelMatrix[1][2], ModelMatrix[2][2], ModelMatrix[3][2]
								, ModelMatrix[0][3], ModelMatrix[1][3], ModelMatrix[2][3], ModelMatrix[3][3]);
}


// 2�������ʂւ̓��e
void TrackingThread::projection3Dto2D(const cv::Vec3f& point3D, const cv::Vec3f& normal3D, cv::Vec2f& point2D, cv::Vec2f& normal2D)
{
	// 3�����ʒu
	cv::Vec3f point3Dnow;
	point3Dnow[0] = estimatedPose(0,0)*point3D[0] + estimatedPose(0,1)*point3D[1] + estimatedPose(0,2)*point3D[2] + estimatedPose(0,3);
	point3Dnow[1] = estimatedPose(1,0)*point3D[0] + estimatedPose(1,1)*point3D[1] + estimatedPose(1,2)*point3D[2] + estimatedPose(1,3);
	point3Dnow[2] = estimatedPose(2,0)*point3D[0] + estimatedPose(2,1)*point3D[1] + estimatedPose(2,2)*point3D[2] + estimatedPose(2,3);

	// 2�����ʒu
	point2D[0] = -camMat(0,0) * point3Dnow[0] / point3Dnow[2] + camMat(0,2);
	point2D[1] = camMat(1,1) * point3Dnow[1] / point3Dnow[2] + camMat(1,2);

	// 3�����@��
	cv::Vec3f normal3Dnow;
	normal3Dnow[0] = estimatedPose(0,0)*normal3D[0] + estimatedPose(0,1)*normal3D[1] + estimatedPose(0,2)*normal3D[2] + estimatedPose(0,3);
	normal3Dnow[1] = estimatedPose(1,0)*normal3D[0] + estimatedPose(1,1)*normal3D[1] + estimatedPose(1,2)*normal3D[2] + estimatedPose(1,3);
	normal3Dnow[2] = estimatedPose(2,0)*normal3D[0] + estimatedPose(2,1)*normal3D[1] + estimatedPose(2,2)*normal3D[2] + estimatedPose(2,3);

	// 2�����@��
	normal2D[0] = -camMat(0,0) * normal3Dnow[0] / normal3Dnow[2] + camMat(0,2);
	normal2D[1] = camMat(1,1) * normal3Dnow[1] / normal3Dnow[2] + camMat(1,2);


	if (point3Dnow[2] == 0.f) {
		point2D[0] = point2D[1] = std::numeric_limits<float>::infinity();
	} else {
		float th_max = 10000.0;
		float th_min = -10000.0;
		if (point2D[0] < th_min || point2D[0] > th_max) {
			point2D[0] = std::numeric_limits<float>::infinity();
		} 
		if (point2D[1] < th_min || point2D[1] > th_max) {
			point2D[1] = std::numeric_limits<float>::infinity();
		} 
	}
}



/**
 * @brief   �_�Q�̓ǂݍ���(PLY�`���@�ʖ���)
 * 
 * @param   fileName[in]			�t�@�C����
 * @param   point_cloud[in,out]		3�����_�Q
 * @param   point_normal[in,out]	3�����_�Q�̖@��
 */
void TrackingThread::loadPointCloudPly(const std::string& fileName, std::vector<glm::vec3>& point_cloud, std::vector<glm::vec3>& point_normal)
{
	std::ifstream ifs(fileName);
	std::string buf;
	unsigned int i;

	// �t�@�C���I�[�v��
	if(!ifs.is_open()){
		std::cerr << "�t�@�C�����J���܂���F"<< fileName << std::endl;
		return;
	}

	// �w�b�_�̓ǂݍ���
	int vertex_size = 0;
	while(ifs >> buf)
	{
		if(buf =="vertex"){
			ifs >> vertex_size;
			if(ifs.fail()){
				std::cerr <<"error! vertices_num is not int"<<std::endl;
			}
		}
		if(buf == "end_header"){
			break;
		}
	
	}

	if(vertex_size == 0){
		std::cerr << "vertices_num is 0" << std::endl;
		return;
	}

	std::getline(ifs,buf);
	point_cloud.resize(vertex_size);
	point_normal.resize(vertex_size);

	// ���_�Ɩ@����ǂݍ���
	for (i = 0; i < vertex_size; ++i)
	{
		std::getline(ifs,buf);
		std::vector<std::string> offset = split(buf, ' ');

		point_cloud[i].x = (float)atof(offset[0].c_str());
		point_cloud[i].y = (float)atof(offset[1].c_str());
		point_cloud[i].z = (float)atof(offset[2].c_str());
		point_normal[i].x = (float)atof(offset[3].c_str());
		point_normal[i].y = (float)atof(offset[4].c_str());
		point_normal[i].z = (float)atof(offset[5].c_str());
	}
}


// ��؂蕶���ɂ�镪��
std::vector<std::string> TrackingThread::split(const std::string &str, char sep)
{
	std::vector<std::string> v;
	std::stringstream ss(str);
    std::string buffer;
    while( std::getline(ss, buffer, sep) ) {
        v.push_back(buffer);
    }
    return v;
}


// �J���[�o�b�t�@�����ւ��ăC�x���g�����o��
void TrackingThread::swapBuffers()
{
	// �J���[�o�b�t�@�����ւ���
	glfwSwapBuffers(window);

	glfwPollEvents();
}