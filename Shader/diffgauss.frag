#version 330 core

uniform sampler2D inputImage;   // 入力するテクスチャ
uniform int       imageWidth;   // テクスチャの幅
uniform int       imageHeight;  // テクスチャの高さ
in vec2      pos;				// バーテックスシェーダから得られる画素位置

out vec4 outColor;

///////////////////////////////////////////////////////////////////////////////
void main(void)
{
	// テクスチャ座標では0～1に正規化
	vec2 textureCoord = vec2(pos.x/float(imageWidth), pos.y/float(imageHeight));

	// 重み付きの差分
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


	// 出力
	if (sqrt(dx * dx + dy * dy).r > 0.1)
	{
		outColor = sqrt(dx * dx + dy * dy);
	}
}