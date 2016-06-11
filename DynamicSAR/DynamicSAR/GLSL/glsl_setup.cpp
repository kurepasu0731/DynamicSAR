#include "glsl_setup.h"


/**
 * @brief   �V�F�[�_�t�@�C����ǂݍ��݃R���p�C��
 * 
 * @param   shader[in]		�V�F�[�_�I�u�W�F�N�g
 * @param   file[in]		�V�F�[�_�t�@�C����
 */
void readShaderCompile(GLuint shader, const char *file)
{
	FILE *fp;
	char *buf;
	GLsizei size, len;
	GLint compiled;
  
	// �t�@�C�����J��
	if( fopen_s( &fp, file, "rb") != 0)
	{
		printf("�t�@�C�����J�����Ƃ��ł��܂��� %s\n", file);
	}
  
	// �t�@�C���̖����Ɉړ������݈ʒu�𓾂� 
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);		// �t�@�C���T�C�Y���擾
  
	// �t�@�C���T�C�Y�̃��������m�� 
	buf = (GLchar *)malloc(size);
	if (buf == NULL) {
		printf("���������m�ۂł��܂���ł��� \n");
	}
  
	// �t�@�C����擪����ǂݍ��� 
	fseek(fp, 0, SEEK_SET);
	fread(buf, 1, size, fp);

	// �V�F�[�_�I�u�W�F�N�g�Ƀv���O�������Z�b�g 
	glShaderSource(shader, 1, (const GLchar **)&buf, &size);

	// �V�F�[�_�ǂݍ��ݗ̈�̉�� 
	free(buf);
	fclose(fp);

	// �V�F�[�_�̃R���p�C�� 
	glCompileShader(shader);
	glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );

	if ( compiled == GL_FALSE ) 
	{
		printf( "�R���p�C���ł��܂���ł���!!: %s \n ", file);
		glGetProgramiv( shader, GL_INFO_LOG_LENGTH, &size );
		if ( size > 0 ) 
		{
			buf = (char *)malloc(size);
			glGetShaderInfoLog( shader, size, &len, buf);
			printf(buf);
			free(buf);
		}
	}
}



/**
 * @brief   �V�F�[�_�v���O�����̃����N
 * 
 * @param   prog[in]		�V�F�[�_�v���O����
 */
void shaderLink( GLuint prog )
{
	GLsizei	size, len;
	GLint linked;
	char *infoLog ;

	glLinkProgram( prog );

	glGetProgramiv( prog, GL_LINK_STATUS, &linked );

	if ( linked == GL_FALSE ) 
	{
		printf("�����N�ł��܂���ł���!! \n");
	  	
		glGetProgramiv( prog, GL_INFO_LOG_LENGTH, &size );
		if ( size > 0 ) 
		{
			infoLog = (char *)malloc(size);
			glGetProgramInfoLog( prog, size, &len, infoLog );
			printf(infoLog);
			free(infoLog);
		}
	}
}



/**
 * @brief   GLSL�̃Z�b�g�A�b�v(���_�V�F�[�_)
 * 
 * @param   program[in,out]			�V�F�[�_�v���O����
 * @param   vertexFile[in]			���_�V�F�[�_�̃t�@�C����
 */
void init_GLSL(GLuint *program, const char *vertexFile)
{
	// glew�̏�����
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		printf("Error: %s\n", glewGetErrorString(err));
	}
	// GPU,OpenGL���
	printf("VENDOR= %s \n", glGetString(GL_VENDOR));
	printf("GPU= %s \n", glGetString(GL_RENDERER));
	printf("OpenGL= %s \n", glGetString(GL_VERSION));
	printf("GLSL= %s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// �V�F�[�_�I�u�W�F�N�g�̍쐬
	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	// �V�F�[�_�[�̓ǂݍ��݂ƃR���p�C��
	readShaderCompile(vertexShader, vertexFile);

	// �V�F�[�_�v���O�����̍쐬 
	*program = glCreateProgram();

	// �V�F�[�_�I�u�W�F�N�g���V�F�[�_�v���O�����Ɋ֘A�t����
	glAttachShader(*program, vertexShader);

	// �V�F�[�_�I�u�W�F�N�g�̍폜 
	glDeleteShader(vertexShader);

	// �V�F�[�_�v���O�����̃����N 
	shaderLink(*program);
}


/**
 * @brief   GLSL�̃Z�b�g�A�b�v(���_�V�F�[�_, �t���O�����g�V�F�[�_)
 * 
 * @param   program[in,out]			�V�F�[�_�v���O����
 * @param   vertexFile[in]			���_�V�F�[�_�̃t�@�C����
 * @param   fragmentFile[in]		�t���O�����g�V�F�[�_�̃t�@�C����	
 */
void init_GLSL(GLuint *program, const char *vertexFile, const char *fragmentFile)
{
	// glew�̏�����
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		printf("Error: %s\n", glewGetErrorString(err));
	}
	// GPU,OpenGL���
	printf("VENDOR= %s \n", glGetString(GL_VENDOR));
	printf("GPU= %s \n", glGetString(GL_RENDERER));
	printf("OpenGL= %s \n", glGetString(GL_VERSION));
	printf("GLSL= %s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// �V�F�[�_�I�u�W�F�N�g�̍쐬
	GLuint vertexShader, fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// �V�F�[�_�[�̓ǂݍ��݂ƃR���p�C��
	readShaderCompile(vertexShader, vertexFile);
	readShaderCompile(fragmentShader, fragmentFile);

	// �V�F�[�_�v���O�����̍쐬 
	*program = glCreateProgram();

	// �V�F�[�_�I�u�W�F�N�g���V�F�[�_�v���O�����Ɋ֘A�t����
	glAttachShader(*program, vertexShader);
	glAttachShader(*program, fragmentShader);

	// �V�F�[�_�I�u�W�F�N�g�̍폜 
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// �V�F�[�_�v���O�����̃����N 
	shaderLink(*program);
}
