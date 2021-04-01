#pragma once

#include <zuazo/Instance.h>

#include <memory>

struct NDIlib_v4;

namespace Zuazo::Modules {

class NDI final
	: public Instance::Module
{
public:
	~NDI();

	static constexpr std::string_view name = "NDI";
	static constexpr Version version = Version(0, 1, 0);

	static const NDI& 					get();

	const NDIlib_v4&					getNDI() const noexcept;

private:
	class DynamicLoad;
	std::unique_ptr<DynamicLoad>		m_dynamicLoad;
	const NDIlib_v4&					m_ndi;

	NDI();
	NDI(const NDI& other) = delete;

	NDI& 								operator=(const NDI& other) = delete;

	static std::unique_ptr<NDI> 		s_singleton;
};

}