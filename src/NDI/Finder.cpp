#include <zuazo/NDI/Finder.h>

#include <cstddef>
#include "../Processing.NDI/Processing.NDI.Lib.h"

#include <zuazo/Modules/NDI.h>

namespace Zuazo::NDI {

static_assert(sizeof(Finder) == sizeof(NDIlib_find_instance_t), "Sizes do not match");
static_assert(alignof(Finder) == alignof(NDIlib_find_instance_t), "Alignments do not match");

static NDIlib_find_instance_t createFindInstance(	bool showLocalSources,
													const char* groups,
													const char* extraIp )
{
	const auto& ndi = Modules::NDI::get().getNDI();

	const NDIlib_find_create_t createInfo(
		showLocalSources,
		groups,
		extraIp
	);
	return ndi.find_create_v2(&createInfo);
}

static void destroyFindInstance(NDIlib_find_instance_t instance) {
	const auto& ndi = Modules::NDI::get().getNDI();
	ndi.find_destroy(instance);
}



Finder::Finder(void* ptr) noexcept
	: m_impl(ptr)
{
}

Finder::Finder(	bool showLocalSources,
				const char* groups,
				const char* extraIps )
	: Finder(createFindInstance(showLocalSources, groups, extraIps))
{
}

Finder::Finder(Finder&& other) noexcept 
	: Finder(other.m_impl)
{
	other.m_impl = nullptr;
}

Finder::~Finder() {
	if(m_impl) {
		destroyFindInstance(m_impl);
	}
}

Finder&	Finder::operator=(Finder&& other) noexcept {
	m_impl = other.m_impl;
	other.m_impl = nullptr;
	return *this;
}



bool Finder::waitForSources(uint32_t timeo) const noexcept {
	const auto& ndi = Modules::NDI::get().getNDI();
	return ndi.find_wait_for_sources(m_impl, timeo);
}

Utils::BufferView<const Source>	Finder::getCurrentSources() const noexcept {
	const auto& ndi = Modules::NDI::get().getNDI();
	uint32_t count;
	const auto sources = ndi.find_get_current_sources(m_impl, &count);
	return Utils::BufferView<const Source>(
		reinterpret_cast<const Source*>(sources),
		count
	);
}

}