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

#ifndef ADVENTUREMAKER2_DETECTION_H
#define ADVENTUREMAKER2_DETECTION_H

#include "engines/advancedDetector.h"

namespace AdventureMaker2 {

enum AdventureMaker2DebugChannels {
	kDebugGraphics = 1 << 0,
	kDebugPath     = 1 << 1,
	kDebugScan     = 1 << 2,
	kDebugFilePath = 1 << 3,
	kDebugScript   = 1 << 4
};

extern const PlainGameDescriptor GAME_NAMES[];

extern const ADGameDescription GAME_DESCRIPTIONS[];

} // namespace AdventureMaker2

class AdventureMaker2MetaEngineDetection : public AdvancedMetaEngineDetection {
	static const DebugChannelDef debugFlagList[];

public:
	AdventureMaker2MetaEngineDetection();
	~AdventureMaker2MetaEngineDetection() override {}

	const char *getName() const override {
		return "adventuremaker2";
	}

	const char *getEngineName() const override {
		return "AdventureMaker2";
	}

	const char *getOriginalCopyright() const override {
		return "AdventureMaker2 (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif
