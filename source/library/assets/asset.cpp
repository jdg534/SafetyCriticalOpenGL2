#include "asset.h"

#include <string>
#include <string_view>

asset::asset(const std::string& name)
	: m_name(name)
{

}

asset::~asset()
{

}

const std::string& asset::get_name() const
{
	return m_name;
}