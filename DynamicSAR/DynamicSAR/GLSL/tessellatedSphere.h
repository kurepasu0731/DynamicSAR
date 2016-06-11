#ifndef TESSELLATED_SPHERE_H
#define TESSELLATED_SPHERE_H

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>


/**
 * @brief   三角形で分割した球を作成するクラス
 */
class TessellatedSphere
{
public:
	TessellatedSphere(){};
	~TessellatedSphere(){};

	// 球の頂点を生成
	void createSphereVector(int division);



	/***** メンバ変数 *****/

	std::vector<std::vector<glm::vec3>> tessell_vector;		// 球上の頂点座標(象限毎のベクトル番号)
	std::vector<glm::vec3> sphere_vector;					// 球上の頂点座標
};


#endif