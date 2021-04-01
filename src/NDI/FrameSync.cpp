#include <zuazo/NDI/FrameSync.h>

#include <cstddef>
#include "../Processing.NDI/Processing.NDI.Lib.h"

#include <zuazo/Modules/NDI.h>

namespace Zuazo::NDI {

static_assert(sizeof(FrameSync) == sizeof(NDIlib_framesync_instance_t), "Sizes do not match");
static_assert(alignof(FrameSync) == alignof(NDIlib_framesync_instance_t), "Alignments do not match");

static NDIlib_framesync_instance_t createFrameSyncInstance(const Recv& recv)
{
	const auto& ndi = Modules::NDI::get().getNDI();
	return ndi.framesync_create(reinterpret_cast<const NDIlib_recv_instance_t&>(recv));
}

static void destroyFrameSyncInstance(NDIlib_framesync_instance_t instance) {
	const auto& ndi = Modules::NDI::get().getNDI();
	ndi.framesync_destroy(instance);
}



FrameSync::FrameSync(void* ptr) noexcept
	: m_impl(ptr)
{
}

FrameSync::FrameSync(const Recv& recv)
	: FrameSync(createFrameSyncInstance(recv))
{
}

FrameSync::FrameSync(FrameSync&& other) noexcept 
	: FrameSync(other.m_impl)
{
	other.m_impl = nullptr;
}

FrameSync::~FrameSync() {
	if(m_impl) {
		destroyFrameSyncInstance(m_impl);
	}
}

FrameSync& FrameSync::operator=(FrameSync&& other) noexcept {
	m_impl = other.m_impl;
	other.m_impl = nullptr;
	return *this;
}



void FrameSync::capture(VideoFrame& frame, VideoFrame::Format format) noexcept {
	const auto& ndi = Modules::NDI::get().getNDI();
	ndi.framesync_capture_video(
		m_impl,
		&static_cast<NDIlib_video_frame_v2_t&>(frame),
		static_cast<NDIlib_frame_format_type_e>(format)
	);
}

void FrameSync::free(VideoFrame& frame) noexcept {
	const auto& ndi = Modules::NDI::get().getNDI();
	ndi.framesync_free_video(
		m_impl,
		&static_cast<NDIlib_video_frame_v2_t&>(frame)
	);
}

}