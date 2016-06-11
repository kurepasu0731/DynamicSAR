#version 330 core

uniform sampler2D inputImage0;   // 入力するテクスチャ(元モデルのデプス)
uniform sampler2D inputImage1;   // 入力するテクスチャ(エッジ画像)
uniform sampler2D inputImage2;   // 入力するテクスチャ(トラッキングモデルのインデックス)
uniform sampler2D inputImage3;   // 入力するテクスチャ(トラッキングモデルのデプス)
uniform int       imageWidth;   // テクスチャの幅
uniform int       imageHeight;  // テクスチャの高さ
in vec2      pos;				// バーテックスシェーダから得られる画素位置

out vec3 outColor;

///////////////////////////////////////////////////////////////////////////////
void main(void)
{
	// テクスチャ座標では0～1に正規化
	vec2 textureCoord = vec2(pos.x/float(imageWidth), pos.y/float(imageHeight));
	// 反転
	vec2 textureCoord_inv = vec2(pos.x/float(imageWidth), 1.0-pos.y/float(imageHeight));

	// オクルージョン判定
	vec3 occlusion = vec3(1,1,1);
	if ( texture2D(inputImage3,textureCoord).r < texture2D(inputImage0,textureCoord).r && texture2D(inputImage1,textureCoord_inv).r > 0.05)
	{
		occlusion = texture2D(inputImage2,textureCoord).rgb;
	}

	// 出力
	outColor = occlusion;
}