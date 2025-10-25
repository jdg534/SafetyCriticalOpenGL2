#ifndef _RENDERABLE_2D_H_
#define _RENDERABLE_2D_H_

#include "../vertex_types.h"
#include "../renderable.h"

/* to cover:
* 1. quad DONE!
* 2. text_block (Array of quads texture from font atlas)
*/

class renderable_2d : public renderable
{
public:

	renderable_2d() = delete;
	explicit renderable_2d(const glm::vec4& tint);
	virtual ~renderable_2d();

	glm::vec4 get_tint() const;
	void set_tint(const glm::vec4& tint);

private:

	glm::vec4 m_tint;
};

#endif // _RENDERABLE_2D_H_