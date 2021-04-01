#pragma once

#include "Recv.h"
#include "VideoFrame.h"

#include <cstdint>

namespace Zuazo::NDI {

class FrameSync {
public:
	FrameSync(void* ptr) noexcept;
	explicit FrameSync(const Recv& source);
	FrameSync(const FrameSync& other) = delete;
	FrameSync(FrameSync&& other) noexcept;
	~FrameSync();

	FrameSync&						operator=(const FrameSync& other) = delete;
	FrameSync&						operator=(FrameSync&& other) noexcept;
	
	

	void							capture(VideoFrame& frame, VideoFrame::Format format) noexcept;
	void							free(VideoFrame& frame) noexcept;

private:
	void*							m_impl;

};

}