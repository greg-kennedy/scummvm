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

#include "adventuremaker2/cursor.h"

#include "common/substream.h"
#include "image/codecs/codec.h"

namespace AdventureMaker2 {

// Grab the frame data
Cursor::Cursor(Common::SeekableReadStream &stream) {
	// Send this to the BMP header reader
	//  In an ideal world, we could directly use Image::BMP for this.
	//  However, that does not support the "double-height" AND / XOR masks of ICO and CUR
	uint32 infoSize = stream.readUint32LE();
	if (infoSize != 40 && infoSize != 52 && infoSize != 56 && infoSize != 108 && infoSize != 124) {
		warning("Only Windows v1-v5 bitmaps are supported, unknown header: %d", infoSize);
		return;
	}

	uint32 width = stream.readUint32LE();
	int32 height = stream.readSint32LE();

	if (width == 0 || height == 0)
		return;

	if (height < 0) {
		warning("Right-side up bitmaps not supported");
		return;
	}

	/* uint16 planes = */ stream.readUint16LE();
	uint16 bitsPerPixel = stream.readUint16LE();

	if (bitsPerPixel != 1 && bitsPerPixel != 2 && bitsPerPixel != 4 && bitsPerPixel != 8 && bitsPerPixel != 16 && bitsPerPixel != 24 && bitsPerPixel != 32) {
		warning("%dbpp bitmaps not supported", bitsPerPixel);
		return;
	}

	uint32 compression = stream.readUint32BE();

	if (bitsPerPixel == 16 && compression != SWAP_CONSTANT_32(0)) {
		warning("only RGB555 raw mode supported for %dbpp bitmaps", bitsPerPixel);
		return;
	}

	uint32 imageSize = stream.readUint32LE();
	/* uint32 pixelsPerMeterX = */ stream.readUint32LE();
	/* uint32 pixelsPerMeterY = */ stream.readUint32LE();
	uint32 _paletteColorCount = stream.readUint32LE();
	/* uint32 colorsImportant = */ stream.readUint32LE();

	stream.seek(infoSize - 40, SEEK_CUR);

	if (bitsPerPixel <= 8) {
		if (_paletteColorCount == 0)
			_paletteColorCount = (1 << bitsPerPixel);

		// Read the palette
		byte * _palette = new byte[_paletteColorCount * 3];
		for (uint16 i = 0; i < _paletteColorCount; i++) {
			_palette[i * 3 + 2] = stream.readByte();
			_palette[i * 3 + 1] = stream.readByte();
			_palette[i * 3 + 0] = stream.readByte();
			stream.readByte();
		}

		mapXOR.setPalette(_palette, 0, _paletteColorCount);
		delete[] _palette;
	}

	// Create the codec (it will warn about unhandled compression)
	Image::Codec *_XORcodec = Image::createBitmapCodec(compression, 0, width, height / 2, bitsPerPixel);
	if (!_XORcodec)
		return;

	mapXOR.format = _XORcodec->getPixelFormat();
	// read the XOR mask
	mapXOR.copyFrom(_XORcodec->decodeFrame(stream));
	delete _XORcodec;

	// next up is the AND mask
	//  this is always a 1bpp surface
	Image::Codec *_ANDcodec = Image::createBitmapCodec(compression, 0, width, height / 2, 1);
	if (!_ANDcodec)
		return;

	// read the AND mask
	mapAND.format = _ANDcodec->getPixelFormat();
	mapAND.copyFrom(_ANDcodec->decodeFrame(stream));
	delete _ANDcodec;
}

// Draws a CURSOR on the screen at point P
void Cursor::draw(Graphics::ManagedSurface &dest, Common::Point &p) {
//	dest.surfacePtr()->
}

CursorGroup::CursorGroup(Common::SeekableReadStream &stream) {
	// ICONDIR structure
	assert(stream.readUint16LE() == 0);                   // reserved, should be 0
	uint16 fileType = stream.readUint16LE(); // Image type - 1 for ICO, 2 for CUR
	assert(fileType == 1 || fileType == 2);
	uint16 imageCount = stream.readUint16LE();

//	debug("%s: %hu images", (fileType == 1 ? "ICO" : "CUR"), imageCount);

	// image information
	for (int i = 0; i < imageCount; i++) {
		uint16 width = stream.readByte(); // Specifies image width in pixels. Can be any number between 0 and 255. Value 0 means image width is 256 pixels.
		if (width == 0)
			width = 256;
		uint16 height = stream.readByte(); // Specifies image height in pixels.
		if (height == 0)
			height = 256;
		uint8 palette_size = stream.readByte(); // Specifies number of colors in the color palette. Should be 0 if the image does not use a color palette.
		assert(stream.readByte() == 0);         // reserved, should be 0

		uint16 color_planes, bits_per_pixel, hotspotX, hotspotY;
		if (fileType == 1) {
			color_planes = stream.readUint16LE();
			bits_per_pixel = stream.readUint16LE();
			hotspotX = hotspotY = 0;
		} else {
			hotspotX = stream.readUint16LE();
			hotspotY = stream.readUint16LE();
			color_planes = bits_per_pixel = 0;
		}

		// location and size of sub-BMP in this file container
		uint32 imageSize = stream.readUint32LE();
		uint32 imageOffset = stream.readUint32LE();

//		debug("\tImage %u: %hu x %hu, %hhu colors, hotspot (%hu, %hu), size=%u offset=%u", i, width, height, palette_size, hotspotX, hotspotY, imageSize, imageOffset);

		// Grab the frame data
		Common::SeekableSubReadStream subStream(&stream, imageOffset, imageOffset + imageSize);

		cursor.push_back(Cursor(subStream));
	}
}

void CursorGroup::draw(Graphics::ManagedSurface& dest, Common::Point& p) {

}

} // namespace AdventureMaker2
