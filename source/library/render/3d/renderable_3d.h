#ifndef _RENDERABLE_3D_H_
#define _RENDERABLE_3D_H_

#include "../renderable.h"

class renderable_3d : public renderable
{

public:

	renderable_3d();
	renderable_3d(const renderable_3d& other) = delete;
	renderable_3d(renderable_3d&& to_move) = delete;
	~renderable_3d() override = default;

	renderable_3d& operator=(const renderable_3d&) = delete;
	renderable_3d& operator=(renderable_3d&&) = delete;

private:

};

#endif // _RENDERABLE_3D_H_