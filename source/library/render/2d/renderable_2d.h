#ifndef _RENDERABLE_2D_H_
#define _RENDERABLE_2D_H_

#include "../vertex_types.h"
#include "../renderable.h"

class renderable_2d : public renderable
{
public:

	renderable_2d() = delete;
	renderable_2d(const renderable_2d& other) = delete;
	renderable_2d(renderable_2d&& to_move) = delete;
	explicit renderable_2d(const glm::vec4& tint);
	
	~renderable_2d() override = default;

	renderable_2d& operator=(const renderable_2d&) = delete;
	renderable_2d& operator=(renderable_2d&&) = delete;

	glm::vec4 get_tint() const;
	void set_tint(const glm::vec4& tint);

private:

	glm::vec4 m_tint;
};

#endif // _RENDERABLE_2D_H_