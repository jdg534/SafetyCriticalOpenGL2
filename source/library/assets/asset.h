#ifndef _ASSET_H_
#define _ASSET_H_

#include <string>
#include <string_view>
#include <memory>

#include "asset_types.h"

#include "asset_manager.decl.h"

class asset
{
public:

	asset() = delete;
	asset(const std::string& name, std::weak_ptr<const asset_manager> asset_manager);
	virtual ~asset();

	virtual void initialise(std::string_view file_path) = 0;
	virtual void shutdown() = 0;
	virtual asset_type get_type() const = 0;
	
	std::string_view get_name() const;

protected:

	std::weak_ptr<const asset_manager> get_asset_manager() const;

private:
	std::string m_name;
	std::weak_ptr<const asset_manager> m_asset_manager;
};

#endif // _ASSET_H_
