#version 330 core

// ���̓f�[�^
flat in vec3 model_index;			// ���_�̃C���f�b�N�X


// �o�̓f�[�^
out vec3 color;

void main()
{	
	color = model_index;
}