#include <zuazo/NDI/VideoFrame.h>

#include <cstddef>
#include "../Processing.NDI/Processing.NDI.Lib.h"

namespace Zuazo::NDI {

static_assert(	std::is_same<	typename std::underlying_type<VideoFrame::Format>::type, 
								typename std::underlying_type<NDIlib_frame_format_type_e>::type >::value,
				"Underlaying types must match" );
static_assert(static_cast<int>(VideoFrame::Format::PROGRESSIVE) == NDIlib_frame_format_type_progressive, "PROGRESSIVE format must match");
static_assert(static_cast<int>(VideoFrame::Format::INTERLEAVED) == NDIlib_frame_format_type_interleaved, "PROGRESSIVE format must match");
static_assert(static_cast<int>(VideoFrame::Format::FIELD0) == NDIlib_frame_format_type_field_0, "PROGRESSIVE format must match");
static_assert(static_cast<int>(VideoFrame::Format::FIELD1) == NDIlib_frame_format_type_field_1, "PROGRESSIVE format must match");


static_assert(	std::is_same<	typename std::underlying_type<FourCC>::type, 
								typename std::underlying_type<NDIlib_FourCC_video_type_e>::type >::value,
				"Underlaying types must match" );
static_assert(static_cast<int>(FourCC::UYVY) == NDIlib_FourCC_video_type_UYVY, "UYVY fourCC value must match");
static_assert(static_cast<int>(FourCC::UYVA) == NDIlib_FourCC_video_type_UYVA, "UYVA fourCC value must match");
static_assert(static_cast<int>(FourCC::P216) == NDIlib_FourCC_video_type_P216, "P216 fourCC value must match");
static_assert(static_cast<int>(FourCC::PA16) == NDIlib_FourCC_video_type_PA16, "PA16 fourCC value must match");
static_assert(static_cast<int>(FourCC::YV12) == NDIlib_FourCC_video_type_YV12, "YV12 fourCC value must match");
static_assert(static_cast<int>(FourCC::I420) == NDIlib_FourCC_video_type_I420, "I420 fourCC value must match");
static_assert(static_cast<int>(FourCC::NV12) == NDIlib_FourCC_video_type_NV12, "NV12 fourCC value must match");
static_assert(static_cast<int>(FourCC::BGRA) == NDIlib_FourCC_video_type_BGRA, "BGRA fourCC value must match");
static_assert(static_cast<int>(FourCC::BGRX) == NDIlib_FourCC_video_type_BGRX, "BGRX fourCC value must match");
static_assert(static_cast<int>(FourCC::RGBA) == NDIlib_FourCC_video_type_RGBA, "RGBA fourCC value must match");
static_assert(static_cast<int>(FourCC::RGBX) == NDIlib_FourCC_video_type_RGBX, "RGBX fourCC value must match");



VideoFrame::VideoFrame(	Resolution resolution,
						FourCC fourCC,
						Math::Rational<int>	frameRate,
						float pictureAspectRatio,
						Format format,
						int64_t timecode,
						std::byte* data,
						int lineStride,
						const char* metadata,
						int64_t timestamp ) noexcept
	: m_resolution(resolution)
	, m_fourCC(fourCC)
	, m_frameRate(frameRate)
	, m_pictureAspectRatio(pictureAspectRatio)
	, m_format(format)
	, m_timecode(timecode)
	, m_data(data)
	, m_lineStride(lineStride)
	, m_metadata(metadata)
	, m_timestamp(timestamp)
{
	//These checks are done here as they involve private members
	static_assert(sizeof(*this) == sizeof(NDIlib_video_frame_v2_t), "Sizes must match");
	static_assert(alignof(VideoFrame) == alignof(NDIlib_video_frame_v2_t), "Alignments must match");
	static_assert(offsetof(VideoFrame, m_resolution.x) == offsetof(NDIlib_video_frame_v2_t, xres), "Offset of x resolution must match");
	static_assert(offsetof(VideoFrame, m_resolution.y) == offsetof(NDIlib_video_frame_v2_t, yres), "Offset of y resolution must match");
	static_assert(offsetof(VideoFrame, m_fourCC) == offsetof(NDIlib_video_frame_v2_t, FourCC), "Offset of fourcc must match");
	static_assert(offsetof(VideoFrame, m_frameRate) == offsetof(NDIlib_video_frame_v2_t, frame_rate_N), "Offset of framerate must match");
	static_assert(offsetof(VideoFrame, m_pictureAspectRatio) == offsetof(NDIlib_video_frame_v2_t, picture_aspect_ratio), "Offset of DAR must match");
	static_assert(offsetof(VideoFrame, m_format) == offsetof(NDIlib_video_frame_v2_t, frame_format_type), "Offset of DAR must match");
	static_assert(offsetof(VideoFrame, m_timecode) == offsetof(NDIlib_video_frame_v2_t, timecode), "Offset of timecode must match");
	static_assert(offsetof(VideoFrame, m_data) == offsetof(NDIlib_video_frame_v2_t, p_data), "Offset of data must match");
	static_assert(offsetof(VideoFrame, m_lineStride) == offsetof(NDIlib_video_frame_v2_t, line_stride_in_bytes), "Offset of stride must match");
	static_assert(offsetof(VideoFrame, m_metadata) == offsetof(NDIlib_video_frame_v2_t, p_metadata), "Offset of metadata must match");
	static_assert(offsetof(VideoFrame, m_timestamp) == offsetof(NDIlib_video_frame_v2_t, timestamp), "Offset of timestamp must match");
}



VideoFrame::operator NDIlib_video_frame_v2_t&() noexcept {
	return reinterpret_cast<NDIlib_video_frame_v2_t&>(*this);
}

VideoFrame::operator const NDIlib_video_frame_v2_t&() const noexcept {
	return reinterpret_cast<const NDIlib_video_frame_v2_t&>(*this);
}



void VideoFrame::setResolution(Resolution res) noexcept {
	m_resolution = res;
}

Resolution VideoFrame::getResolution() const noexcept {
	return m_resolution;
}


void VideoFrame::setFourCC(FourCC fcc) noexcept {
	m_fourCC = fcc;
}

FourCC VideoFrame::getFourCC() const noexcept {
	return m_fourCC;
}


void VideoFrame::setFrameRate(Math::Rational<int> rate) noexcept {
	m_frameRate = rate;
}

Math::Rational<int> VideoFrame::getFrameRate() const noexcept {
	return m_frameRate;
}


void VideoFrame::setPictureAspectRatio(float dar) noexcept {
	m_pictureAspectRatio = dar;
}

float VideoFrame::getPictureAspectRatio() const noexcept {
	return m_pictureAspectRatio;
}


void VideoFrame::setFormat(Format format) noexcept {
	m_format = format;
}

VideoFrame::Format VideoFrame::getFormat() const noexcept {
	return m_format;
}


void VideoFrame::setTimecode(int64_t timecode) noexcept {
	m_timecode = timecode;
}

int64_t VideoFrame::getTimecode() const noexcept {
	return m_timecode;
}


void VideoFrame::setData(std::byte* data) noexcept {
	m_data = data;
}

std::byte* VideoFrame::getData() const noexcept {
	return m_data;
}

VideoFrame::SlicedData VideoFrame::getSlicedData() const noexcept {
	SlicedData result = {};

	const auto data = getData();
	const auto planeSize = getStride()*getResolution().width;

	switch(getFourCC()) {
	case FourCC::BGRX:
	case FourCC::BGRA:
	case FourCC::RGBX:
	case FourCC::RGBA:
	case FourCC::UYVY:
		result = {
			SlicedData::value_type(data, planeSize*sizeof(uint8_t))
		};
		break;

	case FourCC::UYVA:
		result = {
			SlicedData::value_type(data + 0*planeSize*sizeof(uint8_t), planeSize*sizeof(uint8_t)),
			SlicedData::value_type(data + 1*planeSize*sizeof(uint8_t), planeSize*sizeof(uint8_t) / 2),
		};
		break;

	case FourCC::YV12:
	case FourCC::I420:
		result = {
			SlicedData::value_type(data, planeSize*sizeof(uint8_t)),
			SlicedData::value_type(data + planeSize*sizeof(uint8_t),  planeSize*sizeof(uint8_t) / 4),
			SlicedData::value_type(data + planeSize*sizeof(uint8_t) + planeSize*sizeof(uint8_t) / 4, planeSize*sizeof(uint8_t) / 4),
		};
		break;
	
	case FourCC::NV12:
		result = {
			SlicedData::value_type(data + 0*planeSize*sizeof(uint8_t), planeSize*sizeof(uint8_t)),
			SlicedData::value_type(data + 1*planeSize*sizeof(uint8_t), planeSize*sizeof(uint8_t)),
		};
		break;

	case FourCC::P216:
		result = {
			SlicedData::value_type(data + 0*planeSize*sizeof(uint16_t), planeSize*sizeof(uint16_t)),
			SlicedData::value_type(data + 1*planeSize*sizeof(uint16_t), planeSize*sizeof(uint16_t)),
		};
		break;	

	case FourCC::PA16:
		result = {
			SlicedData::value_type(data + 0*planeSize*sizeof(uint16_t), planeSize*sizeof(uint16_t)),
			SlicedData::value_type(data + 1*planeSize*sizeof(uint16_t), planeSize*sizeof(uint16_t)),
			SlicedData::value_type(data + 2*planeSize*sizeof(uint16_t), planeSize*sizeof(uint16_t) / 2),
		};
		break;

	default:
		assert(false); //Not expected
		break;
	}

	return result;
}


void VideoFrame::setStride(int stride) noexcept {
	m_lineStride = stride;
}

int VideoFrame::getStride() const noexcept {
	return m_lineStride;
}


void VideoFrame::setMetadata(const char* metadata) noexcept {
	m_metadata = metadata;
}

const char* VideoFrame::getMetadata() const noexcept {
	return m_metadata;
}


void VideoFrame::setTimestamp(int64_t timestamp) noexcept {
	m_timestamp = timestamp;
}

int64_t VideoFrame::getTimestamp() const noexcept {
	return m_timestamp;
}

}