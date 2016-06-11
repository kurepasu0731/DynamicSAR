#include "primitive.h"




/**
 * @brief   頂点バッファオブジェクトの初期設定
 * 
 * @param   vertex_vect[in]		頂点情報
 * @param   normal_vect[in]		法線情報
 * @param   index_vect[in]		インデックス情報
 */
void Primitive::init(const std::vector<glm::vec3> &vertex_vect, const std::vector<glm::vec3> &normal_vect, const std::vector<unsigned int> &index_vect)
{
	// VBOの生成
    glGenBuffers(2, VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertex_vect.size(), &vertex_vect[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*normal_vect.size(), &normal_vect[0], GL_STATIC_DRAW);

	// IBOの生成
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*index_vect.size(), &index_vect[0], GL_STATIC_DRAW);

	//　バインドしたものをもどす
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



/**
 * @brief   球の初期化
 * 
 * @param   division[in]		分割数
 */
void Primitive::initSphere(int division)
{
	if (division <= 0)
		division = 1;

	std::vector<glm::vec3> vertex_vect;			// 頂点
	std::vector<glm::vec3> normal_vect;			// 法線
	std::vector<unsigned int> index_vect;		// インデックス


	// 初期のベクトル
	std::vector<glm::vec3> init_vectors(24);
	init_vectors[0] = glm::vec3(1.0, 0.0, 0.0);		// (1,1,1)
	init_vectors[1] = glm::vec3(0.0, 1.0, 0.0);
	init_vectors[2] = glm::vec3(0.0, 0.0, 1.0);
	init_vectors[3] = glm::vec3(0.0, 1.0, 0.0);		// (-1,1,1)
	init_vectors[4] = glm::vec3(-1.0, 0.0, 0.0);
	init_vectors[5] = glm::vec3(0.0, 0.0, 1.0);
	init_vectors[6] = glm::vec3(-1.0, 0.0, 0.0);	// (-1,-1,1)
	init_vectors[7] = glm::vec3(0.0, -1.0, 0.0);
	init_vectors[8] = glm::vec3(0.0, 0.0, 1.0);
	init_vectors[9] = glm::vec3(0.0, -1.0, 0.0);	// (1,-1,1)
	init_vectors[10] = glm::vec3(1.0, 0.0, 0.0);
	init_vectors[11] = glm::vec3(0.0, 0.0, 1.0);
	init_vectors[12] = glm::vec3(0.0, 1.0, 0.0);	// (1,1,-1)
	init_vectors[13] = glm::vec3(1.0, 0.0, 0.0);
	init_vectors[14] = glm::vec3(0.0, 0.0, -1.0);
	init_vectors[15] = glm::vec3(-1.0, 0.0, 0.0);	// (-1,1,-1)
	init_vectors[16] = glm::vec3(0.0, 1.0, 0.0);
	init_vectors[17] = glm::vec3(0.0, 0.0, -1.0);
	init_vectors[18] = glm::vec3(0.0, -1.0, 0.0);	// (-1,-1,-1)
	init_vectors[19] = glm::vec3(-1.0, 0.0, 0.0);
	init_vectors[20] = glm::vec3(0.0, 0.0, -1.0);
	init_vectors[21] = glm::vec3(1.0, 0.0, 0.0);	// (1,-1,-1)
	init_vectors[22] = glm::vec3(0.0, -1.0, 0.0);
	init_vectors[23] = glm::vec3(0.0, 0.0, -1.0);



	int j = 0;

	// 正8面体の各三角形を分割
	for (int i = 0; i < 24; i+=3)
	{
		// 分割1
		for (int p = 0; p < division; ++p)
		{			
			glm::vec3 edge_p1 = glm::slerp<float>(init_vectors[i], init_vectors[i+1], (float)p/(float)division);			// p left
			glm::vec3 edge_p2 = glm::slerp<float>(init_vectors[i+2], init_vectors[i+1], (float)p/(float)division);			// p right
			glm::vec3 edge_p3 = glm::slerp<float>(init_vectors[i], init_vectors[i+1], (float)(p+1)/(float)division);		// p+1 left
			glm::vec3 edge_p4 = glm::slerp<float>(init_vectors[i+2], init_vectors[i+1], (float)(p+1)/(float)division);		// p+1 right

			// 分割2
			for (int q = 0; q <(division-p); ++q)
			{
				glm::vec3 a, b, c, d;
				a = glm::slerp<float>(edge_p1, edge_p2, (float)q/(float)(division-p));
				b = glm::slerp<float>(edge_p1, edge_p2, (float)(q+1)/(float)(division-p));
				if (edge_p3 == edge_p4)
				{
					c = edge_p3;
					d = edge_p4;
				} else {
					c = glm::slerp<float>(edge_p3, edge_p4, (float)q/(float)(division-p-1));
					d = glm::slerp<float>(edge_p3, edge_p4, (float)(q+1)/(float)(division-p-1));
				}

				vertex_vect.push_back(a);
				vertex_vect.push_back(b);
				vertex_vect.push_back(c);
				index_vect.push_back(j++);
				index_vect.push_back(j++);
				index_vect.push_back(j++);

				if (q < division-p-1) {
					vertex_vect.push_back(c);
					vertex_vect.push_back(b);
					vertex_vect.push_back(d);
					index_vect.push_back(j++);
					index_vect.push_back(j++);
					index_vect.push_back(j++);
				}
			}
		}
	}

	// インデックスのサイズ
	index_size = (unsigned int)index_vect.size();

	// 頂点法線を求める
	calcVertexNormals(vertex_vect, index_vect, normal_vect);

	// vbo iboの初期設定
	init(vertex_vect, normal_vect, index_vect);
}


 /**
 * @brief   ボックスの初期化
 */
void Primitive::initCube()
{
	std::vector<glm::vec3> vertex_vect(8);	// 頂点

	vertex_vect[0] = glm::vec3(-1.0f, -1.0f, -1.0f);
	vertex_vect[1] = glm::vec3(-1.0f, -1.0f,  1.0f);
	vertex_vect[2] = glm::vec3(-1.0f,  1.0f,  1.0f);
	vertex_vect[3] = glm::vec3(-1.0f,  1.0f, -1.0f);
	vertex_vect[4] = glm::vec3( 1.0f,  1.0f, -1.0f);
	vertex_vect[5] = glm::vec3( 1.0f, -1.0f, -1.0f);
	vertex_vect[6] = glm::vec3( 1.0f, -1.0f,  1.0f);
	vertex_vect[7] = glm::vec3( 1.0f,  1.0f,  1.0f);


	static const GLuint index[] = {
		0, 1, 2,
		2, 3, 0,
		5, 0, 3,
		3, 4, 5,
		5, 6, 1,
		1, 0, 5,
		6, 5, 4,
		4, 7, 6,
		7, 2, 1,
		1, 6, 7,
		7, 4, 3,
		3, 2, 7
	};

	std::vector<glm::vec3> normal_vect;		// 法線
	std::vector<unsigned int> index_vect(index, std::end(index));	// インデックス

	// インデックスのサイズ
	index_size = (unsigned int)index_vect.size();

	// 頂点法線を求める
	calcVertexNormals(vertex_vect, index_vect, normal_vect);

	// vbo iboの初期設定
	init(vertex_vect, normal_vect, index_vect);
}


/**
 * @brief   ユーザ定義の配列の初期化
 * 
 * @param   vertex_vect[in]		頂点情報
 */
void Primitive::initVector(const std::vector<glm::vec3> &vertex_vect)
{

	std::vector<glm::vec3> normal_vect;		// 法線
	std::vector<unsigned int> index_vect(vertex_vect.size());	// インデックス

	// インデックスは入力配列の順番をそのまま用いる
	for (int i = 0; i < index_vect.size(); ++i)
	{
		index_vect[i] = i;
	}

	// インデックスのサイズ
	index_size = (unsigned int)index_vect.size();

	// 頂点法線を求める
	calcVertexNormals(vertex_vect, index_vect, normal_vect);

	// vbo iboの初期設定
	init(vertex_vect, normal_vect, index_vect);
}


/**
 * @brief	レンダリング
 */
void Primitive::render()
{
	// シェーダの変数に頂点情報を対応付ける
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// VBOの指定
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	// 頂点情報の格納場所と書式の指定
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);			// 頂点

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	// 頂点情報の格納場所と書式の指定
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);			// 法線

	// IBOの指定
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	glDrawElements(GL_TRIANGLES, index_size, GL_UNSIGNED_INT, (void*)0);


	// 頂点の対応付けの解除
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

	//　バインドしたものをもどす
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/**
 * @brief	ワイヤーフレームによるレンダリング
 */
void Primitive::render_wire()
{
	// ワイヤーフレームでレンダリング
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );

	// レンダリング
	render();

	// 元に戻す
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
}


/**
 * @brief	点によるレンダリング
 */
void Primitive::render_point(GLfloat size)
{
	// 点の大きさ
	glPointSize(size);

	// 点でレンダリング
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT );

	// レンダリング
	render();

	// 元に戻す
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
}


/**
 * @brief   頂点とポリゴンのインデックスから頂点法線を求める
 * 
 * @param   vertex_vect[in]			頂点情報
 * @param   index_vect[in]			インデックス情報
 * @param   normal_vect[in,out]		法線情報
 */
void Primitive::calcVertexNormals(const std::vector<glm::vec3> &vertex_vect, const std::vector<unsigned int> &index_vect, std::vector<glm::vec3> &normal_vect)
{
	// 初期化
	normal_vect.clear();
	normal_vect.resize(vertex_vect.size(), glm::vec3(0.0f,0.0f,0.0f));

	// 頂点の法線を求める
	for(unsigned int n = 0; n < normal_vect.size(); ++n)
	{
		glm::vec3 normal = glm::vec3(0.0f, 0.0f, 0.0f);

		// インデックスの中から自身の番号を探す
		for(unsigned int i = 0; i < index_size; i = i+3)
		{
			int index1 = index_vect.at(i);
			int index2 = index_vect.at(i+1);
			int index3 = index_vect.at(i+2);

			// 三角形の頂点に含まれていたら
			if(n==index1 || n==index2 || n==index3)
			{
				// 法線を求める
				glm::vec3 normal0 = glm::normalize(glm::cross(
					vertex_vect[index2] - vertex_vect[index1],
					vertex_vect[index3] - vertex_vect[index1]));

				// 法線の総和
				normal += normal0;
			}
		}

		// 正規化
		normal = glm::normalize(normal);

		// 追加
		normal_vect[n] = normal;
	}
}