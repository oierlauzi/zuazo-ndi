#pragma once

#include "Source.h"
#include "VideoFrame.h"

#include <cstdint>

namespace Zuazo::NDI {

class Recv {
public:
	enum class Bandwidth : int32_t {
		METADATA_ONLY	= -10,
		AUDIO_ONLY 		= 10,
		LOWEST 			= 0,
		HIGHEST 		= 100,
	};

	enum class ColorFormat : uint32_t {
		BGRX_BGRA,
		UYVY_BGRA,
		RGBX_RGBA,
		UYVY_RGBA,
		
		FASTEST			= 100,
		BEST,
	};
	
	enum class FrameType {
		NONE,
		VIDEO,
		AUDIO,
		METADATA,
		ERROR,

		STATUS_CHANGE 	= 100
	};

	Recv(void* ptr) noexcept;
	Recv(	const Source& source,
			ColorFormat format,
			Bandwidth bandwidth,
			bool allowVideoFields,
			const char* name );
	Recv(const Recv& other) = delete;
	Recv(Recv&& other) noexcept;
	~Recv();

	Recv&							operator=(const Recv& other) = delete;
	Recv&							operator=(Recv&& other) noexcept;
	
	
	void							connect(const Source& source) noexcept;	

	FrameType						capture(VideoFrame& frame, uint32_t timeo) const noexcept;
	void							free(VideoFrame& frame) const noexcept;

	bool							setTally(bool pgm, bool pvw) noexcept;

private:
	void*							m_impl;

};

}