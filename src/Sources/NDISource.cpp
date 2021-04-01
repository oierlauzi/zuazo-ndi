#include <zuazo/Sources/NDI.h>

namespace Zuazo::Sources {

NDI::Source::Source(std::string name, std::string url)
	: m_name(std::move(name))
	, m_url(std::move(url))
{
}

NDI::Source::Source(const Zuazo::NDI::Source& src)
	: Source(src.getName(), src.getURL())
{
}


NDI::Source::operator Zuazo::NDI::Source() const noexcept {
	return Zuazo::NDI::Source(getName().c_str(), getURL().c_str());
}
	

void NDI::Source::setName(std::string name) {
	m_name = std::move(name);
}

const std::string& NDI::Source::getName() const noexcept {
	return m_name;
}


void NDI::Source::setURL(std::string url) {
	m_url = std::move(url);
}

const std::string& NDI::Source::getURL() const noexcept {
	return m_url;
}
	
}