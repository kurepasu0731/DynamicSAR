#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "glsl_setup.h"
#include "texture.h"

#include <stdio.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>



/**
 * @brief   背景の描画クラス
 */
class Background
{

public:
	Background(){};

	// 初期処理(ファイルから読み込み)
	void init(const std::string& imageFile,const std::string &vertFile, const std::string &fragFile, int width, int height);
	// 初期処理(Matから読み込み)
	void init(const cv::Mat& image,const std::string &vertFile, const std::string &fragFile, int width, int height);

	// サイズの変更
	void resizeRectangle(int width, int height);

	// 行列の受け渡し
	void changeMatrix(const glm::mat4 &matrix);

	// テクスチャの更新
	void textureUpdate(const cv::Mat &image);

	// 描画
	void draw();


	/***** メンバ変数 *****/

	GLuint program;				// 背景のプログラムオブジェクト
	GLfloat position[12];		// 画像の頂点	
	GLuint vboID;				// vboのID

	GLuint locW;				// シェーダ用の画像の幅
	GLuint locH;				// シェーダ用の画像の高さ

	Texture texture;			// テクスチャクラス

	GLuint MatrixID;			// 配置する行列
};


#endif