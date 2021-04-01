#pragma once

#include "Source.h"

#include <zuazo/Utils/BufferView.h>

#include <cstdint>

namespace Zuazo::NDI {

class Finder {
public:
	Finder(void* ptr) noexcept;
	explicit Finder(bool showLocalSources = true,
					const char* groups = nullptr,
					const char* extraIps = nullptr);
	Finder(const Finder& other) = delete;
	Finder(Finder&& other) noexcept;
	~Finder();

	Finder&							operator=(const Finder& other) = delete;
	Finder&							operator=(Finder&& other) noexcept;

	bool							waitForSources(uint32_t timeo) const noexcept;
	Utils::BufferView<const Source>	getCurrentSources() const noexcept;

private:
	void*							m_impl;

};

}