#ifndef _RENDERABLE_H_
#define _RENDERABLE_H_

#include "include_opengl.h"

#include <glm/glm.hpp>
#include <memory>

// Note this DOES NOT own the buffers, child classes may own them 

// note this is used to sort the render list in the renderer
enum class renderable_type : uint8_t
{
	INVALID,
	TERRAIN,
	STATIC_GEOMETRY,
	RIGGED_GEOMETRY,
	_2D_GEOMETRY
};

class renderable
{
public:

	virtual ~renderable();

	virtual void initialise() = 0;
	virtual void shutdown() = 0;
	virtual void draw() = 0;

	renderable_type get_renderable_type() const;
	gl::GLuint get_vertex_buffer_id() const;
	gl::GLuint get_index_buffer_id() const;
	gl::GLuint get_start_in_index_buffer() const;
	gl::GLuint get_index_count() const;
	const std::weak_ptr<renderable> get_parent() const;
	bool is_parent_set() const;
	const glm::mat4x4& get_transform() const;
	glm::mat4x4 get_net_transform() const;

	void set_vertex_buffer_id(gl::GLuint vertex_buffer_id);
	void set_index_buffer_id(gl::GLuint index_buffer_id);
	void set_start_in_index_buffer(gl::GLuint start_index_buffer);
	void set_index_count(gl::GLuint index_count);
	void set_parent(std::weak_ptr<renderable> parent);
	void clear_parent();
	void set_transform(const glm::mat4x4& transform);

protected:

	void set_renderable_type(renderable_type renderable_type);

private:

	// look into the byte order and alignment later if needed.
	renderable_type m_renderable_type { renderable_type::INVALID };
	gl::GLuint m_vertex_buffer_id { 0 };
	gl::GLuint m_index_buffer_id { 0 };
	gl::GLuint m_start_in_index_buffer { 0 };
	gl::GLuint m_index_count { 0 };
	std::weak_ptr<renderable> m_parent;
	glm::mat4x4 m_transform;
};


#endif // _RENDERABLE_H_