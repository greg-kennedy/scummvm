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

#include "adventuremaker2/metaengine.h"
#include "adventuremaker2/detection.h"
#include "adventuremaker2/adventuremaker2.h"

const char *AdventureMaker2MetaEngine::getName() const {
	return "adventuremaker2";
}

Common::Error AdventureMaker2MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new AdventureMaker2::AdventureMaker2Engine(syst, desc);
	return Common::kNoError;
}

bool AdventureMaker2MetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSavesUseExtendedFormat) ||
		(f == kSimpleSavesNames) ||
	    (f == kSupportsListSaves) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(ADVENTUREMAKER2)
REGISTER_PLUGIN_DYNAMIC(ADVENTUREMAKER2, PLUGIN_TYPE_ENGINE, AdventureMaker2MetaEngine);
#else
REGISTER_PLUGIN_STATIC(ADVENTUREMAKER2, PLUGIN_TYPE_ENGINE, AdventureMaker2MetaEngine);
#endif
