#pragma once

typedef struct
{
	int vs;
	int fs;
	int program;
}glshader_t;

#define MAX_SHADERS 16

extern glshader_t shaders[MAX_SHADERS];
extern int numshaders;

GLuint R_CompileShader(const char *vscode, const char *fscode, const char *vsfile, const char *fsfile);
void GL_UseProgram(GLuint program);
void GL_EndProgram(void);
GLuint GL_GetUniformLoc(GLuint program, const char *name);
GLuint GL_GetAttribLoc(GLuint program, const char *name);
void GL_Uniform1i(GLuint loc, int v0);
void GL_Uniform2i(GLuint loc, int v0, int v1);
void GL_Uniform3i(GLuint loc, int v0, int v1, int v2);
void GL_Uniform4i(GLuint loc, int v0, int v1, int v2, int v3);
void GL_Uniform1f(GLuint loc, float v0);
void GL_Uniform2f(GLuint loc, float v0, float v1);
void GL_Uniform3f(GLuint loc, float v0, float v1, float v2);
void GL_Uniform4f(GLuint loc, float v0, int v1, int v2, int v3);
void GL_VertexAttrib3f(GLuint index, float x, float y, float z);
void GL_VertexAttrib3fv(GLuint index, float *v);
void GL_MultiTexCoord2f(GLenum target, float s, float t);
void GL_MultiTexCoord3f(GLenum target, float s, float t, float r);

#define clamp(value, mi, ma) min(max(value, mi), ma)

#define SHADER_DEFINE(name) name##_program_t name;

#define SHADER_UNIFORM(name, loc, locstring) name##.loc = qglGetUniformLocationARB(name.program, locstring);
#define SHADER_ATTRIB(name, loc, locstring) name##.loc = qglGetAttribLocationARB(name.program, locstring);