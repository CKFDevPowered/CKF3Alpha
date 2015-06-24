#ifndef VGUI_VECTOR_H
#define VGUI_VECTOR_H

namespace vgui
{
#include <mathlib/vector2d.h>
}

namespace vgui
{
struct Vertex_t
{
	Vertex_t(void) {}

	Vertex_t(const Vector2D &pos, const Vector2D &coord = Vector2D(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}

	void Init(const Vector2D &pos, const Vector2D &coord = Vector2D(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}

	Vector2D m_Position;
	Vector2D m_TexCoord;
};
}

#endif