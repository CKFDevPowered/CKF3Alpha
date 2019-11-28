#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include <math.h>

//timer
static xywh_t g_xywhTimerPanel;
static xywh_t g_xywhTimerPanelAdditional;
static xywh_t g_xywhTimerClock;
static xy_t g_xyTimerFont;
static xy_t g_xyTimerFontAdditional;

//control points
static xy_t g_xyControlPoint;
static wh_t g_whControlPoint;

static vgui::HFont g_hFontTimer;
static vgui::HFont g_hFontTimerAdditional;
static vgui::HFont g_hFontCapturePlayers;

static int g_texControlPoint[3];
static int g_texControlPointLock[3];
static int g_texControlPointProgressBar[2];
static int g_texControlPointBright;
static int g_texControlPointDefend;
static int g_texControlPointMan;
static int g_texControlPointFans;

static color24 g_ubColor[] = {{235,225,200}, {160,50,50}, {154,205,255}, {49,44,41}};

void DrawHudMask(int col, int x, int y, int w, int h);

struct cp_bar_program_s
{
	int program;
	int base;
	int mask;
} cp_bar_program;

const char *cp_bar_program_vertex = 
"void main(void)\n"
"{\n"
"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"	gl_TexCoord[1] = gl_MultiTexCoord1;\n"
"	gl_Position = ftransform();\n"
"}";

const char *cp_bar_program_fragment = 
"uniform sampler2D base;\n"
"uniform sampler2D mask;\n"
"void main(void)\n"
"{\n"
"	vec4 vBaseColor = texture2D(base, gl_TexCoord[0].xy);\n"
"	vec4 vMaskColor = texture2D(mask, gl_TexCoord[1].xy);\n"
"	vBaseColor.a = vMaskColor.a;\n"
"	gl_FragColor = vBaseColor;\n"
"}";

struct cp_fans_program_s
{
	int program;
	int fade;
} cp_fans_program;

const char *cp_fans_program_vertex = 
"void main(void)\n"
"{\n"
"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"	gl_FrontColor = gl_Color;"
"	gl_Position = ftransform();\n"
"}";

const char *cp_fans_program_fragment = 
"uniform vec2 fade;\n"
"void main(void)\n"
"{\n"
"	gl_FragColor = gl_Color;\n"
"	if(gl_TexCoord[0].y > fade.y)\n"
"		gl_FragColor.a *= (1.0 - gl_TexCoord[0].y) / fade.x;\n"
"}";

struct cp_rings_program_s
{
	int program;
} cp_rings_program;

const char *cp_rings_program_vertex = 
"void main(void)\n"
"{\n"
"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"	gl_FrontColor = gl_Color;"
"	gl_Position = ftransform();\n"
"}";

const char *cp_rings_program_fragment = 
"void main(void)\n"
"{\n"
"	gl_FragColor = gl_Color;\n"

"	if(gl_TexCoord[0].x > 0.94)\n"
"		gl_FragColor.a *= (1.0 - gl_TexCoord[0].x) / 0.06;\n"
"	else if(gl_TexCoord[0].x < 0.06)\n"
"		gl_FragColor.a *= gl_TexCoord[0].x / 0.06;\n"
"	if(gl_TexCoord[0].y > 0.94)\n"
"		gl_FragColor.a *= (1.0 - gl_TexCoord[0].y) / 0.06;\n"
"	else if(gl_TexCoord[0].y < 0.06)\n"
"		gl_FragColor.a *= gl_TexCoord[0].y / 0.06;\n"

"	vec3 vMaxColor = gl_Color.rgb + vec3(0.2);"

"	if(gl_TexCoord[0].x > 0.9)\n"
"		gl_FragColor.rgb += 0.4 * (gl_TexCoord[0].x - 0.9) / 0.1;\n"
"	else if(gl_TexCoord[0].x < 0.1)\n"
"		gl_FragColor.rgb += 0.4 * (0.1 - gl_TexCoord[0].x) / 0.1;\n"
"	if(gl_TexCoord[0].y > 0.9)\n"
"		gl_FragColor.rgb += 0.4 * (gl_TexCoord[0].y - 0.9) / 0.1;\n"
"	else if(gl_TexCoord[0].y < 0.1)\n"
"		gl_FragColor.rgb += 0.4 * (0.1 - gl_TexCoord[0].y) / 0.1;\n"

"	if(gl_FragColor.r > vMaxColor.r) gl_FragColor.r = vMaxColor.r;\n"
"	if(gl_FragColor.g > vMaxColor.g) gl_FragColor.g = vMaxColor.g;\n"
"	if(gl_FragColor.b > vMaxColor.b) gl_FragColor.b = vMaxColor.b;\n"
"}";

void HudObject_Init(void)
{

}

int HudObject_VidInit(void)
{
	g_xywhTimerPanel.w = ScreenHeight * (85 / 600.0);
	g_xywhTimerPanel.h = ScreenHeight * (31 / 600.0);
	g_xywhTimerPanel.x = (ScreenWidth - g_xywhTimerPanel.w) / 2;
	g_xywhTimerPanel.y = ScreenHeight * (16 / 600.0);

	g_xywhTimerPanelAdditional.x = g_xywhTimerPanel.x;
	g_xywhTimerPanelAdditional.y = g_xywhTimerPanel.y + ScreenHeight * (24 / 600.0);
	g_xywhTimerPanelAdditional.w = g_xywhTimerPanel.w;
	g_xywhTimerPanelAdditional.h = ScreenHeight * (20 / 600.0);

	g_xyTimerFont.x = g_xywhTimerPanel.x + ScreenHeight * (6 / 600.0);
	g_xyTimerFont.y = g_xywhTimerPanel.y + ScreenHeight * (5 / 600.0);

	g_xyTimerFontAdditional.x = g_xywhTimerPanelAdditional.x + g_xywhTimerPanelAdditional.w / 2;
	g_xyTimerFontAdditional.y = g_xywhTimerPanelAdditional.y + ScreenHeight * (14 / 600.0);

	g_xywhTimerClock.x = g_xywhTimerPanel.x + ScreenHeight * (70 / 600.0);
	g_xywhTimerClock.y = g_xywhTimerPanel.y + ScreenHeight * (16 / 600.0);
	g_xywhTimerClock.w = ScreenHeight * (10 / 600.0);

	g_whControlPoint.w = ScreenHeight * (51 / 600.0);
	g_whControlPoint.h = g_whControlPoint.w;

	g_xyControlPoint.x = (ScreenWidth - g_whControlPoint.w) / 2;
	g_xyControlPoint.y = ScreenHeight * (520 / 600.0);

	g_hFontTimer = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontTimer, "TF2", ScreenHeight * (22 / 600.0), 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);

	g_hFontTimerAdditional = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontTimerAdditional, "TF2 Secondary", ScreenHeight * (12 / 600.0), 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM, 0x0, 0xFFFF);
	
	g_hFontCapturePlayers = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontCapturePlayers, "TF2", ScreenHeight * (18 / 600.0), 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);

	g_texControlPoint[0] = Surface_LoadTGA("resource\\tga\\cp_neu");
	g_texControlPoint[1] = Surface_LoadTGA("resource\\tga\\cp_red");
	g_texControlPoint[2] = Surface_LoadTGA("resource\\tga\\cp_blu");
	g_texControlPointLock[0] = Surface_LoadTGA("resource\\tga\\cp_neu_l");
	g_texControlPointLock[1] = Surface_LoadTGA("resource\\tga\\cp_red_l");
	g_texControlPointLock[2] = Surface_LoadTGA("resource\\tga\\cp_blu_l");
	g_texControlPointProgressBar[0] = Surface_LoadTGA("resource\\tga\\cp_red_a");
	g_texControlPointProgressBar[1] = Surface_LoadTGA("resource\\tga\\cp_blu_a");
	g_texControlPointBright = Surface_LoadTGA("resource\\tga\\cp_bright");
	g_texControlPointDefend = Surface_LoadTGA("resource\\tga\\cp_defend");
	g_texControlPointMan = Surface_LoadTGA("resource\\tga\\cp_man");
	g_texControlPointFans = Surface_LoadTGA("resource\\tga\\cp_fans");

	cp_bar_program.program = gRefExports.ShaderAPI.R_CompileShader(cp_bar_program_vertex, cp_bar_program_fragment, "cp_bar_program.vsh", "cp_bar_program.fsh");
	if(cp_bar_program.program)
	{
		cp_bar_program.base = gRefExports.ShaderAPI.GL_GetUniformLoc(cp_bar_program.program, "base");
		cp_bar_program.mask = gRefExports.ShaderAPI.GL_GetUniformLoc(cp_bar_program.program, "mask");
	}
	cp_fans_program.program = gRefExports.ShaderAPI.R_CompileShader(cp_fans_program_vertex, cp_fans_program_fragment, "cp_fans_program.vsh", "cp_fans_program.fsh");
	if(cp_fans_program.program)
	{
		cp_fans_program.fade = gRefExports.ShaderAPI.GL_GetUniformLoc(cp_fans_program.program, "fade");
	}
	cp_rings_program.program = gRefExports.ShaderAPI.R_CompileShader(cp_rings_program_vertex, cp_rings_program_fragment, "cp_rings_program.vsh", "cp_rings_program.fsh");
	return 1;
}

void DrawTriangleFans2(int centerX, int centerY, float radius, float start_radian, float end_radian, int step)
{
	if(step < 3 || end_radian < start_radian)
		return;

	float x, y, rad, radstep;

	radstep = (end_radian - start_radian) * (1.0f / step);
	rad = start_radian;

	qglBegin(GL_TRIANGLE_FAN);
	qglTexCoord2f(0, 0);
	qglVertex3f(centerX, centerY, 0);
	for(int i = 0; i <= step; ++i)
	{
		x = centerX + radius * cos(rad);
		y = centerY + radius * sin(rad);
		qglTexCoord2f(0, 1);
		qglVertex3f(x, y, 0);
		rad += radstep;
	}

	qglEnd();
}

void DrawTriangleRings2(int centerX, int centerY, float start_radius, float end_radius, float start_radian, float end_radian, int step)
{
	if(step < 3 || end_radian < start_radian || end_radius < start_radius)
		return;

	float s, x, y, rad, radstep, sstep;

	sstep = (1.0f / step);
	radstep = (end_radian - start_radian) * sstep;
	rad = start_radian;
	s = 0;

	qglBegin(GL_TRIANGLE_STRIP);	
	for(int i = 0; i <= step; ++i)
	{
		x = centerX + start_radius * cos(rad);
		y = centerY + start_radius * sin(rad);
		qglTexCoord2f(s, 0);
		qglVertex3f(x, y, 0);

		x = centerX + end_radius * cos(rad);
		y = centerY + end_radius * sin(rad);
		qglTexCoord2f(s, 1);
		qglVertex3f(x, y, 0);
		rad += radstep;
		s += sstep;
	}

	qglEnd();
}

static void ParseRoundTime(int time, int *mins, int *secs)
{
	if(time >= 0)
	{
		*mins = time / 60;
		*secs = time - (*mins) * 60;
	}
	else
	{
		*mins = 0;
		*secs = 0;
	}
}

void DrawTimer(void)
{
	wchar_t szText[32];
	int time, mins, secs, w, h;

	float frac;
	
	if(g_iLastRoundStatus != g_iRoundStatus && 
		((g_iLastRoundStatus == ROUND_NORMAL && g_iRoundStatus != ROUND_NORMAL) ||
		(g_iLastRoundStatus != ROUND_NORMAL && g_iRoundStatus == ROUND_NORMAL))
		)
		frac = max(min((g_flClientTime - g_flRoundStatusChangeTime) / 0.5f, 1), 0);
	else
		frac = 1;

	if(g_iRoundStatus != ROUND_NORMAL)
	{
		g_pSurface->DrawSetColor(50, 50, 50, 100);
		DrawHudMask(0, g_xywhTimerPanelAdditional.x, g_xywhTimerPanelAdditional.y, g_xywhTimerPanelAdditional.w, g_xywhTimerPanelAdditional.h * frac );

		if(frac == 1)
		{
			switch(g_iRoundStatus)
			{
			case ROUND_END:
				wcscpy(szText, g_wszRoundStatus[1]);
				break;
			case ROUND_OVERTIME:
				wcscpy(szText, g_wszRoundStatus[2]);
				break;
			case ROUND_WAIT:
				wcscpy(szText, g_wszRoundStatus[3]);
				break;
			default:
				wcscpy(szText, g_wszRoundStatus[0]);
				break;
			}
			g_pSurface->DrawSetTextFont(g_hFontTimerAdditional);
			g_pSurface->DrawSetTextColor(g_ubColor[0].r, g_ubColor[0].g, g_ubColor[0].b, 255);
			g_pSurface->GetTextSize(g_hFontTimerAdditional, szText, w, h);
			g_pSurface->DrawSetTextPos(g_xyTimerFontAdditional.x - w / 2, g_xyTimerFontAdditional.y - h / 2);
			g_pSurface->DrawPrintText(szText, wcslen(szText));
			g_pSurface->DrawFlushText();
		}
	}
	else
	{
		if(frac != 1)
		{
			g_pSurface->DrawSetColor(50, 50, 50, 100);
			DrawHudMask(0, g_xywhTimerPanelAdditional.x, g_xywhTimerPanelAdditional.y, g_xywhTimerPanelAdditional.w, g_xywhTimerPanelAdditional.h * (1 - frac) );
		}
	}

	g_pSurface->DrawSetColor(255, 255, 255, 245);
	DrawHudMask(g_iTeam, g_xywhTimerPanel.x, g_xywhTimerPanel.y, g_xywhTimerPanel.w, g_xywhTimerPanel.h);

	time = max(min(g_flRoundEndTime - g_flClientTime, g_iMaxRoundTime), 0);

 	ParseRoundTime(time, &mins, &secs);

	wsprintfW(szText, L"%02d:%02d", mins, secs);
	g_pSurface->DrawSetTextFont(g_hFontTimer);
	g_pSurface->DrawSetTextColor(g_ubColor[0].r, g_ubColor[0].g, g_ubColor[0].b, 255);
	g_pSurface->DrawSetTextPos(g_xyTimerFont.x, g_xyTimerFont.y);
	g_pSurface->DrawPrintText(szText, wcslen(szText));
	g_pSurface->DrawFlushText();

	if(g_iMaxRoundTime > 0)
	{
		if(g_RefSupportExt & r_ext_shader)
		{
			gRefExports.ShaderAPI.GL_UseProgram(cp_fans_program.program);
			gRefExports.ShaderAPI.GL_Uniform2f(cp_fans_program.fade, 0.05, 0.95);
		}

		qglEnable(GL_BLEND);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		qglDisable(GL_TEXTURE_2D);
		
		qglColor4ub(g_ubColor[3].r, g_ubColor[3].g, g_ubColor[3].b, 255);
		DrawTriangleFans2(g_xywhTimerClock.x, g_xywhTimerClock.y, g_xywhTimerClock.w, -0.5 * M_PI, 2 * M_PI - 0.5 * M_PI, 60);

		qglColor4ub(g_ubColor[0].r, g_ubColor[0].g, g_ubColor[0].b, 255);
		DrawTriangleFans2(g_xywhTimerClock.x, g_xywhTimerClock.y, g_xywhTimerClock.w, -0.5 * M_PI, (float)time / g_iMaxRoundTime * 2.0 * M_PI - 0.5 * M_PI, 60);
		
		qglEnable(GL_TEXTURE_2D);

		if(g_RefSupportExt & r_ext_shader)
			gRefExports.ShaderAPI.GL_EndProgram();
	}
}

void DrawControlPoint_Bar(controlpoint_t *cp, int x, int y, int w, int h, float progress)
{
	gRefExports.ShaderAPI.GL_UseProgram(cp_bar_program.program);
	gRefExports.ShaderAPI.GL_Uniform1i(cp_bar_program.base, 0);
	gRefExports.ShaderAPI.GL_Uniform1i(cp_bar_program.mask, 1);

	int iCapTeam = max(min(cp->iCapTeam-1, 2), 0);

	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gRefExports.RefAPI.GL_DisableMultitexture();
	gRefExports.RefAPI.GL_Bind(g_texControlPointProgressBar[iCapTeam]);
	gRefExports.RefAPI.GL_EnableMultitexture();
	gRefExports.RefAPI.GL_Bind(g_texControlPoint[0]);

	float s = (progress * 105 + (1 - progress) * 280) / 512.0f;

	qglBegin(GL_QUADS);

	gRefExports.ShaderAPI.GL_MultiTexCoord2f(GL_TEXTURE0, s, 0);
	gRefExports.ShaderAPI.GL_MultiTexCoord2f(GL_TEXTURE1, 0, 0);
	qglVertex3f(x, y, 0);

	gRefExports.ShaderAPI.GL_MultiTexCoord2f(GL_TEXTURE0, s, 1);
	gRefExports.ShaderAPI.GL_MultiTexCoord2f(GL_TEXTURE1, 0, 1);
	qglVertex3f(x, y+h, 0);

	gRefExports.ShaderAPI.GL_MultiTexCoord2f(GL_TEXTURE0, s + 128 / 512.0f, 1);	
	gRefExports.ShaderAPI.GL_MultiTexCoord2f(GL_TEXTURE1, 1, 1);
	qglVertex3f(x+w, y+h, 0);

	gRefExports.ShaderAPI.GL_MultiTexCoord2f(GL_TEXTURE0, s + 128 / 512.0f, 0);
	gRefExports.ShaderAPI.GL_MultiTexCoord2f(GL_TEXTURE1, 1, 0);
	qglVertex3f(x+w, y, 0);

	qglEnd();

	gRefExports.RefAPI.GL_Bind(0);
	gRefExports.RefAPI.GL_DisableMultitexture();
	gRefExports.ShaderAPI.GL_EndProgram();
}

void DrawControlPoint_Rings(controlpoint_t *cp, int x, int y, float progress, BOOL bReversed)
{
	int x3, y3;

	float radian_start, radian_end;

	float radius_start = ScreenHeight * 7 / 600;
	float radius_end = ScreenHeight * 22 / 600;

	if(g_RefSupportExt & r_ext_shader)
		gRefExports.ShaderAPI.GL_UseProgram(cp_rings_program.program);

	if(bReversed)
	{
		if(progress > 0)
		{
			//top
			x3 = x;
			y3 = y - ScreenHeight * 7 / 600;
			radian_end = M_PI * 1.5;
			radian_start = max(M_PI * 1.25, radian_end - M_PI * 2 * progress);
			DrawTriangleRings2(x3, y3, radius_start, radius_end, radian_start, radian_end, 30);
		}

		if(progress > 1.0f / 8)
		{
			//left
			x3 = x - ScreenHeight * 7 / 600;
			y3 = y;
			radian_end = M_PI * 1.25;
			radian_start = max(M_PI * 0.75, radian_end - M_PI * 2 * (progress - 1.0f / 8));
			DrawTriangleRings2(x3, y3, radius_start, radius_end, radian_start, radian_end, 30);
		}

		if(progress > 3.0f / 8)
		{
			//bottom
			x3 = x ;
			y3 = y + ScreenHeight * 7 / 600;
			radian_end = M_PI * 0.75;
			radian_start = max(M_PI * 0.25, radian_end - M_PI * 2 * (progress - 3.0f / 8));
			DrawTriangleRings2(x3, y3, radius_start, radius_end, radian_start, radian_end, 30);
		}

		if(progress > 5.0f / 8)
		{
			//right
			x3 = x + ScreenHeight * 7 / 600;
			y3 = y;
			radian_end = M_PI * 0.25;
			radian_start = max(-M_PI * 0.25, radian_end - M_PI * 2 * (progress - 5.0f / 8));
			DrawTriangleRings2(x3, y3, radius_start, radius_end, radian_start, radian_end, 30);
		}

		if(progress > 7.0f / 8)
		{
			//top again
			x3 = x;
			y3 = y - ScreenHeight * 7 / 600;
			radian_end = -M_PI * 0.25;
			radian_start = max(-M_PI * 0.5, radian_end - M_PI * 2 * (progress - 7.0f / 8));
			DrawTriangleRings2(x3, y3, radius_start, radius_end, radian_start, radian_end, 30);
		}
	}
	else
	{
		if(progress > 0)
		{
			//top
			x3 = x;
			y3 = y - ScreenHeight * 7 / 600;
			radian_start = -M_PI * 0.5;
			radian_end = min(-M_PI * 0.25, radian_start + M_PI * 2 * progress);
			DrawTriangleRings2(x3, y3, radius_start, radius_end, radian_start, radian_end, 30);
		}

		if(progress > 1.0f / 8)
		{
			//right
			x3 = x + ScreenHeight * 7 / 600;
			y3 = y;
			radian_start = -M_PI * 0.25;
			radian_end = min(M_PI * 0.25, radian_start + M_PI * 2 * (progress - 1.0f / 8));
			DrawTriangleRings2(x3, y3, radius_start, radius_end, radian_start, radian_end, 30);
		}

		if(progress > 3.0f / 8)
		{
			//bottom
			x3 = x ;
			y3 = y + ScreenHeight * 7 / 600;
			radian_start = M_PI * 0.25;
			radian_end = min(M_PI * 0.75, radian_start + M_PI * 2 * (progress - 3.0f / 8));
			DrawTriangleRings2(x3, y3, radius_start, radius_end, radian_start, radian_end, 30);
		}

		if(progress > 5.0f / 8)
		{
			//left
			x3 = x - ScreenHeight * 7 / 600;
			y3 = y;
			radian_start = M_PI * 0.75;
			radian_end = min(M_PI * 1.25, radian_start + M_PI * 2 * (progress - 5.0f / 8));
			DrawTriangleRings2(x3, y3, radius_start, radius_end, radian_start, radian_end, 30);
		}

		if(progress > 7.0f / 8)
		{
			//top again
			x3 = x;
			y3 = y - ScreenHeight * 7 / 600;
			radian_start = M_PI * 1.25;
			radian_end = min(M_PI * 1.5, radian_start + M_PI * 2 * (progress - 7.0f / 8));
			DrawTriangleRings2(x3, y3, radius_start, radius_end, radian_start, radian_end, 30);
		}
	}

	if(g_RefSupportExt & r_ext_shader)
		gRefExports.ShaderAPI.GL_EndProgram();
}

void DrawControlPoint_Fans(controlpoint_t *cp, int x, int y, int w, int h, float progress)
{
	int x2, y2;

	x2 = x + w/2;
	y2 = y - ScreenHeight * (48 / 600.0);

	if(g_RefSupportExt & r_ext_shader)
	{
		gRefExports.ShaderAPI.GL_UseProgram(cp_fans_program.program);
		gRefExports.ShaderAPI.GL_Uniform2f(cp_fans_program.fade, 0.04, 0.96);
	}

	qglDisable(GL_TEXTURE_2D);
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	qglColor4ub(90, 90, 90, 120);

	float radius = ScreenHeight * 34 / 600;
	float radstart = M_PI * 0.75;
	float radend = M_PI * 2.25;
	DrawTriangleFans2(x2, y2, radius, radstart, radend, 60);

	qglBegin(GL_QUADS);

	qglTexCoord2f(0, 0);
	qglVertex3f(x2, y2, 0);

	qglTexCoord2f(0, 1);
	qglVertex3f(x2 + radius * cos(radstart), y2 + radius * sin(radstart), 0);

	qglTexCoord2f(0, 1);
	qglVertex3f(x2, y, 0);

	qglTexCoord2f(0, 1);
	qglVertex3f(x2 + radius * cos(radend), y2 + radius * sin(radend), 0);
	qglEnd();

	if(g_RefSupportExt & r_ext_shader)
		gRefExports.ShaderAPI.GL_EndProgram();

	//bg
	if(cp->iTeam == 1)
		qglColor4ub(174, 54, 55, 255);
	else if(cp->iTeam == 2)
		qglColor4ub(82, 113, 132, 255);
	else
		qglColor4ub(100, 100, 100, 255);
	DrawControlPoint_Rings(cp, x2, y2, 1, false);

	//new
	if(cp->iCapTeam == 1)
	{
		qglColor4ub(174, 54, 55, 255);
		DrawControlPoint_Rings(cp, x2, y2, progress, false);
	}
	else
	{
		qglColor4ub(82, 113, 132, 255);
		DrawControlPoint_Rings(cp, x2, y2, progress, true);
	}	

	qglEnable(GL_TEXTURE_2D);
}

void DrawControlPoints(void)
{
	int x, y, w, h;
	int brightAlpha;
	for(int i = 0; i < g_ControlPoints.Count(); ++i)
	{
		controlpoint_t *cp = &g_ControlPoints[i];

		if (cp->bDisabled)
			continue;

		g_pSurface->DrawSetColor(255,255,255,255);

		if (cp->bLocked)
			g_pSurface->DrawSetTexture(g_texControlPointLock[cp->iTeam]);
		else
			g_pSurface->DrawSetTexture(g_texControlPoint[cp->iTeam]);

		x = g_xyControlPoint.x + cp->iHudPosition * (g_whControlPoint.w + (ScreenHeight * 15 / 600));
		y = g_xyControlPoint.y;
		w = g_whControlPoint.w;
		h = g_whControlPoint.h;
		g_pSurface->DrawTexturedRect(x, y, x+w, y+h);

		if(cp->iState == CP_CAPTURING)
		{
			brightAlpha = 100 + 50 * sin(g_flClientTime - cp->flMessageTime);
			g_pSurface->DrawSetColor(255,255,255, brightAlpha);
			g_pSurface->DrawSetTexture(g_texControlPointBright);
			g_pSurface->DrawTexturedRect(x, y, x+w, y+h);

			g_pSurface->DrawSetColor(255,255,255,255);

			//progress bar
			if(cp->flProgress > 0 || cp->flCapRate != 0)
			{
				float progress = max(min(cp->flProgress + cp->flCapRate * (g_flClientTime - cp->flMessageTime), 100), 0) / 100.0f;
				DrawControlPoint_Bar(cp, x, y, w, h, progress);
				if(g_iCapPointIndex - 1 == i)
				{
					DrawControlPoint_Fans(cp, x, y, w, h, progress);
				}
			}
			//capture players num
			if(cp->iCapPlayers == 1)
			{
				g_pSurface->DrawSetTexture(g_texControlPointMan);
				g_pSurface->DrawTexturedRect(x, y, x+w, y+h);
			}
			else if(cp->iCapPlayers > 1)
			{
				g_pSurface->DrawSetTexture(g_texControlPointMan);
				g_pSurface->DrawTexturedRect(x - (ScreenHeight * 18 / 600), y, x+w, y+h);

				wchar_t szText[8];
				int tw, th;
				wsprintfW(szText, L"x%d", cp->iCapPlayers);
				g_pSurface->GetTextSize(g_hFontCapturePlayers, szText, tw, th);
				g_pSurface->DrawSetTextFont(g_hFontCapturePlayers);
				g_pSurface->DrawSetTextColor(g_ubColor[3].r, g_ubColor[3].g, g_ubColor[3].b, 255);
				g_pSurface->DrawSetTextPos(x + (ScreenHeight * 10 / 600) + (w-tw)/2, y + (h-th)/2);
				g_pSurface->DrawPrintText(szText, wcslen(szText));
				g_pSurface->DrawFlushText();
			}
		}
		else if(cp->iState == CP_BACK)
		{
			//progress bar
			if(cp->flProgress > 0 || cp->flCapRate != 0)
			{
				float progress = max(min(cp->flProgress -cp->flCapRate * (g_flClientTime - cp->flMessageTime), 100), 0) / 100.0f;
				DrawControlPoint_Bar(cp, x, y, w, h, progress);
				if(g_iCapPointIndex - 1 == i)
				{
					DrawControlPoint_Fans(cp, x, y, w, h, progress);
				}
			}
		}
		else if(cp->iState == CP_BLOCKED)
		{
			g_pSurface->DrawSetTexture(g_texControlPointDefend);
			g_pSurface->DrawTexturedRect(x, y, x+w, y+h);
		}
	}
}

int HudObject_Redraw(float flTime, int iIntermission)
{
	if(iIntermission)
		return 0;

	if((g_iHideHUD & HIDEHUD_TIMER))
		return 0;

	if(g_iTeam == 1 || g_iTeam == 2)
	{
		DrawTimer();
	}

	DrawControlPoints();

	return 1;
}