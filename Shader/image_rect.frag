#version 330 core

uniform sampler2D inputImage0;   // ���͂���e�N�X�`��(�����f���̃f�v�X)
uniform sampler2D inputImage1;   // ���͂���e�N�X�`��(�G�b�W�摜)
uniform sampler2D inputImage2;   // ���͂���e�N�X�`��(�g���b�L���O���f���̃C���f�b�N�X)
uniform sampler2D inputImage3;   // ���͂���e�N�X�`��(�g���b�L���O���f���̃f�v�X)
uniform int       imageWidth;   // �e�N�X�`���̕�
uniform int       imageHeight;  // �e�N�X�`���̍���
in vec2      pos;				// �o�[�e�b�N�X�V�F�[�_���瓾�����f�ʒu

out vec3 outColor;

///////////////////////////////////////////////////////////////////////////////
void main(void)
{
	// �e�N�X�`�����W�ł�0�`1�ɐ��K��
	vec2 textureCoord = vec2(pos.x/float(imageWidth), pos.y/float(imageHeight));
	// ���]
	vec2 textureCoord_inv = vec2(pos.x/float(imageWidth), 1.0-pos.y/float(imageHeight));

	// �I�N���[�W��������
	vec3 occlusion = vec3(1,1,1);
	if ( texture2D(inputImage3,textureCoord).r < texture2D(inputImage0,textureCoord).r && texture2D(inputImage1,textureCoord_inv).r > 0.05)
	{
		occlusion = texture2D(inputImage2,textureCoord).rgb;
	}

	// �o��
	outColor = occlusion;
}