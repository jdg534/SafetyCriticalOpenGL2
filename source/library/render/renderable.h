#ifndef _RENDERABLE_H_
#define _RENDERABLE_H_

#include "include_opengl.h"

#include <glm/glm.hpp>
#include <memory>

// Note this DOES NOT own the buffers

class renderable
{
public:

	virtual ~renderable();

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

	virtual void draw();

private:

	gl::GLuint m_vertex_buffer_id { 0 };
	gl::GLuint m_index_buffer_id { 0 };
	gl::GLuint m_start_in_index_buffer { 0 };
	gl::GLuint m_index_count { 0 };
	std::weak_ptr<renderable> m_parent;
	glm::mat4x4 m_transform;
};


#endif // _RENDERABLE_H_