#include "Graycode.h"
#include "../Projection_thread.h"

/**
 * @brief   �ۑ���̃t�H���_�̐���		
 */
void GRAYCODE::createDirs()
{
	std::string mainFolder = saveFolder+"/GrayCodeImage";
	std::string captureFolder = saveFolder+"/GrayCodeImage/CaptureImage";
	std::string grayFolder = saveFolder+"/GrayCodeImage/ProjectionGrayCode";
	std::string thresholdFolder = saveFolder+"/GrayCodeImage/ThresholdImage";
	_mkdir(mainFolder.c_str());
	// �O���C�R�[�h�B�e�摜
	_mkdir(captureFolder.c_str());
	// �O���C�R�[�h���摜
	_mkdir(grayFolder.c_str());
	// �O���C�R�[�h�B�e�摜�̓�l�������摜
	_mkdir(thresholdFolder.c_str());
}

/***************************
** �O���C�R�[�h�̍쐬�֘A **
****************************/


/**
 * @brief   �r�b�g���̌v�Z�ƃO���C�R�[�h�̍쐬
 */
void GRAYCODE::initGraycode()
{
	std::vector<int> bin_code_h(prj_height);  // 2�i�R�[�h�i�c�j
	std::vector<int> bin_code_w(prj_width);   // 2�i�R�[�h�i���j
	std::vector<int> graycode_h(prj_height);  // �O���C�R�[�h�i�c�j
	std::vector<int> graycode_w(prj_width);   // �O���C�R�[�h�i���j


	/***** 2�i�R�[�h�쐬 *****/
	// �s�ɂ���
	for( int y = 0; y < prj_height; y++ )
		bin_code_h[y] = y + 1;
	// ��ɂ���
	for( int x = 0; x < prj_width; x++ )
		bin_code_w[x] = x + 1;

	/***** �O���C�R�[�h�쐬 *****/
	// �s�ɂ���
	for( int y = 0; y < prj_height; y++ )
		graycode_h[y] = bin_code_h[y] ^ ( bin_code_h[y] >> 1 );
	// ��ɂ���
	for( int x = 0; x < prj_width; x++ )
		graycode_w[x] = bin_code_w[x] ^ ( bin_code_w[x] >> 1 );
	// �s������킹��i�s + ��j
	for( int y = 0; y < prj_height; y++ ) {
		for( int x = 0; x < prj_width; x++ )
			c->g.graycode[y][x] = ( graycode_h[y] << c->g.w_bit) | graycode_w[x];
	}
}


/**
 * @brief   �p�^�[���R�[�h�摜�쐬�i��x���΃v���W�F�N�^�̉𑜓x���ς��Ȃ������蒼���K�v�͂Ȃ�
 */
void GRAYCODE::makeGraycodeImage()
{
	std::cout << "���e�p�O���C�R�[�h�쐬��" << std::endl;

	cv::Mat posi_img ( prj_height, prj_width, CV_8UC3, cv::Scalar(0, 0, 0) );
	cv::Mat nega_img ( prj_height, prj_width, CV_8UC3, cv::Scalar(0, 0, 0) );
	int bit = c->g.all_bit-1;
	std::stringstream *Filename_posi = new std::stringstream[c->g.all_bit];  // �����t���o��
	std::stringstream *Filename_nega = new std::stringstream[c->g.all_bit];  // �����t���o��

	// �|�W�p�^�[���R�[�h�摜�쐬
	for( unsigned int z = 0; z < c->g.all_bit; z++) {
		for( int y = 0; y < prj_height; y++ ) {
			for( int x = 0; x < prj_width; x++ ) {
				if( ( (c->g.graycode[y][x] >> (bit-z)) & 1 ) == 0 ) {  // �ŏ�ʃr�b�g���珇�ɒ��o���C���̃r�b�g��0��������
					posi_img.at<cv::Vec3b>( y, x )[0] = 0;  // B
					posi_img.at<cv::Vec3b>( y, x )[1] = 0;  // G
					posi_img.at<cv::Vec3b>( y, x )[2] = 0;  // R
				}else if( ( (c->g.graycode[y][x] >> (bit-z)) & 1 ) == 1 ) {
					posi_img.at<cv::Vec3b>( y, x )[0] = 255;  // B
					posi_img.at<cv::Vec3b>( y, x )[1] = 255;  // G
					posi_img.at<cv::Vec3b>( y, x )[2] = 255;  // R
				}
			}
		}
		// �A�ԂŃt�@�C������ۑ��i������X�g���[���j
		Filename_posi[z] << saveFolder+"/GrayCodeImage/ProjectionGrayCode/posi" << std::setw(2) << std::setfill('0') << z << ".bmp"; 
		cv::imwrite(Filename_posi[z].str(), posi_img);
		Filename_posi[z] << std::endl;
	}

	// �l�K�p�^�[���R�[�h�摜�쐬
	for( unsigned int z = 0; z < c->g.all_bit; z++) {
		for( int y = 0; y < prj_height; y++ ) {
			for( int x = 0; x < prj_width; x++ ) {
				if( ( (c->g.graycode[y][x] >> (bit-z)) & 1 ) == 1 ) {
					nega_img.at<cv::Vec3b>( y, x )[0] = 0;  // B
					nega_img.at<cv::Vec3b>( y, x )[1] = 0;  // G
					nega_img.at<cv::Vec3b>( y, x )[2] = 0;  // R
				}else if( ( (c->g.graycode[y][x] >> (bit-z)) & 1 ) == 0 ) {
					nega_img.at<cv::Vec3b>( y, x )[0] = 255;  // B
					nega_img.at<cv::Vec3b>( y, x )[1] = 255;  // G
					nega_img.at<cv::Vec3b>( y, x )[2] = 255;  // R
				}
			}
		}
		// �A�ԂŃt�@�C������ۑ��i������X�g���[���j
		Filename_nega[z] << saveFolder+"/GrayCodeImage/ProjectionGrayCode/nega" << std::setw(2) << std::setfill('0') << z << ".bmp"; 
		cv::imwrite(Filename_nega[z].str(), nega_img);
		Filename_nega[z] << std::endl;
	}

	delete[] Filename_posi;
	delete[] Filename_nega;
}



/**
 * @brief   �p�^�[���R�[�h���e & �B�e
 *
 * @param	projection[in,out]	���e�p�N���X
 */
void GRAYCODE::code_projection(ProjectionThread *projection)
{
	// �萔
	typedef enum flag
	{
		POSI = true,
		NEGA = false,
		VERTICAL = true,
		HORIZONTAL = false,
	} flag;


	// ������&�J�����N��
	initGraycode();

	cv::Mat *posi_img = new cv::Mat[c->g.all_bit];  // �|�W�p�^�[���p
	cv::Mat *nega_img = new cv::Mat[c->g.all_bit];  // �l�K�p�^�[���p

	// �����t���o�́i�O���C�R�[�h�ǂݍ��ݗp�j
	std::stringstream *Filename_posi = new std::stringstream[c->g.all_bit]; 
	std::stringstream *Filename_nega = new std::stringstream[c->g.all_bit];
	// �����t���o�́i�B�e�摜�������ݗp�j
	std::stringstream *Filename_posi_cam = new std::stringstream[c->g.all_bit]; 
	std::stringstream *Filename_nega_cam = new std::stringstream[c->g.all_bit];

	// �A�ԂŃt�@�C������ǂݍ��ށi������X�g���[���j
	std::cout << "���e�p�O���C�R�[�h�摜�ǂݍ��ݒ�" << std::endl;
	for( unsigned int i = 0; i < c->g.all_bit; i++ ) 
	{
		Filename_posi[i] << saveFolder+"/GrayCodeImage/ProjectionGrayCode/posi" << std::setw(2) << std::setfill('0') << i << ".bmp";
		Filename_nega[i] << saveFolder+"/GrayCodeImage/ProjectionGrayCode/nega" << std::setw(2) << std::setfill('0') << i << ".bmp";
		// �ǂݍ���
		posi_img[i] = cv::imread(Filename_posi[i].str(), 1);
		nega_img[i] = cv::imread(Filename_nega[i].str(), 1);
		Filename_posi[i] << std::endl;
		Filename_nega[i] << std::endl;
		// �ǂݍ��ޖ���������Ȃ�������O���C�R�[�h�摜����蒼��
		if(posi_img[i].empty() || nega_img[i].empty()){
			std::cout << "ERROR(1)�F���e�p�̃O���C�R�[�h�摜���s�����Ă��܂��B" << std::endl;
			std::cout << "ERROR(2)�F�O���C�R�[�h�摜���쐬���܂��B" << std::endl;
			makeGraycodeImage();
			code_projection(projection);
			return;
		}
	}

	/***** �O���C�R�[�h���e & �B�e *****/
	/*  �S��ʕ\���p�E�B���h�E�̍쐬  */
	cv::namedWindow("check", cv::WINDOW_NORMAL);


	// �|�W�p�^�[�����e & �B�e

	std::cout << "�|�W�p�^�[���B�e��" << std::endl;
	for( unsigned int i = 0; i < c->g.all_bit; ++i ) 
	{
		// ���e
		projection->img_projection(posi_img[i]);

		// �x���҂�
		cv::waitKey(2.0*delay);
		// �L���v�`��
		cv::Mat cap;
		camDev->captureImage(cap);
		camDev->captureImage(cap);

		cv::waitKey(1);
		cv::imshow("check", cap);

		// �|�W�p�^�[���B�e���ʂ�ۑ�
		// ����
		if(i < c->g.h_bit)
			Filename_posi_cam[i] << saveFolder+"/GrayCodeImage/CaptureImage/CameraImg" << HORIZONTAL << "_" << std::setw(2) << std::setfill('0') << i+1 << "_" << POSI << ".bmp"; 
		// �c��
		else
			Filename_posi_cam[i] << saveFolder+"/GrayCodeImage/CaptureImage/CameraImg" << VERTICAL << "_" << std::setw(2) << std::setfill('0') << i-c->g.h_bit+1 << "_" << POSI << ".bmp"; 
		
		// �ۑ�
		cv::imwrite(Filename_posi_cam[i].str(), cap);
		Filename_posi_cam[i] << std::endl;
	}

	// �l�K�p�^�[�����e & �B�e
	std::cout << "�l�K�p�^�[���B�e��" << std::endl;
	for( unsigned int i = 0; i < c->g.all_bit; ++i ) 
	{
		// ���e
		projection->img_projection(nega_img[i]);

		// �x���҂�
		cv::waitKey(2*delay);
		// �L���v�`��
		cv::Mat cap;
		camDev->captureImage(cap);
		camDev->captureImage(cap);

		cv::waitKey(1);
		cv::imshow("check", cap);

		// ����
		if(i < c->g.h_bit)
			Filename_nega_cam[i] << saveFolder+"/GrayCodeImage/CaptureImage/CameraImg" << HORIZONTAL << "_" << std::setw(2) << std::setfill('0') << i+1 << "_" << NEGA << ".bmp"; 
		// �c��
		else
			Filename_nega_cam[i] << saveFolder+"/GrayCodeImage/CaptureImage/CameraImg" << VERTICAL << "_" << std::setw(2) << std::setfill('0') << i-c->g.h_bit+1 << "_" << NEGA << ".bmp"; 
		
		// �ۑ�
		cv::imwrite(Filename_nega_cam[i].str(), cap);
		Filename_nega_cam[i] << std::endl;
	}

	/***** ���e & �B�e�I�� *****/

	cv::waitKey(2*delay);

	cv::destroyWindow("check");

	/**** �I�� *****/

	// �������̊J��
	delete[] posi_img;
	delete[] nega_img;
	delete[] Filename_posi;
	delete[] Filename_nega;
	delete[] Filename_posi_cam;
	delete[] Filename_nega_cam;
}


/***************
** ��l���֘A **
****************/

/**
 * @brief   �J�����B�e�摜��ǂݍ��ފ֐�
 * 
 * @param   mat[in,out]		�B�e�摜
 * @param   div_bin[in]		�ԍ�
 * @param   vh[in]			�ԍ�
 * @param   pn[in]			�ԍ�
 */
void GRAYCODE::loadCam(cv::Mat &mat, int div_bin, bool vh, bool pn)
{
	char buf[256];
	std::string folder = saveFolder + "/GrayCodeImage/CaptureImage/CameraImg%d_%02d_%d.bmp";
	sprintf_s(buf, folder.c_str(), vh, div_bin, pn);
	mat = cv::imread(buf, 0);
}


/**
 * @brief   �}�X�N���쐬����C���^�t�F�[�X
 * 
 * @param   mask[in,out]	�}�X�N�摜		
 */
void GRAYCODE::makeMask(cv::Mat &mask)
{
	cv::Mat posi_img;
	cv::Mat nega_img;

	// �}�X�N�摜����
	cv::Mat mask_vert, mask_hor;
	static int useImageNumber = 6;
	// y�����̃O���C�R�[�h�摜�ǂݍ���
	loadCam(posi_img, useImageNumber, 0, 1);
	loadCam(nega_img, useImageNumber, 0, 0);

	// ���̃}�X�N�摜Y����
	makeMaskFromCam(posi_img, nega_img, mask_vert);

	// x�����̃O���C�R�[�h�摜�ǂݍ���
	loadCam(posi_img, useImageNumber, 1, 1);
	loadCam(nega_img, useImageNumber, 1, 0);

	// ���̃}�X�N�摜X����
	makeMaskFromCam(posi_img, nega_img, mask_hor);

	// X��Y��OR�����
	// �}�X�N�O�͂ǂ�������Ȃ̂ō�
	// �}�X�N���́i���_�I�ɂ́j�K����������ł�����������Ȃ̂ŁA���ɂȂ�
	// ���ۂ͂��܉��m�C�Y���c���Ă��܂�
	cv::bitwise_or(mask_vert, mask_hor, mask);

	// �c�������܉��m�C�Y�������i���S�}�����S�}���œK�p�����t�ɂȂ�j
	dilate(mask, mask, cv::Mat(), cv::Point(-1, -1), 5);
	erode(mask, mask, cv::Mat(), cv::Point(-1, -1), 5);

	cv::imwrite(saveFolder+"/GrayCodeImage/mask.bmp", mask);
}



/**
 * @brief   �O���C�R�[�h�̉摜�𗘗p���ă}�X�N�𐶐�����֐�
 * 
 * @param   posi[in,out]			�|�W�摜
 * @param   nega[in,out]			�l�K�摜
 * @param   result[in,out]			���ʉ摜
 * @param   thresholdValue[in]		臒l
 *
 * @note	�|�W�ƃl�K�̍����������thresholdValue�ȏ�̋P�x�̃s�N�Z���𔒂ɂ���
 */
void GRAYCODE::makeMaskFromCam(cv::Mat &posi, cv::Mat &nega, cv::Mat &result, int thresholdValue)
{
	result = cv::Mat::zeros(cv::Size(cam_width, cam_height), CV_8UC1);

	for(int j=0; j<cam_height; ++j)
	{
		for(int i=0; i<cam_width; ++i)
		{
			int posi_i = posi.at<uchar>(j, i);
			int nega_i = nega.at<uchar>(j, i);

			if (abs(posi_i - nega_i) > thresholdValue){
				result.at<uchar>(j, i) = 255;
			}else{
				result.at<uchar>(j, i) = 0;
			}
		}
	}
}


/**
 * @brief   �B�e�摜��2�l��������C���^�t�F�[�X	
 */
void GRAYCODE::make_thresh()
{
	cv::Mat posi_img;
	cv::Mat nega_img;
	cv::Mat Geometric_thresh_img;  // 2�l�����ꂽ�摜
	cv::Mat mask;

	// �}�X�N�𐶐�
	makeMask(mask);

	int h_bit = (int)ceil( log(prj_height+1) / log(2) );
	int w_bit = (int)ceil( log(prj_width+1) / log(2) );
	int all_bit = h_bit + w_bit;

	std::cout << "��l���J�n" << std::endl;
	// �A�ԂŃt�@�C������ǂݍ���
	for( int i = 0; i < h_bit; ++i ) 
	{
		// �ǂݍ���
		char buf[256];
		// �|�W�p�^�[���ǂݍ���
		loadCam(posi_img, i+1, 0, 1);
		// �l�K�p�^�[���ǂݍ���
		loadCam(nega_img, i+1, 0, 0);

		// 2�l��
		cv::Mat masked_img;
		thresh( posi_img, nega_img, Geometric_thresh_img, 0 );
		// �}�X�N��K�p����2�l��
		Geometric_thresh_img.copyTo( masked_img, mask );
		std::string folder = saveFolder + "/GrayCodeImage/ThresholdImage/Geometric_thresh%02d.bmp";
		sprintf_s(buf, folder.c_str(), i);
		cv::imwrite(buf, masked_img);

		std::cout << i << ", ";
	}
	for( int i = 0; i < w_bit; ++i ) 
	{
		// �ǂݍ���
		char buf[256];
		// �|�W�p�^�[���ǂݍ���
		loadCam(posi_img, i+1, 1, 1);
		// �l�K�p�^�[���ǂݍ���
		loadCam(nega_img, i+1, 1, 0);

		// 2�l��
		cv::Mat masked_img;
		thresh( posi_img, nega_img, Geometric_thresh_img, 0 );
		// �}�X�N��K�p����2�l��
		Geometric_thresh_img.copyTo( masked_img, mask );
		std::string folder = saveFolder + "/GrayCodeImage/ThresholdImage/Geometric_thresh%02d.bmp";
		sprintf_s(buf, folder.c_str(), i+h_bit);
		cv::imwrite(buf, masked_img);

		std::cout << i+h_bit << ", ";
	}
	std::cout << std::endl;
	std::cout << "��l���I��" << std::endl;
}


/**
 * @brief   ���ۂ�2�l������ 
 * 
 * @param   posi[in,out]			�|�W�摜
 * @param   nega[in,out]			�l�K�摜
 * @param   result[in,out]			���ʉ摜
 * @param   thresholdValue[in]		臒l		
 */
void GRAYCODE::thresh( cv::Mat &posi, cv::Mat &nega, cv::Mat &thresh_img, int thresh_value )
{
	thresh_img = cv::Mat(posi.rows, posi.cols, CV_8UC1);
	for( int y = 0; y < posi.rows; ++y ) 
	{
		for(int x = 0; x < posi.cols; ++x ) 
		{
			int posi_pixel = posi.at<uchar>( y, x );
			int nega_pixel = nega.at<uchar>( y, x );

			// thresh_value���傫�����ǂ����œ�l��
			if( posi_pixel - nega_pixel >= thresh_value )
				thresh_img.at<uchar>( y, x ) = 255;
			else
				thresh_img.at<uchar>( y, x ) = 0;
		}
	}
}


/***********************************
** �v���W�F�N�^�ƃJ�����̑Ή��t�� **
************************************/

/**
 * @brief   2�l���R�[�h����	
 */
void GRAYCODE::code_restore()
{
	// 2�l���R�[�h����
	for( unsigned int i = 0; i < c->g.all_bit; ++i )
	{
		char buf[256];
		std::string folder = saveFolder + "/GrayCodeImage/ThresholdImage/Geometric_thresh%02d.bmp";
		sprintf_s(buf, folder.c_str(), i);
		cv::Mat a = cv::imread(buf, 0);

		for( int y = 0; y < cam_height; ++y )
		{
			for( int x = 0; x < cam_width; ++x )
			{
				if( a.at<uchar>( y, x ) == 255)
					c->graycode[y][x] = ( 1 << (c->g.all_bit-i-1) ) | c->graycode[y][x]; 
			}
		}
	}

	c->code_map->clear();

	// �A�z�z��ŃO���C�R�[�h�̒l�̏ꏊ�ɍ��W���i�[
	for( int y = 0; y < cam_height; ++y )
	{
		for( int x = 0; x < cam_width; ++x ) 
		{
			int a = c->graycode[y][x];
			if( a != 0 )
				(*c->code_map)[a] = cv::Point(x, y);

			// ������
			c->ProCam[y][x] = cv::Point(-1, -1);
		}
	}

	// 0�Ԗڂ͎g��Ȃ�
	(*c->code_map)[0] = cv::Point(-1, -1);

	// �v���W�F�N�^�ƃJ�����̑Ή��t��
	for( int y = 0; y < prj_height; ++y ) 
	{
		for( int x = 0; x < prj_width; ++x ) 
		{
			// �O���C�R�[�h�擾
			int a = c->g.graycode[y][x];
			// map���ɑ��݂��Ȃ��R�[�h�i�J�����ŎB�e����肭�����Ȃ����������j�̏ꏊ�ɂ̓G���[�l-1���i�[
			if ( (*c->code_map).find(a) == (*c->code_map).end() ) {
				c->CamPro[y][x] = cv::Point(-1, -1);
			}
			// ���݂���ꍇ�́A�Ή�����O���C�R�[�h�̍��W���i�[
			else {
				c->CamPro[y][x] = (*c->code_map)[a];
				c->ProCam[(*c->code_map)[a].y][(*c->code_map)[a].x] = cv::Point(x, y);
			}
		}
	}
}


/**
 * @brief   �v���W�F�N�^ - �J�����\���̏�����	
 */
void GRAYCODE::initCorrespondence()
{
	initGraycode();

	for( int y = 0; y < cam_height; y++ ) {
		for( int x = 0; x < cam_width; x++ ){
			c->graycode[y][x] = 0;
		}
	}
}


/**
 * @brief   �Ή��t�����s���C���^�[�t�F�[�X	
 */
void GRAYCODE::makeCorrespondence()
{
	initCorrespondence();
	code_restore();
}


/***********************************
** ���̑��i�p�r�s���ȉߋ��̈╨�j **
************************************/


/**
 * @brief   �J�����B�e�̈悩��v���W�F�N�^���e�̈��؂�o��
 * 
 * @param   src[in]			���͉摜
 * @param   dst[in,out]		�o�͉摜
 */
void GRAYCODE::transport_camera_projector(const cv::Mat &src, cv::Mat &dst)
{
	cv::Mat src_resize;  // ���T�C�Y�����摜
	cv::resize( src, src_resize, cv::Size(cam_width, cam_height) );

	dst = cv::Mat( prj_height, prj_width, CV_8UC3, cv::Scalar(0, 0, 0) );  // �􉽕␳���ꂽ�摜�i���e�摜�j

	for( int y = 0; y < prj_height; ++y ) 
	{
		for( int x = 0; x < prj_width; ++x ) 
		{
			cv::Point p = c->CamPro[y][x];
			if( p.x != -1 ) {
				if(src_resize.at<uchar>( p.y, 3*p.x ) != 0 && src_resize.at<uchar>( p.y, 3*p.x+1 ) != 0 && src_resize.at<uchar>( p.y, 3*p.x+2 ) != 0)
				{
					dst.at<uchar>( y, 3*x ) = src_resize.at<uchar>( p.y, 3*p.x );      // B
					dst.at<uchar>( y, 3*x+1 ) = src_resize.at<uchar>( p.y, 3*p.x+1 );  // G
					dst.at<uchar>( y, 3*x+2 ) = src_resize.at<uchar>( p.y, 3*p.x+2 );  // R
				}
			}
		}
	}
}


/**
 * @brief   ���͉摜���J�����B�e�̈�ɕό`
 * 
 * @param   src[in]			���͉摜
 * @param   dst[in,out]		�o�͉摜
 */
void GRAYCODE::transport_projector_camera(const cv::Mat &src, cv::Mat &dst)
{
	cv::Mat src_resize;  // ���T�C�Y�����摜
	cv::resize( src, src_resize, cv::Size(prj_width, prj_width) );

	dst = cv::Mat( cam_height, cam_width, CV_8UC3, cv::Scalar(0, 0, 0) );  // �􉽕␳���ꂽ�摜�i���e�摜�j

	for( int y = 0; y < prj_height; ++y ) 
	{
		for( int x = 0; x < prj_width; ++x )
		{
			cv::Point p = c->CamPro[y][x];
			if( p.x != -1 )
			{
				dst.at<uchar>( p.y, 3*p.x ) = src_resize.at<uchar>( y, 3*x );      // B
				dst.at<uchar>( p.y, 3*p.x+1 ) = src_resize.at<uchar>( y, 3*x+1 );  // G
				dst.at<uchar>( p.y, 3*p.x+2 ) = src_resize.at<uchar>( y, 3*x+2 );  // R
			}
		}
	}
}


/***********************************
** �ǉ� **
************************************/


/**
 * @brief   �J�������W�ɑ΂���v���W�F�N�^�̑Ή��_��Ԃ�
 * 
 * @param   projPoint[in,out]		�v���W�F�N�^���W�l
 * @param   imagePoint[in]			�J�������W�l
 */
void GRAYCODE::getCorrespondProjPoints(std::vector<cv::Point2f> &projPoint, const std::vector<cv::Point2f> &imagePoint)
{
	for (int i=0; i < imagePoint.size(); ++i)
	{
		cv::Point2f point = c->ProCam[int(imagePoint[i].y+0.5f)][int(imagePoint[i].x+0.5f)];

		if( point.x != -1.0f) {
			projPoint.emplace_back(point);
		}
	}
}



/**
 * @brief   �J�������W�ɑ΂���v���W�F�N�^�̑Ή��_��Ԃ�(�����x��)
 * 
 * @param   projPoint[in,out]		�v���W�F�N�^���W�l
 * @param   imagePoint[in]			�J�������W�l
 * @param	size[in]				�l������T�C�Y
 *
 * @note	���͂̍��W�l�����Homography���s���m�C�Y����
 */
void GRAYCODE::getCorrespondSubPixelProjPoints(std::vector<cv::Point2f> &projPoint, const std::vector<cv::Point2f> &imagePoint, int size)
{
	for (int i=0; i < imagePoint.size(); ++i)
	{
		std::vector<cv::Point2f> iPoints, pPoints;
		if(imagePoint[i].x > size && imagePoint[i].x+size < cam_width && imagePoint[i].y > size && imagePoint[i].y+size < cam_height)
		{
			// �̈斈�̑Ή��_
			for( float h = imagePoint[i].y-size; h < imagePoint[i].y+size; h+=1.0f){
				for( float w = imagePoint[i].x-size; w < imagePoint[i].x+size; w+=1.0f){
					cv::Point2f point = c->ProCam[int(h+0.5f)][int(w+0.5f)];
					if( point.x != -1.0f) {
						iPoints.emplace_back(cv::Point2f(w, h));
						pPoints.emplace_back(point);
					}
				}
			}

			// �Ή��_���m��Homography�̌v�Z
			cv::Mat H = cv::findHomography(iPoints, pPoints, CV_RANSAC, 2.0);
			// Homography���g���ă`�F�b�J�[�p�^�[���̌�_���ˉe
			cv::Point3d Q = cv::Point3d(cv::Mat(H * cv::Mat(cv::Point3d(imagePoint[i].x,imagePoint[i].y,1.0))));
			projPoint.emplace_back(cv::Point2f(Q.x/Q.z, Q.y/Q.z));
		}
		else
		{
			cv::Point2f point = c->ProCam[int(imagePoint[i].y+0.5f)][int(imagePoint[i].x+0.5f)];

			if( point.x != -1.0f) {
				projPoint.emplace_back(point);
			}
		}
	}
}


/**
 * @brief   �Ή��̂Ƃꂽ�_��S�ĕԂ�
 * 
 * @param   projPoint[in,out]		�v���W�F�N�^���W�l
 * @param   imagePoint[in,out]		�J�������W�l
 */
void GRAYCODE::getCorrespondAllPoints(std::vector<cv::Point2f> &projPoint, std::vector<cv::Point2f> &imagePoint)
{
	for( int y = 0; y < prj_height; ++y ) {
		for( int x = 0; x < prj_width; ++x ) {
			cv::Point p = c->CamPro[y][x];
			if( p.x != -1 ) {
				projPoint.emplace_back(cv::Point2f(x, y));
				imagePoint.emplace_back(cv::Point2f(p.x, p.y));
			}
		}
	}
}