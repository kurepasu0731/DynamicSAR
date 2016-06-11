#include "background.h"



/**
 * @brief   初期処理(ファイルから読み込み)
 * 
 * @param   imageFile[in]		背景画像ファイル名
 * @param   vertFile[in]		バーテックスファイル名
 * @param   fragFile[in]		フラグメントファイル名
 * @param   width[in]			画像の幅
 * @param   height[in]			画像の高さ
 */
void Background::init(const std::string& imageFile,const std::string &vertFile, const std::string &fragFile, int width, int height)
{

	// プログラムオブジェクトを作成
	init_GLSL( &program, vertFile.c_str(), fragFile.c_str());

	// テクスチャの読み込み
	if(!texture.loadImage(GL_TEXTURE_2D, imageFile))
	{
		std::cerr << "背景画像を読み込めませんでした" << std::endl;
		exit(-1);
	}

	glUseProgram(program);


	// uniform変数の場所を取得
	MatrixID = glGetUniformLocation(program, "MVP");

	// シェーダのuniform変数の位置を取得
	GLint locTexture = glGetUniformLocation(program,"inputImage");
	locW = glGetUniformLocation(program,"imageWidth");
	locH = glGetUniformLocation(program,"imageHeight");

	// uniform変数に値を渡す
	glUniform1i(locTexture, 0);
	glUniform1i(locW, width);
	glUniform1i(locH, height);


	///// 頂点配列オブジェクトの作成 /////
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vboID);   
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
        
    //データをVBOにコピー
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), position, GL_DYNAMIC_DRAW);

	// 初期の矩形
	resizeRectangle(width, height);

	glUseProgram(0);
}


/**
 * @brief   初期処理(Matから読み込み)
 * 
 * @param   imageFile[in]		背景画像
 * @param   vertFile[in]		バーテックスファイル名
 * @param   fragFile[in]		フラグメントファイル名
 * @param   width[in]			画像の幅
 * @param   height[in]			画像の高さ
 */
void Background::init(const cv::Mat& image,const std::string &vertFile, const std::string &fragFile, int width, int height)
{

	// プログラムオブジェクトを作成
	init_GLSL( &program, vertFile.c_str(), fragFile.c_str());

	// テクスチャの読み込み
	texture.loadFromMat(GL_TEXTURE_2D, image);

	glUseProgram(program);


	// uniform変数の場所を取得
	MatrixID = glGetUniformLocation(program, "MVP");

	// シェーダのuniform変数の位置を取得
	GLint locTexture = glGetUniformLocation(program,"inputImage");
	locW = glGetUniformLocation(program,"imageWidth");
	locH = glGetUniformLocation(program,"imageHeight");

	// uniform変数に値を渡す
	glUniform1i(locTexture, 0);
	glUniform1i(locW, width);
	glUniform1i(locH, height);


	///// 頂点配列オブジェクトの作成 /////
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vboID);   
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
        
    //データをVBOにコピー
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), position, GL_DYNAMIC_DRAW);

	// 初期の矩形
	resizeRectangle(width, height);

	glUseProgram(0);
}



/**
 * @brief   矩形データのリサイズ
 * 
 * @param   width[in]			画像の幅
 * @param   height[in]			画像の高さ
 */
void Background::resizeRectangle(int width, int height)
{
	//アップデート
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    GLfloat *ptr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    if(ptr)
    {       
		ptr[0] = 0.f;
		ptr[1] = 0.f;
		ptr[2] = 0.f;
		ptr[3] = 0.f;
		ptr[4] = (float)height;
		ptr[5] = 0.f;
		ptr[6] = (float)width;
		ptr[7] = (float)height;
		ptr[8] = 0.f;
		ptr[9] = (float)width;
		ptr[10] = 0.f;
		ptr[11] = 0.f;

		glUnmapBufferARB(GL_ARRAY_BUFFER);
    }

	// 画像サイズをシェーダに送る
	glUseProgram(program);
	glUniform1i(locW, width);
	glUniform1i(locH, height); 

	glUseProgram(0);
}


/**
 * @brief   シェーダへ行列の受け渡し
 * 
 * @param   matrix[in]			配置する行列
 */
void Background::changeMatrix(const glm::mat4 &matrix)
{
	// シェーダプログラムの開始
	glUseProgram(program);
	// Uniform変数に行列を送る
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &matrix[0][0]);
}


/**
 * @brief   テクスチャのアップデート
 * 
 * @param   image[in]			テクスチャの更新
 */
void Background::textureUpdate(const cv::Mat &image)
{
	texture.updateFromMat(GL_TEXTURE_2D, image);
}


/**
 * @brief   描画
 */
void Background::draw()
{
	// シェーダプログラムの開始
	glUseProgram(program);

	// テクスチャをバインド
	texture.bind(GL_TEXTURE0);

	//VBOで描画
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    
	// 描画
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// デフォルトに戻す
    glDisableVertexAttribArray(0);
}