#ifndef IMAGE_RECT_H
#define IMAGE_RECT_H


#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>


/**
 * @brief   矩形を生成するクラス
 * 
 * @note	ポストエフェクト処理等で使用
 */
class ImageRect
{
public:
	ImageRect(){}

	virtual ~ImageRect(){}

	// 初期処理
	void init();

	// 矩形のリサイズ
	void resizeRectangle(int width, int height);

	// 描画
	void draw();


	GLfloat position[12];		// 画像の頂点
	GLuint vboID;				// vboのID
};


#endif