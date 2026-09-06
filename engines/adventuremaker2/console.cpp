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

#include "adventuremaker2/console.h"

namespace AdventureMaker2 {

Console::Console(AdventureMaker2Engine *vm) : _vm(vm), GUI::Debugger() {
	registerCmd("state", WRAP_METHOD(Console, Cmd_state));
	registerCmd("global", WRAP_METHOD(Console, Cmd_global));
	registerCmd("var",    WRAP_METHOD(Console, Cmd_var));
}

Console::~Console() {
}

// prints current state
bool Console::Cmd_state(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Current frame: '%s'\n", _vm->frameName.c_str());
	} else {
		debugPrintf("Usage: %s\n", argv[0]);
		debugPrintf("Prints current state of the game.\n");
	}
	return true;
}

// prints global config
bool Console::Cmd_global(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("%s", _vm->am2_global.dump().c_str());
	} else {
		debugPrintf("Usage: %s\n", argv[0]);
		debugPrintf("Dumps loaded am2 global config to debug screen.\n");
	}
	return true;
}

bool Console::Cmd_var(int argc, const char **argv) {
	if (argc == 2) {
		if (scumm_stricmp(argv[1], "dump")) {

		} else {
			debugPrintf("Usage: %s [dump|set|clear] [varname]\n", argv[0]);
			debugPrintf("Command for working with variables.\n");
		}
	} else if (argc == 3) {
		if (scumm_stricmp(argv[1], "set")) {
		} else if (scumm_stricmp(argv[1], "clear")) {
		} else {
			debugPrintf("Usage: %s [dump|set|clear] [varname]\n", argv[0]);
			debugPrintf("Command for working with variables.\n");
		}
	} else {
		debugPrintf("Usage: %s [dump|set|clear] [varname]\n", argv[0]);
		debugPrintf("Command for working with variables.\n");
	}
	return true;
}

} // namespace AdventureMaker2
