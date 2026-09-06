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

#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/str-array.h"
#include "common/translation.h"
#include "common/util.h"
#include "adventuremaker2/detection.h"
#include "adventuremaker2/detection_tables.h"

const DebugChannelDef AdventureMaker2MetaEngineDetection::debugFlagList[] = {
	{ AdventureMaker2::kDebugGraphics, "Graphics", "Graphics debug level" },
	{ AdventureMaker2::kDebugPath, "Path", "Pathfinding debug level" },
	{ AdventureMaker2::kDebugFilePath, "FilePath", "File path debug level" },
	{ AdventureMaker2::kDebugScan, "Scan", "Scan for unrecognised games" },
	{ AdventureMaker2::kDebugScript, "Script", "Enable debug script dump" },
	DEBUG_CHANNEL_END
};

AdventureMaker2MetaEngineDetection::AdventureMaker2MetaEngineDetection() : AdvancedMetaEngineDetection(AdventureMaker2::GAME_DESCRIPTIONS,
	sizeof(ADGameDescription), AdventureMaker2::GAME_NAMES)
{
	_flags = kADFlagMatchFullPaths;
}

REGISTER_PLUGIN_STATIC(ADVENTUREMAKER2_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AdventureMaker2MetaEngineDetection);
