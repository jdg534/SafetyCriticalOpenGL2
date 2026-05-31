#ifndef _STATIC_MODEL_H_
#define _STATIC_MODEL_H_

#include <memory>

#include "renderable_3d.h"
#include "../../assets/3d/model.h"

// note that this doesn't own any data.

class static_model : public renderable_3d
{

public:

	static_model() = delete;
	static_model(const static_model& other) = delete;
	static_model(static_model&& to_move) = delete;
	explicit static_model(std::weak_ptr<const model> model);
	~static_model() override = default;

	static_model& operator=(const static_model&) = delete;
	static_model& operator=(static_model&&) = delete;


	void initialise() override;
	void shutdown() override;
	void draw() override;

private:

	std::weak_ptr<const model> m_model;
};

#endif // _STATIC_MODEL_H_
