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

#ifndef ADVENTUREMAKER2_GLOBAL_H
#define ADVENTUREMAKER2_GLOBAL_H

#include "adventuremaker2/vb50file.h"

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/array.h"

namespace AdventureMaker2 {

class AdventureMaker2Global {
public:
	void read();
	Common::String dump();

	Common::String initial_frame;
	long frame_width, frame_height;

	// Inventory appearance
	long invHSpacing, invVSpacing;
	bool invFancyEnabled;

	// "advanced" inventory settings
	bool invFancySlideIn, invFancySlideOut, invFancyHideEmpty, invFancyAutoAppear;
	bool invFancyCustomGfx;
	Common::String invFancyButton, invFancyBackground;

	Common::Array<Common::String> text;
	Common::HashMap<Common::String, bool> variables;


private:
	void readInfo(Common::SeekableReadStream *f);

	void readGeneral(VB50File &f);
	void readVariables(VB50File &f);
	void readTexts(VB50File &f);
	void readOptions(VB50File &f);
	void readDistribution(VB50File &f);
	void readVersion(VB50File &f);
};

} // namespace AdventureMaker2

#endif
