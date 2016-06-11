#include "tessellatedSphere.h"



/**
 * @brief   ���̒��_�𐶐�
 * 
 * @param   division[in]		������
 */
void TessellatedSphere::createSphereVector(int division)
{
	// �����ʑ̂̃x�N�g��
	std::vector<glm::vec3> init_vectors(6);
	init_vectors[0] = glm::vec3(0.0, 0.0, 1.0);
	init_vectors[1] = glm::vec3(0.0, 1.0, 0.0);
	init_vectors[2] = glm::vec3(-1.0, 0.0, 0.0);
	init_vectors[3] = glm::vec3(0.0, 0.0, -1.0);
	init_vectors[4] = glm::vec3(0.0, -1.0, 0.0);
	init_vectors[5] = glm::vec3(1.0, 0.0, 0.0);

	// ������
	sphere_vector.clear();
	tessell_vector.clear();
	tessell_vector.resize(8, std::vector<glm::vec3>()); 

	///// �ی����ɎO�p�`�����ʕ�Ԃŕ��� /////

	// ��1�ی�
	tessell_vector[0].emplace_back(init_vectors[0]);
	tessell_vector[0].emplace_back(init_vectors[1]);
	sphere_vector.emplace_back(init_vectors[0]);
	sphere_vector.emplace_back(init_vectors[1]);
	
	for (int i = 0; i < division; ++i)
	{
		glm::vec3 p1 = glm::slerp<float>(init_vectors[0], init_vectors[1], (float)i/(float)division);
		glm::vec3 p2 = glm::slerp<float>(init_vectors[2], init_vectors[1], (float)i/(float)division);

		for (int j = 0; j < (division-i); ++j)
		{
			if ( i != 0 || j!= 0 )
			{
				glm::vec3 p3 = glm::slerp<float>(p1, p2, (float)j/(float)(division-i));
				tessell_vector[0].emplace_back(p3);
				sphere_vector.emplace_back(p3);
			}
		}
	}

	// ��2�ی�
	tessell_vector[1].emplace_back(init_vectors[2]);
	sphere_vector.emplace_back(init_vectors[2]);

	for (int i = 0; i < division; ++i)
	{
		glm::vec3 p1 = glm::slerp<float>(init_vectors[2], init_vectors[1], (float)i/(float)division);
		glm::vec3 p2 = glm::slerp<float>(init_vectors[3], init_vectors[1], (float)i/(float)division);

		for (int j = 0; j < (division-i); ++j)
		{
			if ( i != 0 || j!= 0 )
			{
				glm::vec3 p3 = glm::slerp<float>(p1, p2, (float)j/(float)(division-i));
				tessell_vector[1].emplace_back(p3);
				sphere_vector.emplace_back(p3);
			}
		}
	}

	// ��3�ی�
	tessell_vector[2].emplace_back(init_vectors[3]);
	sphere_vector.emplace_back(init_vectors[3]);

	for (int i = 0; i < division; ++i)
	{
		glm::vec3 p1 = glm::slerp<float>(init_vectors[3], init_vectors[1], (float)i/(float)division);
		glm::vec3 p2 = glm::slerp<float>(init_vectors[5], init_vectors[1], (float)i/(float)division);

		for (int j = 0; j < (division-i); ++j)
		{
			if ( i != 0 || j!= 0 )
			{
				glm::vec3 p3 = glm::slerp<float>(p1, p2, (float)j/(float)(division-i));
				tessell_vector[2].emplace_back(p3);
				sphere_vector.emplace_back(p3);
			}
		}
	}

	// ��4�ی�
	tessell_vector[3].emplace_back(init_vectors[5]);
	sphere_vector.emplace_back(init_vectors[5]);
	
	for (int i = 0; i < division; ++i)
	{
		glm::vec3 p1 = glm::slerp<float>(init_vectors[5], init_vectors[1], (float)i/(float)division);
		glm::vec3 p2 = glm::slerp<float>(init_vectors[0], init_vectors[1], (float)i/(float)division);

		for (int j = 0; j < (division-i); ++j)
		{
			if ( i != 0 || j!= 0 )
			{
				glm::vec3 p3 = glm::slerp<float>(p1, p2, (float)j/(float)(division-i));
				tessell_vector[3].emplace_back(p3);
				sphere_vector.emplace_back(p3);
			}
		}
	}

	// ��5�ی�
	tessell_vector[4].emplace_back(init_vectors[4]);
	sphere_vector.emplace_back(init_vectors[4]);
	
	for (int i = 0; i < division; ++i)
	{
		glm::vec3 p1 = glm::slerp<float>(init_vectors[0], init_vectors[4], (float)i/(float)division);
		glm::vec3 p2 = glm::slerp<float>(init_vectors[2], init_vectors[4], (float)i/(float)division);

		for (int j = 0; j < (division-i); ++j)
		{
			if ( i != 0 )
			{
				glm::vec3 p3 = glm::slerp<float>(p1, p2, (float)j/(float)(division-i));
				tessell_vector[4].emplace_back(p3);
				sphere_vector.emplace_back(p3);
			}
		}
	}

	// ��6�ی�
	for (int i = 0; i < division; ++i)
	{
		glm::vec3 p1 = glm::slerp<float>(init_vectors[2], init_vectors[4], (float)i/(float)division);
		glm::vec3 p2 = glm::slerp<float>(init_vectors[3], init_vectors[4], (float)i/(float)division);

		for (int j = 0; j < (division-i); ++j)
		{
			if ( i != 0 )
			{
				glm::vec3 p3 = glm::slerp<float>(p1, p2, (float)j/(float)(division-i));
				tessell_vector[5].emplace_back(p3);
				sphere_vector.emplace_back(p3);
			}
		}
	}

	// ��7�ی�	
	for (int i = 0; i < division; ++i)
	{
		glm::vec3 p1 = glm::slerp<float>(init_vectors[3], init_vectors[4], (float)i/(float)division);
		glm::vec3 p2 = glm::slerp<float>(init_vectors[5], init_vectors[4], (float)i/(float)division);

		for (int j = 0; j < (division-i); ++j)
		{
			if ( i != 0 )
			{
				glm::vec3 p3 = glm::slerp<float>(p1, p2, (float)j/(float)(division-i));
				tessell_vector[6].emplace_back(p3);
				sphere_vector.emplace_back(p3);
			}
		}
	}

	// ��8�ی�	
	for (int i = 0; i < division; ++i)
	{
		glm::vec3 p1 = glm::slerp<float>(init_vectors[5], init_vectors[4], (float)i/(float)division);
		glm::vec3 p2 = glm::slerp<float>(init_vectors[0], init_vectors[4], (float)i/(float)division);

		for (int j = 0; j < (division-i); ++j)
		{
			if ( i != 0 )
			{
				glm::vec3 p3 = glm::slerp<float>(p1, p2, (float)j/(float)(division-i));
				tessell_vector[7].emplace_back(p3);
				sphere_vector.emplace_back(p3);
			}
		}
	}
}