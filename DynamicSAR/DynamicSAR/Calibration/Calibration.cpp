#include "Calibration.h"
#include "../Projection_thread.h"


/**
 * @brief   �O���C�R�[�h���e��ProCam�Ԃ̊􉽑Ή����擾
 * 
 * @param	src[in]				���͉摜(IR�J�����p)
 * @param	ProCam_src[in]		���͉摜(ProCam�p)
 * @param	projection[in,out]	���e�p�N���X
 */
void Calibration::runGetCorrespond(const cv::Mat& src, const cv::Mat& ProCam_src, ProjectionThread *projection)
{
	// �`�F�b�J�[�p�^�[���̌�_��`��(�J����)
	std::vector<cv::Point2f> procam_imagePoint;
	std::vector<cv::Point2f> imagePoint;
	std::vector<cv::Point2f> projPoint;
	cv::Mat color_src, draw_corner, draw_corner2;
	if (src.channels() == 1)
		cv::cvtColor(src, color_src, CV_GRAY2BGR);
	else
		color_src = src.clone();
	bool detect_flag = getCorners(procam_imagePoint, ProCam_src, draw_corner);
	bool detect_flag2 = getCorners(imagePoint, color_src, draw_corner2);
	cv::namedWindow("Image Corner", cv::WINDOW_NORMAL);
	cv::imshow( "Image Corner", draw_corner);
	cv::namedWindow("IR Image Corner", cv::WINDOW_NORMAL);
	cv::imshow( "IR Image Corner", draw_corner2);

	// �J������Ń`�F�b�J�[�p�^�[�������o�ł�����
	if (detect_flag && detect_flag2)
	{
		// �O���C�R�[�h���e
		graycode->code_projection(projection);
		graycode->make_thresh();
		graycode->makeCorrespondence();


		// �`�F�b�J�[�p�^�[���̌�_��`��(�v���W�F�N�^)
		cv::Mat dst;
		graycode->getCorrespondSubPixelProjPoints(projPoint, procam_imagePoint, 20);
		graycode->transport_camera_projector(ProCam_src, dst);

		if(procam_imagePoint.size() == projPoint.size()) {
			cv::drawChessboardCorners( dst, cornerSize, projPoint, true );
		} else {
			cv::drawChessboardCorners( dst, cornerSize, projPoint, false );
		}

		cv::namedWindow("prj", cv::WINDOW_NORMAL);
		cv::imshow("prj",dst);
		cv::imwrite("./prj.jpg", dst );

		// �`�F�b�J�[�p�^�[���̌�_�𓊉e
		projection->img_projection(dst);

		Sleep(3000);

		// �v���W�F�N�^��Ń`�F�b�J�[�p�^�[�������o�ł�����
		if(procam_imagePoint.size() == projPoint.size())
		{
			std::cout << calib_count+1 << "��ڂ̌��o����" << std::endl;

			// �ǉ�
			myWorldPoints.emplace_back(worldPoint);
			myCameraPoints.emplace_back(imagePoint);
			myProjectorPoints.emplace_back(projPoint);

			// ���o�_�̕ۑ�
			std::string fileName = saveCalibFolder + "/checkerPoint" + std::to_string(calib_count) + ".xml";
			cv::FileStorage fs(fileName, cv::FileStorage::WRITE);
			cv::write(fs,"world", worldPoint);
			cv::write(fs,"camera", imagePoint);
			cv::write(fs,"projector", projPoint);

			calib_count++;
		}else{
			std::cout << "�v���W�F�N�^��̃`�F�b�J�[�p�^�[���̌��o���s" << std::endl;
		}
	} else {
		std::cout << "�J������̃`�F�b�J�[�p�^�[���̌��o���s" << std::endl;
	}
}


/**
 * @brief   �Ή��_�̃t�@�C������ProCam�L�����u���[�V����	
 *
 * @param   camSize[in]				�J�����̃T�C�Y
 * @param   projSize[in]			�v���W�F�N�^�̃T�C�Y
 */
void Calibration::runCorrespondFileCalib(const cv::Size &camSize, const cv::Size &projSize)
{
	myWorldPoints.clear();
	myCameraPoints.clear();
	myProjectorPoints.clear();

	// �t�@�C���̒T��
	WIN32_FIND_DATA ffd;
	HANDLE hF;

	std::string imageFlieName = saveCalibFolder+"/checkerPoint*.*";
	hF = FindFirstFile( imageFlieName.c_str(), &ffd);
	if (hF != INVALID_HANDLE_VALUE) {
		// �t�H���_���̃t�@�C���̒T��
		do {
			std::string fullpath = ffd.cFileName;

			std::vector<cv::Point3f> worldPoint;
			std::vector<cv::Point2f> imagePoint;
			std::vector<cv::Point2f> projPoint;

			// xml�t�@�C���̓ǂݍ���
			cv::FileStorage cvfs(fullpath, cv::FileStorage::READ);

			cvfs["world"] >> worldPoint;
			cvfs["camera"] >> imagePoint;
			cvfs["projector"] >> projPoint;
						
			// �ǉ�
			myWorldPoints.emplace_back(worldPoint);
			myCameraPoints.emplace_back(imagePoint);
			myProjectorPoints.emplace_back(projPoint);

		} while (FindNextFile(hF, &ffd ) != 0);
		FindClose(hF);
	}

	if(myWorldPoints.size() > 0)
	{
		std::cout << "�L�����u���[�V�������c" << std::endl;

		// �L�����u���[�V����
		proCamCalibration(myWorldPoints, myCameraPoints, myProjectorPoints, camSize, projSize);

	}
}



/**
 * @brief   �摜����`�F�b�J�[�p�^�[���̌�_���擾
 * 
 * @param   imagePoint[in,out]		�摜�̍��W�l
 * @param   image[in]				�摜
 * @param   draw_image[in,out]		��_�`��摜
 */
bool Calibration::getCorners(std::vector<cv::Point2f> &imagePoint, const cv::Mat &image, cv::Mat &draw_image)
{
	// ��_���o
	bool detect;

	// �`�F�X�{�[�h�̏ꍇ
	if (use_chessboard)
		detect = cv::findChessboardCorners( image, cornerSize, imagePoint);
	else
		detect = cv::findCirclesGrid( image, cornerSize, imagePoint, cv::CALIB_CB_ASYMMETRIC_GRID );

	// ���o�_�̕`��
	image.copyTo(draw_image);
	if(detect) {

		// �T�u�s�N�Z������
		cv::Mat	gray;
		if(image.channels() == 3) {
			cv::cvtColor( image, gray, cv::COLOR_BGR2GRAY );
		} else {
			gray = image.clone();
		}

		cv::Size winSize;
		// �𑜓x�ɂ���ăT�u�s�N�Z������̒T���͈͂�ς���
		if (image.cols > 640) {
			winSize = cv::Size(11, 11);
		} else {
			winSize = cv::Size(3, 3);
		}
		if (use_chessboard)
			cv::cornerSubPix( gray, imagePoint, winSize, cv::Size( -1, -1 ), cv::TermCriteria( cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 20, 0.001 ) );

		cv::drawChessboardCorners( draw_image, cornerSize, imagePoint, true );
	} else {
		cv::drawChessboardCorners( draw_image, cornerSize, imagePoint, false );
	}

	return detect;
}


/**
 * @brief   �ē��e�덷�̌v�Z
 * 
 * @param   worldPoints[in]			���E���W�l
 * @param   cameraPoints[in]		�J�������W�l
 * @param   projectorPoints[in]		�v���W�F�N�^���W�l
 * @param   cam_error[in,out]		�J�����̍ē��e�덷
 * @param   proj_error[in,out]		�v���W�F�N�^�̍ē��e�덷
 */
void Calibration::calcReprojectionError(const std::vector<std::vector<cv::Point3f>> &worldPoints, const std::vector<std::vector<cv::Point2f>> &cameraPoints, const std::vector<std::vector<cv::Point2f>> &projectorPoints,
											double &cam_error, double &proj_error)
{
	cv::Mat camera_R = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat camera_T = cv::Mat::zeros(3, 1, CV_64F);

	// �J�����̍ē��e�덷
	for(int i=0; i<worldPoints.size(); ++i)
	{
		cv::Mat rvec, tvec;
		cv::solvePnP(worldPoints[i], cameraPoints[i], cam_K, cam_dist, rvec, tvec);		// �`�F�b�J�[�p�^�[���̈ʒu���o

		cv::Mat rmat;
		cv::Rodrigues(rvec, rmat);

		// �`�F�b�J�[�p�^�[�����S����J�������S�ɍ��W�ϊ�
		rmat = rmat.t();	// �]�u�s��

		cv::Mat extrinsic(4, 4, CV_64F);
		extrinsic.at<double>(0,0) = rmat.at<double>(0,0);
		extrinsic.at<double>(0,1) = rmat.at<double>(0,1);
		extrinsic.at<double>(0,2) = rmat.at<double>(0,2);
		extrinsic.at<double>(1,0) = rmat.at<double>(1,0);
		extrinsic.at<double>(1,1) = rmat.at<double>(1,1);
		extrinsic.at<double>(1,2) = rmat.at<double>(1,2);
		extrinsic.at<double>(2,0) = rmat.at<double>(2,0);
		extrinsic.at<double>(2,1) = rmat.at<double>(2,1);
		extrinsic.at<double>(2,2) = rmat.at<double>(2,2);
		extrinsic.at<double>(0,3) = cv::Mat(-rmat*tvec).at<double>(0,0);
		extrinsic.at<double>(1,3) = cv::Mat(-rmat*tvec).at<double>(1,0);
		extrinsic.at<double>(2,3) = cv::Mat(-rmat*tvec).at<double>(2,0);
		extrinsic.at<double>(3,0) = 0.0;
		extrinsic.at<double>(3,1) = 0.0;
		extrinsic.at<double>(3,2) = 0.0;
		extrinsic.at<double>(3,3) = 1.0;

		// �`�F�b�J�[�p�^�[���̌�_�ʒu
		std::vector<cv::Point3f> new_worldPoint;
		for(int j=0; j<worldPoints[0].size(); ++j)
		{
			cv::Mat checker_pos = extrinsic.inv() * cv::Mat((cv::Mat_<double>(4,1) << worldPoints[i][j].x, worldPoints[i][j].y, worldPoints[i][j].z, 1.0));		// �`�F�b�J�[�p�^�[���̈ʒu
			new_worldPoint.emplace_back(cv::Point3f(checker_pos.at<double>(0)/checker_pos.at<double>(3), checker_pos.at<double>(1)/checker_pos.at<double>(3), checker_pos.at<double>(2)/checker_pos.at<double>(3)));
		}

		// �J�������W�ւ̓��e
		std::vector<cv::Point2f> cam_projection;
		cv::projectPoints(new_worldPoint, camera_R, camera_T, cam_K, cam_dist, cam_projection);

		// �v���W�F�N�^���W�ւ̓��e
		std::vector<cv::Point2f> proj_projection;
		cv::projectPoints(new_worldPoint, R, T, proj_K, proj_dist, proj_projection);

		// �J�������W�ւ̍ē��e�덷
		for(int j=0; j<cameraPoints[0].size(); ++j)
		{
			cam_error += std::sqrt((cameraPoints[i][j].x - cam_projection[j].x)*(cameraPoints[i][j].x - cam_projection[j].x) + (cameraPoints[i][j].y - cam_projection[j].y)*(cameraPoints[i][j].y - cam_projection[j].y));
		}

		// �v���W�F�N�^���W�ւ̍ē��e�덷
		for(int j=0; j<projectorPoints[0].size(); ++j)
		{
			proj_error += std::sqrt((projectorPoints[i][j].x - proj_projection[j].x)*(projectorPoints[i][j].x - proj_projection[j].x) + (projectorPoints[i][j].y - proj_projection[j].y)*(projectorPoints[i][j].y - proj_projection[j].y));
		}
	}

	double sum = worldPoints.size() * worldPoints[0].size();

	cam_error /= sum;
	proj_error /= sum;
}



/**
 * @brief   �v���W�F�N�^�ƃJ�����̃L�����u���[�V����
 * 
 * @param   worldPoints[in]			���E���W�l
 * @param   cameraPoints[in]		�J�������W�l
 * @param   projectorPoints[in]		�v���W�F�N�^���W�l
 * @param   camSize[in]				�J�����̃T�C�Y
 * @param   projSize[in]			�v���W�F�N�^�̃T�C�Y
 */
void Calibration::proCamCalibration(const std::vector<std::vector<cv::Point3f>> &worldPoints, const std::vector<std::vector<cv::Point2f>> &cameraPoints, const std::vector<std::vector<cv::Point2f>> &projectorPoints,
										const cv::Size &camSize, const cv::Size &projSize)
{
	// �J�����L�����u���[�V����
	double cam_error = cv::calibrateCamera(worldPoints, cameraPoints, camSize, cam_K, cam_dist, cam_R, cam_T, cv::CALIB_FIX_K3, 
									cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 200, DBL_EPSILON));

	// �v���W�F�N�^�L�����u���[�V����
	double proj_error = cv::calibrateCamera(worldPoints, projectorPoints, projSize, proj_K, proj_dist, proj_R, proj_T, cv::CALIB_FIX_K3, 
									cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 200, DBL_EPSILON));

	// �X�e���I�œK��
	double stereo_error = cv::stereoCalibrate(worldPoints, cameraPoints, projectorPoints, cam_K, cam_dist, proj_K, proj_dist, camSize, R, T, E, F, 
                                                cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 200, DBL_EPSILON), cv::CALIB_FIX_INTRINSIC /*cv::CALIB_USE_INTRINSIC_GUESS*/+cv::CALIB_FIX_K3);

	// �œK����̍ē��e�덷�̌v�Z
	double cam_error2 = 0;
	double proj_error2 = 0;
	calcReprojectionError(worldPoints, cameraPoints, projectorPoints, cam_error2, proj_error2);
	
	// ����
	std::cout << "***** Calibration results *****" << std::endl << std::endl;

	std::cout	<< "Camera Calibration results:" << std::endl
				<< " - Reprojection error: " << cam_error << std::endl
				<< " - Reprojection error2: " << cam_error2 << std::endl
				<< " - K:\n" << cam_K << std::endl
				<< " - Distortion:" << cam_dist << std::endl << std::endl;

	std::cout	<< "Projector Calibration results:" << std::endl
				<< " - Reprojection error: " << proj_error << std::endl
				<< " - Reprojection error2: " << proj_error2 << std::endl
				<< " - K:\n" << proj_K << std::endl
				<< " - Distortion:" << proj_dist << std::endl << std::endl;

	std::cout	<< "Stereo Calibration results:" << std::endl
				<< " - Reprojection error: " << stereo_error << std::endl
				<< " - R:\n" << R << std::endl
				<< " - T:" << T << std::endl << std::endl;


	// ���ʂ̕ۑ�
	cv::FileStorage fs(saveCalibFolder+"/calibration.xml", cv::FileStorage::WRITE);
	fs << "cam_reprojection_error" << cam_error
	   << "cam_reprojection_error2" << cam_error2
	   << "proj_reprojection_error" << proj_error
	   << "proj_reprojection_error2" << proj_error2
	   << "stereo_reprojection_error" << stereo_error
	   << "cam_K" << cam_K << "cam_dist" << cam_dist
	   << "cam_R" << cam_R << "cam_T" << cam_T
       << "proj_K" << proj_K << "proj_dist" << proj_dist
       << "proj_R" << proj_R << "proj_T" << proj_T
	   << "R" << R << "T" << T << "E" << E << "F" << F;
	fs.release();


	// �J�����p�����[�^�̕ۑ�
	cv::FileStorage fs2(saveCalibFolder+"/camera.xml", cv::FileStorage::WRITE);

	fs2 << "cameraMatrix" << cam_K;
	fs2 << "distCoeffs" << cam_dist;
	fs2 << "imageWidth" << camSize.width;
	fs2 << "imageHeight" << camSize.height;

	calib_flag = true;
}


/**
 * @brief   �L�����u���[�V�������ʂ̓ǂݍ���
 * 
 * @param   fileName[in]		�L�����u���[�V�����t�@�C��
 */
void Calibration::loadCalibParam(const std::string &fileName)
{
	// xml�t�@�C���̓ǂݍ���
	cv::FileStorage cvfs(fileName, cv::FileStorage::READ);

	cvfs["cam_K"] >> cam_K;
	cvfs["cam_dist"] >> cam_dist;
	cvfs["cam_R"] >> cam_R;
	cvfs["cam_T"] >> cam_T;
	cvfs["proj_K"] >> proj_K;
	cvfs["proj_dist"] >> proj_dist;
	cvfs["proj_R"] >> proj_R;
	cvfs["proj_T"] >> proj_T;
	cvfs["R"] >> R;
	cvfs["T"] >> T;
	cvfs["E"] >> E;
	cvfs["F"] >> F;

	calib_flag = true;
}


/**
 * @brief   �������e�ϊ��s��̎擾(�J����)	
 */
cv::Mat Calibration::getCamPerspectiveMat()
{
	// ��]�ƕ��i������
	cv::Mat extrinsic = cv::Mat::eye(4, 4, CV_64F);

	// �����p�����[�^�̕ό`
	cv::Mat intrinsic(3, 4, CV_64F);
	intrinsic.at<double>(0,0) = cam_K.at<double>(0,0);
	intrinsic.at<double>(0,1) = cam_K.at<double>(0,1);
	intrinsic.at<double>(0,2) = cam_K.at<double>(0,2);
	intrinsic.at<double>(1,0) = cam_K.at<double>(1,0);
	intrinsic.at<double>(1,1) = cam_K.at<double>(1,1);
	intrinsic.at<double>(1,2) = cam_K.at<double>(1,2);
	intrinsic.at<double>(2,0) = cam_K.at<double>(2,0);
	intrinsic.at<double>(2,1) = cam_K.at<double>(2,1);
	intrinsic.at<double>(2,2) = cam_K.at<double>(2,2);
	intrinsic.at<double>(0,3) = 0.0;
	intrinsic.at<double>(1,3) = 0.0;
	intrinsic.at<double>(2,3) = 0.0;

	return intrinsic * extrinsic;
}


/**
 * @brief   �������e�ϊ��s��̎擾(�v���W�F�N�^)	
 */
cv::Mat Calibration::getProjPerspectiveMat()
{
	// ��]�ƕ��i������
	cv::Mat extrinsic(4, 4, CV_64F);
	extrinsic.at<double>(0,0) = R.at<double>(0,0);
	extrinsic.at<double>(0,1) = R.at<double>(0,1);
	extrinsic.at<double>(0,2) = R.at<double>(0,2);
	extrinsic.at<double>(1,0) = R.at<double>(1,0);
	extrinsic.at<double>(1,1) = R.at<double>(1,1);
	extrinsic.at<double>(1,2) = R.at<double>(1,2);
	extrinsic.at<double>(2,0) = R.at<double>(2,0);
	extrinsic.at<double>(2,1) = R.at<double>(2,1);
	extrinsic.at<double>(2,2) = R.at<double>(2,2);
	extrinsic.at<double>(0,3) = T.at<double>(0,0);
	extrinsic.at<double>(1,3) = T.at<double>(1,0);
	extrinsic.at<double>(2,3) = T.at<double>(2,0);
	extrinsic.at<double>(3,0) = 0.0;
	extrinsic.at<double>(3,1) = 0.0;
	extrinsic.at<double>(3,2) = 0.0;
	extrinsic.at<double>(3,3) = 1.0;

	// �����p�����[�^�̕ό`
	cv::Mat intrinsic(3, 4, CV_64F);
	intrinsic.at<double>(0,0) = proj_K.at<double>(0,0);
	intrinsic.at<double>(0,1) = proj_K.at<double>(0,1);
	intrinsic.at<double>(0,2) = proj_K.at<double>(0,2);
	intrinsic.at<double>(1,0) = proj_K.at<double>(1,0);
	intrinsic.at<double>(1,1) = proj_K.at<double>(1,1);
	intrinsic.at<double>(1,2) = proj_K.at<double>(1,2);
	intrinsic.at<double>(2,0) = proj_K.at<double>(2,0);
	intrinsic.at<double>(2,1) = proj_K.at<double>(2,1);
	intrinsic.at<double>(2,2) = proj_K.at<double>(2,2);
	intrinsic.at<double>(0,3) = 0.0;
	intrinsic.at<double>(1,3) = 0.0;
	intrinsic.at<double>(2,3) = 0.0;

	return intrinsic * extrinsic;
}



/**
 * @brief   �J�����ʒu�����[���h���W�Ƃ����ۂ̑Ώە��̂̈ʒu�̎擾
 * 
 * @param   camWorldPoint[in,out]		���[���h���W�l
 * @param   imagePoint[in]				�摜���W�l
 */
void Calibration::getCameraWorldPoint(std::vector<cv::Point3f> &camWorldPoint, const std::vector<cv::Point2f> &imagePoint)
{
	cv::Mat rvec, tvec, rmat;

	// �`�F�b�J�[�p�^�[���̈ʒu���o
	cv::solvePnP(worldPoint, imagePoint, cam_K, cv::Mat(), rvec, tvec);		

	cv::Rodrigues(rvec, rmat);		// ��]�s��ɕϊ�

	// �`�F�b�J�[�p�^�[�����S����J�������S�ɍ��W�ϊ�
	rmat = rmat.t();	// �]�u�s��

	cv::Mat extrinsic(4, 4, CV_64F);
	extrinsic.at<double>(0,0) = rmat.at<double>(0,0);
	extrinsic.at<double>(0,1) = rmat.at<double>(0,1);
	extrinsic.at<double>(0,2) = rmat.at<double>(0,2);
	extrinsic.at<double>(1,0) = rmat.at<double>(1,0);
	extrinsic.at<double>(1,1) = rmat.at<double>(1,1);
	extrinsic.at<double>(1,2) = rmat.at<double>(1,2);
	extrinsic.at<double>(2,0) = rmat.at<double>(2,0);
	extrinsic.at<double>(2,1) = rmat.at<double>(2,1);
	extrinsic.at<double>(2,2) = rmat.at<double>(2,2);
	extrinsic.at<double>(0,3) = cv::Mat(-rmat*tvec).at<double>(0,0);
	extrinsic.at<double>(1,3) = cv::Mat(-rmat*tvec).at<double>(1,0);
	extrinsic.at<double>(2,3) = cv::Mat(-rmat*tvec).at<double>(2,0);
	extrinsic.at<double>(3,0) = 0.0;
	extrinsic.at<double>(3,1) = 0.0;
	extrinsic.at<double>(3,2) = 0.0;
	extrinsic.at<double>(3,3) = 1.0;

	// �`�F�b�J�[�p�^�[���̌�_�ʒu
	for(int i=0; i<worldPoint.size(); ++i)
	{
		cv::Mat checker_pos = extrinsic.inv() * cv::Mat((cv::Mat_<double>(4,1) << worldPoint[i].x, worldPoint[i].y, worldPoint[i].z, 1.0));		// �`�F�b�J�[�p�^�[���̈ʒu
		camWorldPoint.emplace_back(cv::Point3f(checker_pos.at<double>(0)/checker_pos.at<double>(3), checker_pos.at<double>(1)/checker_pos.at<double>(3), checker_pos.at<double>(2)/checker_pos.at<double>(3)));
	}
}