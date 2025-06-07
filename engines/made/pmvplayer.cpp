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

#include "made/pmvplayer.h"
#include "made/made.h"
#include "made/screen.h"
#include "made/graphics.h"

#include "common/file.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/events.h"

#include "audio/decoders/raw.h"
#include "audio/audiostream.h"

#include "graphics/surface.h"

namespace Made {

PmvPlayer::PmvPlayer(MadeEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer), _fd(nullptr), _surface(nullptr), frameData(nullptr), _audioStream(nullptr), soundDecoderData(nullptr) {}

PmvPlayer::~PmvPlayer() {
	// make sure to clean up any currently playing video at exit
	close();
}

bool PmvPlayer::load(const char* filename) {

	// open file. sometimes the script uses slashes for subdir,
	//  this should make it OS agnostic
	_fd = new Common::File();
	if (!_fd->open(Common::Path(filename, '\\'))) {
		warning("Failed to open movie file '%s'", filename);
		delete _fd;
		return false;
	}

	// expected IFF blocks at start of a PMV
	uint32 chunkType, chunkSize;

	readChunk(chunkType, chunkSize);	// "MOVE"
	if (chunkType != MKTAG('M','O','V','E')) {
		warning("Unexpected PMV video header, expected 'MOVE'");
		delete _fd;
		return false;
	}

	readChunk(chunkType, chunkSize);	// "MHED"
	if (chunkType != MKTAG('M','H','E','D')) {
		warning("Unexpected PMV video header, expected 'MHED'");
		delete _fd;
		return false;
	}

	frameDelay = _fd->readUint16LE();
	_fd->skip(4);	// always 0?
	frameCount = _fd->readUint16LE();
	_fd->skip(4);	// always 0?

	soundFreq = _fd->readUint16LE();
	// Note: There seem to be weird sound frequencies in PMV videos.
	// Not sure why, but leaving those original frequencies intact
	// results to sound being choppy. Therefore, we set them to more
	// "common" values here (11025 instead of 11127 and 22050 instead
	// of 22254)
	if (soundFreq == 11127)
		soundFreq = 11025;

	if (soundFreq == 22254)
		soundFreq = 22050;

	for (int i = 0; i < 22; i++) {
		int unk = _fd->readUint16LE();
		debug(2, "%i ", unk);
	}

	// Read and set initial palette
	_fd->read(_paletteRGB, 768);
	_vm->_screen->setRGBPalette(_paletteRGB);

	// TODO: Sound can still be a little choppy. A bug in the decoder or -
	// perhaps more likely - do we have to implement double buffering to
	// get it to work well?
	_mixer->stopAll();
	_audioStream = Audio::makeQueuingAudioStream(soundFreq, false);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_audioStreamHandle, _audioStream);

	soundDecoderData = new SoundDecoderData();

	// ready to go!
	frameNumber = 0;

	return true;
}

bool PmvPlayer::decode_frame() {
	// Decode one frame, updating 'destinations' with new contents

	uint32 chunkType, chunkSize;

	readChunk(chunkType, chunkSize);
	if (chunkType != MKTAG('M','F','R','M')) {
		warning("Unknown chunk type");
		return false;
	}

	// Only reallocate the frame data buffer if it needs to grow
	if (frameDataSize < chunkSize || !frameData) {
		delete[] frameData;

		frameDataSize = chunkSize;
		frameData = new byte[frameDataSize];
	}

	uint32 bytesRead = _fd->read(frameData, chunkSize);
	if (bytesRead < chunkSize || _fd->eos())
		return false;

	uint32 soundChunkOfs = READ_LE_UINT32(frameData + 8);
	uint32 imageDataOfs = READ_LE_UINT32(frameData + 12);
	uint32 palChunkOfs = READ_LE_UINT32(frameData + 16);

	// Handle audio
	if (soundChunkOfs) {
		byte *audioData = frameData + soundChunkOfs - 8;
		uint16 soundChunkSize = READ_LE_UINT16(audioData + 4);
		uint16 chunkCount = READ_LE_UINT16(audioData + 6);

		debug(2, "SOUND: chunkCount = %d; chunkSize = %d; total = %d\n", chunkCount, soundChunkSize, chunkCount * soundChunkSize);

		uint32 soundSize = chunkCount * soundChunkSize;
		byte *soundData = (byte *)malloc(soundSize);
		decompressSound(audioData + 8, soundData, soundChunkSize, chunkCount, nullptr, soundDecoderData);
		_audioStream->queueBuffer(soundData, soundSize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
	}

	// Handle palette changes
	if (palChunkOfs) {
		byte *palData = frameData + palChunkOfs - 8;
		uint32 palSize = READ_LE_UINT32(palData + 4);
		decompressPalette(palData + 8, _paletteRGB, palSize);
		_vm->_screen->setRGBPalette(_paletteRGB);
	}

	// Handle video
	if (imageDataOfs) {
		byte *imageData = frameData + imageDataOfs - 8;

		// frameNum @0
		uint32 imageChunkSize = READ_LE_UINT32(imageData) + 4;
		// uint32 unknown = READ_LE_UINT32(imageData); // zero?
		uint16 width = READ_LE_UINT16(imageData + 8);
		uint16 height = READ_LE_UINT16(imageData + 10);

		uint16 cmdOffs = READ_LE_UINT16(imageData + 12);
		uint16 cmdFlags = READ_LE_UINT16(imageData + 14);

		uint16 pixelOffs = READ_LE_UINT16(imageData + 16);
		uint16 pixelFlags = READ_LE_UINT16(imageData + 18);

		uint16 maskOffs = READ_LE_UINT16(imageData + 20);
		uint16 maskFlags = READ_LE_UINT16(imageData + 22);

		uint16 lineSize = READ_LE_UINT16(imageData + 24);

		debug(2, "width = %d; height = %d; cmdOffs = %04X; cmdFlags = %04X; pixelOffs = %04X; pixelFlags = %04X; maskOffs = %04X; maskFlags = %04X; lineSize = %d\n",
			  width, height, cmdOffs, cmdFlags, pixelOffs, pixelFlags, maskOffs, maskFlags, lineSize);

		if (!_surface) {
			_surface = new Graphics::Surface();
			_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
		}

		decompressMovieImage(imageData, *_surface, cmdOffs, pixelOffs, maskOffs,
							 pixelOffs - cmdOffs, maskOffs - pixelOffs, imageChunkSize - maskOffs, lineSize,
							 cmdFlags, pixelFlags, maskFlags);
		_vm->_system->copyRectToScreen(_surface->getPixels(), _surface->pitch,
									   (_vm->_screen->getWidth() - _surface->w) / 2, (_vm->_screen->getHeight() - _surface->h) / 2, _surface->w, _surface->h);
	}

	frameNumber++;

	return true;
}

void PmvPlayer::close() {
	// tear down video
	if (_surface)
		_surface->free();
	delete _surface;
	_surface = nullptr;

	delete[] frameData;
	frameData = nullptr;
	frameDataSize = 0;

	// tear down audio
	delete soundDecoderData;
	soundDecoderData = nullptr;
	if (_audioStream)
		_audioStream->finish();
	_mixer->stopHandle(_audioStreamHandle);
	// delete _audioStream;
	_audioStream = nullptr;

	// close file
	delete _fd;
	_fd = nullptr;
}

bool PmvPlayer::play(const char *filename) {
	bool aborted = false;

	if (load(filename)) {
		int32 pmvStartTime = _vm->getTotalPlayTime();

		while (!_vm->shouldQuit() && ! aborted && !_fd->eos() && frameNumber < frameCount) {
			// Decode and stage the next audio / video frame
			if (!decode_frame()) {
				break;
			}

			// delay until time has passed, then flip screen
			int32 delayTime = (frameNumber - 1) * frameDelay - (_vm->getTotalPlayTime() - pmvStartTime);
			if (delayTime < 0)
				warning("Video A/V sync broken - running behind %d ms (%d frames)!", -delayTime, (-delayTime / frameDelay) + 1);
			else
				g_system->delayMillis(delayTime);

			_vm->_system->updateScreen();

			// Check and handle events - user can press ESC to exit early
			Common::Event event;
			while (_vm->_system->getEventManager()->pollEvent(event)) {
				switch (event.type) {
				case Common::EVENT_KEYDOWN:
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
						aborted = true;
					break;
				default:
					break;
				}
			}
		}

		close();
	}

	return !aborted;
}

void PmvPlayer::readChunk(uint32 &chunkType, uint32 &chunkSize) {
	chunkType = _fd->readUint32BE();
	chunkSize = _fd->readUint32LE();

	debug(2, "ofs = %08X; chunkType = %c%c%c%c; chunkSize = %d\n",
		(int)_fd->pos(),
		(chunkType >> 24) & 0xFF, (chunkType >> 16) & 0xFF, (chunkType >> 8) & 0xFF, chunkType & 0xFF,
		chunkSize);

}

void PmvPlayer::decompressPalette(byte *palData, byte *outPal, uint32 palDataSize) {
	byte *palDataEnd = palData + palDataSize;
	while (palData < palDataEnd) {
		byte count = *palData++;
		byte entry = *palData++;
		if (count == 255 && entry == 255)
			break;
		memcpy(&outPal[entry * 3], palData, (count + 1) * 3);
		palData += (count + 1) * 3;
	}
}

}
