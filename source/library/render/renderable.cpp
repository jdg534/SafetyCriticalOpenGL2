#include "renderable.h"

renderable::~renderable()
{

}

gl::GLuint renderable::get_vertex_buffer_id() const
{
	return m_vertex_buffer_id;
}

gl::GLuint renderable::get_index_buffer_id() const
{
	return m_index_buffer_id;
}

gl::GLuint renderable::get_start_in_index_buffer() const
{
	return m_start_in_index_buffer;
}

gl::GLuint renderable::get_index_count() const
{
	return m_index_count;
}

const std::weak_ptr<renderable> renderable::get_parent() const
{
	return m_parent;
}

bool renderable::is_parent_set() const
{
	return !m_parent.expired();
}

const glm::mat4x4& renderable::get_transform() const
{
	return m_transform;
}

glm::mat4x4 renderable::get_net_transform() const
{
	const glm::mat4x4 parent_transform = is_parent_set()
		? get_parent().lock()->get_net_transform()
		: glm::mat4x4(); // identity matrix?
	return parent_transform * m_transform;
}

void renderable::set_vertex_buffer_id(gl::GLuint vertex_buffer_id)
{
	m_vertex_buffer_id = vertex_buffer_id;
}

void renderable::set_index_buffer_id(gl::GLuint index_buffer_id)
{
	m_index_buffer_id = index_buffer_id;
}

void renderable::set_start_in_index_buffer(gl::GLuint start_index_buffer)
{
	m_start_in_index_buffer = start_index_buffer;
}

void renderable::set_index_count(gl::GLuint index_count)
{
	m_index_count = index_count;
}

void renderable::set_parent(std::weak_ptr<renderable> parent)
{
	m_parent = parent;
}

void renderable::clear_parent()
{
	m_parent.reset();
}

void renderable::set_transform(const glm::mat4x4& transform)
{
	m_transform = transform;
}

void draw()
{
	// make this pure virtual?
}
