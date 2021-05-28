#include <zuazo/NDI/Conversions.h>

#include <cassert>
#include <cstring>
#include <cstdint>

namespace Zuazo::NDI {

static void copyPlane(	Utils::BufferView<const std::byte> src, 
						size_t srcStride,
						Utils::BufferView<std::byte> dst,
						size_t dstStride,
						size_t height ) noexcept
{
	assert(src.size() >= srcStride*height);
	assert(dst.size() >= dstStride*height);
	
	if(srcStride != dstStride) {
		//As they have different strides, copy line by line
		const auto minStride = Math::min(srcStride, dstStride);
		for(size_t i = 0; i < height; ++i) {
			std::memcpy(
				dst.data() + i*dstStride,
				src.data() + i*srcStride,
				minStride
			);
		}

	} else {
		//Copy everything at once
		std::memcpy(
			dst.data(),
			src.data(),
			Math::min(src.size(), dst.size())
		);

	}
}

template<size_t WordSize>
static void copyPlaneInterleaved(	Utils::BufferView<const std::byte> src, 
									size_t srcStride,
									Utils::BufferView<std::byte> dst0,
									Utils::BufferView<std::byte> dst1,
									size_t dstStride,
									size_t height ) noexcept
{
	assert(src.size() >= srcStride*height);
	assert(dst0.size() >= dstStride*height);
	assert(dst1.size() >= dstStride*height);
	assert(srcStride % (2*WordSize) == 0);
	assert(dstStride % WordSize == 0);

	//Copy data inteleaving words between planes
	for(size_t i = 0; i < height; ++i) {
		for(size_t j = 0; j < srcStride/WordSize; ++j) {
			//Odd words to dst1, even ones to dst0
			const auto& dst = (j%2) ? dst1 : dst0;

			//Copy a word
			std::memcpy(
				dst.data() + i*dstStride + (j/2)*WordSize,
				src.data() + i*srcStride + (j  )*WordSize,
				WordSize
			);
		}
	}
}




void copyRGBA(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept {
	// Planar 8bit, 4:4:4:4 video format.
	const auto& dstDescriptor = dst.getDescriptor();
	const auto dstResolution = dstDescriptor->getResolution();
	assert(dstDescriptor->getColorFormat() == ColorFormat::R8G8B8A8 || dstDescriptor->getColorFormat() == ColorFormat::B8G8R8A8);
	assert(dstDescriptor->getColorSubsampling() == ColorSubsampling::rb444);

	const auto srcData = src.getSlicedData();
	const auto dstData = dst.getPixelData();
	assert(dstData.size() == 1);

	copyPlane(
		srcData[0],
		src.getStride()*sizeof(uint8_t),
		dstData[0],
		dstResolution.width*sizeof(uint8_t)*4,
		dstResolution.height
	);
}

void copyUYVY(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept {
	// YCbCr color space using 4:2:2.
	const auto& dstDescriptor = dst.getDescriptor();
	const auto dstResolution = dstDescriptor->getResolution();
	assert(dstDescriptor->getColorFormat() == ColorFormat::B8G8R8G8);
	assert(dstDescriptor->getColorSubsampling() == ColorSubsampling::rb422);

	const auto srcData = src.getSlicedData();
	const auto dstData = dst.getPixelData();
	assert(dstData.size() == 1);

	copyPlane(
		srcData[0],
		src.getStride()*sizeof(uint8_t),
		dstData[0],
		dstResolution.width*sizeof(uint8_t)*2,
		dstResolution.height
	);
}


void copyP216(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept {
	// YCbCr color space using 4:2:2 in 16bpp
	// In memory this is a semi-planar format. This is identical to a 16bpp 
	// version of the NV16 format. 
	// The first buffer is a 16bpp luminance buffer. 
	// Immediately after this is an interleaved buffer of 16bpp Cb, Cr pairs.

	const auto& dstDescriptor = dst.getDescriptor();
	const auto dstResolution = dstDescriptor->getResolution();
	assert(dstDescriptor->getColorFormat() == ColorFormat::G16_B16R16);
	assert(dstDescriptor->getColorSubsampling() == ColorSubsampling::rb422);

	const auto srcData = src.getSlicedData();
	const auto dstData = dst.getPixelData();
	assert(dstData.size() == 2);

	copyPlane( //Y plane
		srcData[0],
		src.getStride()*sizeof(uint16_t),
		dstData[0],
		dstResolution.width*sizeof(uint16_t),
		dstResolution.height
	);
	copyPlane( //4:2:2 CbCr plane
		srcData[1],
		src.getStride()*sizeof(uint16_t),
		dstData[1],
		dstResolution.width*sizeof(uint16_t),
		dstResolution.height
	);
}

void copyPA16(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept {
	// YCbCr color space with an alpha channel, using 4:2:2:4
	// In memory this is a semi-planar format. 
	// The first buffer is a 16bpp luminance buffer. 
	// Immediately after this is an interleaved buffer of 16bpp Cb, Cr pairs.
	// Immediately after is a single buffer of 16bpp alpha channel.

	const auto& dstDescriptor = dst.getDescriptor();
	const auto dstResolution = dstDescriptor->getResolution();
	assert(dstDescriptor->getColorFormat() == ColorFormat::G16_B16R16_A16);
	assert(dstDescriptor->getColorSubsampling() == ColorSubsampling::rb422);

	const auto srcData = src.getSlicedData();
	const auto dstData = dst.getPixelData();
	assert(dstData.size() == 3);

	copyPlane( //Y plane
		srcData[0],
		src.getStride()*sizeof(uint16_t),
		dstData[0],
		dstResolution.width*sizeof(uint16_t),
		dstResolution.height
	);
	copyPlane( //4:2:2 CbCr plane
		srcData[1],
		src.getStride()*sizeof(uint16_t),
		dstData[1],
		dstResolution.width*sizeof(uint16_t),
		dstResolution.height
	);
	copyPlane( //A plane
		srcData[2],
		src.getStride()*sizeof(uint16_t),
		dstData[2],
		dstResolution.width*sizeof(uint16_t),
		dstResolution.height
	);
}

void copyI420(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept {
	// The first buffer is an 8bpp luminance buffer.
	// Immediately following this is a 8bpp Cb buffer.
	// Immediately following this is a 8bpp Cr buffer.

	const auto& dstDescriptor = dst.getDescriptor();
	const auto dstResolution = dstDescriptor->getResolution();
	assert(dstDescriptor->getColorFormat() == ColorFormat::G8_B8_R8);
	assert(dstDescriptor->getColorSubsampling() == ColorSubsampling::rb420);

	const auto srcData = src.getSlicedData();
	const auto dstData = dst.getPixelData();
	assert(dstData.size() == 3);

	copyPlane( //Y plane
		srcData[0],
		src.getStride()*sizeof(uint8_t),
		dstData[0],
		dstResolution.width*sizeof(uint8_t),
		dstResolution.height
	);
	copyPlane( //4:2:0 Cb plane
		srcData[1],
		src.getStride()*sizeof(uint8_t),
		dstData[1],
		dstResolution.width*sizeof(uint8_t) / 2,
		dstResolution.height / 2
	);
	copyPlane( //4:2:0 Cr plane
		srcData[2],
		src.getStride()*sizeof(uint8_t),
		dstData[2],
		dstResolution.width*sizeof(uint8_t) / 2,
		dstResolution.height / 2
	);
}

void copyNV12(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept {
	// Planar 8bit 4:2:0 video format.
	// The first buffer is an 8bpp luminance buffer.
	// Immediately following this is in interleaved buffer of 8bpp Cb, Cr pairs

	const auto& dstDescriptor = dst.getDescriptor();
	const auto dstResolution = dstDescriptor->getResolution();
	assert(dstDescriptor->getColorFormat() == ColorFormat::G8_B8R8);
	assert(dstDescriptor->getColorSubsampling() == ColorSubsampling::rb420);

	const auto srcData = src.getSlicedData();
	const auto dstData = dst.getPixelData();
	assert(dstData.size() == 2);

	copyPlane( //Y plane
		srcData[0],
		src.getStride()*sizeof(uint8_t),
		dstData[0],
		dstResolution.width*sizeof(uint8_t),
		dstResolution.height
	);
	copyPlane( //4:2:0 CbCr plane
		srcData[1],
		src.getStride()*sizeof(uint8_t),
		dstData[1],
		dstResolution.width*sizeof(uint8_t),
		dstResolution.height / 2
	);
}



void copyUYVYtoNV16(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept {
	// YCbCr color space using 4:2:2.

	const auto& dstDescriptor = dst.getDescriptor();
	const auto dstResolution = dstDescriptor->getResolution();
	assert(dstDescriptor->getColorFormat() == ColorFormat::G8_B8R8);
	assert(dstDescriptor->getColorSubsampling() == ColorSubsampling::rb422);

	const auto srcData = src.getSlicedData();
	const auto dstData = dst.getPixelData();
	assert(dstData.size() == 2);

	copyPlaneInterleaved<1>( 
		srcData[0],
		src.getStride()*sizeof(uint8_t),
		dstData[0],
		dstData[1],
		dstResolution.width*sizeof(uint8_t),
		dstResolution.height
	);

}

void copyUYVAtoPA8(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept {
	// YCbCr + Alpha color space, using 4:2:2:4.
	// In memory there are two separate planes. The first is a regular
	// UYVY 4:2:2 buffer. Immediately following this in memory is a 
	// alpha channel buffer.

	const auto& dstDescriptor = dst.getDescriptor();
	const auto dstResolution = dstDescriptor->getResolution();
	assert(dstDescriptor->getColorFormat() == ColorFormat::G8_B8R8_A8);
	assert(dstDescriptor->getColorSubsampling() == ColorSubsampling::rb422);

	const auto srcData = src.getSlicedData();
	const auto dstData = dst.getPixelData();
	assert(dstData.size() == 3);

	copyPlaneInterleaved<1>( 
		srcData[0],
		src.getStride()*sizeof(uint8_t),
		dstData[0],
		dstData[1],
		dstResolution.width*sizeof(uint8_t),
		dstResolution.height
	);
	copyPlane( //A plane
		srcData[2],
		src.getStride()*sizeof(uint8_t),
		dstData[2],
		dstResolution.width*sizeof(uint8_t),
		dstResolution.height
	);

}

void copyYV12toI420(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept {
	// Planar 8bit 4:2:0 video format.
	// The first buffer is an 8bpp luminance buffer.
	// Immediately following this is a 8bpp Cr buffer.
	// Immediately following this is a 8bpp Cb buffer.

	const auto& dstDescriptor = dst.getDescriptor();
	const auto dstResolution = dstDescriptor->getResolution();
	assert(dstDescriptor->getColorFormat() == ColorFormat::G8_B8_R8);
	assert(dstDescriptor->getColorSubsampling() == ColorSubsampling::rb420);

	const auto srcData = src.getSlicedData();
	const auto dstData = dst.getPixelData();
	assert(dstData.size() == 3);

	copyPlane( //Y plane
		srcData[0],
		src.getStride()*sizeof(uint8_t),
		dstData[0],
		dstResolution.width*sizeof(uint8_t),
		dstResolution.height
	);
	copyPlane( //4:2:0 Cr plane
		srcData[1],
		src.getStride()*sizeof(uint8_t),
		dstData[2],
		dstResolution.width*sizeof(uint8_t) / 2,
		dstResolution.height / 2
	);
	copyPlane( //4:2:0 Cb plane
		srcData[2],
		src.getStride()*sizeof(uint8_t),
		dstData[1],
		dstResolution.width*sizeof(uint8_t) / 2,
		dstResolution.height / 2
	);
}


}