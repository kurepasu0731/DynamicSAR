#version 330 core

// 入力データ
flat in vec3 model_index;			// 頂点のインデックス


// 出力データ
out vec3 color;

void main()
{	
	color = model_index;
}