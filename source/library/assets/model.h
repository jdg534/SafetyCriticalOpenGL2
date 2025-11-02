#ifndef _MODEL_H_
#define _MODEL_H_

#include <memory>
#include <string>
#include <string_view>


#include "asset.h"

class model : public asset
{
public:

	model() = delete;
	model(const std::string& name, std::weak_ptr<const asset_manager> asset_manager);
	virtual ~model();

	void initialise(std::string_view file_path) override;
	void shutdown() override;
	asset_type get_type() const override;

private:

};

#endif // _MODEL_H_
