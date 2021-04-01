#pragma once

struct NDIlib_source_t;

namespace Zuazo::NDI {

class Source {
public:
	Source(const char* name, const char* url) noexcept;
	Source(const Source& other) = default;
	~Source() = default;

	Source&			operator=(const Source& other) = default;

	operator NDIlib_source_t&() noexcept;
	operator const NDIlib_source_t&() const noexcept;

	void			setName(const char* name) noexcept;
	const char*		getName() const noexcept;

	void			setURL(const char* url) noexcept;
	const char*		getURL() const noexcept;

private:
	const char*		m_name;
	const char*		m_url;
};

}
