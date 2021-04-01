#include <zuazo/NDI/Source.h>

#include <cstddef>
#include "../Processing.NDI/Processing.NDI.Lib.h"

namespace Zuazo::NDI {

Source::Source(const char* name, const char* url) noexcept
	: m_name(name)
	, m_url(url)
{
	//These checks are done here as they involve private members
	static_assert(sizeof(*this) == sizeof(NDIlib_source_t), "Sizes must match");
	static_assert(alignof(Source) == alignof(NDIlib_source_t), "Alignments must match");
	static_assert(offsetof(Source, m_name) == offsetof(NDIlib_source_t, p_ndi_name), "Offset of name must match");
	static_assert(offsetof(Source, m_url) == offsetof(NDIlib_source_t, p_url_address), "Offset of url must match");
}


Source::operator NDIlib_source_t&() noexcept {
	return reinterpret_cast<NDIlib_source_t&>(*this);
}

Source::operator const NDIlib_source_t&() const noexcept {
	return reinterpret_cast<const NDIlib_source_t&>(*this);
}


void Source::setName(const char* name) noexcept {
	m_name = name;
}

const char* Source::getName() const noexcept {
	return m_name;
}


void Source::setURL(const char* url) noexcept {
	m_url = url;
}

const char* Source::getURL() const noexcept {
	return m_url;
}

}