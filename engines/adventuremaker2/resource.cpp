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

#include "adventuremaker2/resource.h"

#include "common/util.h"
#include "common/debug.h"

#include "image/bmp.h"
#include "image/gif.h"
#include "image/jpeg.h"
// also, wmf and emf

namespace AdventureMaker2 {

const Common::String hiddenExtension(const Common::String filename) {
	if (filename.size() < 3)
		return filename;

	uint limit = filename.size() - 3;

	if (filename.hasSuffixIgnoreCase(".ico"))
		return filename.substr(0, limit) + "002";
	else if (filename.hasSuffixIgnoreCase(".cur"))
		return filename.substr(0, limit) + "003";
	else if (filename.hasSuffixIgnoreCase(".bmp"))
		return filename.substr(0, limit) + "004";
	else if (filename.hasSuffixIgnoreCase(".jpg"))
		return filename.substr(0, limit) + "005";
	else if (filename.hasSuffixIgnoreCase(".gif"))
		return filename.substr(0, limit) + "006";
	else if (filename.hasSuffixIgnoreCase(".wmf"))
		return filename.substr(0, limit) + "007";
	else if (filename.hasSuffixIgnoreCase(".emf"))
		return filename.substr(0, limit) + "008";
	else if (filename.hasSuffixIgnoreCase(".wav"))
		return filename.substr(0, limit) + "010";
	else if (filename.hasSuffixIgnoreCase(".mid"))
		return filename.substr(0, limit) + "011";
	else if (filename.hasSuffixIgnoreCase(".mp3"))
		return filename.substr(0, limit) + "012";
	else if (filename.hasSuffixIgnoreCase(".avi"))
		return filename.substr(0, limit) + "014";
	else
		return filename;
}

Graphics::ManagedSurface *loadImage(Common::SeekableReadStream &srs) {
	// Attempt various image conversions of this file.
	//  It's possible they are mis-named by the dev (or have purposely mangled extension)
	// Read up to 4 bytes and compare them against magic numbers

	uint32 magic = srs.readUint32BE();
	// rewind
	srs.seek(0);
	
#ifdef USE_GIF
	if (magic == 0x47494638) {
		Image::GIFDecoder decoder;
		if (decoder.loadStream(srs))
			return new Graphics::ManagedSurface(decoder.getSurface());
	}
#endif // USE_GIF

#ifdef USE_JPEG
	if ((magic & 0xFFFFFF00) == 0xFFD8FF00) {
		Image::JPEGDecoder decoder;
		if (decoder.loadStream(srs))
			return new Graphics::ManagedSurface(decoder.getSurface());
	}
#endif // USE_JPEG

#ifdef USE_BMP
	if ((magic & 0xFFFF0000) == 0x424D0000) {
		Image::BMPDecoder decoder;
		if (decoder.loadStream(srs))
			return new Graphics::ManagedSurface(decoder.getSurface());
	}
#endif // USE_BMP

	return nullptr;
}

Graphics::ManagedSurface *loadImage(Common::String filename) {
	Graphics::ManagedSurface *ret = nullptr;

	// We look for a file matching this file extension in the archive.
	Common::SeekableReadStream *srs = SearchMan.createReadStreamForMember(filename);

	if (srs != nullptr) {
		ret = loadImage(*srs);
		delete srs;
	}

	if (ret == nullptr) {
		// either didn't find the expected file or it failed to load.
		//  With "file protection" enabled, some extensions are changed to numbers instead.
		// There is a lookup table to convert between these.

		srs = SearchMan.createReadStreamForMember(hiddenExtension(filename));
		if (srs != nullptr) {
			ret = loadImage(*srs);
			delete srs;
		}
	}

	return ret;
}

Video::AVIDecoder *loadVideo(Common::SeekableReadStream *srs) {
	Video::AVIDecoder *ret = new Video::AVIDecoder;

	if (ret->loadStream(srs))
		return ret;

	delete ret;
	return nullptr;
}

Video::AVIDecoder *loadVideo(Common::String filename) {
	Video::AVIDecoder *ret = nullptr;

	// We look for a file matching this file extension in the archive.
	Common::SeekableReadStream *srs = SearchMan.createReadStreamForMember(filename);

	if (srs != nullptr) {
		ret = loadVideo(srs);
//		delete srs;
	}

	if (ret == nullptr) {
		// either didn't find the expected file or it failed to load.
		//  With "file protection" enabled, some extensions are changed to numbers instead.
		// There is a lookup table to convert between these.

		srs = SearchMan.createReadStreamForMember(hiddenExtension(filename));
		if (srs != nullptr) {
			ret = loadVideo(srs);
//			delete srs;
		}
	}

	return ret;
}

} // namespace AdventureMaker2
