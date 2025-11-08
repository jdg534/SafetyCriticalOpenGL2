#ifndef _STATIC_MODEL_3D_H_
#define _STATIC_MODEL_3D_H_

#include "renderable_3d.h"

// note that this doesn't own the buffers or vertex arrays

class static_model : public renderable_3d
{

public:

	void initialise() override;
	void shutdown() override;
	void draw() override;

private:

};

// This is a placeholder.
// This will be the base class for geomentry.
// Terrain will be another class (inheriting from this)

#endif // _STATIC_MODEL_3D_H_
