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

	// テクスチャの色をそのまま出力
	outColor = vec4(texture2D(inputImage,textureCoord).rgb, 1.0);
}