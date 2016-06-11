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

	// �d�ݕt���̍���
	vec4 dx = (textureOffset(inputImage, textureCoord, ivec2( 1,  0))
			- textureOffset(inputImage, textureCoord, ivec2(-1,  0))) * 0.196640663
        
			+ (textureOffset(inputImage, textureCoord, ivec2( 1,  1))
			+ textureOffset(inputImage, textureCoord, ivec2( 1, -1))
			- textureOffset(inputImage, textureCoord, ivec2(-1,  1))
			- textureOffset(inputImage, textureCoord, ivec2(-1, -1))) * 0.119268591

			+ (textureOffset(inputImage, textureCoord, ivec2( 2,  0))
			- textureOffset(inputImage, textureCoord, ivec2(-2,  0))) * 0.043876463

			+ (textureOffset(inputImage, textureCoord, ivec2( 1,  2))
			- textureOffset(inputImage, textureCoord, ivec2(-1,  2))
			+ textureOffset(inputImage, textureCoord, ivec2( 1, -2))
			- textureOffset(inputImage, textureCoord, ivec2(-1, -2))
			+ textureOffset(inputImage, textureCoord, ivec2( 2,  1))
			- textureOffset(inputImage, textureCoord, ivec2(-2,  1))
			+ textureOffset(inputImage, textureCoord, ivec2( 2, -1))
			- textureOffset(inputImage, textureCoord, ivec2(-2, -1))) * 0.02661242

			+ (textureOffset(inputImage, textureCoord, ivec2( 2,  2))
			- textureOffset(inputImage, textureCoord, ivec2(-2,  2))
			+ textureOffset(inputImage, textureCoord, ivec2( 2, -2))
			- textureOffset(inputImage, textureCoord, ivec2(-2, -2))) * 0.005938033;

	vec4 dy = (textureOffset(inputImage, textureCoord, ivec2( 0,  1))
			- textureOffset(inputImage, textureCoord, ivec2( 0, -1))) * 0.196640663
        
			+ (textureOffset(inputImage, textureCoord, ivec2( 1,  1))
			- textureOffset(inputImage, textureCoord, ivec2( 1, -1))
			+ textureOffset(inputImage, textureCoord, ivec2(-1,  1))
			- textureOffset(inputImage, textureCoord, ivec2(-1, -1))) * 0.119268591

			+ (textureOffset(inputImage, textureCoord, ivec2( 0,  2))
			- textureOffset(inputImage, textureCoord, ivec2( 0, -2))) * 0.043876463

			+ (textureOffset(inputImage, textureCoord, ivec2( 1,  2))
			- textureOffset(inputImage, textureCoord, ivec2( 1, -2))
			+ textureOffset(inputImage, textureCoord, ivec2(-1,  2))
			- textureOffset(inputImage, textureCoord, ivec2(-1, -2))
			+ textureOffset(inputImage, textureCoord, ivec2( 2,  1))
			- textureOffset(inputImage, textureCoord, ivec2( 2, -1))
			+ textureOffset(inputImage, textureCoord, ivec2(-2,  1))
			- textureOffset(inputImage, textureCoord, ivec2(-2, -1))) * 0.02661242

			+ (textureOffset(inputImage, textureCoord, ivec2( 2,  2))
			- textureOffset(inputImage, textureCoord, ivec2( 2, -2))
			+ textureOffset(inputImage, textureCoord, ivec2(-2,  2))
			- textureOffset(inputImage, textureCoord, ivec2(-2, -2))) * 0.005938033;


	// �o��
	if (sqrt(dx * dx + dy * dy).r > 0.1)
	{
		outColor = sqrt(dx * dx + dy * dy);
	}
}