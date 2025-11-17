#ifndef _STATIC_MODEL_3D_H_
#define _STATIC_MODEL_3D_H_

#include <memory>

#include "renderable_3d.h"
#include "../../assets/3d/model.h"

// note that this doesn't own any 

class static_model : public renderable_3d
{

public:

	static_model() = delete;
	static_model(std::weak_ptr<const model> model);
	virtual ~static_model();

	void initialise() override;
	void shutdown() override;
	void draw() override;

private:


	std::weak_ptr<const model> m_model;
};

#endif // _STATIC_MODEL_3D_H_
