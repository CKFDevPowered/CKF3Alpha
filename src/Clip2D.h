#ifndef CLIP2D_H
#define CLIP2D_H

namespace vgui
{
	struct Vertex_t;
}

void EnableScissor(bool enable);
void SetScissorRect(int left, int top, int right, int bottom);
void GetScissorRect(int &left, int &top, int &right, int &bottom, bool &enabled);
bool ClipLine(const vgui::Vertex_t *pInVerts, vgui::Vertex_t *pOutVerts);
bool ClipRect(const vgui::Vertex_t &inUL, const vgui::Vertex_t &inLR, vgui::Vertex_t *pOutUL, vgui::Vertex_t *pOutLR);
int ClipPolygon(int iCount, vgui::Vertex_t *pVerts, int iTranslateX, int iTranslateY, vgui::Vertex_t ***pppOutVertex);

#endif