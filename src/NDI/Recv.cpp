#include <zuazo/NDI/Recv.h>

#include <cstddef>
#include "../Processing.NDI/Processing.NDI.Lib.h"

#include <zuazo/Modules/NDI.h>

#include <type_traits>

namespace Zuazo::NDI {

static_assert(sizeof(Recv) == sizeof(NDIlib_recv_instance_t), "Sizes do not match");
static_assert(alignof(Recv) == alignof(NDIlib_recv_instance_t), "Alignments do not match");

static_assert(	std::is_same<	typename std::underlying_type<Recv::ColorFormat>::type, 
								typename std::underlying_type<NDIlib_recv_color_format_e>::type >::value,
				"Underlaying types must match" );
static_assert(static_cast<int>(Recv::ColorFormat::BGRX_BGRA) == NDIlib_recv_color_format_BGRX_BGRA, "BGRX_BGRA format must match");
static_assert(static_cast<int>(Recv::ColorFormat::UYVY_BGRA) == NDIlib_recv_color_format_UYVY_BGRA, "UYVY_BGRA format must match");
static_assert(static_cast<int>(Recv::ColorFormat::RGBX_RGBA) == NDIlib_recv_color_format_RGBX_RGBA, "RGBX_RGBA format must match");
static_assert(static_cast<int>(Recv::ColorFormat::UYVY_RGBA) == NDIlib_recv_color_format_UYVY_RGBA, "UYVY_RGBA format must match");
static_assert(static_cast<int>(Recv::ColorFormat::FASTEST) == NDIlib_recv_color_format_fastest, "FASTEST format must match");
static_assert(static_cast<int>(Recv::ColorFormat::BEST) == NDIlib_recv_color_format_best, "FASTEST format must match");

static_assert(	std::is_same<	typename std::underlying_type<Recv::Bandwidth>::type, 
								typename std::underlying_type<NDIlib_recv_bandwidth_e>::type >::value,
				"Underlaying types must match" );
static_assert(static_cast<int>(Recv::Bandwidth::METADATA_ONLY) == NDIlib_recv_bandwidth_metadata_only, "METADATA_ONLY bandwidth must match");
static_assert(static_cast<int>(Recv::Bandwidth::AUDIO_ONLY) == NDIlib_recv_bandwidth_audio_only, "AUDIO_ONLY bandwidth must match");
static_assert(static_cast<int>(Recv::Bandwidth::LOWEST) == NDIlib_recv_bandwidth_lowest, "LOWEST bandwidth must match");
static_assert(static_cast<int>(Recv::Bandwidth::HIGHEST) == NDIlib_recv_bandwidth_highest, "HIGHEST bandwidth must match");

static NDIlib_recv_instance_t createRecvInstance(	const Source& source,
													Recv::ColorFormat format,
													Recv::Bandwidth bandwidth,
													bool allowVideoFields,
													const char* name )
{
	const auto& ndi = Modules::NDI::get().getNDI();

	const NDIlib_recv_create_v3_t createInfo(
		source,
		static_cast<NDIlib_recv_color_format_e>(format),
		static_cast<NDIlib_recv_bandwidth_e>(bandwidth),
		allowVideoFields,
		name
	);
	return ndi.recv_create_v3(&createInfo);
}

static void destroyRecvInstance(NDIlib_recv_instance_t instance) {
	const auto& ndi = Modules::NDI::get().getNDI();
	ndi.recv_destroy(instance);
}



Recv::Recv(void* ptr) noexcept
	: m_impl(ptr)
{
}

Recv::Recv(	const Source& source,
			ColorFormat format,
			Bandwidth bandwidth,
			bool allowVideoFields,
			const char* name)
	: Recv(createRecvInstance(source, format, bandwidth, allowVideoFields, name))
{
}

Recv::Recv(Recv&& other) noexcept 
	: Recv(other.m_impl)
{
	other.m_impl = nullptr;
}

Recv::~Recv() {
	if(m_impl) {
		destroyRecvInstance(m_impl);
	}
}

Recv& Recv::operator=(Recv&& other) noexcept {
	m_impl = other.m_impl;
	other.m_impl = nullptr;
	return *this;
}



void Recv::connect(const Source& source) noexcept {
	const auto& ndi = Modules::NDI::get().getNDI();
	ndi.recv_connect(
		m_impl, 
		&static_cast<const NDIlib_source_t&>(source)
	);
}


Recv::FrameType	Recv::capture(VideoFrame& frame, uint32_t timeo) const noexcept {
	const auto& ndi = Modules::NDI::get().getNDI();
	const auto result = ndi.recv_capture_v3(
		m_impl,
		&static_cast<NDIlib_video_frame_v2_t&>(frame),
		nullptr,
		nullptr,
		timeo
	);
	return static_cast<FrameType>(result);
}

void Recv::free(VideoFrame& frame) const noexcept {
	const auto& ndi = Modules::NDI::get().getNDI();
	ndi.recv_free_video_v2(
		m_impl,
		&static_cast<NDIlib_video_frame_v2_t&>(frame)
	);
}



bool Recv::setTally(bool pgm, bool pvw) noexcept {
	const auto& ndi = Modules::NDI::get().getNDI();
	const NDIlib_tally_t tally(pgm, pvw);
	return ndi.recv_set_tally(m_impl, &tally);
}

}