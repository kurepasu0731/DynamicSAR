#include "offscreenRender.h"


/**
 * @brief   ��������
 * 
 * @param   width[in]		�摜�̕�
 * @param   height[in]		�摜�̍���	
 * @param   MSAA[in]		�}���`�T���v���A���`�G�C���A�V���O���邩�ǂ���
 * @param   sanmple[in]		�}���`�T���v���A���`�G�C���A�V���O����ꍇ�̃T���v�����O��
 */
void OffscreenRender::init(int width, int height, bool MSAA, int sanmple)
{
	offscreen_width = width;
	offscreen_height = height;

	// MSAA���g���ꍇ
	if (MSAA)
	{
		MSAA_flag = true;

		// �����_�[�o�b�t�@(RGB)�̍쐬
		glGenRenderbuffers( 1, &rboMSAAID[0] );
		glBindRenderbuffer( GL_RENDERBUFFER, rboMSAAID[0] );
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, sanmple, GL_RGB, width, height);	// �������̊m��
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );

		// �����_�[�o�b�t�@(depth)�̍쐬
		glGenRenderbuffers( 1, &rboMSAAID[1] );
		glBindRenderbuffer( GL_RENDERBUFFER, rboMSAAID[1] );
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, sanmple, GL_DEPTH24_STENCIL8, width, height);	// �������̊m��
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );

		// �t���[���o�b�t�@�̍쐬
		glGenFramebuffers( 1, &fboMSAAID );
		glBindFramebuffer( GL_FRAMEBUFFER, fboMSAAID );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboMSAAID[0] );	// �����_�[�o�b�t�@�ƑΉ��t����
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboMSAAID[1] );	// �����_�[�o�b�t�@�ƑΉ��t����
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	// �����_�[�o�b�t�@(RGB)�̍쐬
	glGenRenderbuffers( 1, &rboID[0] );
	glBindRenderbuffer( GL_RENDERBUFFER, rboID[0] );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_RGB, width, height);		// �������̊m��
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	// �����_�[�o�b�t�@(depth)�̍쐬
	glGenRenderbuffers( 1, &rboID[1] );
	glBindRenderbuffer( GL_RENDERBUFFER, rboID[1] );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);		// �������̊m��
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	// �e�N�X�`���o�b�t�@(RGB)�̍쐬
	glGenTextures( 1, &texID[0]);
	glBindTexture( GL_TEXTURE_2D, texID[0]);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);

	// �e�N�X�`���o�b�t�@(depth)�̍쐬
	glGenTextures( 1, &texID[1]);
	glBindTexture( GL_TEXTURE_2D, texID[1]);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);


	// �t���[���o�b�t�@�̍쐬
	glGenFramebuffers( 1, &fboID );
	glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboID[0] );	// �����_�[�o�b�t�@�ƑΉ��t����
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID[1] );	// �����_�[�o�b�t�@�ƑΉ��t����
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID[0], 0);	// �e�N�X�`���o�b�t�@�ƑΉ��t����
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texID[1], 0);	// �e�N�X�`���o�b�t�@�ƑΉ��t����
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );


	// FBO���ł��Ă��邩�̃`�F�b�N
    if(checkFramebufferStatus() == false ){
        exit(0);
    }
}


/**
 * @brief   �����_�����O�T�C�Y�̕ύX
 * 
 * @param   width[in]		�摜�̕�
 * @param   height[in]		�摜�̍���	
 * @param   MSAA[in]		�}���`�T���v���A���`�G�C���A�V���O���邩�ǂ���
 * @param   sanmple[in]		�}���`�T���v���A���`�G�C���A�V���O����ꍇ�̃T���v�����O��
 */
void OffscreenRender::changeRenderSize(int width, int height, bool MSAA, int sanmple)
{
	offscreen_width = width;
	offscreen_height = height;

	// MSAA���g���ꍇ
	if (MSAA)
	{
		MSAA_flag = true;

		// �����_�[�o�b�t�@(RGB)�̍쐬
		glBindRenderbuffer( GL_RENDERBUFFER, rboMSAAID[0] );
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, sanmple, GL_RGB, width, height);	// �������̊m��
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );

		// �����_�[�o�b�t�@(depth)�̍쐬
		glBindRenderbuffer( GL_RENDERBUFFER, rboMSAAID[1] );
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, sanmple, GL_DEPTH24_STENCIL8, width, height);	// �������̊m��
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );

		// �t���[���o�b�t�@�̍쐬
		glBindFramebuffer( GL_FRAMEBUFFER, fboMSAAID );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboMSAAID[0] );	// �����_�[�o�b�t�@�ƑΉ��t����
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboMSAAID[1] );	// �����_�[�o�b�t�@�ƑΉ��t����
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	// �����_�[�o�b�t�@(RGB)�̍쐬
	glBindRenderbuffer( GL_RENDERBUFFER, rboID[0] );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_RGB, width, height);		// �������̊m��
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	// �����_�[�o�b�t�@(depth)�̍쐬
	glBindRenderbuffer( GL_RENDERBUFFER, rboID[1] );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);		// �������̊m��
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );

	// �e�N�X�`���o�b�t�@(RGB)�̍쐬
	glBindTexture( GL_TEXTURE_2D, texID[0]);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);

	// �e�N�X�`���o�b�t�@(depth)�̍쐬
	glBindTexture( GL_TEXTURE_2D, texID[1]);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);

	// �t���[���o�b�t�@�̍쐬
	glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboID[0] );	// �����_�[�o�b�t�@�ƑΉ��t����
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID[1] );	// �����_�[�o�b�t�@�ƑΉ��t����
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID[0], 0);	// �e�N�X�`���o�b�t�@�ƑΉ��t����
	glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texID[1], 0);	// �e�N�X�`���o�b�t�@�ƑΉ��t����
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


/**
 * @brief   �����_�����O���ʂ̕ۑ�(RGB)
 * 
 * @param   fileName[in]		�ۑ�����摜�̃t�@�C����
 */
void OffscreenRender::saveRenderRGB(const std::string& fileName)
{
	// MSAA���g���ꍇ
	if(MSAA_flag)
	{
		// MSAA�p�̃t���[���o�b�t�@����ʏ�̃t���[���o�b�t�@�֑���
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// �ǂݍ���Texture���w��
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_8UC3, cv::Scalar(0,0,0));	//���ŏ�����

	// Texture����ǂݍ���
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_RGB, GL_UNSIGNED_BYTE, (void*)cvmtx.data);

	cv::flip(cvmtx, cvmtx, 0);					// �摜�̔��](x������)

	cv::cvtColor(cvmtx, cvmtx, CV_RGB2BGR);		// �F�ϊ�

	// �摜�̕ۑ�
	cv::imwrite(fileName, cvmtx);
}


/**
 * @brief   �����_�����O���ʂ̕ۑ�(RGB)(�����̈���N���b�s���O)
 * 
 * @param   fileName[in]		�ۑ�����摜�̃t�@�C����
 * @param   width[in]			�N���b�s���O���镝
 * @param   height[in]			�N���b�s���O���鍂��
 */
void OffscreenRender::saveRenderRGB_Clip(const std::string& fileName, int width, int height)
{
	// MSAA���g���ꍇ
	if(MSAA_flag)
	{
		// MSAA�p�̃t���[���o�b�t�@����ʏ�̃t���[���o�b�t�@�֑���
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// �ǂݍ���Texture���w��
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_8UC3, cv::Scalar(0,0,0));	//���ŏ�����

	// Texture����ǂݍ���
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_RGB, GL_UNSIGNED_BYTE, (void*)cvmtx.data);

	cv::flip(cvmtx, cvmtx, 0);					// �摜�̔��](x������)

	cv::cvtColor(cvmtx, cvmtx, CV_RGB2BGR);		// �F�ϊ�

	if (width < cvmtx.cols && height < cvmtx.rows) {
		// �N���b�s���O
		cv::Mat clip(cvmtx, cv::Rect((cvmtx.cols-width)/2, (cvmtx.rows-height)/2, width, height));
		cv::imwrite(fileName, clip);

	} else {
		// �摜�̕ۑ�
		cv::imwrite(fileName, cvmtx);
	}
}


/**
 * @brief   �����_�����O���ʂ̕ۑ�(depth)
 * 
 * @param   fileName[in]		�ۑ�����摜�̃t�@�C����
 */
void OffscreenRender::saveRenderDepth(const std::string& fileName)
{
	// MSAA���g���ꍇ
	if(MSAA_flag)
	{
		// MSAA�p�̃t���[���o�b�t�@����ʏ�̃t���[���o�b�t�@�֑���
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// �ǂݍ���Texture���w��
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//���ŏ�����

	// Texture����ǂݍ���
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);

	cv::flip(cvmtx, cvmtx, 0);					// �摜�̔��](x������)

	cvmtx.convertTo(cvmtx, CV_8UC1, 255);		// 255�i�K�ɕϊ�

	// �摜�̕ۑ�
	cv::imwrite(fileName, cvmtx);
}


/**
 * @brief   �����_�����O���ʂ̕ۑ�(depth��xml)
 * 
 * @param   fileName[in]		�ۑ�����t�@�C����(xml)
 * @param   near[in]			Near�N���b�v��
 * @param   far[in]				Far�N���b�v��
 *
 * @note	���A���̐[�x�l���i�[(x,y:�s�N�Z��,z:�[�x�l(m))
 */
void OffscreenRender::saveRenderDepthXML(const std::string& fileName, float near, float far)
{
	// MSAA���g���ꍇ
	if(MSAA_flag)
	{
		// MSAA�p�̃t���[���o�b�t�@����ʏ�̃t���[���o�b�t�@�֑���
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// �ǂݍ���Texture���w��
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//���ŏ�����

	// Texture����ǂݍ���
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);

	cv::flip(cvmtx, cvmtx, 0);					// �摜�̔��](x������)

	// ���A���̐[�x�l�ɕϊ�
	for(int i = 0; i < cvmtx.total(); ++i)
	{
		cvmtx.at<float>(i) = near / (1.0f - cvmtx.at<float>(i) * (1.0f - near / far));

		// far���ȏ�̏ꍇ�͖����l(-1)�Ƃ���
		if( cvmtx.at<float>(i) >= far)
			cvmtx.at<float>(i) = -1.0f;
	}

	// xml�t�@�C���ŕۑ�
	cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
	cv::write(cvfs, "depthMat", cvmtx);
}


/**
 * @brief   �����_�����O���ʂ̕ۑ�(depth��xml)(�����̈���N���b�s���O)
 * 
 * @param   fileName[in]		�ۑ�����t�@�C����(xml)
 * @param   near[in]			Near�N���b�v��
 * @param   far[in]				Far�N���b�v��
 * @param   width[in]			�N���b�s���O���镝
 * @param   height[in]			�N���b�s���O���鍂��
 *
 * @note	���A���̐[�x�l���i�[(x,y:�s�N�Z��,z:�[�x�l(m))
 */
void OffscreenRender::saveRenderDepthXML_Clip(const std::string& fileName, float near, float far, int width, int height)
{
	// MSAA���g���ꍇ
	if(MSAA_flag)
	{
		// MSAA�p�̃t���[���o�b�t�@����ʏ�̃t���[���o�b�t�@�֑���
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// �ǂݍ���Texture���w��
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//���ŏ�����

	// Texture����ǂݍ���
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);

	cv::flip(cvmtx, cvmtx, 0);					// �摜�̔��](x������)

	// ���A���̐[�x�l�ɕϊ�
	for(int i = 0; i < cvmtx.total(); ++i)
	{
		cvmtx.at<float>(i) = near / (1.0f - cvmtx.at<float>(i) * (1.0f - near / far));

		// far���ȏ�̏ꍇ�͖����l(-1)�Ƃ���
		if( cvmtx.at<float>(i) >= far)
			cvmtx.at<float>(i) = -1.0f;
	}

	
	if (width < cvmtx.cols && height < cvmtx.rows) {
		// �N���b�s���O
		cv::Mat clip(cvmtx, cv::Rect((cvmtx.cols-width)/2, (cvmtx.rows-height)/2, width, height));
		
		cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
		cv::write(cvfs, "depthMat", clip);

	} else {
		// xml�t�@�C���ŕۑ�
		cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
		cv::write(cvfs, "depthMat", cvmtx);
	}
}


/**
 * @brief   �����_�����O���ʂ̕ۑ�(Point��xml)
 * 
 * @param   fileName[in]				�ۑ�����t�@�C����(xml)
 * @param   viewMatrix[in,out]			���f���r���[�s��
 * @param   projectionMatrix[in,out]	�������e�s��
 *
 * @note	3�������W���i�[(x,y,z:3������Ԃ̍��W(m))
 */
void OffscreenRender::saveRenderPointCloud(const std::string& fileName, glm::mat4& viewMatrix, glm::mat4& projectionMatrix)
{
	// MSAA���g���ꍇ
	if(MSAA_flag)
	{
		// MSAA�p�̃t���[���o�b�t�@����ʏ�̃t���[���o�b�t�@�֑���
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// �ǂݍ���Texture���w��
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//���ŏ�����

	// Texture����ǂݍ���
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);


	// ���[���h���W�̓_
	cv::Mat points(cv::Size(offscreen_width, offscreen_height), CV_32FC3, cv::Scalar(0));

	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );

	for(int y = 0; y < cvmtx.rows; ++y)
	{
		for(int x = 0; x < cvmtx.cols; ++x)
		{
			// �[�x�摜
			glm::vec3 screen = glm::vec3(x, y, cvmtx.at<float>(y,x));

			// 2.5�����ɕϊ�
			glm::vec3 pos = glm::unProject(screen, viewMatrix, projectionMatrix, glm::vec4(viewport[0],viewport[1],viewport[2],viewport[3]));

			if(cvmtx.at<float>(y,x) < 1.0)
			{
				points.at<cv::Vec3f>(y,x)[0] = pos.x;
				points.at<cv::Vec3f>(y,x)[1] = pos.y;
				points.at<cv::Vec3f>(y,x)[2] = pos.z;
			}
			else
			{
				points.at<cv::Vec3f>(y,x)[0] = FLT_MAX;
				points.at<cv::Vec3f>(y,x)[1] = FLT_MAX;
				points.at<cv::Vec3f>(y,x)[2] = FLT_MAX;
			}
		}
	}

	// xml�t�@�C���ŕۑ�
	cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
	cv::write(cvfs, "pointMat", points);
}


/**
 * @brief   �����_�����O���ʂ̕ۑ�(Point��xml)(�����̈���N���b�s���O)
 * 
 * @param   fileName[in]				�ۑ�����t�@�C����(xml)
 * @param   viewMatrix[in,out]			���f���r���[�s��
 * @param   projectionMatrix[in,out]	�������e�s��
 * @param   width[in]					�N���b�s���O���镝
 * @param   height[in]					�N���b�s���O���鍂��
 *
 * @note	3�������W���i�[(x,y,z:3������Ԃ̍��W(m))
 */
void OffscreenRender::saveRenderPointCloud_Clip(const std::string& fileName, glm::mat4& viewMatrix, glm::mat4& projectionMatrix, int width, int height)
{
	// MSAA���g���ꍇ
	if(MSAA_flag)
	{
		// MSAA�p�̃t���[���o�b�t�@����ʏ�̃t���[���o�b�t�@�֑���
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// �ǂݍ���Texture���w��
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//���ŏ�����

	// Texture����ǂݍ���
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);


	if(width < cvmtx.cols && height < cvmtx.rows)
	{
		// ���[���h���W�̓_
		cv::Mat points(cv::Size(width, height), CV_32FC3, cv::Scalar(0));

		GLint viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );

		int count_x = 0;
		int count_y = 0;
		int start_x = (cvmtx.cols-width)/2;
		int start_y = (cvmtx.rows-height)/2;

		for(int y = start_y; y < start_y+height; ++y)
		{
			count_x = 0;
			for(int x = start_x; x < start_x+width; ++x)
			{
				// �[�x�摜
				glm::vec3 screen = glm::vec3(x, y, cvmtx.at<float>(y,x));

				// 2.5�����ɕϊ�
				glm::vec3 pos = glm::unProject(screen, viewMatrix, projectionMatrix, glm::vec4(viewport[0],viewport[1],viewport[2],viewport[3]));

				if(cvmtx.at<float>(y,x) < 1.0)
				{
					points.at<cv::Vec3f>(count_y,count_x)[0] = pos.x;
					points.at<cv::Vec3f>(count_y,count_x)[1] = pos.y;
					points.at<cv::Vec3f>(count_y,count_x)[2] = pos.z;
				}
				else
				{
					points.at<cv::Vec3f>(count_y,count_x)[0] = FLT_MAX;
					points.at<cv::Vec3f>(count_y,count_x)[1] = FLT_MAX;
					points.at<cv::Vec3f>(count_y,count_x)[2] = FLT_MAX;
				}
				count_x++;
			}
			count_y++;
		}
		// xml�t�@�C���ŕۑ�
		cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
		cv::write(cvfs, "pointMat", points);
	}
	else
	{	
		// ���[���h���W�̓_
		cv::Mat points(cv::Size(offscreen_width, offscreen_height), CV_32FC3, cv::Scalar(0));

		GLint viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );

		for(int y = 0; y < cvmtx.rows; ++y)
		{
			for(int x = 0; x < cvmtx.cols; ++x)
			{
				// �[�x�摜
				glm::vec3 screen = glm::vec3(x, y, cvmtx.at<float>(y,x));

				// 2.5�����ɕϊ�
				glm::vec3 pos = glm::unProject(screen, viewMatrix, projectionMatrix, glm::vec4(viewport[0],viewport[1],viewport[2],viewport[3]));

				if(cvmtx.at<float>(y,x) < 1.0)
				{
					points.at<cv::Vec3f>(y,x)[0] = pos.x;
					points.at<cv::Vec3f>(y,x)[1] = pos.y;
					points.at<cv::Vec3f>(y,x)[2] = pos.z;
				}
				else
				{
					points.at<cv::Vec3f>(y,x)[0] = FLT_MAX;
					points.at<cv::Vec3f>(y,x)[1] = FLT_MAX;
					points.at<cv::Vec3f>(y,x)[2] = FLT_MAX;
				}
			}
		}

		// xml�t�@�C���ŕۑ�
		cv::FileStorage cvfs(fileName, CV_STORAGE_WRITE);
		cv::write(cvfs, "pointMat", points);
	}
}


/**
 * @brief   �����_�����O���ʂ�Mat�^�ŕԂ�(RGB)
 * 
 * @param   colorMat[in]		�����_�����O�����摜
 */
void OffscreenRender::getRenderRGB(cv::Mat& colorMat)
{
	// MSAA���g���ꍇ
	if(MSAA_flag)
	{
		// MSAA�p�̃t���[���o�b�t�@����ʏ�̃t���[���o�b�t�@�֑���
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// �ǂݍ���Texture���w��
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	colorMat = cv::Mat(cv::Size(offscreen_width, offscreen_height), CV_8UC4, cv::Scalar(0,0,0));	//���ŏ�����

	// Texture����ǂݍ���
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)colorMat.data);

	cv::cvtColor(colorMat, colorMat, CV_RGBA2BGR);		// �F�ϊ�
}



/**
 * @brief   �����_�����O���ʂ�Mat�^�ŕԂ�(depth)
 * 
 * @param   colorMat[in]		�����_�����O�����摜
 */
void OffscreenRender::getRenderDepth(cv::Mat& depthMat)
{
	// MSAA���g���ꍇ
	if(MSAA_flag)
	{
		// MSAA�p�̃t���[���o�b�t�@����ʏ�̃t���[���o�b�t�@�֑���
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// �ǂݍ���Texture���w��
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	depthMat = cv::Mat(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//���ŏ�����

	// Texture����ǂݍ���
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)depthMat.data);
}


/**
 * @brief   �����_�����O���ʂ�Mat�^�ŕԂ�(Normal)
 * 
 * @param   normalMat[in]		�����_�����O�����摜
 */
void OffscreenRender::getRenderNormal(cv::Mat& normalMat)
{
	// MSAA���g���ꍇ
	if(MSAA_flag)
	{
		// MSAA�p�̃t���[���o�b�t�@����ʏ�̃t���[���o�b�t�@�֑���
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// �ǂݍ���Texture���w��
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	normalMat = cv::Mat(cv::Size(offscreen_width, offscreen_height), CV_32FC3, cv::Scalar(0,0,0));	//���ŏ�����

	// Texture����ǂݍ���
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_RGB, GL_FLOAT, (void*)normalMat.data);

	// 0�`1�͈̔͂�-1�`1�֕ϊ�
	float * ptr = (float *)normalMat.data;		// �|�C���^
	const int length = static_cast<const int>(normalMat.step1());	// 1�X�e�b�v�̃`�����l����
	const int length_x = normalMat.cols * 3;						// ���̃T�C�Y

	for (int y = 0; y < normalMat.rows; ++y)
	{
		for (int x = 0; x < length_x; x+=3)
		{
			ptr[x+0] = ptr[x+0] * 2.f - 1.f;
			ptr[x+1] = ptr[x+1] * 2.f - 1.f;
			ptr[x+2] = ptr[x+2] * 2.f - 1.f;
		}
		ptr += length;
	}
}


/**
 * @brief   �����_�����O���ʂ�Mat�^�ŕԂ�(PointCloud)
 * 
 * @param   colorMat[in]				�����_�����O�����摜
 * @param   viewMatrix[in,out]			���f���r���[�s��
 * @param   projectionMatrix[in,out]	�������e�s��
 */
void OffscreenRender::getRenderPointCloud(cv::Mat& pointcloudMat, glm::mat4& viewMatrix, glm::mat4& projectionMatrix)
{
	// MSAA���g���ꍇ
	if(MSAA_flag)
	{
		// MSAA�p�̃t���[���o�b�t�@����ʏ�̃t���[���o�b�t�@�֑���
		glBindFramebuffer( GL_READ_FRAMEBUFFER, fboMSAAID );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, fboID );
		glBlitFramebuffer( 0, 0, offscreen_width, offscreen_height, 0, 0, offscreen_width, offscreen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
		glBindFramebuffer( GL_FRAMEBUFFER, fboID );
	}

	// �ǂݍ���Texture���w��
	glReadBuffer(GL_DEPTH_ATTACHMENT);

	cv::Mat cvmtx(cv::Size(offscreen_width, offscreen_height), CV_32FC1, cv::Scalar(0));	//���ŏ�����

	// Texture����ǂݍ���
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(0, 0, offscreen_width, offscreen_height, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)cvmtx.data);

	// ���[���h���W�̓_
	pointcloudMat = cv::Mat(cv::Size(offscreen_width, offscreen_height), CV_32FC3, cv::Scalar(0));

	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );

	for(int y = 0; y < pointcloudMat.rows; ++y)
	{
		for(int x = 0; x < pointcloudMat.cols; ++x)
		{
			// �[�x�摜
			glm::vec3 screen = glm::vec3(x, y, cvmtx.at<float>(y,x));

			// 2.5�����ɕϊ�
			glm::vec3 pos = glm::unProject(screen, viewMatrix, projectionMatrix, glm::vec4(viewport[0],viewport[1],viewport[2],viewport[3]));

			if(cvmtx.at<float>(y,x) < 1.0)
			{
				pointcloudMat.at<cv::Vec3f>(y,x)[0] = pos.x;
				pointcloudMat.at<cv::Vec3f>(y,x)[1] = pos.y;
				pointcloudMat.at<cv::Vec3f>(y,x)[2] = pos.z;
			}
			else
			{
				pointcloudMat.at<cv::Vec3f>(y,x)[0] = FLT_MAX;
				pointcloudMat.at<cv::Vec3f>(y,x)[1] = FLT_MAX;
				pointcloudMat.at<cv::Vec3f>(y,x)[2] = FLT_MAX;
			}
		}
	}
}




/**
 * @brief   �t���[���o�b�t�@�̎擾�󋵂̊m�F	
 * 
 * @returns �����������ǂ���
 */
bool OffscreenRender::checkFramebufferStatus()
{
	// check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status)
    {
        case GL_FRAMEBUFFER_COMPLETE:
                std::cout << "Framebuffer complete." << std::endl;
                return true;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
                return false;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
                return false;

        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
                std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
                return false;

        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
                std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
                return false;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
                return false;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
                return false;

        case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cout << "[ERROR] Unsupported by FBO implementation." << std::endl;
                return false;

        default:
                std::cout << "[ERROR] Unknow error." << std::endl;
                return false;
    }
}