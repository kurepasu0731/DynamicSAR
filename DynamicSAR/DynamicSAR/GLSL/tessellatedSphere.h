#ifndef TESSELLATED_SPHERE_H
#define TESSELLATED_SPHERE_H

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>


/**
 * @brief   �O�p�`�ŕ������������쐬����N���X
 */
class TessellatedSphere
{
public:
	TessellatedSphere(){};
	~TessellatedSphere(){};

	// ���̒��_�𐶐�
	void createSphereVector(int division);



	/***** �����o�ϐ� *****/

	std::vector<std::vector<glm::vec3>> tessell_vector;		// ����̒��_���W(�ی����̃x�N�g���ԍ�)
	std::vector<glm::vec3> sphere_vector;					// ����̒��_���W
};


#endif