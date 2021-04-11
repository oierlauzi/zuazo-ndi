#pragma once

#include "VideoFrame.h"

#include <zuazo/Graphics/StagedFrame.h>

namespace Zuazo::NDI {

void copyRGBA(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept;
void copyUYVY(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept;
void copyP216(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept;
void copyPA16(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept;
void copyI420(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept;
void copyNV12(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept;

void copyUYVYtoNV16(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept;
void copyUYVAtoPA8(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept;
void copyYV12toI420(const VideoFrame& src, Graphics::StagedFrame& dst) noexcept;

}