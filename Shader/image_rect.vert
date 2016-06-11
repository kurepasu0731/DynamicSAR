#version 330 core

layout(location = 0) in vec3 vertex;
out vec2 pos;			// フラグメントシェーダに渡す画素の位置

uniform mat4 MVP;

void main(void)
{

	// 2次元の画像の位置
	pos = vec2(vertex.xy);

	// 画像の位置をそのまま出力
	gl_Position =  MVP * vec4(vertex,1);
}
