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

#ifndef ADVENTUREMAKER2_CURSOR_H
#define ADVENTUREMAKER2_CURSOR_H

#include "graphics/managed_surface.h"
#include "common/stream.h"

namespace AdventureMaker2 {

/* Specialized loader for Windows .CUR / .ICO files */
class Cursor {
private:

	Graphics::ManagedSurface mapXOR;
	Graphics::ManagedSurface mapAND;

public:
//	Cursor() = default;
	Cursor(Common::SeekableReadStream &stream);

	void draw(Graphics::ManagedSurface &dest, Common::Point &p);
};

class CursorGroup {
private:
	Common::Array<Cursor> cursor;

public:
	CursorGroup() = default;
	CursorGroup(Common::SeekableReadStream &stream);

	void draw(Graphics::ManagedSurface &dest, Common::Point &p);
};

} // namespace AdventureMaker2

#endif
