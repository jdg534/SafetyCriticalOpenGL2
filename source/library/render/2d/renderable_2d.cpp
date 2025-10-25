#include "renderable_2d.h"

renderable_2d::renderable_2d(const glm::vec4& tint)
	: renderable()
	, m_tint(tint)
{
	set_renderable_type(renderable_type::_2D_GEOMETRY);
}

renderable_2d::~renderable_2d()
{

}

glm::vec4 renderable_2d::get_tint() const
{
	return m_tint;
}

void renderable_2d::set_tint(const glm::vec4& tint)
{
	m_tint = tint;
}
