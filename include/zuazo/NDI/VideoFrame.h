#pragma once

#include <zuazo/FourCC.h>
#include <zuazo/Resolution.h>
#include <zuazo/Math/Rational.h>

#include <cstdint>

struct NDIlib_video_frame_v2_t;

namespace Zuazo::NDI {

class VideoFrame {
public:
	enum class Format : uint32_t {
		PROGRESSIVE = 1,
		INTERLEAVED = 0,
		FIELD0		= 2,
		FIELD1		= 3
	};

	using SlicedData = std::array<Utils::BufferView<std::byte>, 4>;

	static constexpr auto SYNTHETIZE_TIMECODE = std::numeric_limits<int64_t>::max();

	explicit VideoFrame(Resolution resolution = Resolution(0, 0),
						FourCC fourCC = FourCC::UYVY,
						Math::Rational<int>	frameRate = Math::Rational<int>(30000, 1001),
						float pictureAspectRatio = 0.0f,
						Format format = Format::PROGRESSIVE,
						int64_t timecode = SYNTHETIZE_TIMECODE,
						std::byte* data = nullptr,
						int lineStride = 0,
						const char* metadata = nullptr,
						int64_t timestamp = 0 ) noexcept;
	VideoFrame(const VideoFrame& other) = default;
	~VideoFrame() = default;

	VideoFrame&			operator=(const VideoFrame& other) = default;

	operator NDIlib_video_frame_v2_t&() noexcept;
	operator const NDIlib_video_frame_v2_t&() const noexcept;

	void				setResolution(Resolution res) noexcept;
	Resolution			getResolution() const noexcept;

	void				setFourCC(FourCC fcc) noexcept;
	FourCC				getFourCC() const noexcept;

	void				setFrameRate(Math::Rational<int> rate) noexcept;
	Math::Rational<int>	getFrameRate() const noexcept;

	void				setPictureAspectRatio(float dar) noexcept;
	float				getPictureAspectRatio() const noexcept;

	void				setFormat(Format format) noexcept;
	Format				getFormat() const noexcept;

	void				setTimecode(int64_t timecode) noexcept;
	int64_t				getTimecode() const noexcept;

	void				setData(std::byte* data) noexcept;
	std::byte*			getData() const noexcept;
	SlicedData			getSlicedData() const noexcept;

	void				setStride(int stride) noexcept;
	int					getStride() const noexcept;

	void				setMetadata(const char* metadata) noexcept;
	const char*			getMetadata() const noexcept;

	void				setTimestamp(int64_t timestamp) noexcept;
	int64_t				getTimestamp() const noexcept;

private:
	Resolution			m_resolution;
	FourCC				m_fourCC;
	Math::Rational<int>	m_frameRate;
	float				m_pictureAspectRatio;
	Format				m_format;
	int64_t				m_timecode;
	std::byte*			m_data;
	int					m_lineStride;
	const char*			m_metadata;
	int64_t				m_timestamp;



};

}