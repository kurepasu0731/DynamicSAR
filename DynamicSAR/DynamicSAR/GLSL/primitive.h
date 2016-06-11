#ifndef PRIMITIVE_H
#define	PRIMITIVE_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>

/**
 * @brief   プリミティブな図形を生成するクラス
 */
class Primitive
{

public:
	Primitive()
	{
		VBO[0] = 0xffffffff;
		IBO = 0xffffffff;
	};

	~Primitive()
	{
		if (VBO[0] != 0xffffffff){
			glDeleteBuffers(2, VBO);
		}

		if (IBO != 0xffffffff){
			glDeleteBuffers(1, &IBO);
		}
	};

	// 頂点バッファオブジェクトの初期設定
	void init(const std::vector<glm::vec3> &vertex_vect, const std::vector<glm::vec3> &normal_vect, const std::vector<unsigned int> &index_vect);

	// 球の初期化
	void initSphere(int division);

	// ボックスの初期化
	void initCube();

	// ユーザ定義の配列の初期化
	void initVector(const std::vector<glm::vec3> &vertex_vect);

	// レンダリング
    void render();

	// ワイヤーフレームによるレンダリング
	void render_wire();

	// 点によるレンダリング
	void render_point(GLfloat size);

	// 頂点とポリゴンのインデックスから頂点法線を求める
	void calcVertexNormals(const std::vector<glm::vec3> &vertex_vect, const std::vector<unsigned int> &index_vect, std::vector<glm::vec3> &normal_vect);

private:
	GLuint VBO[2];		// Vertex Buffer Object
	GLuint IBO;			// Index Buffer Object
	unsigned int index_size; 
};


#endif