#include "gl_local.h"
#include "screen.h"

qboolean reflect;

float reflect_texture_width;
float reflect_texture_height;
float reflect_texture_minsize;

ref_params_t water_refparam;
r_water_t water[MAX_WATERS];
r_water_t *curwater;

mplane_t reflect_frustum[4];
byte *reflect_pixels;
byte *refract_pixels;
int numwaters;

qboolean use_water_program;
//int reflect_water_fp_program = 0;
//int reflect_water_vp_program = 0;

int water_vp_depth_program = 0;
int water_vp_radial_program = 0;
int water_fp_aw_reg_program = 0;
int water_fp_aw_fog_program = 0;
int water_fp_uw_reg_program = 0;
int water_fp_uw_fog_program = 0;

double VectorLength(vec3_t v);

//===========================================
//	ARB SHADERS
//===========================================
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
"DP4 R1.w, R0, program.local[7];"

"MUL result.texcoord[0].xy, vertex.texcoord, 0.0078125;"
"MOV result.texcoord[1], R1;"
"MOV result.texcoord[2].xyz, vertex.position;"

"MOV result.position, R1;"
"MOV result.fogcoord.x, R1.z;"
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
//===========================================
//	ARB SHADERS
//===========================================

char reflect_water_fp[] =
{
	"!!ARBfp1.0"

	"TEMP distortColor;"
	"TEMP distortColor2;"
	"TXP distortColor, fragment.texcoord[1], texture[1], 2D;"
	"TXP distortColor2, fragment.texcoord[2], texture[1], 2D;"
	"SUB distortColor, distortColor, distortColor2;"

	"TEMP recipW;"
	"TEMP reflectionUV;"
	"RCP recipW, fragment.texcoord[0].w;"
	"MAD reflectionUV, recipW, fragment.texcoord[0], 0.5;"

	"TEMP endColor;"
	"MAD reflectionUV, distortColor, recipW, reflectionUV;"
	"TEX endColor, reflectionUV, texture, 2D;"

	"TEMP normalColor;"
	"TEMP normalColor2;"

	"TXP normalColor, fragment.texcoord[1], texture[2], 2D;"
	"TXP normalColor2, fragment.texcoord[2], texture[2], 2D;"

	"ADD normalColor, normalColor, normalColor2;"
	"SUB normalColor, normalColor, 1.0;"

	"TEMP eyeVec;"
	"TEMP inverseLength;"
	"DP3 inverseLength, fragment.texcoord[3], fragment.texcoord[3];"
	"RSQ inverseLength, inverseLength.x;"
	"MUL eyeVec, fragment.texcoord[3], inverseLength;"

	"TEMP transparency;"
	"DP3 transparency.w, normalColor, eyeVec;"
	"SUB endColor.w, 0.5, transparency.w;"
	"MAX endColor.w, endColor.w, 0.35;"
	"MIN endColor.w, endColor.w, 0.45;"

	"MOV result.color, endColor;"

	"END"
};

char reflect_water_vp[] =
{
	"!!ARBvp1.0"

	"TEMP pos;"

	"DP4 pos.x, state.matrix.mvp.row[0], vertex.position;"
	"DP4 pos.y, state.matrix.mvp.row[1], vertex.position;"
	"DP4 pos.z, state.matrix.mvp.row[2], vertex.position;"
	"DP4 pos.w, state.matrix.mvp.row[3], vertex.position;"
	"MOV result.position, pos;"

	"PARAM scaleFlipReflection = { 0.5, -0.5, 1.0, 1.0 };"
	"MUL result.texcoord[0], pos, scaleFlipReflection;"

	"END"
};

extern "C" __declspec(dllexport) int R_IsReflectPasses(void)
{
	return reflect;
}

inline float sgn(float a)
{
    if (a > 0.0F) return (1.0F);
    if (a < 0.0F) return (-1.0F);
    return (0.0F);
}

void R_InitReflect(void)
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

	reflect_texture_width = (r_refdef->vrect.width < min_texture_size) ? r_refdef->vrect.width : min_texture_size;
	reflect_texture_height = (r_refdef->vrect.width < min_texture_size) ? r_refdef->vrect.width : min_texture_size;
	reflect_texture_minsize = min_texture_size;

	if (gl_program && gl_shaderprogram)
	{
		//reflect_water_fp_program = GL_CompileProgram(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, reflect_water_fp);
		//reflect_water_vp_program = GL_CompileProgram(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, reflect_water_vp);
		water_vp_depth_program = GL_CompileProgram(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_vp_depth);
		water_vp_radial_program = GL_CompileProgram(GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_vp_radial);
		water_fp_aw_reg_program = GL_CompileProgram(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_fp_aw_reg);
		water_fp_aw_fog_program = GL_CompileProgram(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_fp_aw_fog);
		water_fp_uw_reg_program = GL_CompileProgram(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_fp_uw_reg);
		water_fp_uw_fog_program = GL_CompileProgram(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, water_fp_uw_fog);
	}

	use_water_program = (water_vp_depth_program && water_vp_radial_program && 
		water_fp_aw_reg_program && water_fp_aw_fog_program && water_fp_uw_reg_program && water_fp_uw_fog_program);//(reflect_water_fp_program && reflect_water_vp_program);
	reflect_pixels = (byte *)malloc(reflect_texture_width * reflect_texture_height * 3);
	refract_pixels = (byte *)malloc(reflect_texture_width * reflect_texture_height * 3);

	if(g_pfn_SurfaceLoadTGA)
	{
		water_normalmap = g_pfn_SurfaceLoadTGA("resource\\tga\\watershader.tga");
	}

}

void R_ShutdownReflect(void)
{
	free(reflect_pixels);
	free(refract_pixels);
}

void R_AddWater(cl_entity_t *entity)
{
	if(numwaters == MAX_WATERS)
		return;

	for(int i = 0; i < numwaters; i++)
	{
		if(water[i].entity == entity)
			return;// Already in cache
	}

	if(!entity->model)
		return;

	int isurfacecount = 0;
	msurface_t *psurfaces = entity->model->surfaces + entity->model->firstmodelsurface;
	for(int i = 0; i < entity->model->nummodelsurfaces; i++)
	{
		int j = 0;
		for(; j < psurfaces[i].polys->numverts; j++)
		{
			if(psurfaces[i].polys->verts[0][2] != (entity->curstate.maxs[2]-1))
				break;
		}
		
		if(j != psurfaces[i].polys->numverts)
			continue;

		if(psurfaces[i].flags & SURF_PLANEBACK)
			continue;

		if (psurfaces[i].plane->normal[2] != 1)
			continue;

		isurfacecount++;
	}

	if(!isurfacecount)
		return;

	// Allocate array of pointers
	curwater = &water[numwaters];
	curwater->surfaces = (msurface_t **)malloc(sizeof(msurface_t *)*isurfacecount);
	int numsurfaces = 0;

	for(int i = 0; i < entity->model->nummodelsurfaces; i++)
	{
		int j = 0;
		for(; j < psurfaces[i].polys->numverts; j++)
		{
			if(psurfaces[i].polys->verts[0][2] != (entity->curstate.maxs[2]-1))
				break;
		}
		
		if(j != psurfaces[i].polys->numverts)
			continue;

		if(psurfaces[i].flags & SURF_PLANEBACK)
			continue;

		if (psurfaces[i].plane->normal[2] != 1)
			continue;

		curwater->surfaces[numsurfaces] = &psurfaces[i];
		numsurfaces++;
	}
	
	if(!numsurfaces && curwater->surfaces)
	{
		entity->model = NULL;
		free(curwater->surfaces);
		curwater->surfaces = NULL;
		return;
	}

	//curwater = &water[numwaters];
	curwater->index = numwaters;
	numwaters++;

	curwater->numsurfaces = numsurfaces;

	curwater->mins[0] = 9999;
	curwater->mins[1] = 9999;
	curwater->mins[2] = 9999;
	curwater->maxs[0] = -9999;
	curwater->maxs[1] = -9999;
	curwater->maxs[2] = -9999;

	for(int i = 0; i < curwater->numsurfaces; i++)
	{
		for (glpoly_t *bp = curwater->surfaces[i]->polys; bp; bp = bp->next)
		{
			for(int j = 0; j < bp->numverts; j++)
			{
				if(curwater->mins[0] > bp->verts[j][0])
					curwater->mins[0] = bp->verts[j][0];

				if(curwater->mins[1] > bp->verts[j][1])
					curwater->mins[1] = bp->verts[j][1];

				if(curwater->mins[2] > bp->verts[j][2])
					curwater->mins[2] = bp->verts[j][2];

				if(curwater->maxs[0] < bp->verts[j][0])
					curwater->maxs[0] = bp->verts[j][0];

				if(curwater->maxs[1] < bp->verts[j][1])
					curwater->maxs[1] = bp->verts[j][1];

				if(curwater->maxs[2] < bp->verts[j][2])
					curwater->maxs[2] = bp->verts[j][2];
			}
		}
	}

	curwater->entity = entity;
	curwater->entity->efrag = (efrag_s *)curwater;
	//entity->curstate.effects |= EF_NODRAW;

	curwater->wplane.dist = psurfaces->plane->dist;
	curwater->wplane.type = psurfaces->plane->type;
	curwater->wplane.pad[0] = psurfaces->plane->pad[0];
	curwater->wplane.pad[1] = psurfaces->plane->pad[1];
	curwater->wplane.signbits = psurfaces->plane->signbits;
	curwater->wplane.normal[2] = 1;

	curwater->reflect = 0;
	curwater->refract = 0;
	if(g_pfn_SurfaceGenTexture)
	{
		curwater->reflect = g_pfn_SurfaceGenTexture();
		curwater->refract = g_pfn_SurfaceGenTexture();
	}

	curwater->origin[0] = (curwater->mins[0] + curwater->maxs[0]) * 0.5f;
	curwater->origin[1] = (curwater->mins[1] + curwater->maxs[1]) * 0.5f;
	curwater->origin[2] = (curwater->mins[2] + curwater->maxs[2]) * 0.5f;

	GL_Bind(curwater->reflect);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexImage2D(GL_TEXTURE_2D, 0, 3, reflect_texture_width, reflect_texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, reflect_pixels);

	GL_Bind(curwater->refract);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	qglTexImage2D(GL_TEXTURE_2D, 0, 3, reflect_texture_width, reflect_texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, refract_pixels);
}

void R_SetupClipping( ref_params_t *pparams, bool negative ) 
{
	float	dot;
	float	eq1[4];
	float	eq2[4];
	float	projection[16];

	vec3_t	vDist;
	vec3_t	vNorm;

	vec3_t	vForward;
	vec3_t	vRight;
	vec3_t	vUp;
	vec3_t	vTemp;

	AngleVectors(pparams->viewangles, vForward, vRight, vUp );
	VectorSubtract(curwater->origin, pparams->vieworg, vDist);
	
	VectorInverse(vRight); 
	VectorInverse(vUp);

	if(negative)
	{
		VectorCopy(curwater->wplane.normal, vTemp);
		VectorInverse(vTemp);
		eq1[0] = DotProduct(vRight, vTemp);
		eq1[1] = DotProduct(vUp, vTemp);
		eq1[2] = DotProduct(vForward, vTemp);
		eq1[3] = DotProduct(vDist, vTemp);
	}
	else
	{
		eq1[0] = DotProduct(vRight, curwater->wplane.normal);
		eq1[1] = DotProduct(vUp, curwater->wplane.normal);
		eq1[2] = DotProduct(vForward, curwater->wplane.normal);
		eq1[3] = DotProduct(vDist, curwater->wplane.normal);
	}

	// Change current projection matrix into an oblique projection matrix
	qglGetFloatv(GL_PROJECTION_MATRIX, projection);

	eq2[0] = (sgn(eq1[0]) + projection[8]) / projection[0];
	eq2[1] = (sgn(eq1[1]) + projection[9]) / projection[5];
	eq2[2] = -1.0F;
	eq2[3] = (1.0F + projection[10]) / projection[14];

	dot = eq1[0]*eq2[0] + eq1[1]*eq2[1] + eq1[2]*eq2[2] + eq1[3]*eq2[3];

    projection[2] = eq1[0]*(2.0f/dot);
    projection[6] = eq1[1]*(2.0f/dot);
    projection[10] = eq1[2]*(2.0f/dot) + 1.0F;
    projection[14] = eq1[3]*(2.0f/dot);

	qglMatrixMode(GL_PROJECTION);
	qglPushMatrix();
	qglLoadMatrixf(projection);

	qglMatrixMode(GL_MODELVIEW);
}

int R_ViewInWater(void)
{
	if(r_params->vieworg[0] > curwater->entity->curstate.mins[0]
	&& r_params->vieworg[1] > curwater->entity->curstate.mins[1]
	&& r_params->vieworg[2] > curwater->entity->curstate.mins[2]
	&& r_params->vieworg[0] < curwater->entity->curstate.maxs[0]
	&& r_params->vieworg[1] < curwater->entity->curstate.maxs[1]
	&& r_params->vieworg[2] < curwater->entity->curstate.maxs[2])
		return 1;

	return 0;
}

void R_SetupRefract(void)
{
	qglCullFace(GL_FRONT);
	qglColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();

	qglRotatef(-90, 1, 0, 0);// put X going down
	qglRotatef(90, 0, 0, 1); // put Z going up
	qglRotatef(-r_params->viewangles[2], 1, 0, 0);
	qglRotatef(-r_params->viewangles[0], 0, 1, 0);
	qglRotatef(-r_params->viewangles[1], 0, 0, 1);
	qglTranslatef(-r_params->vieworg[0], -r_params->vieworg[1], -r_params->vieworg[2]);

	qglViewport(GL_ZERO, GL_ZERO, reflect_texture_width, reflect_texture_width);

	if(curwater->origin[2] < r_params->vieworg[2])
	{
		R_SetupClipping(r_params, 0);
	}
	else
	{		
		R_SetupClipping(r_params, 1);
	}
}

void R_FinishRefract(void) 
{
	//Save mirrored image
	qglBindTexture(GL_TEXTURE_2D, curwater->refract);
	qglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, reflect_texture_width, reflect_texture_width, 0);

	//Completely clear everything
	qglClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

	qglMatrixMode(GL_PROJECTION);
	qglPopMatrix();

	//Restore modelview
	qglMatrixMode(GL_MODELVIEW);
	qglPopMatrix();
}

void R_SetupReflect(void) 
{
	vec3_t vForward, vRight, vUp;
	vec3_t vMins, vMaxs;
	AngleVectors(r_params->viewangles, vForward, vRight, vUp);

	float flDist = abs(curwater->origin[2] - r_params->vieworg[2]);
	VectorMA(r_params->vieworg, -2*flDist, curwater->wplane.normal, water_refparam.vieworg);

	vec3_t vTemp;
	VectorCopy(curwater->wplane.normal, vTemp);
	VectorInverse(vTemp);
	flDist = DotProduct(vForward, vTemp);
	VectorMA(vForward, -2*flDist, vTemp, vForward);

	water_refparam.viewangles[0] = -asin(vForward[2])/M_PI*180;
	water_refparam.viewangles[1] = atan2(vForward[1], vForward[0])/M_PI*180;
	water_refparam.viewangles[2] = -water_refparam.viewangles[2];

	AngleVectors(water_refparam.viewangles, water_refparam.forward, water_refparam.right, water_refparam.up);
	VectorCopy(water_refparam.viewangles, water_refparam.cl_viewangles);

	qglCullFace(GL_FRONT);
	qglColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	qglMatrixMode(GL_MODELVIEW);
	qglLoadIdentity();

	qglRotatef(-90, 1, 0, 0);// put X going down
	qglRotatef(90, 0, 0, 1); // put Z going up
	qglRotatef(-water_refparam.viewangles[2], 1, 0, 0);
	qglRotatef(-water_refparam.viewangles[0], 0, 1, 0);
	qglRotatef(-water_refparam.viewangles[1], 0, 0, 1);
	qglTranslatef(-water_refparam.vieworg[0], -water_refparam.vieworg[1], -water_refparam.vieworg[2]);

	qglViewport(GL_ZERO, GL_ZERO, reflect_texture_width, reflect_texture_width);

	R_SetupClipping(&water_refparam, 1);
}

void R_FinishReflect(void)
{
	//Save mirrored image
	qglBindTexture(GL_TEXTURE_2D, curwater->reflect);
	qglCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, reflect_texture_width, reflect_texture_width, 0);

	//Completely clear everything
	qglClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

	qglMatrixMode(GL_PROJECTION);
	qglPopMatrix();

	//Restore modelview
	qglMatrixMode(GL_MODELVIEW);
	qglPopMatrix();
}

void R_DrawWater(void) 
{
	if(!gl_reflectwater->value)
		return;
	if(!numwaters)
		return;
	if(!use_water_program)
		return;
	if(reflect)
		return;

	float flMatrix[16];
	float flTime = gEngfuncs.GetClientTime();

	GL_SaveMultiTexture();

	qglEnable(GL_VERTEX_PROGRAM_ARB);
	qglEnable(GL_FRAGMENT_PROGRAM_ARB);

	qglBindProgramARB(GL_VERTEX_PROGRAM_ARB, water_vp_depth_program);

	qglGetFloatv(GL_MODELVIEW_MATRIX, flMatrix);
	qglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 0, flMatrix[0], flMatrix[4], flMatrix[8], flMatrix[12]);
	qglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 1, flMatrix[1], flMatrix[5], flMatrix[9], flMatrix[13]);
	qglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 2, flMatrix[2], flMatrix[6], flMatrix[10], flMatrix[14]);
	qglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 3, flMatrix[3], flMatrix[7], flMatrix[11], flMatrix[15]);

	qglGetFloatv(GL_PROJECTION_MATRIX, flMatrix);
	qglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 4, flMatrix[0], flMatrix[4], flMatrix[8], flMatrix[12]);
	qglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 5, flMatrix[1], flMatrix[5], flMatrix[9], flMatrix[13]);
	qglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 6, flMatrix[2], flMatrix[6], flMatrix[10], flMatrix[14]);
	qglProgramLocalParameter4fARB(GL_VERTEX_PROGRAM_ARB, 7, flMatrix[3], flMatrix[7], flMatrix[11], flMatrix[15]);

	for(int i = 0; i < numwaters; i++)
	{
		curwater = &water[i];
		
		if(!water[i].draw)
			continue;

		if(r_params->vieworg[2] > curwater->origin[2])
		{
			qglCullFace(GL_FRONT);
			qglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, water_fp_aw_reg_program);
			float flFresnelTerm = 1.0f;

			qglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, r_params->vieworg[0], r_params->vieworg[1], r_params->vieworg[2], 0);
			qglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, .2, .3, .5, 0);
			qglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 2, flFresnelTerm, 0, 0, 0);
			qglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 3, flTime, 0, 0, 0);
		}
		else
		{
			qglCullFace(GL_BACK);
			qglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, water_fp_uw_reg_program);

			qglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, .2, .3, .5, 0);
			qglProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 1, flTime, 0, 0, 0);
		}

		qglEnable(GL_BLEND);

		GL_SelectTexture(GL_TEXTURE0_ARB);
		qglEnable(GL_TEXTURE_2D);
		GL_Bind(water_normalmap);
		qglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		qglTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);

		GL_SelectTexture(GL_TEXTURE1_ARB);
		qglEnable(GL_TEXTURE_2D);
		GL_Bind(curwater->refract);
		qglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		qglTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);

		int j = 0;
		for(; j < i; j++)
		{
			if(water[j].draw)
			{
				if(water[j].origin[2] == curwater->origin[2])
				{
					GL_SelectTexture(GL_TEXTURE2_ARB);
					qglEnable(GL_TEXTURE_2D);
					GL_Bind(water[i].reflect);
					break;
				}
			}
		}

		if(j == i)
		{
			qglActiveTextureARB(GL_TEXTURE2_ARB);
			GL_SelectTexture(GL_TEXTURE2_ARB);
			qglEnable(GL_TEXTURE_2D);
			GL_Bind(curwater->reflect);
		}

		qglTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		qglTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);

		for(int k = 0; k < curwater->numsurfaces; ++k)
			EmitWaterPolys(curwater->surfaces[k], 0);
	}

	qglBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
	qglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);

	qglDisable(GL_VERTEX_PROGRAM_ARB);
	qglDisable(GL_FRAGMENT_PROGRAM_ARB);

	qglCullFace(GL_FRONT);

	qglDisable(GL_BLEND);

	GL_ResetRenderer();

	GL_RestoreMultiTexture();
}

void R_ClearWaters(void)
{
	numwaters = 0;
}

int R_ShouldReflect(int index)
{
	if(curwater->origin[2] > r_params->vieworg[2])
		return false;

	// Optimization: Try and find a water entity on the same z coord
	for(int i = 0; i < index; i++)
	{
		if(water[i].draw)
		{
			if(water[i].origin[2] == curwater->origin[2])
				return false;
		}
	}
	return true;
}

void R_UpdateReflect(void)
{
	if (r_refdef->onlyClientDraws)
		return;

	reflect = true;

	memcpy(&water_refparam, r_params, sizeof(ref_params_t));
	for (int i = 0; i < numwaters; i++)
	{
		curwater = &water[i];
		if(curwater->draw)
		{
			R_SetupRefract();
			R_ClearSkyBox();
			R_DrawWorld();
			R_FinishRefract();
			R_Clear();
			if(R_ShouldReflect(i))
			{

				R_SetupReflect();
				R_ClearSkyBox();
				R_DrawWorld();
				//R_DrawEntitiesOnList();
				//gExportfuncs.HUD_DrawNormalTriangles();
				//R_DrawTEntitiesOnList(r_params->health);
				R_FinishReflect();
				R_Clear();
			}
		}
	}

	GLenum error = qglGetError();

	if (error)
		assert(qglGetError() == GL_NO_ERROR);

	reflect = false;
}

void R_LoadReflectMatrix(int i)
{
	/*qglMatrixMode(GL_TEXTURE);
	qglLoadIdentity();

	qglTranslatef(0.5f, 0.5f, 0.0f);
	qglScalef(0.5f * (reflect_texture_width / reflect_texture_minsize), 0.5f * (reflect_texture_height / reflect_texture_minsize), 1.0f);

	MYgluPerspective(yfov, screenaspect, 4, 4096);

	qglRotatef(-90, 1, 0, 0);
	qglRotatef(90, 0, 0, 1);
	qglRotatef(r_refdef->viewangles[2], 1, 0, 0);
	qglRotatef(r_refdef->viewangles[0], 0, 1, 0);
	qglRotatef(-r_refdef->viewangles[1], 0, 0, 1);
	qglTranslatef(-r_refdef->vieworg[0], -r_refdef->vieworg[1], -((2 * curwater->vecs[2]) - r_refdef->vieworg[2]));
	qglTranslatef(0.0f, 0.0f, 0.0f);
	qglMatrixMode(GL_MODELVIEW);*/
}

void R_ClearReflectMatrix(void)
{
	/*qglMatrixMode(GL_TEXTURE);
	qglLoadIdentity();
	qglMatrixMode(GL_MODELVIEW);*/
}