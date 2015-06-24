#include "gl_local.h"
#include "screen.h"

qboolean reflect;
int reflect_texture_width;
int reflect_texture_height;
int reflect_texture_minsize;
mplane_t reflect_frustum[4];
byte *reflect_pixels;

r_water_t water[MAX_WATERS];
r_water_t *curwater = NULL;
int numwaters;

int water_normalmap = 0;
int water_dudvmap = 0;

qboolean use_water_program;

int water_vp_depth_program;
int water_vp_radial_program;
int water_fp_aw_reg_program;
int water_fp_aw_fog_program;
int water_fp_uw_reg_program;
int water_fp_uw_fog_program;

int water_vertexshader;
int water_fragshader;
int water_program;

int waterfog;
vec3_t waterfog_col;
float waterfog_start;
float waterfog_end;
float waterfog_den;

int save_userfogon;
int waterfog_on;
int *g_bUserFogOn;

vec3_t save_vieworg;
vec3_t save_viewang;
vec3_t save_rorigin;
vec3_t save_viewvec[3];

vec3_t reflect_vpn, reflect_vright, reflect_vup;

char water_vp_depth [] =
"!!ARBvp1.0"
"TEMP R0;"
"TEMP R1;"
"DP4 R0.x, vertex.position, program.local[0];"
"DP4 R0.y, vertex.position, program.local[1];"
"DP4 R0.z, vertex.position, program.local[2];"
"DP4 R0.w, vertex.position, program.local[3];"

"DP4 R1.x, R0, program.local[4];"
"DP4 R1.y, R0, program.local[5];"
"DP4 R1.z, R0, program.local[6];"
"DP4 R1.w, R0, program.local[7];"//vec4 v = gl_ModelViewProjectionMatrix * gl_Vertex;

"MUL result.texcoord[0].xy, vertex.texcoord, 0.0078125;"//gl_TexCoord[0].xy = gl_MultiTexCoord0.xy / 128;
"MOV result.texcoord[1], R1;"//gl_TexCoord[1] = v;
"MOV result.texcoord[2].xyz, vertex.position;"//gl_TexCoord[2].xyz = gl_Position.xyz;

"MOV result.position, R1;"//gl_Position = v;
"MOV result.fogcoord.x, R1.z;"//gl_FogCoord.x = v.z;
"END";

char water_vp_radial [] =
"!!ARBvp1.0"
"TEMP R0;"
"TEMP R1;"
"DP4 R0.x, vertex.position, program.local[0];"
"DP4 R0.y, vertex.position, program.local[1];"
"DP4 R0.z, vertex.position, program.local[2];"
"DP4 R0.w, vertex.position, program.local[3];"

"DP3 R1.x, R0, R0;"
"RSQ R1.x, R1.x;"
"RCP result.fogcoord.x, R1.x;"

"DP4 R1.x, R0, program.local[4];"
"DP4 R1.y, R0, program.local[5];"
"DP4 R1.z, R0, program.local[6];"
"DP4 R1.w, R0, program.local[7];"

"MUL result.texcoord[0].xy, vertex.texcoord, 0.0078125;"
"MOV result.texcoord[1], R1;"
"MOV result.texcoord[2].xyz, vertex.position;"
"MOV result.position, R1;"
"END";

char water_fp_aw_reg [] =
"!!ARBfp1.0"
"OPTION ARB_precision_hint_fastest;"
"PARAM c[8] = { program.local[0..3],"//local[0]=r_origin local[1]=fogcolor local[2]=fen local[3]=time
"{ 1.3, 0.97000003, 0.5, 0 },"
"{ 0.2, 0.15000001, 0.13, 0.11 },"
"{ 0.17, 0.14, 0.16, 1 },"
"{ 0.23, 0.33333334 } };"
"TEMP R0;"
"TEMP R1;"
"TEMP R2;"
"MOV R1, c[5];"//R1={ 0.2, 0.15000001, 0.13, 0.11 }
"MAD R0.zw, R1.xyxy, c[3].x, fragment.texcoord[0].xyxy;"//
"MAD R0.y, R1.w, c[3].x, fragment.texcoord[0];"
"MAD R0.x, R1.z, -c[3], fragment.texcoord[0];"
"TEX R1.xyz, R0, texture[0], 2D;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R2.xyz, R0, R1;"
"MOV R0.xyz, c[6];"
"MAD R1.xy, R0.yzzw, -c[3].x, fragment.texcoord[0];"
"MAD R0.z, R0.x, c[3].x, fragment.texcoord[0].x;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R0.xyz, R2, R0;"
"TEX R1.xyz, R1, texture[0], 2D;"
"ADD R0.xyz, R0, R1;"
"ADD R1.xyz, -fragment.texcoord[2], c[0];"
"DP3 R1.w, R1, R1;"
"RSQ R1.w, R1.w;"
"MUL R0.xyz, R0, c[4].z;"
"ADD R0.xyz, R0, -c[6].w;"
"DP3 R0.z, R0, R0;"
"RSQ R0.z, R0.z;"
"MUL R0.xy, R0.z, R0;"
"MUL R2.xy, R0, c[7].x;"
"RCP R2.z, fragment.texcoord[1].w;"
"MUL R0.xy, R2.z, fragment.texcoord[1];"
"MAD R0.xy, R0, c[4].z, R2;"
"ADD R0.xy, R0, c[4].z;"
"MUL R1.z, R1.w, R1;"//R1.z = R1 * R1.w;
"TEX R0, R0, texture[1], 2D;"//R0 = texture2D(tex1, R0)(R0=texcoord)
"MOV R1.y, -fragment.texcoord[1];"//R1.y = -gl_TexCoord[1].x;
"MOV R1.x, fragment.texcoord[1];"//R1.x = gl_TexCoord[1].x;
"MUL R1.xy, R2.z, R1;"//R1.xy = R1 * R2.z;
"MAD R1.xy, R1, c[4].z, R2;"//R1.xy = R1 * 0.5 + R2
"MUL R2.x, R1.z, c[2];"//R2.x = R1.z * fen
"ADD R1.xy, R1, c[4].z;"//R1.xy = R1.xy + 0.5;
"TEX R1, R1, texture[2], 2D;"//R1 = texture2D(tex2, R1);(R1=texcoord)
"MUL R2.x, R2, c[4];"//R2.x = dot(R2, c[4]);
"ADD R0, R0, -R1;"//R0 -= R1;
"MIN_SAT R2.x, R2, c[4].y;"//R2.x = min(R2.x, 0.97);
"MAD R1, R2.x, R0, R1;"//R1 = R0 * R2.x + R1;
"ADD R0.x, R1, R1.y;"//R0.x = R1 + R1.y;
"ADD R0.x, R0, R1.z;"//R0.x = R0 + R1.z;
"MUL R0.xyz, R0.x, c[1];"//R0.xyz = fogcolor * R0.x;
"MOV R0.w, c[6];"//R0.w = 1;
"MUL R0.xyz, R0, c[7].y;"//R0.xyz = R0.xyz / 3;
"ADD R0, R0, -R1;"//R0 -= R1;;
"MAD result.color, R0, c[5].x, R1;"//g_FragColor=R0*0.2+R1;
"END";

char water_fp_aw_fog [] =
"!!ARBfp1.0"
"OPTION ARB_precision_hint_fastest;"
"OPTION ARB_fog_linear;"
"PARAM c[8] = { program.local[0..3],"
"{ 1.3, 0.97000003, 0.5, 0 },"
"{ 0.2, 0.15000001, 0.13, 0.11 },"
"{ 0.17, 0.14, 0.16, 1 },"
"{ 0.23, 0.33333334 } };"
"TEMP R0;"
"TEMP R1;"
"TEMP R2;"
"MOV R1, c[5];"
"MAD R0.zw, R1.xyxy, c[3].x, fragment.texcoord[0].xyxy;"
"MAD R0.y, R1.w, c[3].x, fragment.texcoord[0];"
"MAD R0.x, R1.z, -c[3], fragment.texcoord[0];"
"TEX R1.xyz, R0, texture[0], 2D;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R2.xyz, R0, R1;"
"MOV R0.xyz, c[6];"
"MAD R1.xy, R0.yzzw, -c[3].x, fragment.texcoord[0];"
"MAD R0.z, R0.x, c[3].x, fragment.texcoord[0].x;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R0.xyz, R2, R0;"
"TEX R1.xyz, R1, texture[0], 2D;"
"ADD R0.xyz, R0, R1;"
"ADD R1.xyz, -fragment.texcoord[2], c[0];"
"DP3 R1.w, R1, R1;"
"RSQ R1.w, R1.w;"
"MUL R0.xyz, R0, c[4].z;"
"ADD R0.xyz, R0, -c[6].w;"
"DP3 R0.z, R0, R0;"
"RSQ R0.z, R0.z;"
"MUL R0.xy, R0.z, R0;"
"MUL R2.xy, R0, c[7].x;"
"RCP R2.z, fragment.texcoord[1].w;"
"MUL R0.xy, R2.z, fragment.texcoord[1];"
"MAD R0.xy, R0, c[4].z, R2;"
"ADD R0.xy, R0, c[4].z;"
"MUL R1.z, R1.w, R1;"
"TEX R0, R0, texture[1], 2D;"
"MOV R1.y, -fragment.texcoord[1];"
"MOV R1.x, fragment.texcoord[1];"
"MUL R1.xy, R2.z, R1;"
"MAD R1.xy, R1, c[4].z, R2;"
"MUL R2.x, R1.z, c[2];"
"ADD R1.xy, R1, c[4].z;"
"TEX R1, R1, texture[2], 2D;"
"MUL R2.x, R2, c[4];"
"ADD R0, R0, -R1;"
"MIN_SAT R2.x, R2, c[4].y;"
"MAD R1, R2.x, R0, R1;"
"ADD R0.x, R1, R1.y;"
"ADD R0.x, R0, R1.z;"
"MUL R0.xyz, R0.x, c[1];"
"MOV R0.w, c[6];"
"MUL R0.xyz, R0, c[7].y;"
"ADD R0, R0, -R1;"
"MAD result.color, R0, c[5].x, R1;"
"END";

char water_fp_uw_reg [] =
"!!ARBfp1.0"
"OPTION ARB_precision_hint_fastest;"
"PARAM c[6] = { program.local[0..1],"
"{ 0.5, 0, 0.2, 0.15000001 },"
"{ 0.13, 0.11, 0.17, 0.14 },"
"{ 0.16, 1, 0.30000001 },"
"{ 0.06666667, 0 } };"
"TEMP R0;"
"TEMP R1;"
"TEMP R2;"
"MOV R1, c[3];"
"MAD R2.xw, R1.yyzz, c[1].x, fragment.texcoord[0].yyzx;"
"MOV R0.zw, c[2];"
"MAD R0.zw, R0, c[1].x, fragment.texcoord[0].xyxy;"
"MAD R0.x, R1, -c[1], fragment.texcoord[0];"
"MOV R0.y, R2.x;"
"TEX R1.xyz, R0, texture[0], 2D;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R2.xyz, R0, R1;"
"MOV R0.y, R0.w;"
"MOV R0.w, c[4].x;"
"MAD R1.x, R1.w, -c[1], fragment.texcoord[0];"
"MAD R1.y, R0.w, -c[1].x, fragment.texcoord[0];"
"MOV R0.x, R2.w;"
"TEX R0.xyz, R0, texture[0], 2D;"
"TEX R1.xyz, R1, texture[0], 2D;"
"ADD R0.xyz, R2, R0;"
"ADD R0.xyz, R0, R1;"
"MUL R0.xyz, R0, c[2].x;"
"ADD R0.xyz, R0, -c[4].y;"
"DP3 R0.z, R0, R0;"
"RSQ R0.z, R0.z;"
"MUL R0.zw, R0.z, R0.xyxy;"
"RCP R0.x, fragment.texcoord[1].w;"
"MUL R0.zw, R0, c[4].z;"
"MUL R0.xy, R0.x, fragment.texcoord[1];"
"MAD R0.xy, R0, c[2].x, R0.zwzw;"
"ADD R0.xy, R0, c[2].x;"
"TEX R0, R0, texture[1], 2D;"
"ADD R2.x, R0, R0.y;"
"MOV R1.w, c[4].y;"
"MOV R1.xyz, c[0];"
"ADD R1, -R0, R1;"
"ADD R2.x, R2, R0.z;"
"MUL R1, R2.x, R1;"
"MAD result.color, R1, c[5].x, R0;"
"END";

char water_fp_uw_fog [] =
"!!ARBfp1.0"
"OPTION ARB_precision_hint_fastest;"
"OPTION ARB_fog_linear;"
"PARAM c[6] = { program.local[0..1],"
"{ 0.5, 0, 0.2, 0.15000001 },"
"{ 0.13, 0.11, 0.17, 0.14 },"
"{ 0.16, 1, 0.30000001 },"
"{ 0.06666667, 0 } };"
"TEMP R0;"
"TEMP R1;"
"TEMP R2;"
"MOV R1, c[3];"
"MAD R2.xw, R1.yyzz, c[1].x, fragment.texcoord[0].yyzx;"
"MOV R0.zw, c[2];"
"MAD R0.zw, R0, c[1].x, fragment.texcoord[0].xyxy;"
"MAD R0.x, R1, -c[1], fragment.texcoord[0];"
"MOV R0.y, R2.x;"
"TEX R1.xyz, R0, texture[0], 2D;"
"TEX R0.xyz, R0.zwzw, texture[0], 2D;"
"ADD R2.xyz, R0, R1;"
"MOV R0.y, R0.w;"
"MOV R0.w, c[4].x;"
"MAD R1.x, R1.w, -c[1], fragment.texcoord[0];"
"MAD R1.y, R0.w, -c[1].x, fragment.texcoord[0];"
"MOV R0.x, R2.w;"
"TEX R0.xyz, R0, texture[0], 2D;"
"TEX R1.xyz, R1, texture[0], 2D;"
"ADD R0.xyz, R2, R0;"
"ADD R0.xyz, R0, R1;"
"MUL R0.xyz, R0, c[2].x;"
"ADD R0.xyz, R0, -c[4].y;"
"DP3 R0.z, R0, R0;"
"RSQ R0.z, R0.z;"
"MUL R0.zw, R0.z, R0.xyxy;"
"RCP R0.x, fragment.texcoord[1].w;"
"MUL R0.zw, R0, c[4].z;"
"MUL R0.xy, R0.x, fragment.texcoord[1];"
"MAD R0.xy, R0, c[2].x, R0.zwzw;"
"ADD R0.xy, R0, c[2].x;"
"TEX R0, R0, texture[1], 2D;"
"ADD R2.x, R0, R0.y;"
"MOV R1.w, c[4].y;"
"MOV R1.xyz, c[0];"
"ADD R1, -R0, R1;"
"ADD R2.x, R2, R0.z;"
"MUL R1, R2.x, R1;"
"MAD result.color, R1, c[5].x, R0;"
"END";

inline float sgn(float a)
{
    if (a > 0.0F) return (1.0F);
    if (a < 0.0F) return (-1.0F);
    return (0.0F);
}

void R_RenderWaterFog(void) 
{
	if(waterfog)
	{
		save_userfogon = *g_bUserFogOn;
		*g_bUserFogOn = 1;
		waterfog_on = 1;
		qglEnable(GL_FOG);
		qglFogi(GL_FOG_MODE, GL_LINEAR);
		qglFogf(GL_FOG_DENSITY, waterfog_den);
		qglHint(GL_FOG_HINT, GL_NICEST);

		qglFogf(GL_FOG_START, waterfog_start);
		qglFogf(GL_FOG_END, (r_params.waterlevel >= 3) ? waterfog_end / 4 : waterfog_end);
		qglFogfv(GL_FOG_COLOR, waterfog_col);

		if(gl_radialfog_support)
			qglFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
		else
			qglFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_PLANE_ABSOLUTE_NV);
	}
	else
	{
		waterfog_on = 0;
	}
}

void R_FinalWaterFog(void) 
{
	if(waterfog_on)
	{
		waterfog_on = 0;
		*g_bUserFogOn = save_userfogon;
		qglDisable(GL_FOG);
	}
}

void R_InitWater(void)
{
	int power;
	int min_texture_size;
	int max_texture_size;

	for (power = 2; power < r_refdef->vrect.height; power *= 2)
		min_texture_size = power;

	qglGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

	if (min_texture_size > max_texture_size)
	{
		for (power = 2; power < max_texture_size; power *= 2)
			min_texture_size = power;
	}

	reflect_texture_width = g_iVideoWidth;//;(r_refdef->vrect.width < min_texture_size) ? r_refdef->vrect.width : min_texture_size;
	reflect_texture_height = g_iVideoHeight;//;(r_refdef->vrect.width < min_texture_size) ? r_refdef->vrect.width : min_texture_size;
	reflect_texture_minsize = min_texture_size;

	refract_texture_width = reflect_texture_width;
	refract_texture_height = reflect_texture_height;
	refract_texture_minsize = reflect_texture_minsize;

	if (gl_program_support && gl_shaderprogram && gl_shaderprogram->value)
	{
		water_vp_depth_program = GL_CompileProgram(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_vp_depth);
		water_vp_radial_program = GL_CompileProgram(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_vp_radial);
		water_fp_aw_reg_program = GL_CompileProgram(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_fp_aw_reg);
		water_fp_aw_fog_program = GL_CompileProgram(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_fp_aw_fog);
		water_fp_uw_reg_program = GL_CompileProgram(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_fp_uw_reg);
		water_fp_uw_fog_program = GL_CompileProgram(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_fp_uw_fog);
	}

	use_water_program = (water_vp_depth_program && water_fp_aw_reg_program	&& water_fp_uw_reg_program);
	//reflect_pixels = (byte *)malloc(reflect_texture_width * reflect_texture_height * 3);
	//refract_pixels = (byte *)malloc(refract_texture_width * refract_texture_height * 3);
	//depth_pixels = (byte *)malloc(refract_texture_width * refract_texture_height * 3);

	water_normalmap = gCKFExports.Surface_LoadTGA("resource\\tga\\water_normalmap");
	//water_dudvmap = gCKFExports.Surface_LoadTGA("resource\\tga\\water_dudvmap");

	waterfog_col[0] = 64.0f/255;
	waterfog_col[1] = 80.0f/255;
	waterfog_col[2] = 90.0f/255;
	waterfog_start = 100;
	waterfog_end = 3000;
	waterfog_den = 1;
	waterfog = 1;

	R_ClearWaters();
}

void R_ShutdownWater(void)
{
	//free(reflect_pixels);
	//free(refract_pixels);
	//free(depth_pixels);
}

int R_ShouldReflect(void) 
{
	if(curwater->vecs[2] > r_refdef->vieworg[2])
		return 0;

	for(int i = 0; i < numwaters; i++)
	{
		if(water[i].ent && curwater->ent != water[i].ent)
		{
			if(fabs(water[i].vecs[2] - curwater->vecs[2]) < 1.0f)
				return 0;
		}
	}
	return 1;
}

void R_AddWater(cl_entity_t *ent, vec3_t p)
{
	int i;
	vec3_t v;
	float distance;

	curwater = NULL;

	for (i = 0; i < numwaters; i++)
	{
		if (!water[i].reflect)
		{
			water[i].reflect = gCKFExports.Surface_GenTexture();

			GL_Bind(water[i].reflect);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, reflect_texture_width, reflect_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		}

		if (!water[i].refract)
		{
			water[i].refract = gCKFExports.Surface_GenTexture();

			GL_Bind(water[i].refract);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, refract_texture_width, refract_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		}

		//if (!water[i].depth && g_pfn_SurfaceGenTexture)
		//{
		//	water[i].depth = g_pfn_SurfaceGenTexture();

		//	GL_Bind(water[i].depth);
		//	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//	qglTexImage2D(GL_TEXTURE_2D, 0, 3, refract_texture_width, refract_texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, depth_pixels);
		//}

		if (water[i].ent == ent || fabs(water[i].vecs[2] - p[2]) < 1.0f)
		{
			//found one
			curwater = &water[i];
			return;
		}
	}

	//not found

	VectorSubtract(p, r_refdef->vieworg, v);
	distance = VectorLength(v);

	if (numwaters < MAX_WATERS)
	{
		VectorCopy(p, water[numwaters].vecs);
		water[numwaters].distances = distance;
		curwater = &water[numwaters];
		curwater->ent = ent;
		curwater->org[0] = (ent->curstate.mins[0] + ent->curstate.maxs[0]) / 2;
		curwater->org[1] = (ent->curstate.mins[1] + ent->curstate.maxs[1]) / 2;
		curwater->org[2] = (ent->curstate.mins[2] + ent->curstate.maxs[2]) / 2;
		numwaters++;		
	}
}

void R_ClearWaters(void)
{
	numwaters = 0;
}

void R_SetReflectFrustum(void)
{
	int i;

	if (scr_fov_value == 90) 
	{
		VectorAdd(vpn, vright, reflect_frustum[0].normal);
		VectorSubtract(vpn, vright, reflect_frustum[1].normal);

		VectorAdd(vpn, vup, reflect_frustum[2].normal);
		VectorSubtract(vpn, vup, reflect_frustum[3].normal);
	}
	else
	{
		RotatePointAroundVector(reflect_frustum[0].normal, vup, vpn, -(90 - scr_fov_value / 2));
		RotatePointAroundVector(reflect_frustum[1].normal, vup, vpn, 90 - scr_fov_value / 2);
		RotatePointAroundVector(reflect_frustum[2].normal, vright, vpn, 90 - yfov / 2);
		RotatePointAroundVector(reflect_frustum[3].normal, vright, vpn, -(90 - yfov / 2));
	}

	for (i = 0; i < 4; i++)
	{
		reflect_frustum[i].type = PLANE_ANYZ;
		reflect_frustum[i].dist = DotProduct(r_origin, reflect_frustum[i].normal);
		reflect_frustum[i].signbits = SignbitsForPlane(&reflect_frustum[i]);
	}
}

void R_SetupClip(qboolean isdrawworld)
{
	double clipPlane[] = {0, 0, 0, 0};

	if(reflect)
	{
		if(save_vieworg[2] < curwater->vecs[2])
		{
			clipPlane[2] = -1.0;
			clipPlane[3] = curwater->vecs[2];
		}
		else
		{
			clipPlane[2] = 1.0;
			clipPlane[3] = -curwater->vecs[2];
		}
	}
	if(refract)
	{
		if (save_vieworg[2] < curwater->vecs[2])
		{
			clipPlane[2] = 1.0;
			clipPlane[3] = -curwater->vecs[2];
		}
		else
		{
			clipPlane[2] = -1.0;
			clipPlane[3] = curwater->vecs[2];
		}
	}
	if(isdrawworld)
	{
		clipPlane[3] += 32;
	}
	if(reflect || refract)
	{
		qglEnable(GL_CLIP_PLANE0);
		qglClipPlane(GL_CLIP_PLANE0, clipPlane);
	}
}

void R_SetupReflect(void)
{
	R_SaveRefDef();

	r_refdef->vieworg[2] = (2 * curwater->vecs[2]) - r_refdef->vieworg[2];
	r_refdef->viewangles[0] *= -1;
	r_refdef->viewangles[2] *= -1;

	R_SetupGL();

	R_RestoreRefDef();
	r_refdef->viewangles[0] = -r_refdef->viewangles[0];
	R_UpdateRefDef();

	qglViewport(0, 0, reflect_texture_width, reflect_texture_height);

	r_refdef->onlyClientDraws = true;

	R_SetReflectFrustum();

	reflect = true;

	R_SetupClip(true);

	R_RenderWaterFog();

	if(s_BackBufferFBO.s_hBackBufferFBO)
	{
		qglFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curwater->reflect, 0);
	}

	qglClearColor(waterfog_col[0], waterfog_col[1], waterfog_col[2], 1);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void R_FinishReflect(void)
{
	R_FinalWaterFog();

	qglDisable(GL_CLIP_PLANE0);

	if(!s_BackBufferFBO.s_hBackBufferFBO)
	{
		GL_Bind(curwater->reflect);
		qglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, reflect_texture_width, reflect_texture_height, 0);
	}

	R_RestoreRefDef();

	r_refdef->onlyClientDraws = false;
	reflect = false;
}

void R_UpdateWater(void)
{
	if (r_refdef->onlyClientDraws)
		return;
	if(!use_water_program || !gl_watershader->value)
		return;

	if(s_BackBufferFBO.s_hBackBufferFBO)
	{
		qglBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, s_BackBufferFBO.s_hBackBufferFBO);
	}

	qglClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < numwaters; i++)
	{
		curwater = &water[i];
		if(R_ShouldReflect())
		{
			R_SetupReflect();
			R_ClearSkyBox();
			R_DrawWorld();
			R_SetupClip(false);
			R_DrawEntitiesOnList();
			gExportfuncs.HUD_DrawNormalTriangles();
			R_DrawTEntitiesOnList(0);
			R_FinishReflect();
			R_Clear();
		}

		R_SetupRefract();
		R_ClearSkyBox();
		R_DrawWorld();
		R_SetupClip(false);
		R_DrawEntitiesOnList();
		gExportfuncs.HUD_DrawNormalTriangles();
		R_DrawTEntitiesOnList(0);
		R_FinishRefract();
		R_Clear();
	}

	if(s_BackBufferFBO.s_hBackBufferFBO)
	{
		qglFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_BackBufferFBO.s_hBackBufferTex, 0);
		if (s_MSAAFBO.s_hBackBufferFBO)
		{
			qglBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, s_MSAAFBO.s_hBackBufferFBO);
		}
	}

	GLenum error = qglGetError();

	if (error)
		assert(qglGetError() == GL_NO_ERROR);
}