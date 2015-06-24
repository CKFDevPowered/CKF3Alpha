#ifndef R_OPENGL_H
#define R_OPENGL_H

#include <windows.h>
#include <gl/gl.h>
#include <gl/glext.h>

enum
{
	R_OPENGL_110 = 0,
	R_WGL_PROCADDRESS,
	R_ARB_VERTEX_BUFFER_OBJECT_EXT,
	R_ENV_COMBINE_EXT,
	R_ARB_MULTITEXTURE,
	R_TEXTURECUBEMAP_EXT,
	R_DOT3_ARB_EXT,
	R_ANISOTROPY_EXT,
	R_TEXTURE_LODBIAS,
	R_OCCLUSION_QUERIES_EXT,
	R_TEXTURE_COMPRESSION_EXT,
	R_SHADER_GLSL100_EXT,
	R_SGIS_MIPMAPS_EXT,
	R_DRAW_RANGEELEMENTS_EXT,
	R_LOCKARRAYS_EXT,
	R_TEXTURE_3D_EXT,
	R_CLAMPTOEDGE_EXT,
	R_BLEND_MINMAX_EXT,
	R_STENCILTWOSIDE_EXT,
	R_BLEND_SUBTRACT_EXT,
	R_SHADER_OBJECTS_EXT,
	R_VERTEX_PROGRAM_EXT,
	R_FRAGMENT_PROGRAM_EXT,
	R_VERTEX_SHADER_EXT,
	R_FRAGMENT_SHADER_EXT,
	R_EXT_POINTPARAMETERS,
	R_SEPARATESTENCIL_EXT,
	R_ARB_TEXTURE_NPOT_EXT,
	R_CUSTOM_VERTEX_ARRAY_EXT,
	R_TEXTURE_ENV_ADD_EXT,
	R_CLAMP_TEXBORDER_EXT,
	R_DEPTH_TEXTURE,
	R_SHADOW_EXT,
	R_FRAMEBUFFER_OBJECT,
	R_PARANOIA_EXT,
	R_EXTCOUNT,
};

#ifndef GL_TEXTURE_RECTANGLE_EXT
#define GL_TEXTURE_RECTANGLE_EXT 0x84F5
#endif

#ifndef GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT 0x84F8
#endif

extern GLenum (APIENTRY *pglGetError)(void);
extern const GLcharARB * (APIENTRY *pglGetString)(GLenum name);

extern void (APIENTRY *pglAccum)(GLenum op, GLfloat value);
extern void (APIENTRY *pglAlphaFunc)(GLenum func, GLclampf ref);
extern void (APIENTRY *pglBegin)(GLenum mode);
extern void (APIENTRY *pglBindTexture)(GLenum target, GLuint texture);
extern void (APIENTRY *pglBitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
extern void (APIENTRY *pglBlendFunc)(GLenum sfactor, GLenum dfactor);
extern void (APIENTRY *pglCallList)(GLuint list);
extern void (APIENTRY *pglCallLists)(GLsizei n, GLenum type, const GLvoid *lists);
extern void (APIENTRY *pglClear)(GLbitfield mask);
extern void (APIENTRY *pglClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void (APIENTRY *pglClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern void (APIENTRY *pglClearDepth)(GLclampd depth);
extern void (APIENTRY *pglClearIndex)(GLfloat c);
extern void (APIENTRY *pglClearStencil)(GLint s);
extern GLboolean (APIENTRY *pglIsEnabled)(GLenum cap);
extern GLboolean (APIENTRY *pglIsList)(GLuint list);
extern GLboolean (APIENTRY *pglIsTexture)(GLuint texture);
extern void (APIENTRY *pglClipPlane)(GLenum plane, const GLdouble *equation);
extern void (APIENTRY *pglColor3b)(GLbyte red, GLbyte green, GLbyte blue);
extern void (APIENTRY *pglColor3bv)(const GLbyte *v);
extern void (APIENTRY *pglColor3d)(GLdouble red, GLdouble green, GLdouble blue);
extern void (APIENTRY *pglColor3dv)(const GLdouble *v);
extern void (APIENTRY *pglColor3f)(GLfloat red, GLfloat green, GLfloat blue);
extern void (APIENTRY *pglColor3fv)(const GLfloat *v);
extern void (APIENTRY *pglColor3i)(GLint red, GLint green, GLint blue);
extern void (APIENTRY *pglColor3iv)(const GLint *v);
extern void (APIENTRY *pglColor3s)(GLshort red, GLshort green, GLshort blue);
extern void (APIENTRY *pglColor3sv)(const GLshort *v);
extern void (APIENTRY *pglColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
extern void (APIENTRY *pglColor3ubv)(const GLubyte *v);
extern void (APIENTRY *pglColor3ui)(GLuint red, GLuint green, GLuint blue);
extern void (APIENTRY *pglColor3uiv)(const GLuint *v);
extern void (APIENTRY *pglColor3us)(GLushort red, GLushort green, GLushort blue);
extern void (APIENTRY *pglColor3usv)(const GLushort *v);
extern void (APIENTRY *pglColor4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
extern void (APIENTRY *pglColor4bv)(const GLbyte *v);
extern void (APIENTRY *pglColor4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
extern void (APIENTRY *pglColor4dv)(const GLdouble *v);
extern void (APIENTRY *pglColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void (APIENTRY *pglColor4fv)(const GLfloat *v);
extern void (APIENTRY *pglColor4i)(GLint red, GLint green, GLint blue, GLint alpha);
extern void (APIENTRY *pglColor4iv)(const GLint *v);
extern void (APIENTRY *pglColor4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
extern void (APIENTRY *pglColor4sv)(const GLshort *v);
extern void (APIENTRY *pglColor4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
extern void (APIENTRY *pglColor4ubv)(const GLubyte *v);
extern void (APIENTRY *pglColor4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
extern void (APIENTRY *pglColor4uiv)(const GLuint *v);
extern void (APIENTRY *pglColor4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
extern void (APIENTRY *pglColor4usv)(const GLushort *v);
extern void (APIENTRY *pglColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern void (APIENTRY *pglColorMaterial)(GLenum face, GLenum mode);
extern void (APIENTRY *pglCopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
extern void (APIENTRY *pglCopyTexImage1D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
extern void (APIENTRY *pglCopyTexImage2D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void (APIENTRY *pglCopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
extern void (APIENTRY *pglCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void (APIENTRY *pglCullFace)(GLenum mode);
extern void (APIENTRY *pglDeleteLists)(GLuint list, GLsizei range);
extern void (APIENTRY *pglDeleteTextures)(GLsizei n, const GLuint *textures);
extern void (APIENTRY *pglDepthFunc)(GLenum func);
extern void (APIENTRY *pglDepthMask)(GLboolean flag);
extern void (APIENTRY *pglDepthRange)(GLclampd zNear, GLclampd zFar);
extern void (APIENTRY *pglDisable)(GLenum cap);
extern void (APIENTRY *pglDisableClientState)(GLenum array);
extern void (APIENTRY *pglDrawArrays)(GLenum mode, GLint first, GLsizei count);
extern void (APIENTRY *pglDrawBuffer)(GLenum mode);
extern void (APIENTRY *pglDrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern void (APIENTRY *pglEdgeFlag)(GLboolean flag);
extern void (APIENTRY *pglEdgeFlagPointer)(GLsizei stride, const GLvoid *pointer);
extern void (APIENTRY *pglEdgeFlagv)(const GLboolean *flag);
extern void (APIENTRY *pglEnable)(GLenum cap);
extern void (APIENTRY *pglEnableClientState)(GLenum array);
extern void (APIENTRY *pglEnd)(void);
extern void (APIENTRY *pglEndList)(void);
extern void (APIENTRY *pglEvalCoord1d)(GLdouble u);
extern void (APIENTRY *pglEvalCoord1dv)(const GLdouble *u);
extern void (APIENTRY *pglEvalCoord1f)(GLfloat u);
extern void (APIENTRY *pglEvalCoord1fv)(const GLfloat *u);
extern void (APIENTRY *pglEvalCoord2d)(GLdouble u, GLdouble v);
extern void (APIENTRY *pglEvalCoord2dv)(const GLdouble *u);
extern void (APIENTRY *pglEvalCoord2f)(GLfloat u, GLfloat v);
extern void (APIENTRY *pglEvalCoord2fv)(const GLfloat *u);
extern void (APIENTRY *pglEvalMesh1)(GLenum mode, GLint i1, GLint i2);
extern void (APIENTRY *pglEvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
extern void (APIENTRY *pglEvalPoint1)(GLint i);
extern void (APIENTRY *pglEvalPoint2)(GLint i, GLint j);
extern void (APIENTRY *pglFeedbackBuffer)(GLsizei size, GLenum type, GLfloat *buffer);
extern void (APIENTRY *pglFinish)(void);
extern void (APIENTRY *pglFlush)(void);
extern void (APIENTRY *pglFogf)(GLenum pname, GLfloat param);
extern void (APIENTRY *pglFogfv)(GLenum pname, const GLfloat *params);
extern void (APIENTRY *pglFogi)(GLenum pname, GLint param);
extern void (APIENTRY *pglFogiv)(GLenum pname, const GLint *params);
extern void (APIENTRY *pglFrontFace)(GLenum mode);
extern void (APIENTRY *pglFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void (APIENTRY *pglGenTextures)(GLsizei n, GLuint *textures);
extern void (APIENTRY *pglGetBooleanv)(GLenum pname, GLboolean *params);
extern void (APIENTRY *pglGetClipPlane)(GLenum plane, GLdouble *equation);
extern void (APIENTRY *pglGetDoublev)(GLenum pname, GLdouble *params);
extern void (APIENTRY *pglGetFloatv)(GLenum pname, GLfloat *params);
extern void (APIENTRY *pglGetIntegerv)(GLenum pname, GLint *params);
extern void (APIENTRY *pglGetLightfv)(GLenum light, GLenum pname, GLfloat *params);
extern void (APIENTRY *pglGetLightiv)(GLenum light, GLenum pname, GLint *params);
extern void (APIENTRY *pglGetMapdv)(GLenum target, GLenum query, GLdouble *v);
extern void (APIENTRY *pglGetMapfv)(GLenum target, GLenum query, GLfloat *v);
extern void (APIENTRY *pglGetMapiv)(GLenum target, GLenum query, GLint *v);
extern void (APIENTRY *pglGetMaterialfv)(GLenum face, GLenum pname, GLfloat *params);
extern void (APIENTRY *pglGetMaterialiv)(GLenum face, GLenum pname, GLint *params);
extern void (APIENTRY *pglGetPixelMapfv)(GLenum map, GLfloat *values);
extern void (APIENTRY *pglGetPixelMapuiv)(GLenum map, GLuint *values);
extern void (APIENTRY *pglGetPixelMapusv)(GLenum map, GLushort *values);
extern void (APIENTRY *pglGetPointerv)(GLenum pname, GLvoid* *params);
extern void (APIENTRY *pglGetPolygonStipple)(GLubyte *mask);
extern void (APIENTRY *pglGetTexEnvfv)(GLenum target, GLenum pname, GLfloat *params);
extern void (APIENTRY *pglGetTexEnviv)(GLenum target, GLenum pname, GLint *params);
extern void (APIENTRY *pglGetTexGendv)(GLenum coord, GLenum pname, GLdouble *params);
extern void (APIENTRY *pglGetTexGenfv)(GLenum coord, GLenum pname, GLfloat *params);
extern void (APIENTRY *pglGetTexGeniv)(GLenum coord, GLenum pname, GLint *params);
extern void (APIENTRY *pglGetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
extern void (APIENTRY *pglGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params);
extern void (APIENTRY *pglGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params);
extern void (APIENTRY *pglGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
extern void (APIENTRY *pglGetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
extern void (APIENTRY *pglHint)(GLenum target, GLenum mode);
extern void (APIENTRY *pglIndexMask)(GLuint mask);
extern void (APIENTRY *pglIndexPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
extern void (APIENTRY *pglIndexd)(GLdouble c);
extern void (APIENTRY *pglIndexdv)(const GLdouble *c);
extern void (APIENTRY *pglIndexf)(GLfloat c);
extern void (APIENTRY *pglIndexfv)(const GLfloat *c);
extern void (APIENTRY *pglIndexi)(GLint c);
extern void (APIENTRY *pglIndexiv)(const GLint *c);
extern void (APIENTRY *pglIndexs)(GLshort c);
extern void (APIENTRY *pglIndexsv)(const GLshort *c);
extern void (APIENTRY *pglIndexub)(GLubyte c);
extern void (APIENTRY *pglIndexubv)(const GLubyte *c);
extern void (APIENTRY *pglInitNames)(void);
extern void (APIENTRY *pglInterleavedArrays)(GLenum format, GLsizei stride, const GLvoid *pointer);
extern void (APIENTRY *pglLightModelf)(GLenum pname, GLfloat param);
extern void (APIENTRY *pglLightModelfv)(GLenum pname, const GLfloat *params);
extern void (APIENTRY *pglLightModeli)(GLenum pname, GLint param);
extern void (APIENTRY *pglLightModeliv)(GLenum pname, const GLint *params);
extern void (APIENTRY *pglLightf)(GLenum light, GLenum pname, GLfloat param);
extern void (APIENTRY *pglLightfv)(GLenum light, GLenum pname, const GLfloat *params);
extern void (APIENTRY *pglLighti)(GLenum light, GLenum pname, GLint param);
extern void (APIENTRY *pglLightiv)(GLenum light, GLenum pname, const GLint *params);
extern void (APIENTRY *pglLineStipple)(GLint factor, GLushort pattern);
extern void (APIENTRY *pglLineWidth)(GLfloat width);
extern void (APIENTRY *pglListBase)(GLuint base);
extern void (APIENTRY *pglLoadIdentity)(void);
extern void (APIENTRY *pglLoadMatrixd)(const GLdouble *m);
extern void (APIENTRY *pglLoadMatrixf)(const GLfloat *m);
extern void (APIENTRY *pglLoadName)(GLuint name);
extern void (APIENTRY *pglLogicOp)(GLenum opcode);
extern void (APIENTRY *pglMap1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
extern void (APIENTRY *pglMap1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
extern void (APIENTRY *pglMap2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
extern void (APIENTRY *pglMap2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
extern void (APIENTRY *pglMapGrid1d)(GLint un, GLdouble u1, GLdouble u2);
extern void (APIENTRY *pglMapGrid1f)(GLint un, GLfloat u1, GLfloat u2);
extern void (APIENTRY *pglMapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
extern void (APIENTRY *pglMapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
extern void (APIENTRY *pglMaterialf)(GLenum face, GLenum pname, GLfloat param);
extern void (APIENTRY *pglMaterialfv)(GLenum face, GLenum pname, const GLfloat *params);
extern void (APIENTRY *pglMateriali)(GLenum face, GLenum pname, GLint param);
extern void (APIENTRY *pglMaterialiv)(GLenum face, GLenum pname, const GLint *params);
extern void (APIENTRY *pglMatrixMode)(GLenum mode);
extern void (APIENTRY *pglMultMatrixd)(const GLdouble *m);
extern void (APIENTRY *pglMultMatrixf)(const GLfloat *m);
extern void (APIENTRY *pglNewList)(GLuint list, GLenum mode);
extern void (APIENTRY *pglNormal3b)(GLbyte nx, GLbyte ny, GLbyte nz);
extern void (APIENTRY *pglNormal3bv)(const GLbyte *v);
extern void (APIENTRY *pglNormal3d)(GLdouble nx, GLdouble ny, GLdouble nz);
extern void (APIENTRY *pglNormal3dv)(const GLdouble *v);
extern void (APIENTRY *pglNormal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
extern void (APIENTRY *pglNormal3fv)(const GLfloat *v);
extern void (APIENTRY *pglNormal3i)(GLint nx, GLint ny, GLint nz);
extern void (APIENTRY *pglNormal3iv)(const GLint *v);
extern void (APIENTRY *pglNormal3s)(GLshort nx, GLshort ny, GLshort nz);
extern void (APIENTRY *pglNormal3sv)(const GLshort *v);
extern void (APIENTRY *pglOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
extern void (APIENTRY *pglPassThrough)(GLfloat token);
extern void (APIENTRY *pglPixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat *values);
extern void (APIENTRY *pglPixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint *values);
extern void (APIENTRY *pglPixelMapusv)(GLenum map, GLsizei mapsize, const GLushort *values);
extern void (APIENTRY *pglPixelStoref)(GLenum pname, GLfloat param);
extern void (APIENTRY *pglPixelStorei)(GLenum pname, GLint param);
extern void (APIENTRY *pglPixelTransferf)(GLenum pname, GLfloat param);
extern void (APIENTRY *pglPixelTransferi)(GLenum pname, GLint param);
extern void (APIENTRY *pglPixelZoom)(GLfloat xfactor, GLfloat yfactor);
extern void (APIENTRY *pglPointSize)(GLfloat size);
extern void (APIENTRY *pglPolygonMode)(GLenum face, GLenum mode);
extern void (APIENTRY *pglPolygonOffset)(GLfloat factor, GLfloat units);
extern void (APIENTRY *pglPolygonStipple)(const GLubyte *mask);
extern void (APIENTRY *pglPopAttrib)(void);
extern void (APIENTRY *pglPopClientAttrib)(void);
extern void (APIENTRY *pglPopMatrix)(void);
extern void (APIENTRY *pglPopName)(void);
extern void (APIENTRY *pglPushAttrib)(GLbitfield mask);
extern void (APIENTRY *pglPushClientAttrib)(GLbitfield mask);
extern void (APIENTRY *pglPushMatrix)(void);
extern void (APIENTRY *pglPushName)(GLuint name);
extern void (APIENTRY *pglRasterPos2d)(GLdouble x, GLdouble y);
extern void (APIENTRY *pglRasterPos2dv)(const GLdouble *v);
extern void (APIENTRY *pglRasterPos2f)(GLfloat x, GLfloat y);
extern void (APIENTRY *pglRasterPos2fv)(const GLfloat *v);
extern void (APIENTRY *pglRasterPos2i)(GLint x, GLint y);
extern void (APIENTRY *pglRasterPos2iv)(const GLint *v);
extern void (APIENTRY *pglRasterPos2s)(GLshort x, GLshort y);
extern void (APIENTRY *pglRasterPos2sv)(const GLshort *v);
extern void (APIENTRY *pglRasterPos3d)(GLdouble x, GLdouble y, GLdouble z);
extern void (APIENTRY *pglRasterPos3dv)(const GLdouble *v);
extern void (APIENTRY *pglRasterPos3f)(GLfloat x, GLfloat y, GLfloat z);
extern void (APIENTRY *pglRasterPos3fv)(const GLfloat *v);
extern void (APIENTRY *pglRasterPos3i)(GLint x, GLint y, GLint z);
extern void (APIENTRY *pglRasterPos3iv)(const GLint *v);
extern void (APIENTRY *pglRasterPos3s)(GLshort x, GLshort y, GLshort z);
extern void (APIENTRY *pglRasterPos3sv)(const GLshort *v);
extern void (APIENTRY *pglRasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void (APIENTRY *pglRasterPos4dv)(const GLdouble *v);
extern void (APIENTRY *pglRasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void (APIENTRY *pglRasterPos4fv)(const GLfloat *v);
extern void (APIENTRY *pglRasterPos4i)(GLint x, GLint y, GLint z, GLint w);
extern void (APIENTRY *pglRasterPos4iv)(const GLint *v);
extern void (APIENTRY *pglRasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w);
extern void (APIENTRY *pglRasterPos4sv)(const GLshort *v);
extern void (APIENTRY *pglReadBuffer)(GLenum mode);
extern void (APIENTRY *pglReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
extern void (APIENTRY *pglRectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
extern void (APIENTRY *pglRectdv)(const GLdouble *v1, const GLdouble *v2);
extern void (APIENTRY *pglRectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
extern void (APIENTRY *pglRectfv)(const GLfloat *v1, const GLfloat *v2);
extern void (APIENTRY *pglRecti)(GLint x1, GLint y1, GLint x2, GLint y2);
extern void (APIENTRY *pglRectiv)(const GLint *v1, const GLint *v2);
extern void (APIENTRY *pglRects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
extern void (APIENTRY *pglRectsv)(const GLshort *v1, const GLshort *v2);
extern void (APIENTRY *pglRotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
extern void (APIENTRY *pglRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
extern void (APIENTRY *pglScaled)(GLdouble x, GLdouble y, GLdouble z);
extern void (APIENTRY *pglScalef)(GLfloat x, GLfloat y, GLfloat z);
extern void (APIENTRY *pglScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
extern void (APIENTRY *pglSelectBuffer)(GLsizei size, GLuint *buffer);
extern void (APIENTRY *pglShadeModel)(GLenum mode);
extern void (APIENTRY *pglStencilFunc)(GLenum func, GLint ref, GLuint mask);
extern void (APIENTRY *pglStencilMask)(GLuint mask);
extern void (APIENTRY *pglStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
extern void (APIENTRY *pglTexCoord1d)(GLdouble s);
extern void (APIENTRY *pglTexCoord1dv)(const GLdouble *v);
extern void (APIENTRY *pglTexCoord1f)(GLfloat s);
extern void (APIENTRY *pglTexCoord1fv)(const GLfloat *v);
extern void (APIENTRY *pglTexCoord1i)(GLint s);
extern void (APIENTRY *pglTexCoord1iv)(const GLint *v);
extern void (APIENTRY *pglTexCoord1s)(GLshort s);
extern void (APIENTRY *pglTexCoord1sv)(const GLshort *v);
extern void (APIENTRY *pglTexCoord2d)(GLdouble s, GLdouble t);
extern void (APIENTRY *pglTexCoord2dv)(const GLdouble *v);
extern void (APIENTRY *pglTexCoord2f)(GLfloat s, GLfloat t);
extern void (APIENTRY *pglTexCoord2fv)(const GLfloat *v);
extern void (APIENTRY *pglTexCoord2i)(GLint s, GLint t);
extern void (APIENTRY *pglTexCoord2iv)(const GLint *v);
extern void (APIENTRY *pglTexCoord2s)(GLshort s, GLshort t);
extern void (APIENTRY *pglTexCoord2sv)(const GLshort *v);
extern void (APIENTRY *pglTexCoord3d)(GLdouble s, GLdouble t, GLdouble r);
extern void (APIENTRY *pglTexCoord3dv)(const GLdouble *v);
extern void (APIENTRY *pglTexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
extern void (APIENTRY *pglTexCoord3fv)(const GLfloat *v);
extern void (APIENTRY *pglTexCoord3i)(GLint s, GLint t, GLint r);
extern void (APIENTRY *pglTexCoord3iv)(const GLint *v);
extern void (APIENTRY *pglTexCoord3s)(GLshort s, GLshort t, GLshort r);
extern void (APIENTRY *pglTexCoord3sv)(const GLshort *v);
extern void (APIENTRY *pglTexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void (APIENTRY *pglTexCoord4dv)(const GLdouble *v);
extern void (APIENTRY *pglTexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void (APIENTRY *pglTexCoord4fv)(const GLfloat *v);
extern void (APIENTRY *pglTexCoord4i)(GLint s, GLint t, GLint r, GLint q);
extern void (APIENTRY *pglTexCoord4iv)(const GLint *v);
extern void (APIENTRY *pglTexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q);
extern void (APIENTRY *pglTexCoord4sv)(const GLshort *v);
extern void (APIENTRY *pglTexEnvf)(GLenum target, GLenum pname, GLfloat param);
extern void (APIENTRY *pglTexEnvfv)(GLenum target, GLenum pname, const GLfloat *params);
extern void (APIENTRY *pglTexEnvi)(GLenum target, GLenum pname, GLint param);
extern void (APIENTRY *pglTexEnviv)(GLenum target, GLenum pname, const GLint *params);
extern void (APIENTRY *pglTexGend)(GLenum coord, GLenum pname, GLdouble param);
extern void (APIENTRY *pglTexGendv)(GLenum coord, GLenum pname, const GLdouble *params);
extern void (APIENTRY *pglTexGenf)(GLenum coord, GLenum pname, GLfloat param);
extern void (APIENTRY *pglTexGenfv)(GLenum coord, GLenum pname, const GLfloat *params);
extern void (APIENTRY *pglTexGeni)(GLenum coord, GLenum pname, GLint param);
extern void (APIENTRY *pglTexGeniv)(GLenum coord, GLenum pname, const GLint *params);
extern void (APIENTRY *pglTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern void (APIENTRY *pglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
extern void (APIENTRY *pglTexParameterf)(GLenum target, GLenum pname, GLfloat param);
extern void (APIENTRY *pglTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
extern void (APIENTRY *pglTexParameteri)(GLenum target, GLenum pname, GLint param);
extern void (APIENTRY *pglTexParameteriv)(GLenum target, GLenum pname, const GLint *params);
extern void (APIENTRY *pglTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
extern void (APIENTRY *pglTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
extern void (APIENTRY *pglTranslated)(GLdouble x, GLdouble y, GLdouble z);
extern void (APIENTRY *pglTranslatef)(GLfloat x, GLfloat y, GLfloat z);
extern void (APIENTRY *pglVertex2d)(GLdouble x, GLdouble y);
extern void (APIENTRY *pglVertex2dv)(const GLdouble *v);
extern void (APIENTRY *pglVertex2f)(GLfloat x, GLfloat y);
extern void (APIENTRY *pglVertex2fv)(const GLfloat *v);
extern void (APIENTRY *pglVertex2i)(GLint x, GLint y);
extern void (APIENTRY *pglVertex2iv)(const GLint *v);
extern void (APIENTRY *pglVertex2s)(GLshort x, GLshort y);
extern void (APIENTRY *pglVertex2sv)(const GLshort *v);
extern void (APIENTRY *pglVertex3d)(GLdouble x, GLdouble y, GLdouble z);
extern void (APIENTRY *pglVertex3dv)(const GLdouble *v);
extern void (APIENTRY *pglVertex3f)(GLfloat x, GLfloat y, GLfloat z);
extern void (APIENTRY *pglVertex3fv)(const GLfloat *v);
extern void (APIENTRY *pglVertex3i)(GLint x, GLint y, GLint z);
extern void (APIENTRY *pglVertex3iv)(const GLint *v);
extern void (APIENTRY *pglVertex3s)(GLshort x, GLshort y, GLshort z);
extern void (APIENTRY *pglVertex3sv)(const GLshort *v);
extern void (APIENTRY *pglVertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
extern void (APIENTRY *pglVertex4dv)(const GLdouble *v);
extern void (APIENTRY *pglVertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void (APIENTRY *pglVertex4fv)(const GLfloat *v);
extern void (APIENTRY *pglVertex4i)(GLint x, GLint y, GLint z, GLint w);
extern void (APIENTRY *pglVertex4iv)(const GLint *v);
extern void (APIENTRY *pglVertex4s)(GLshort x, GLshort y, GLshort z, GLshort w);
extern void (APIENTRY *pglVertex4sv)(const GLshort *v);
extern void (APIENTRY *pglViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
extern void (APIENTRY *pglPointParameterfEXT)(GLenum param, GLfloat value);
extern void (APIENTRY *pglPointParameterfvEXT)(GLenum param, const GLfloat *value);
extern void (APIENTRY *pglLockArraysEXT)(int , int);
extern void (APIENTRY *pglUnlockArraysEXT)(void);
extern void (APIENTRY *pglActiveTextureARB)(GLenum);
extern void (APIENTRY *pglClientActiveTextureARB)(GLenum);
extern void (APIENTRY *pglGetCompressedTexImage)(GLenum target, GLint lod, const void* data);
extern void (APIENTRY *pglDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
extern void (APIENTRY *pglDrawRangeElementsEXT)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
extern void (APIENTRY *pglDrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices);
extern void (APIENTRY *pglVertexPointer)(GLint size, GLenum type, GLsizei stride, const void *ptr);
extern void (APIENTRY *pglNormalPointer)(GLenum type, GLsizei stride, const void *ptr);
extern void (APIENTRY *pglColorPointer)(GLint size, GLenum type, GLsizei stride, const void *ptr);
extern void (APIENTRY *pglTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const void *ptr);
extern void (APIENTRY *pglArrayElement)(GLint i);
extern void (APIENTRY *pglMultiTexCoord1f)(GLenum, GLfloat);
extern void (APIENTRY *pglMultiTexCoord2f)(GLenum, GLfloat, GLfloat);
extern void (APIENTRY *pglMultiTexCoord3f)(GLenum, GLfloat, GLfloat, GLfloat);
extern void (APIENTRY *pglMultiTexCoord4f)(GLenum, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (APIENTRY *pglActiveTexture)(GLenum);
extern void (APIENTRY *pglClientActiveTexture)(GLenum);
extern void (APIENTRY *pglCompressedTexImage3DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
extern void (APIENTRY *pglCompressedTexImage2DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
extern void (APIENTRY *pglCompressedTexImage1DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
extern void (APIENTRY *pglCompressedTexSubImage3DARB)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
extern void (APIENTRY *pglCompressedTexSubImage2DARB)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
extern void (APIENTRY *pglCompressedTexSubImage1DARB)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
extern void (APIENTRY *pglDeleteObjectARB)(GLhandleARB obj);
extern GLhandleARB (APIENTRY *pglGetHandleARB)(GLenum pname);
extern void (APIENTRY *pglDetachObjectARB)(GLhandleARB containerObj, GLhandleARB attachedObj);
extern GLhandleARB (APIENTRY *pglCreateShaderObjectARB)(GLenum shaderType);
extern void (APIENTRY *pglShaderSourceARB)(GLhandleARB shaderObj, GLsizei count, const GLcharARB **string, const GLint *length);
extern void (APIENTRY *pglCompileShaderARB)(GLhandleARB shaderObj);
extern GLhandleARB (APIENTRY *pglCreateProgramObjectARB)(void);
extern void (APIENTRY *pglAttachObjectARB)(GLhandleARB containerObj, GLhandleARB obj);
extern void (APIENTRY *pglLinkProgramARB)(GLhandleARB programObj);
extern void (APIENTRY *pglUseProgramObjectARB)(GLhandleARB programObj);
extern void (APIENTRY *pglValidateProgramARB)(GLhandleARB programObj);
extern void (APIENTRY *pglBindProgramARB)(GLenum target, GLuint program);
extern void (APIENTRY *pglDeleteProgramsARB)(GLsizei n, const GLuint *programs);
extern void (APIENTRY *pglGenProgramsARB)(GLsizei n, GLuint *programs);
extern void (APIENTRY *pglProgramStringARB)(GLenum target, GLenum format, GLsizei len, const void *string);
extern void (APIENTRY *pglProgramEnvParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void (APIENTRY *pglProgramLocalParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void (APIENTRY *pglGetProgramivARB)(GLenum target, GLenum pname, GLint *params);
extern void (APIENTRY *pglUniform1fARB)(GLint location, GLfloat v0);
extern void (APIENTRY *pglUniform2fARB)(GLint location, GLfloat v0, GLfloat v1);
extern void (APIENTRY *pglUniform3fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void (APIENTRY *pglUniform4fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void (APIENTRY *pglUniform1iARB)(GLint location, GLint v0);
extern void (APIENTRY *pglUniform2iARB)(GLint location, GLint v0, GLint v1);
extern void (APIENTRY *pglUniform3iARB)(GLint location, GLint v0, GLint v1, GLint v2);
extern void (APIENTRY *pglUniform4iARB)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void (APIENTRY *pglUniform1fvARB)(GLint location, GLsizei count, const GLfloat *value);
extern void (APIENTRY *pglUniform2fvARB)(GLint location, GLsizei count, const GLfloat *value);
extern void (APIENTRY *pglUniform3fvARB)(GLint location, GLsizei count, const GLfloat *value);
extern void (APIENTRY *pglUniform4fvARB)(GLint location, GLsizei count, const GLfloat *value);
extern void (APIENTRY *pglUniform1ivARB)(GLint location, GLsizei count, const GLint *value);
extern void (APIENTRY *pglUniform2ivARB)(GLint location, GLsizei count, const GLint *value);
extern void (APIENTRY *pglUniform3ivARB)(GLint location, GLsizei count, const GLint *value);
extern void (APIENTRY *pglUniform4ivARB)(GLint location, GLsizei count, const GLint *value);
extern void (APIENTRY *pglUniformMatrix2fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (APIENTRY *pglUniformMatrix3fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (APIENTRY *pglUniformMatrix4fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (APIENTRY *pglGetObjectParameterfvARB)(GLhandleARB obj, GLenum pname, GLfloat *params);
extern void (APIENTRY *pglGetObjectParameterivARB)(GLhandleARB obj, GLenum pname, GLint *params);
extern void (APIENTRY *pglGetInfoLogARB)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
extern void (APIENTRY *pglGetAttachedObjectsARB)(GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
extern GLint (APIENTRY *pglGetUniformLocationARB)(GLhandleARB programObj, const GLcharARB *name);
extern void (APIENTRY *pglGetActiveUniformARB)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
extern void (APIENTRY *pglGetUniformfvARB)(GLhandleARB programObj, GLint location, GLfloat *params);
extern void (APIENTRY *pglGetUniformivARB)(GLhandleARB programObj, GLint location, GLint *params);
extern void (APIENTRY *pglGetShaderSourceARB)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);
extern void (APIENTRY *pglTexImage3D)(GLenum target, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
extern void (APIENTRY *pglTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
extern void (APIENTRY *pglCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void (APIENTRY *pglBlendEquationEXT)(GLenum);
extern void (APIENTRY *pglStencilOpSeparate)(GLenum, GLenum, GLenum, GLenum);
extern void (APIENTRY *pglStencilFuncSeparate)(GLenum, GLenum, GLint, GLuint);
extern void (APIENTRY *pglActiveStencilFaceEXT)(GLenum);
extern void (APIENTRY *pglVertexAttribPointerARB)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
extern void (APIENTRY *pglEnableVertexAttribArrayARB)(GLuint index);
extern void (APIENTRY *pglDisableVertexAttribArrayARB)(GLuint index);
extern void (APIENTRY *pglBindAttribLocationARB)(GLhandleARB programObj, GLuint index, const GLcharARB *name);
extern void (APIENTRY *pglGetActiveAttribARB)(GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
extern GLint (APIENTRY *pglGetAttribLocationARB)(GLhandleARB programObj, const GLcharARB *name);
extern void (APIENTRY *pglBindBufferARB)(GLenum target, GLuint buffer);
extern void (APIENTRY *pglDeleteBuffersARB)(GLsizei n, const GLuint *buffers);
extern void (APIENTRY *pglGenBuffersARB)(GLsizei n, GLuint *buffers);
extern GLboolean (APIENTRY *pglIsBufferARB)(GLuint buffer);
extern void* (APIENTRY *pglMapBufferARB)(GLenum target, GLenum access);
extern GLboolean (APIENTRY *pglUnmapBufferARB)(GLenum target);
extern void (APIENTRY *pglBufferDataARB)(GLenum target, GLsizeiptrARB size, const void *data, GLenum usage);
extern void (APIENTRY *pglBufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const void *data);
extern void (APIENTRY *pglGenQueriesARB)(GLsizei n, GLuint *ids);
extern void (APIENTRY *pglDeleteQueriesARB)(GLsizei n, const GLuint *ids);
extern GLboolean (APIENTRY *pglIsQueryARB)(GLuint id);
extern void (APIENTRY *pglBeginQueryARB)(GLenum target, GLuint id);
extern void (APIENTRY *pglEndQueryARB)(GLenum target);
extern void (APIENTRY *pglGetQueryivARB)(GLenum target, GLenum pname, GLint *params);
extern void (APIENTRY *pglGetQueryObjectivARB)(GLuint id, GLenum pname, GLint *params);
extern void (APIENTRY *pglGetQueryObjectuivARB)(GLuint id, GLenum pname, GLuint *params);
extern void (APIENTRY * pglSelectTextureSGIS)(GLenum);
extern void (APIENTRY * pglMTexCoord2fSGIS)(GLenum, GLfloat, GLfloat);
extern void (APIENTRY * pglSwapInterval)(int interval);
extern GLboolean (APIENTRY *pglIsRenderbuffer)(GLuint renderbuffer);
extern void (APIENTRY *pglBindRenderbuffer)(GLenum target, GLuint renderbuffer);
extern void (APIENTRY *pglDeleteRenderbuffers)(GLsizei n, const GLuint *renderbuffers);
extern void (APIENTRY *pglGenRenderbuffers)(GLsizei n, GLuint *renderbuffers);
extern void (APIENTRY *pglRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern void (APIENTRY *pglRenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
extern void (APIENTRY *pglGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint *params);
extern GLboolean (APIENTRY *pglIsFramebuffer)(GLuint framebuffer);
extern void (APIENTRY *pglBindFramebuffer)(GLenum target, GLuint framebuffer);
extern void (APIENTRY *pglDeleteFramebuffers)(GLsizei n, const GLuint *framebuffers);
extern void (APIENTRY *pglGenFramebuffers)(GLsizei n, GLuint *framebuffers);
extern GLenum (APIENTRY *pglCheckFramebufferStatus)(GLenum target);
extern void (APIENTRY *pglFramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void (APIENTRY *pglFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void (APIENTRY *pglFramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer);
extern void (APIENTRY *pglFramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern void (APIENTRY *pglFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern void (APIENTRY *pglGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
extern void (APIENTRY *pglBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern void (APIENTRY *pglGenerateMipmap)(GLenum target);
extern PROC (WINAPI *pwglGetProcAddress)(const char *);

#endif