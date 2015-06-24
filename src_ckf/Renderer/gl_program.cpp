#include "gl_local.h"

int programs[MAX_PROGRAMS];
int numprograms;

void R_InitPrograms(void)
{
	numprograms = 0;
}

qboolean GL_IsProgramError(void)
{
	GLint iErrorPos, iIsNative;

	qglGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &iErrorPos);
	char *sb = (char *)qglGetString(GL_PROGRAM_ERROR_STRING_ARB);
	qglGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &iIsNative);

	if (iErrorPos != -1 || !iIsNative)
		return true;

	return false;
}

GLuint GL_CompileProgram(GLuint target, GLenum format, const GLvoid *string)
{
	GLuint program;
	qglGenProgramsARB(1, &program);
	qglEnable(target);
	qglBindProgramARB(target, program);
	qglProgramStringARB(target, format, strlen((const char *)string), string);
	qglDisable(target);

	if (GL_IsProgramError())
		return 0;

	programs[numprograms] = program;
	numprograms++;

	return program;
}

void R_FreePrograms(void)
{
	for (int i = 0; i < numprograms; i++)
	{
		if(programs[i])
		{
			qglDeleteProgramsARB(1, (GLuint *)&programs[i]);
		}
	}
}

GLuint R_CompileShader(const char *vscode, const char *fscode)
{
	GLuint program = qglCreateProgramObjectARB();
	GLuint vs = qglCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	GLuint fs = qglCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	qglShaderSourceARB(vs, 1, &vscode, NULL);
	qglShaderSourceARB(fs, 1, &fscode, NULL);

	qglCompileShaderARB(vs);
	if(GL_IsShaderError(vs))
		return 0;

	qglCompileShaderARB(fs);
	if(GL_IsShaderError(fs))
		return 0;

	qglAttachObjectARB(program, vs);
	qglAttachObjectARB(program, fs);
	qglLinkProgramARB(program);

	return program;
}

qboolean GL_IsShaderError(GLuint shader)
{
	static char compiler_log[4096];
	int blen;

	qglGetShaderiv(shader, GL_INFO_LOG_LENGTH , &blen);
	if(blen > 1)
	{
		int slen;
		qglGetInfoLogARB(shader, blen, &slen, compiler_log);
		gCKFExports.Con_SafePrintf("Shader compiled with error: %s\n", compiler_log);
		return true;
	}

	return false;
}