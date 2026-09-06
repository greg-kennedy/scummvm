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

#ifndef ADVENTUREMAKER2_VB50FILE_H
#define ADVENTUREMAKER2_VB50FILE_H

#include "common/str.h"
#include "common/stream.h"

namespace AdventureMaker2 {

/* Reader class for Visual Basic 5.0 created output files */
class VB50File : public Common::SeekableReadStream {
private:
	Common::SeekableReadStream *_s;

	bool skipWhitespace();

public:
	VB50File(Common::SeekableReadStream *s);

	// passthrough to underlying methods
	bool eos() const override { return _s->eos(); }
	uint32 read(void *dataPtr, uint32 dataSize) override { return _s->read(dataPtr, dataSize); }
	int64 pos() const override { return _s->pos(); }
	int64 size() const override { return _s->size(); }
	bool seek(int64 offset, int whence = SEEK_SET) override { return _s->seek(offset, whence); }

	// VB constructs on top
	//  These will also debug() the value, with a label
	Common::String readString(Common::String label = "");
	double readNumber(Common::String label = "");
	bool readBoolean(Common::String label = "");
};

} // namespace AdventureMaker2

#endif
