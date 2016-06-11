#include "Graycode.h"
#include "../Projection_thread.h"

/**
 * @brief   保存先のフォルダの生成		
 */
void GRAYCODE::createDirs()
{
	std::string mainFolder = saveFolder+"/GrayCodeImage";
	std::string captureFolder = saveFolder+"/GrayCodeImage/CaptureImage";
	std::string grayFolder = saveFolder+"/GrayCodeImage/ProjectionGrayCode";
	std::string thresholdFolder = saveFolder+"/GrayCodeImage/ThresholdImage";
	_mkdir(mainFolder.c_str());
	// グレイコード撮影画像
	_mkdir(captureFolder.c_str());
	// グレイコード生画像
	_mkdir(grayFolder.c_str());
	// グレイコード撮影画像の二値化した画像
	_mkdir(thresholdFolder.c_str());
}

/***************************
** グレイコードの作成関連 **
****************************/


/**
 * @brief   ビット数の計算とグレイコードの作成
 */
void GRAYCODE::initGraycode()
{
	std::vector<int> bin_code_h(prj_height);  // 2進コード（縦）
	std::vector<int> bin_code_w(prj_width);   // 2進コード（横）
	std::vector<int> graycode_h(prj_height);  // グレイコード（縦）
	std::vector<int> graycode_w(prj_width);   // グレイコード（横）


	/***** 2進コード作成 *****/
	// 行について
	for( int y = 0; y < prj_height; y++ )
		bin_code_h[y] = y + 1;
	// 列について
	for( int x = 0; x < prj_width; x++ )
		bin_code_w[x] = x + 1;

	/***** グレイコード作成 *****/
	// 行について
	for( int y = 0; y < prj_height; y++ )
		graycode_h[y] = bin_code_h[y] ^ ( bin_code_h[y] >> 1 );
	// 列について
	for( int x = 0; x < prj_width; x++ )
		graycode_w[x] = bin_code_w[x] ^ ( bin_code_w[x] >> 1 );
	// 行列を合わせる（行 + 列）
	for( int y = 0; y < prj_height; y++ ) {
		for( int x = 0; x < prj_width; x++ )
			c->g.graycode[y][x] = ( graycode_h[y] << c->g.w_bit) | graycode_w[x];
	}
}


/**
 * @brief   パターンコード画像作成（一度作ればプロジェクタの解像度が変わらない限り作り直す必要はない
 */
void GRAYCODE::makeGraycodeImage()
{
	std::cout << "投影用グレイコード作成中" << std::endl;

	cv::Mat posi_img ( prj_height, prj_width, CV_8UC3, cv::Scalar(0, 0, 0) );
	cv::Mat nega_img ( prj_height, prj_width, CV_8UC3, cv::Scalar(0, 0, 0) );
	int bit = c->g.all_bit-1;
	std::stringstream *Filename_posi = new std::stringstream[c->g.all_bit];  // 書式付入出力
	std::stringstream *Filename_nega = new std::stringstream[c->g.all_bit];  // 書式付入出力

	// ポジパターンコード画像作成
	for( unsigned int z = 0; z < c->g.all_bit; z++) {
		for( int y = 0; y < prj_height; y++ ) {
			for( int x = 0; x < prj_width; x++ ) {
				if( ( (c->g.graycode[y][x] >> (bit-z)) & 1 ) == 0 ) {  // 最上位ビットから順に抽出し，そのビットが0だった時
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
		// 連番でファイル名を保存（文字列ストリーム）
		Filename_posi[z] << saveFolder+"/GrayCodeImage/ProjectionGrayCode/posi" << std::setw(2) << std::setfill('0') << z << ".bmp"; 
		cv::imwrite(Filename_posi[z].str(), posi_img);
		Filename_posi[z] << std::endl;
	}

	// ネガパターンコード画像作成
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
		// 連番でファイル名を保存（文字列ストリーム）
		Filename_nega[z] << saveFolder+"/GrayCodeImage/ProjectionGrayCode/nega" << std::setw(2) << std::setfill('0') << z << ".bmp"; 
		cv::imwrite(Filename_nega[z].str(), nega_img);
		Filename_nega[z] << std::endl;
	}

	delete[] Filename_posi;
	delete[] Filename_nega;
}



/**
 * @brief   パターンコード投影 & 撮影
 *
 * @param	projection[in,out]	投影用クラス
 */
void GRAYCODE::code_projection(ProjectionThread *projection)
{
	// 定数
	typedef enum flag
	{
		POSI = true,
		NEGA = false,
		VERTICAL = true,
		HORIZONTAL = false,
	} flag;


	// 初期化&カメラ起動
	initGraycode();

	cv::Mat *posi_img = new cv::Mat[c->g.all_bit];  // ポジパターン用
	cv::Mat *nega_img = new cv::Mat[c->g.all_bit];  // ネガパターン用

	// 書式付入出力（グレイコード読み込み用）
	std::stringstream *Filename_posi = new std::stringstream[c->g.all_bit]; 
	std::stringstream *Filename_nega = new std::stringstream[c->g.all_bit];
	// 書式付入出力（撮影画像書き込み用）
	std::stringstream *Filename_posi_cam = new std::stringstream[c->g.all_bit]; 
	std::stringstream *Filename_nega_cam = new std::stringstream[c->g.all_bit];

	// 連番でファイル名を読み込む（文字列ストリーム）
	std::cout << "投影用グレイコード画像読み込み中" << std::endl;
	for( unsigned int i = 0; i < c->g.all_bit; i++ ) 
	{
		Filename_posi[i] << saveFolder+"/GrayCodeImage/ProjectionGrayCode/posi" << std::setw(2) << std::setfill('0') << i << ".bmp";
		Filename_nega[i] << saveFolder+"/GrayCodeImage/ProjectionGrayCode/nega" << std::setw(2) << std::setfill('0') << i << ".bmp";
		// 読み込み
		posi_img[i] = cv::imread(Filename_posi[i].str(), 1);
		nega_img[i] = cv::imread(Filename_nega[i].str(), 1);
		Filename_posi[i] << std::endl;
		Filename_nega[i] << std::endl;
		// 読み込む枚数が足りなかったらグレイコード画像を作り直す
		if(posi_img[i].empty() || nega_img[i].empty()){
			std::cout << "ERROR(1)：投影用のグレイコード画像が不足しています。" << std::endl;
			std::cout << "ERROR(2)：グレイコード画像を作成します。" << std::endl;
			makeGraycodeImage();
			code_projection(projection);
			return;
		}
	}

	/***** グレイコード投影 & 撮影 *****/
	/*  全画面表示用ウィンドウの作成  */
	cv::namedWindow("check", cv::WINDOW_NORMAL);


	// ポジパターン投影 & 撮影

	std::cout << "ポジパターン撮影中" << std::endl;
	for( unsigned int i = 0; i < c->g.all_bit; ++i ) 
	{
		// 投影
		projection->img_projection(posi_img[i]);

		// 遅延待ち
		cv::waitKey(2.0*delay);
		// キャプチャ
		cv::Mat cap;
		camDev->captureImage(cap);
		camDev->captureImage(cap);

		cv::waitKey(1);
		cv::imshow("check", cap);

		// ポジパターン撮影結果を保存
		// 横縞
		if(i < c->g.h_bit)
			Filename_posi_cam[i] << saveFolder+"/GrayCodeImage/CaptureImage/CameraImg" << HORIZONTAL << "_" << std::setw(2) << std::setfill('0') << i+1 << "_" << POSI << ".bmp"; 
		// 縦縞
		else
			Filename_posi_cam[i] << saveFolder+"/GrayCodeImage/CaptureImage/CameraImg" << VERTICAL << "_" << std::setw(2) << std::setfill('0') << i-c->g.h_bit+1 << "_" << POSI << ".bmp"; 
		
		// 保存
		cv::imwrite(Filename_posi_cam[i].str(), cap);
		Filename_posi_cam[i] << std::endl;
	}

	// ネガパターン投影 & 撮影
	std::cout << "ネガパターン撮影中" << std::endl;
	for( unsigned int i = 0; i < c->g.all_bit; ++i ) 
	{
		// 投影
		projection->img_projection(nega_img[i]);

		// 遅延待ち
		cv::waitKey(2*delay);
		// キャプチャ
		cv::Mat cap;
		camDev->captureImage(cap);
		camDev->captureImage(cap);

		cv::waitKey(1);
		cv::imshow("check", cap);

		// 横縞
		if(i < c->g.h_bit)
			Filename_nega_cam[i] << saveFolder+"/GrayCodeImage/CaptureImage/CameraImg" << HORIZONTAL << "_" << std::setw(2) << std::setfill('0') << i+1 << "_" << NEGA << ".bmp"; 
		// 縦縞
		else
			Filename_nega_cam[i] << saveFolder+"/GrayCodeImage/CaptureImage/CameraImg" << VERTICAL << "_" << std::setw(2) << std::setfill('0') << i-c->g.h_bit+1 << "_" << NEGA << ".bmp"; 
		
		// 保存
		cv::imwrite(Filename_nega_cam[i].str(), cap);
		Filename_nega_cam[i] << std::endl;
	}

	/***** 投影 & 撮影終了 *****/

	cv::waitKey(2*delay);

	cv::destroyWindow("check");

	/**** 終了 *****/

	// メモリの開放
	delete[] posi_img;
	delete[] nega_img;
	delete[] Filename_posi;
	delete[] Filename_nega;
	delete[] Filename_posi_cam;
	delete[] Filename_nega_cam;
}


/***************
** 二値化関連 **
****************/

/**
 * @brief   カメラ撮影画像を読み込む関数
 * 
 * @param   mat[in,out]		撮影画像
 * @param   div_bin[in]		番号
 * @param   vh[in]			番号
 * @param   pn[in]			番号
 */
void GRAYCODE::loadCam(cv::Mat &mat, int div_bin, bool vh, bool pn)
{
	char buf[256];
	std::string folder = saveFolder + "/GrayCodeImage/CaptureImage/CameraImg%d_%02d_%d.bmp";
	sprintf_s(buf, folder.c_str(), vh, div_bin, pn);
	mat = cv::imread(buf, 0);
}


/**
 * @brief   マスクを作成するインタフェース
 * 
 * @param   mask[in,out]	マスク画像		
 */
void GRAYCODE::makeMask(cv::Mat &mask)
{
	cv::Mat posi_img;
	cv::Mat nega_img;

	// マスク画像生成
	cv::Mat mask_vert, mask_hor;
	static int useImageNumber = 6;
	// y方向のグレイコード画像読み込み
	loadCam(posi_img, useImageNumber, 0, 1);
	loadCam(nega_img, useImageNumber, 0, 0);

	// 仮のマスク画像Y生成
	makeMaskFromCam(posi_img, nega_img, mask_vert);

	// x方向のグレイコード画像読み込み
	loadCam(posi_img, useImageNumber, 1, 1);
	loadCam(nega_img, useImageNumber, 1, 0);

	// 仮のマスク画像X生成
	makeMaskFromCam(posi_img, nega_img, mask_hor);

	// XとYのORを取る
	// マスク外はどちらも黒なので黒
	// マスク内は（理論的には）必ず一方が白でもう一方が黒なので、白になる
	// 実際はごま塩ノイズが残ってしまう
	cv::bitwise_or(mask_vert, mask_hor, mask);

	// 残ったごま塩ノイズを除去（白ゴマか黒ゴマかで適用順が逆になる）
	dilate(mask, mask, cv::Mat(), cv::Point(-1, -1), 5);
	erode(mask, mask, cv::Mat(), cv::Point(-1, -1), 5);

	cv::imwrite(saveFolder+"/GrayCodeImage/mask.bmp", mask);
}



/**
 * @brief   グレイコードの画像を利用してマスクを生成する関数
 * 
 * @param   posi[in,out]			ポジ画像
 * @param   nega[in,out]			ネガ画像
 * @param   result[in,out]			結果画像
 * @param   thresholdValue[in]		閾値
 *
 * @note	ポジとネガの差分を取ってthresholdValue以上の輝度のピクセルを白にする
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
 * @brief   撮影画像の2値化をするインタフェース	
 */
void GRAYCODE::make_thresh()
{
	cv::Mat posi_img;
	cv::Mat nega_img;
	cv::Mat Geometric_thresh_img;  // 2値化された画像
	cv::Mat mask;

	// マスクを生成
	makeMask(mask);

	int h_bit = (int)ceil( log(prj_height+1) / log(2) );
	int w_bit = (int)ceil( log(prj_width+1) / log(2) );
	int all_bit = h_bit + w_bit;

	std::cout << "二値化開始" << std::endl;
	// 連番でファイル名を読み込む
	for( int i = 0; i < h_bit; ++i ) 
	{
		// 読み込み
		char buf[256];
		// ポジパターン読み込み
		loadCam(posi_img, i+1, 0, 1);
		// ネガパターン読み込み
		loadCam(nega_img, i+1, 0, 0);

		// 2値化
		cv::Mat masked_img;
		thresh( posi_img, nega_img, Geometric_thresh_img, 0 );
		// マスクを適用して2値化
		Geometric_thresh_img.copyTo( masked_img, mask );
		std::string folder = saveFolder + "/GrayCodeImage/ThresholdImage/Geometric_thresh%02d.bmp";
		sprintf_s(buf, folder.c_str(), i);
		cv::imwrite(buf, masked_img);

		std::cout << i << ", ";
	}
	for( int i = 0; i < w_bit; ++i ) 
	{
		// 読み込み
		char buf[256];
		// ポジパターン読み込み
		loadCam(posi_img, i+1, 1, 1);
		// ネガパターン読み込み
		loadCam(nega_img, i+1, 1, 0);

		// 2値化
		cv::Mat masked_img;
		thresh( posi_img, nega_img, Geometric_thresh_img, 0 );
		// マスクを適用して2値化
		Geometric_thresh_img.copyTo( masked_img, mask );
		std::string folder = saveFolder + "/GrayCodeImage/ThresholdImage/Geometric_thresh%02d.bmp";
		sprintf_s(buf, folder.c_str(), i+h_bit);
		cv::imwrite(buf, masked_img);

		std::cout << i+h_bit << ", ";
	}
	std::cout << std::endl;
	std::cout << "二値化終了" << std::endl;
}


/**
 * @brief   実際の2値化処理 
 * 
 * @param   posi[in,out]			ポジ画像
 * @param   nega[in,out]			ネガ画像
 * @param   result[in,out]			結果画像
 * @param   thresholdValue[in]		閾値		
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

			// thresh_valueより大きいかどうかで二値化
			if( posi_pixel - nega_pixel >= thresh_value )
				thresh_img.at<uchar>( y, x ) = 255;
			else
				thresh_img.at<uchar>( y, x ) = 0;
		}
	}
}


/***********************************
** プロジェクタとカメラの対応付け **
************************************/

/**
 * @brief   2値化コード復元	
 */
void GRAYCODE::code_restore()
{
	// 2値化コード復元
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

	// 連想配列でグレイコードの値の場所に座標を格納
	for( int y = 0; y < cam_height; ++y )
	{
		for( int x = 0; x < cam_width; ++x ) 
		{
			int a = c->graycode[y][x];
			if( a != 0 )
				(*c->code_map)[a] = cv::Point(x, y);

			// 初期化
			c->ProCam[y][x] = cv::Point(-1, -1);
		}
	}

	// 0番目は使わない
	(*c->code_map)[0] = cv::Point(-1, -1);

	// プロジェクタとカメラの対応付け
	for( int y = 0; y < prj_height; ++y ) 
	{
		for( int x = 0; x < prj_width; ++x ) 
		{
			// グレイコード取得
			int a = c->g.graycode[y][x];
			// map内に存在しないコード（カメラで撮影が上手くいかなかった部分）の場所にはエラー値-1を格納
			if ( (*c->code_map).find(a) == (*c->code_map).end() ) {
				c->CamPro[y][x] = cv::Point(-1, -1);
			}
			// 存在する場合は、対応するグレイコードの座標を格納
			else {
				c->CamPro[y][x] = (*c->code_map)[a];
				c->ProCam[(*c->code_map)[a].y][(*c->code_map)[a].x] = cv::Point(x, y);
			}
		}
	}
}


/**
 * @brief   プロジェクタ - カメラ構造体初期化	
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
 * @brief   対応付けを行うインターフェース	
 */
void GRAYCODE::makeCorrespondence()
{
	initCorrespondence();
	code_restore();
}


/***********************************
** その他（用途不明な過去の遺物） **
************************************/


/**
 * @brief   カメラ撮影領域からプロジェクタ投影領域を切り出し
 * 
 * @param   src[in]			入力画像
 * @param   dst[in,out]		出力画像
 */
void GRAYCODE::transport_camera_projector(const cv::Mat &src, cv::Mat &dst)
{
	cv::Mat src_resize;  // リサイズした画像
	cv::resize( src, src_resize, cv::Size(cam_width, cam_height) );

	dst = cv::Mat( prj_height, prj_width, CV_8UC3, cv::Scalar(0, 0, 0) );  // 幾何補正された画像（投影画像）

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
 * @brief   入力画像をカメラ撮影領域に変形
 * 
 * @param   src[in]			入力画像
 * @param   dst[in,out]		出力画像
 */
void GRAYCODE::transport_projector_camera(const cv::Mat &src, cv::Mat &dst)
{
	cv::Mat src_resize;  // リサイズした画像
	cv::resize( src, src_resize, cv::Size(prj_width, prj_width) );

	dst = cv::Mat( cam_height, cam_width, CV_8UC3, cv::Scalar(0, 0, 0) );  // 幾何補正された画像（投影画像）

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
** 追加 **
************************************/


/**
 * @brief   カメラ座標に対するプロジェクタの対応点を返す
 * 
 * @param   projPoint[in,out]		プロジェクタ座標値
 * @param   imagePoint[in]			カメラ座標値
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
 * @brief   カメラ座標に対するプロジェクタの対応点を返す(高精度版)
 * 
 * @param   projPoint[in,out]		プロジェクタ座標値
 * @param   imagePoint[in]			カメラ座標値
 * @param	size[in]				考慮するサイズ
 *
 * @note	周囲の座標値を基にHomographyを行いノイズ除去
 */
void GRAYCODE::getCorrespondSubPixelProjPoints(std::vector<cv::Point2f> &projPoint, const std::vector<cv::Point2f> &imagePoint, int size)
{
	for (int i=0; i < imagePoint.size(); ++i)
	{
		std::vector<cv::Point2f> iPoints, pPoints;
		if(imagePoint[i].x > size && imagePoint[i].x+size < cam_width && imagePoint[i].y > size && imagePoint[i].y+size < cam_height)
		{
			// 領域毎の対応点
			for( float h = imagePoint[i].y-size; h < imagePoint[i].y+size; h+=1.0f){
				for( float w = imagePoint[i].x-size; w < imagePoint[i].x+size; w+=1.0f){
					cv::Point2f point = c->ProCam[int(h+0.5f)][int(w+0.5f)];
					if( point.x != -1.0f) {
						iPoints.emplace_back(cv::Point2f(w, h));
						pPoints.emplace_back(point);
					}
				}
			}

			// 対応点同士でHomographyの計算
			cv::Mat H = cv::findHomography(iPoints, pPoints, CV_RANSAC, 2.0);
			// Homographyを使ってチェッカーパターンの交点を射影
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
 * @brief   対応のとれた点を全て返す
 * 
 * @param   projPoint[in,out]		プロジェクタ座標値
 * @param   imagePoint[in,out]		カメラ座標値
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