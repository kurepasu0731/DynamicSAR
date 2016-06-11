#include "Projection_thread.h"


/**
 * @brief   ��������
 *
 * @param	modelFile[in]	���f���t�@�C����
 */
void ProjectionThread::init(const std::string& modelFile)
{
	// �w�i�F
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// �f�v�X�e�X�g
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);

	// �|�C���g�X�v���C�g�̐ݒ�
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);


	// �I�t�X�N���[�������_�����O�̏�����
	offscreen.init(offscreen.offscreen_width, offscreen.offscreen_height);

	/////////////// �Ώە��̂̃��f�� //////////////////

	//	�v���O�����I�u�W�F�N�g���쐬����
	init_GLSL( &program, "../../Shader/shading_tex.vert", "../../Shader/shading_tex.frag");
	glUseProgram(program);

	// uniform�ϐ��̏ꏊ���擾
	MatrixID = glGetUniformLocation(program, "MVP");
	ViewMatrixID = glGetUniformLocation(program, "V");
	ModelMatrixID = glGetUniformLocation(program, "M");

	// �����ʒu�̎擾
	LightID = glGetUniformLocation(program, "LightPosition_worldspace");
	LightPowerID = glGetUniformLocation(program, "LightPower");

	// ���f���̓ǂݍ���
	mesh.loadMesh(modelFile);


	/////////////// �w�i //////////////////

	// �J�����摜
	cv::Mat Black(windowHeight, windowWidth, CV_8UC3, cv::Scalar(0.0, 0.0, 0.0));

	// �w�i�̏����ݒ�
	projection_background.init(Black, "../../Shader/background.vert", "../../Shader/background.frag", windowWidth, windowHeight);


	/////////////// �`�F�b�J�[�{�[�h�_�Q�p //////////////////
	init_GLSL( &program_point, "../../Shader/point.vert", "../../Shader/point.frag");
	glUseProgram(program_point);

	// uniform�ϐ��̏ꏊ���擾
	PointMatrixID = glGetUniformLocation(program_point, "MVP");
	
	// �_�̑傫��
	glUseProgram(program_point);
	GLuint pointSizeID = glGetUniformLocation(program_point, "point_size");
	GLfloat p_size = 10.f;
	glUniform1f(pointSizeID, p_size);

	// VBO�̐���
    glGenBuffers(1, &PointVBO);
    glGenBuffers(1, &PointIBO);

	// �J�����̊m�F�p
	cv::Mat ir_img;
	irCamDev->getImage(ir_img);
	camera_background.init(ir_img, "../../Shader/background.vert", "../../Shader/background.frag", ir_img.cols, ir_img.rows);
	camera_offscreen.offscreen_width = ir_img.cols;
	camera_offscreen.offscreen_height = ir_img.rows;
	camera_offscreen.init(ir_img.cols, ir_img.rows);
}



/**
 * @brief   ���t���[���s������
 */
void ProjectionThread::display()
{

	///// �O���C�R�[�h���e /////
	if (critical_section->graycode_flag)
	{
		std::cout << "�O���C�R�[�h���e" << std::endl;

		// ���F���e
		cv::Mat white(windowHeight, windowWidth, CV_8UC3, cv::Scalar(255.0, 255.0, 255.0));
		img_projection(white);
		Sleep(400);

		// �J�����̎B�e
		cv::Mat ir_img, rgb_img;
		irCamDev->getImage(ir_img);
		rgbCamDev->captureImage(rgb_img);
		rgbCamDev->captureImage(rgb_img);
		rgbCamDev->captureImage(rgb_img);

		// �O���C�R�[�h�𓊉e���đΉ��_���擾
		calib->runGetCorrespond(ir_img, rgb_img, this);

		critical_section->graycode_flag = false;
	}

	///// �L�����u���[�V�����t���O /////
	else if (critical_section->calib_flag)
	{
		if(calib->calib_count > 2)
		{
			std::cout << "�L�����u���[�V�������c" << std::endl;

			// �L�����u���[�V����
			cv::Mat ir_img;
			irCamDev->getImage(ir_img);
			calib->proCamCalibration(calib->myWorldPoints, calib->myCameraPoints, calib->myProjectorPoints, cv::Size(ir_img.cols, ir_img.rows), cv::Size(windowWidth, windowHeight));
			
			loadProjectorParam();		// �v���W�F�N�^�p�����[�^�̓ǂݍ���
		} else {
			std::cout << "�Ή��_��������܂���" << std::endl;
		}
		critical_section->calib_flag = false;
	}

	///// �L�����u���[�V�����t�@�C���̓ǂݍ��� /////
	else if (critical_section->load_calib_flag)
	{
		cv::Mat ir_img;
		irCamDev->getImage(ir_img);
		calib->loadCalibParam(saveCalibFolder+"/calibration.xml");

		loadProjectorParam();			// �v���W�F�N�^�p�����[�^�̓ǂݍ���

		critical_section->load_calib_flag = false;
	}

	///// �`�F�b�J�[�{�[�h�֍ē��e /////
	else if (critical_section->run_reprojection_flag)
	{
		runCheckerReprojection();
	}
	///// ���o���ǐ� /////
	else if (critical_section->detect_tracking_flag)
	{
		///// �I�t�X�N���[�� //////
		offscreen.startRender();

		// �E�B���h�E����������
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, windowWidth, windowHeight);
		glClearColor(0.0, 0.0, 0.0, 1.0);

		// �J�����ƏƖ��̈ʒu�����킹��
		lightPos = camera.eyePosition;

		// �g���b�L���O�������̂�
		if (critical_section->tracking_success_flag)
		{
			// ���f���̕`��
			draw_model();
		}

		// �����_�����O�摜
		cv::Mat renderImg;
		offscreen.getRenderRGB(renderImg);
		offscreen.endRender();

		// �摜���]
		cv::flip(renderImg, renderImg, 0);

		// �c�ݕ␳
		cv::Mat map1, map2;
		critical_section->getProjUndistortMap(map1, map2);
		if (critical_section->use_projCalib_flag && map1.cols > 0)
		{
			cv::remap(renderImg, renderImg, map1, map2, cv::INTER_LINEAR);
		}

		// �w�i�̃����_�����O
		img_projection(renderImg);
	}
	///// ���̓��e /////
	else if (critical_section->project_white_flag)
	{
		cv::Mat white(windowHeight, windowWidth, CV_8UC3, cv::Scalar(255, 255, 255));
		
		// �w�i�̃����_�����O
		img_projection(white);
	}
	else
	{
		///// �I�t�X�N���[�� //////
		offscreen.startRender();

		// �E�B���h�E����������
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, windowWidth, windowHeight);
		glClearColor(0.0, 0.0, 0.0, 1.0);

		// �J�����ƏƖ��̈ʒu�����킹��
		lightPos = camera.eyePosition;

		// ���f���̕`��
		draw_model();

		// �����_�����O�摜
		cv::Mat renderImg;
		offscreen.getRenderRGB(renderImg);
		offscreen.endRender();

		// �摜���]
		cv::flip(renderImg, renderImg, 0);

		// �c�ݕ␳
		cv::Mat map1, map2;
		critical_section->getProjUndistortMap(map1, map2);
		if (critical_section->use_projCalib_flag && map1.cols > 0)
		{
			cv::remap(renderImg, renderImg, map1, map2, cv::INTER_LINEAR);
		}

		// �w�i�̃����_�����O
		img_projection(renderImg);
	}
}


/**
 * @brief   ���f���̕`��
 */
void ProjectionThread::draw_model()
{
	// �V�F�[�_�v���O�����̎g�p�J�n
	glUseProgram(program);

	// �J�������W�n�֕ϊ�
	glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), windowWidth, windowHeight);
	glm::mat4 ViewMatrix = camera.updateViewMatrix(camera.eyePosition, camera.eyeVector, camera.eyeUp);

	// ���f���̍��W�ϊ�
	glm::mat4 RotationMatrix = glm::mat4_cast(glm::normalize(quatOrientation));
	glm::mat4 TranslationMatrix = glm::translate(glm::mat4(), glm::vec3(0.0,0.0,0.0)); 
	glm::mat4 ScalingMatrix = scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));
	//glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;
	glm::mat4 ModelMatrix = critical_section->getModelMatrix();

	// �x���⏞
	if(critical_section->tracking_flag && critical_section->compensation_delay_flag)
	{
		ModelMatrix = critical_section->getCompensatePoseMatrix();
	}


	glm::mat4 MVP = ProjectionMatrix * ModelMatrix;

	// �L�����u���[�V�����ς݂ł����
	if (calib->calib_flag)
	{
		ProjectionMatrix = proj_intrinsicMat;
		ModelMatrix = proj_extrinsicMat * ModelMatrix;
		MVP = ProjectionMatrix * ModelMatrix;
	}

	// Uniform�ϐ��ɍs��𑗂�
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(LightPowerID, critical_section->getLightPower()+1.0f);

	// ���f���̃����_�����O
	mesh.render();
}



/**
 * @brief   �摜�̓��e
 * 
 * @param	projMat[in]		���e�摜
 */
void ProjectionThread::img_projection(const cv::Mat& projMat)
{
	// �E�B���h�E����������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	// �w�i�F
	glClearColor(0.0, 0.0, 0.0, 1.0);


	// �w�i�̃����_�����O
	glm::mat4 OrthoMatrix = glm::ortho(0.f, (float)windowWidth, (float)windowHeight, 0.f, -100.f, 1.f);		// far�N���b�v�ʋ߂��ŕ`��
	projection_background.changeMatrix(OrthoMatrix);
	projection_background.textureUpdate(projMat);
	projection_background.draw();

	// �J���[�o�b�t�@�����ւ�,�C�x���g���擾
	swapBuffers();
}


/**
 * @brief   ���_�̕`��
 * 
 * @param	point_cloud[in]		���_�z��
 */
void ProjectionThread::draw_point(const std::vector<glm::vec3>& point_cloud)
{
	std::vector<unsigned int> index_vect(point_cloud.size());
	for (int i = 0; i < point_cloud.size(); ++i){
		index_vect[i] = i;
	}

	// VBO�̐���
    glBindBuffer(GL_ARRAY_BUFFER, PointVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*point_cloud.size(), &point_cloud[0], GL_DYNAMIC_DRAW);

	// IBO�̐���
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PointIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*index_vect.size(), &index_vect[0], GL_STATIC_DRAW);

	// �V�F�[�_�̕ϐ��ɒ��_����Ή��t����
	glEnableVertexAttribArray(0);

	// ���_���̊i�[�ꏊ�Ə����̎w��
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);			// ���_

	// IBO�̎w��
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PointIBO);

	glDrawElements(GL_POINTS, point_cloud.size(), GL_UNSIGNED_INT, 0);

	// ���_�̑Ή��t���̉���
    glDisableVertexAttribArray(0);

	//�@�o�C���h�������̂����ǂ�
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}



/**
 * @brief   �L�����u���[�V�������ʂ�p���ă`�F�b�J�[�{�[�h�ɍē��e(OpenGL�)
 */
void ProjectionThread::runCheckerReprojection()
{
	if(calib->calib_flag)
	{
		std::cout << "�`�F�b�J�[�p�^�[���ւ̍ē��e���c" << std::endl;

		// �L���v�`��
		cv::Mat ir_img;
		irCamDev->getImage(ir_img);
					
		// �`�F�b�J�[�p�^�[���̌�_�����o(�J����)
		cv::Mat ir_undistort = ir_img.clone();
		// �c�ݕ␳
		if(critical_section->use_calib_flag)
		{
			cv::Mat map1, map2;
			critical_section->getUndistortMap(map1, map2);
			cv::remap(ir_img, ir_undistort, map1, map2, cv::INTER_LINEAR);
		}

		std::vector<cv::Point3f> worldPoint;
		std::vector<cv::Point2f> imagePoint;
		std::vector<cv::Point2f> projPoint;
		bool detect_flag = calib->getCorners(imagePoint, ir_undistort, ir_undistort);		// �`�F�b�J�[�p�^�[�����o

		cv::Mat proj_img = cv::Mat(windowHeight, windowWidth, CV_8UC3, cv::Scalar(0, 0, 0));

		if(detect_flag) 
		{
			// �J�����𒆐S�Ƃ����`�F�b�J�[�p�^�[���̈ʒu�擾
			calib->getCameraWorldPoint(worldPoint, imagePoint);

			// �J�������W�ւ̓��e(�c�ݏ�����̂��ߘc�݃p�����[�^�͗p���Ȃ�)
			std::vector<cv::Point2f> cam_projection;
			cv::Mat cam_R = cv::Mat::eye(3, 3, CV_64F);
			cv::Mat cam_T = cv::Mat::zeros(3, 1, CV_64F);


			///// �J�����摜�̃I�t�X�N���[�������_�����O /////
			camera_offscreen.startRender();

			// �E�B���h�E����������
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, ir_undistort.cols, ir_undistort.rows);
			glClearColor(0.0, 0.0, 0.0, 1.0);

			// �i�q�_�̕`��
			glUseProgram(program_point);

			// �J�������W�n�֕ϊ�
			glm::mat4 ProjectionMatrix = camera.updateProjectionMatrix(camera.getFov(), camera.getNear(), camera.getFar(), ir_undistort.cols, ir_undistort.rows);

			// ���f���̍��W�ϊ�
			glm::mat4 RotationMatrix(1.f);
			glm::mat4 TranslationMatrix = glm::translate(glm::mat4(), glm::vec3(cam_T.at<double>(0), cam_T.at<double>(1), cam_T.at<double>(2))); 
			glm::mat4 GL2CV = glm::scale(glm::mat4(), glm::vec3(1.0f, -1.0f, -1.0f));		// CV���W�n�ւ̕ϊ�
			glm::mat4 CV2GL = glm::scale(glm::mat4(), glm::vec3(1.0f, -1.0f, -1.0f));		// GL���W�n�ւ̕ϊ�

			glm::mat4 ModelMatrix = CV2GL * TranslationMatrix * RotationMatrix * GL2CV;
			glm::mat4 MVP = ProjectionMatrix * ModelMatrix;

			// �����p�����[�^���g���ꍇ
			if(critical_section->use_calib_flag)
			{
				MVP = critical_section->getCameraIntrinsicMatrix() * ModelMatrix;
			}

			// Uniform�ϐ��ɍs��𑗂�
			glUniformMatrix4fv(PointMatrixID, 1, GL_FALSE, &MVP[0][0]);

			// �i�q�_
			std::vector<glm::vec3> checker_point;
			for (int i = 0; i < worldPoint.size(); ++i)
			{
				checker_point.emplace_back(glm::vec3(worldPoint[i].x, -worldPoint[i].y, -worldPoint[i].z));					// GL���W�_
			}
			draw_point(checker_point);

			// �w�i�̃����_�����O
			glm::mat4 Ortho = glm::ortho(0.f, (float)ir_undistort.cols, (float)ir_undistort.rows, 0.f, -100.f, 1.f);		// far�N���b�v�ʋ߂��ŕ`��
			camera_background.changeMatrix(Ortho);
			camera_background.textureUpdate(ir_undistort);
			camera_background.draw();

			// �����_�����O�摜
			cv::Mat renderImg;
			camera_offscreen.getRenderRGB(renderImg);

			// ���]
			cv::flip(renderImg, renderImg, 0);

			cv::imshow("IR_camera", renderImg);
			cv::waitKey(1);

			camera_offscreen.endRender();

			// �J�����̍ē��e�덷
			cv::projectPoints(worldPoint, cam_R, cam_T, calib->cam_K, cv::Mat(), cam_projection);
			float cam_error = 0;
			for(int i=0; i<imagePoint.size(); ++i)
			{
				cam_error += std::sqrt((imagePoint[i].x - cam_projection[i].x)*(imagePoint[i].x - cam_projection[i].x) + (imagePoint[i].y - cam_projection[i].y)*(imagePoint[i].y - cam_projection[i].y));
			}

			std::cout << "�J�����̍ē��e�덷�F" << cam_error/imagePoint.size() << std::endl;


			///// �v���W�F�N�^�摜�̃I�t�X�N���[�������_�����O //////
			offscreen.startRender();

			// �E�B���h�E����������
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, windowWidth, windowHeight);
			glClearColor(0.0, 0.0, 0.0, 1.0);

			// �i�q�_�̕`��
			glUseProgram(program_point);

			// ���f���̍��W�ϊ�
			// �����p�����[�^�ƊO���p�����[�^
			glm::mat4 MVP2 = proj_intrinsicMat * proj_extrinsicMat;

			// Uniform�ϐ��ɍs��𑗂�
			glUniformMatrix4fv(PointMatrixID, 1, GL_FALSE, &MVP2[0][0]);

			// �i�q�_
			draw_point(checker_point);

			// �����_�����O�摜
			offscreen.getRenderRGB(proj_img);
			offscreen.endRender();

			// ���]
			cv::flip(proj_img, proj_img, 0);

			// �c�ݕ␳
			cv::Mat map1, map2;
			critical_section->getProjUndistortMap(map1, map2);
			if (critical_section->use_projCalib_flag && map1.cols > 0)
			{
				cv::remap(proj_img, proj_img, map1, map2, cv::INTER_LINEAR);
			}
		}

		// �f���̓��e
		img_projection(proj_img);
	}
	else 
	{
		std::cerr << "�L�����u���[�V�������s���Ă�������" << std::endl;
	}
}


/**
 * @brief   �v���W�F�N�^�p�����[�^�̓ǂݍ���		
 */
void ProjectionThread::loadProjectorParam()
{
	// �v���W�F�N�^�̊O���p�����[�^
	proj_extrinsicMat[0][0] = calib->R.at<double>(0,0);
	proj_extrinsicMat[1][0] = calib->R.at<double>(0,1);
	proj_extrinsicMat[2][0] = calib->R.at<double>(0,2);
	proj_extrinsicMat[3][0] = calib->T.at<double>(0,0);
	proj_extrinsicMat[0][1] = calib->R.at<double>(1,0);
	proj_extrinsicMat[1][1] = calib->R.at<double>(1,1);
	proj_extrinsicMat[2][1] = calib->R.at<double>(1,2);
	proj_extrinsicMat[3][1] = calib->T.at<double>(1,0);
	proj_extrinsicMat[0][2] = calib->R.at<double>(2,0);
	proj_extrinsicMat[1][2] = calib->R.at<double>(2,1);
	proj_extrinsicMat[2][2] = calib->R.at<double>(2,2);
	proj_extrinsicMat[3][2] = calib->T.at<double>(2,0);
	proj_extrinsicMat[0][3] = 0.0;
	proj_extrinsicMat[1][3] = 0.0;
	proj_extrinsicMat[2][3] = 0.0;
	proj_extrinsicMat[3][3] = 1.0;

	glm::mat4 GL2CV = glm::scale(glm::mat4(), glm::vec3(1.0f, -1.0f, -1.0f));		// CV���W�n�ւ̕ϊ�
	glm::mat4 CV2GL = glm::scale(glm::mat4(), glm::vec3(1.0f, -1.0f, -1.0f));		// GL���W�n�ւ̕ϊ�

	// �O���p�����[�^��CV���W�n
	proj_extrinsicMat = CV2GL * proj_extrinsicMat * GL2CV;

	// �v���W�F�N�^�̓����p�����[�^
	proj_intrinsic = calib->proj_K;

	// OpenGL�`���ɕϊ�
	glm::mat4 intrinsic(2.0*proj_intrinsic(0,0)/windowWidth, 0, 0, 0,
                proj_intrinsic(0,1), 2.0*proj_intrinsic(1,1)/windowHeight, 0, 0,
                (windowWidth-2.0*proj_intrinsic(0,2))/windowWidth, (2.0*proj_intrinsic(1,2)-windowHeight)/windowHeight, -(camera.getFar()+camera.getNear())/(camera.getFar()-camera.getNear()), -1,
                0, 0,  -2*camera.getFar()*camera.getNear()/(camera.getFar()-camera.getNear()), 0);


	// OpenGL�̃v���W�F�N�V�����s��
	proj_intrinsicMat = intrinsic;


	// �c�ݕ␳�}�b�v
	cv::Mat map1, map2, projMat2;

	cv::initUndistortRectifyMap(calib->proj_K, -calib->proj_dist, cv::Mat(), calib->proj_K, cv::Size(windowWidth , windowHeight), CV_32FC1, map1, map2);

	critical_section->setProjUndistortMap(map1, map2);
	critical_section->use_projCalib_flag = true;
}


// �J���[�o�b�t�@�����ւ��ăC�x���g�����o��
void ProjectionThread::swapBuffers()
{
	// �J���[�o�b�t�@�����ւ���
	glfwSwapBuffers(window);

	// �C�x���g�����o��
	glfwPollEvents();
}
