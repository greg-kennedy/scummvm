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

#include "made/graphics.h"

#include "common/endian.h"
#include "common/textconsole.h"
#include "common/debug.h"
#include "common/util.h"

#include "graphics/surface.h"

namespace Made {

byte ValueReader::readPixel() {
	byte value;
	if (_nibbleMode) {
		if (_nibbleSwitch) {
			value = (_buffer[0] >> 4) & 0x0F;
			_buffer++;
		} else {
			value = _buffer[0] & 0x0F;
		}
		_nibbleSwitch = !_nibbleSwitch;
	} else {
		value = _buffer[0];
		_buffer++;
	}
	return value;
}

uint16 ValueReader::readUint16() {
	uint16 value = READ_LE_UINT16(_buffer);
	_buffer += 2;
	return value;
}

uint32 ValueReader::readUint32() {
	uint32 value = READ_LE_UINT32(_buffer);
	_buffer += 4;
	return value;
}

void ValueReader::resetNibbleSwitch() {
	_nibbleSwitch = false;
}

static byte *rleDecompress(byte *source, const byte *end, const int maxSize) {
	byte *dest = new byte[maxSize];
	byte *o = dest;

	while (source < end) {
		byte val = *source;
		source++;
		if (val < 0x80) {
			// copy
			val++;
			memcpy(o, source, val);
			o += val;
			source += val;
		} else {
			// repeat
			val = 257 - val;
			memset(o, *source, val);
			o += val;
			source++;
		}
	}

	return dest;
}

void decompressImage(byte *source, Graphics::Surface &surface, uint16 cmdOffs, uint16 pixelOffs, uint16 maskOffs,
					 uint16 cmdSize, uint16 pixelSize, uint16 maskSize,
					 uint16 lineSize, byte cmdFlags, byte pixelFlags, byte maskFlags, bool deltaFrame) {

	if ((maskFlags & 0b11111100) || (pixelFlags & 0b11111100) || (cmdFlags & 0b11111110))
		error("decompressImage() Unsupported flags: cmdFlags = %02X; maskFlags = %02X, pixelFlags = %02X", cmdFlags, maskFlags, pixelFlags);

	uint16 width = surface.w;
	uint16 height = surface.h;
	uint16 pitch = surface.pitch;
	uint16 alignW = ((width + 3) / 4), alignH = ((height + 3) / 4);

	const int offsets[] = {
		0, 1, 2, 3,
		width, width + 1, width + 2, width + 3,
		width * 2, width * 2 + 1, width * 2 + 2, width * 2 + 3,
		width * 3, width * 3 + 1, width * 3 + 2, width * 3 + 3
	};

	// RLE decompress the buffers as needed (remember to free later!)
	byte *cmdBuffer;
	if (cmdFlags & 1)
		cmdBuffer = rleDecompress(source + cmdOffs, source + cmdOffs + cmdSize, lineSize * height);
	else
		cmdBuffer = source + cmdOffs;

	byte *maskBuffer;
	if (maskFlags & 1)
		maskBuffer = rleDecompress(source + maskOffs, source + maskOffs + maskSize, alignW * alignH * 4);
	else
		maskBuffer = source + maskOffs;
	ValueReader maskReader(maskBuffer, (maskFlags & 2) != 0);

	byte *pixelBuffer;
	if (pixelFlags & 1)
		pixelBuffer = rleDecompress(source + pixelOffs, source + pixelOffs + pixelSize, alignW * alignH * 4);
	else
		pixelBuffer = source + pixelOffs;
	ValueReader pixelReader(pixelBuffer, (pixelFlags & 2) != 0);

	//
	byte *destPtr = (byte *)surface.getPixels();

	byte lineBuf[640 * 4];
	byte bitBuf[40];

	int bitBufLastOfs = (((lineSize + 1) >> 1) << 1) - 2;
	int bitBufLastCount = ((width + 3) >> 2) & 7;
	if (bitBufLastCount == 0)
		bitBufLastCount = 8;

	byte *pCmdBuf = cmdBuffer;
	while (height > 0) {

		int drawDestOfs = 0;

		memset(lineBuf, 0, sizeof(lineBuf));

		memcpy(bitBuf, pCmdBuf, lineSize);
		pCmdBuf += lineSize;

		for (uint16 bitBufOfs = 0; bitBufOfs < lineSize; bitBufOfs += 2) {

			uint16 bits = READ_LE_UINT16(&bitBuf[bitBufOfs]);

			int bitCount;
			if (bitBufOfs == bitBufLastOfs)
				bitCount = bitBufLastCount;
			else
				bitCount = 8;

			for (int curCmd = 0; curCmd < bitCount; curCmd++) {
				int cmd = bits & 3;
				bits >>= 2;

				byte pixels[4];
				uint32 mask;

				switch (cmd) {

				case 0:
					pixels[0] = pixelReader.readPixel();
					for (int i = 0; i < 16; i++)
						lineBuf[drawDestOfs + offsets[i]] = pixels[0];
					break;

				case 1:
					pixels[0] = pixelReader.readPixel();
					pixels[1] = pixelReader.readPixel();
					mask = maskReader.readUint16();
					for (int i = 0; i < 16; i++) {
						lineBuf[drawDestOfs + offsets[i]] = pixels[mask & 1];
						mask >>= 1;
					}
					break;

				case 2:
					pixels[0] = pixelReader.readPixel();
					pixels[1] = pixelReader.readPixel();
					pixels[2] = pixelReader.readPixel();
					pixels[3] = pixelReader.readPixel();
					mask = maskReader.readUint32();
					for (int i = 0; i < 16; i++) {
						lineBuf[drawDestOfs + offsets[i]] = pixels[mask & 3];
						mask >>= 2;
					}
					break;

				case 3:
					if (!deltaFrame) {
						// For EGA pictures: Pixels are read starting from a new byte
						maskReader.resetNibbleSwitch();
						// Yes, it reads from maskReader here
						for (int i = 0; i < 16; i++)
							lineBuf[drawDestOfs + offsets[i]] = maskReader.readPixel();
					}
					break;

				default:
					break;
				}

				drawDestOfs += 4;
			}
		}

		if (deltaFrame) {
			for (int y = 0; y < 4 && height > 0; y++, height--) {
				for (int x = 0; x < width; x++) {
					if (lineBuf[x + y * width] != 0)
						*destPtr = lineBuf[x + y * width];
					destPtr++;
				}
				destPtr += pitch - width;
			}
		} else {
			for (int y = 0; y < 4 && height > 0; y++, height--) {
				memcpy(destPtr, &lineBuf[y * width], width);
				destPtr += pitch;
			}
		}
	}

	// cleanup RLE buffers
	if (cmdFlags & 1)
		delete[] cmdBuffer;
	if (maskFlags & 1)
		delete[] maskBuffer;
	if (pixelFlags & 1)
		delete[] pixelBuffer;
}

void decompressMovieImage(byte *source, Graphics::Surface &surface, uint16 cmdOffs, uint16 pixelOffs, uint16 maskOffs,
		uint16 cmdSize, uint16 pixelSize, uint16 maskSize, uint16 lineSize,
		byte cmdFlags, byte pixelFlags, byte maskFlags) {

	uint16 width = surface.w;
	uint16 height = surface.h;
	uint16 alignW = ((width + 3) / 4), alignH = ((height + 3) / 4);
	uint16 bx = 0, by = 0, bw = alignW * 4;

	// RLE decompress the buffers as needed (remember to free later!)
	byte *cmdBuffer;
	if (cmdFlags & 1)
		cmdBuffer = rleDecompress(source + cmdOffs, source + cmdOffs + cmdSize, lineSize * height);
	else
		cmdBuffer = source + cmdOffs;

	byte *pixelBuffer;
	if (pixelFlags & 1)
		pixelBuffer = rleDecompress(source + pixelOffs, source + pixelOffs + pixelSize, alignW * alignH * 4);
	else
		pixelBuffer = source + pixelOffs;

	byte *maskBuffer;
	if (maskFlags & 1)
		maskBuffer = rleDecompress(source + maskOffs, source + maskOffs + maskSize, alignW * alignH * 4);
	else
		maskBuffer = source + maskOffs;

	//
	byte *destPtr = (byte *)surface.getPixels();

	byte bitBuf[40];

	int bitBufLastOfs = (((lineSize + 1) >> 1) << 1) - 2;
	int bitBufLastCount = ((width + 3) >> 2) & 7;
	if (bitBufLastCount == 0)
		bitBufLastCount = 8;

	byte *pCmdBuf = cmdBuffer;
	byte *pPixelBuf = pixelBuffer;
	byte *pMaskBuf = maskBuffer;
	while (height > 0) {

		memcpy(bitBuf, pCmdBuf, lineSize);
		pCmdBuf += lineSize;

		for (uint16 bitBufOfs = 0; bitBufOfs < lineSize; bitBufOfs += 2) {

			uint16 bits = READ_LE_UINT16(&bitBuf[bitBufOfs]);

			int bitCount;
			if (bitBufOfs == bitBufLastOfs)
				bitCount = bitBufLastCount;
			else
				bitCount = 8;

			for (int curCmd = 0; curCmd < bitCount; curCmd++) {
				uint cmd = bits & 3;
				bits >>= 2;

				byte pixels[4], block[16];
				uint32 mask;

				switch (cmd) {

				case 0:
					pixels[0] = *pPixelBuf++;
					for (int i = 0; i < 16; i++)
						block[i] = pixels[0];
					break;

				case 1:
					pixels[0] = *pPixelBuf++;
					pixels[1] = *pPixelBuf++;
					mask = READ_LE_UINT16(pMaskBuf);
					pMaskBuf += 2;
					for (int i = 0; i < 16; i++) {
						block[i] = pixels[mask & 1];
						mask >>= 1;
					}
					break;

				case 2:
					pixels[0] = *pPixelBuf++;
					pixels[1] = *pPixelBuf++;
					pixels[2] = *pPixelBuf++;
					pixels[3] = *pPixelBuf++;
					mask = READ_LE_UINT32(pMaskBuf);
					pMaskBuf += 4;
					for (int i = 0; i < 16; i++) {
						block[i] = pixels[mask & 3];
						mask >>= 2;
					}
					break;

				case 3:
					break;

				default:
					break;
				}

				if (cmd != 3) {
					uint16 blockPos = 0;
					uint32 maxW = MIN(4, surface.w - bx);
					uint32 maxH = (MIN(4, surface.h - by) + by) * width;
					for (uint32 yc = by * width; yc < maxH; yc += width) {
						for (uint32 xc = 0; xc < maxW; xc++) {
							destPtr[(bx + xc) + yc] = block[xc + blockPos];
						}
						blockPos += 4;
					}
				}

				bx += 4;
				if (bx >= bw) {
					bx = 0;
					by += 4;
				}

			}

		}

		height -= 4;

	}

	// cleanup RLE buffers
	if (cmdFlags & 1)
		delete[] cmdBuffer;
	if (maskFlags & 1)
		delete[] maskBuffer;
	if (pixelFlags & 1)
		delete[] pixelBuffer;
}

} // End of namespace Made
