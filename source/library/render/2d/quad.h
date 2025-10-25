#ifndef _QUAD_H_
#define _QUAD_H_

#include "../vertex_types.h"
#include "renderable_2d.h"
#include "../../assets/texture.h"

#include <glm/glm.hpp>

class quad : public renderable_2d
{
public:

	quad() = delete;
	explicit quad(const std::weak_ptr<texture>& texture, const glm::vec2& position, const glm::vec2& size);
	virtual ~quad();

	void initialise() override;
	void shutdown() override;
	void draw() override;

	glm::vec4 get_tint() const;
	void set_tint(const glm::vec4& tint);

private:

	glm::vec4 m_tint;
	std::weak_ptr<texture> m_texture;
};

#endif // _QUAD_H_