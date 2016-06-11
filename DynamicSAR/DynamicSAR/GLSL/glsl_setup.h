#ifndef GLSL_SETUP
#define GLSL_SETUP

#include <stdio.h>

#include <gl/glew.h>
#include <gl/freeglut.h>
#pragma comment (lib,"glew32.lib")


/***** GLSL�̃Z�b�g�A�b�v(���_�V�F�[�_�̂�) *****/
void init_GLSL(GLuint *program, const char *vertexFile);

/***** GLSL�̃Z�b�g�A�b�v(���_�V�F�[�_, �t���O�����g�V�F�[�_) *****/
void init_GLSL(GLuint *program, const char *vertexFile, const char *fragmentFile);

#endif