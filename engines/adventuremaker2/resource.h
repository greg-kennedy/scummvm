/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ADVENTUREMAKER2_RESOURCE_H
#define ADVENTUREMAKER2_RESOURCE_H

#include "common/str.h"
#include "common/stream.h"

#include "graphics/managed_surface.h"
#include "video/avi_decoder.h"

namespace AdventureMaker2 {

// Given a string with resource extension (.bmp, .mid, etc) return filename with
//  "hidden" extension (.004, .011, etc)
const Common::String hiddenExtension(const Common::String filename);

/* Functions for working with Resources used by AM2 - video, image, cursor, music, etc */
Graphics::ManagedSurface *loadImage(Common::SeekableReadStream &srs);
Graphics::ManagedSurface *loadImage(Common::String filename);

Video::AVIDecoder *loadVideo(Common::SeekableReadStream &srs);
Video::AVIDecoder *loadVideo(Common::String filename);

} // namespace AdventureMaker2

#endif
