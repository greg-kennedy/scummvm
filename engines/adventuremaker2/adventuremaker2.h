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

#ifndef ADVENTUREMAKER2_ADVENTUREMAKER2_H
#define ADVENTUREMAKER2_ADVENTUREMAKER2_H

/*
#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
*/
#include "common/random.h"
#include "common/serializer.h"
/*
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
*/
#include "graphics/screen.h"

#include "adventuremaker2/detection.h"

#include "adventuremaker2/global.h"
#include "adventuremaker2/frame.h"
#include "adventuremaker2/cursor.h"

namespace AdventureMaker2 {

struct AdventureMaker2GameDescription;

class AdventureMaker2Engine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

protected:
	// Engine APIs
	Common::Error run() override;
public:
	// internal settings - public so Console can read them
	Common::String frameName;
	AdventureMaker2Global am2_global;
	Common::HashMap<Common::String, AdventureMaker2Frame> am2_frames;
	Common::HashMap<Common::String, CursorGroup> am2_icons;

	Graphics::Screen *_screen = nullptr;

	AdventureMaker2Engine(OSystem *syst, const ADGameDescription *gameDesc);
	~AdventureMaker2Engine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently() override {
		return true;
	}
	bool canSaveGameStateCurrently() override {
		return true;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}
};

extern AdventureMaker2Engine *g_engine;
#define SHOULD_QUIT ::AdventureMaker2::g_engine->shouldQuit()

} // namespace AdventureMaker2

#endif
