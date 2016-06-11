#version 330 core

uniform sampler2D inputImage;   // ���͂���e�N�X�`��
uniform int       imageWidth;   // �e�N�X�`���̕�
uniform int       imageHeight;  // �e�N�X�`���̍���
in vec2      pos;				// �o�[�e�b�N�X�V�F�[�_���瓾�����f�ʒu

out vec4 outColor;

///////////////////////////////////////////////////////////////////////////////
void main(void)
{
	// �e�N�X�`�����W�ł�0�`1�ɐ��K��
	vec2 textureCoord = vec2(pos.x/float(imageWidth), pos.y/float(imageHeight));

	// �e�N�X�`���̐F�����̂܂܏o��
	outColor = vec4(texture2D(inputImage,textureCoord).rgb, 1.0);
}