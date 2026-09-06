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

#include "adventuremaker2/global.h"

#include "common/debug.h"

namespace AdventureMaker2 {

void AdventureMaker2Global::read()
{
	Common::SeekableReadStream *srs;

	srs = SearchMan.createReadStreamForMember("config/general");
	if (srs != nullptr) {
		VB50File f(srs);
		this->readGeneral(f);
		delete srs;
	}
	srs = SearchMan.createReadStreamForMember("config/variables");
	if (srs != nullptr) {
		VB50File f(srs);
		this->readVariables(f);
		delete srs;
	}
	srs = SearchMan.createReadStreamForMember("config/texts");
	if (srs != nullptr) {
		VB50File f(srs);
		this->readTexts(f);
		delete srs;
	}

	/* these are all unused by anything so commented out */
	srs = SearchMan.createReadStreamForMember("config/info");
	if (srs != nullptr) {
		this->readInfo(srs);
		delete srs;
	}
	srs = SearchMan.createReadStreamForMember("config/version");
	if (srs != nullptr) {
		VB50File f(srs);
		this->readVersion(f);
		delete srs;
	}
	srs = SearchMan.createReadStreamForMember("config/options");
	if (srs != nullptr) {
		VB50File f(srs);
		this->readOptions(f);
		delete srs;
	}
	srs = SearchMan.createReadStreamForMember("config/distribution");
	if (srs != nullptr) {
		VB50File f(srs);
		this->readDistribution(f);
		delete srs;
	}
}

Common::String AdventureMaker2Global::dump() {
	// config settings
	Common::String ret = Common::String::format("Frame width: %ld\nFrame height: %ld\nInitial frame: '%s'\n",
												frame_width, frame_height, initial_frame.c_str());

	// text array
	ret += Common::String::format("Text count: %u\n", text.size());
	for (uint i = 0; i < text.size(); i++)
		ret += Common::String::format("\t%u: '%s'\n", i, text[i].c_str());

	return ret;
}

void AdventureMaker2Global::readGeneral(VB50File &f) {
	// Parse the config file line-by-line and store useful items into the class.
	f.readString("Project Name");
	f.readString("Project Path");
	f.readString("Software Title");

	f.readNumber("Frames Size Preset");

	initial_frame = f.readString("Initial frame");

	f.readString("Comments and Version");

	frame_width = f.readNumber("Frame width"); // int
	frame_height = f.readNumber("Frame height");           // int

	f.readString("Author Name");
	f.readString("Author Email");
	f.readString("Author Website");
	
	f.readNumber("Show option to automatically change resolution"); // int
	f.readString("Dummy 1");
	f.readString("Dummy 2");
	f.readString("Dummy 3");
	f.readString("Scheme");
	f.readString("Dummy 4");
	f.readString("Dummy 5");
	f.readBoolean("High quality single-channel"); // bool
	f.readBoolean("Hide the \"Load\" button"); // bool
	f.readBoolean("Hide the \"Save\" button");    // bool
	f.readNumber("Software Text");                // int
	f.readString("Dummy 6");
	f.readString("Dummy 7");
	invHSpacing = f.readNumber("Inventory horizontal spacing"); // int
	invVSpacing = f.readNumber("Inventory vertical spacing"); // int
	invFancyEnabled = f.readBoolean("Enable advanced inventory system"); // bool
	invFancySlideIn = f.readBoolean("Appear progressively"); // bool
	invFancySlideOut = f.readBoolean("Disappear progressively");             // bool
	invFancyHideEmpty = f.readBoolean("Invisible when empty");                    // bool
	invFancyAutoAppear = f.readBoolean("Automatically pop up when an item is added"); // bool
	invFancyCustomGfx = f.readBoolean("Custom graphics");                       // bool
	invFancyButton = f.readString("Inventory button");
	invFancyBackground = f.readString("Inventory background");
	f.readNumber("Change resolution value"); // int
	f.readString("Dummy 8");
	f.readString("Dummy 9");
	f.readString("Default Pointer");
}

void AdventureMaker2Global::readVariables(VB50File &f) {
	/* Variables (boolean) used by the project */
	int varCount = f.readNumber("Variables count");

	for (int i = 0; i < varCount; i++) {
		Common::String var_name = f.readString(Common::String::format("\tVariable %d", i).c_str());
		if (var_name != "") {
			variables.setVal(var_name, false);
		}
	}
};

void AdventureMaker2Global::readTexts(VB50File &f) {
	/* Text strings used by various things in the game. */
	f.readNumber("Text Preset Type"); // 0 = Applications (English), 1 = Games (English), 2 = Custom
	f.readString("Texts AM2 version");
	f.readString("Text Preset"); // if Custom (2) above
	f.readString("Dummy 1");
	f.readString("Dummy 2");
	f.readString("Dummy 3");

	int textCount = f.readNumber("Text count");

	text.reserve(textCount);
	for (int i = 0; i < textCount; i++) {
		text.push_back(f.readString(Common::String::format("\tText %d", i).c_str()));
	}
};

void AdventureMaker2Global::readInfo(Common::SeekableReadStream *f) {
	// the Info file is a bit unusual format and I don't really know what it's doing
	//  line 1 is a last-modified text string
	//  line 2 is six random lowercase letters
	//  line 3 is 13 or so random characters
	debug(("Info line 1: " + f->readLine()).c_str());
	debug(("Info line 2: " + f->readLine()).c_str());
	debug(("Info line 3: " + f->readLine()).c_str());
}

void AdventureMaker2Global::readVersion(VB50File &f) {
	/* Only a single line, listing the version of AM2 used to create the project */
	f.readString("Project AM2 version");
};

void AdventureMaker2Global::readOptions(VB50File &f) {
	/* "Options" seems to be an older version of Distribution - not included with newer projects */
	f.readString("Unknown String");
	f.readNumber("Unknown Int (1)");
	f.readString("Installer Background Color");
	f.readNumber("Unknown Int (2)");
	f.readNumber("Unknown Int (3)");
	f.readNumber("Unknown Int (4)");
	f.readNumber("Unknown Int (5)");
};

void AdventureMaker2Global::readDistribution(VB50File &f) {
	/* the Distribution file is used for creating the installer */
	f.readString("Dummy 1");
	f.readString("Dummy 2");
	f.readString("Dummy 3");
	f.readString("Distribution AM2 version");
	f.readString("Last Modified Date");
	f.readBoolean("Run from CD");
	f.readBoolean("Distribute on Floppy");
	f.readBoolean("UNprotect Resources");
	f.readString("Install Wizard Background Color");
	f.readString("Start Menu Icon");
	f.readString("README Filename");
	f.readBoolean("README in Start Menu");
	f.readString("License Agreement Filename");
	f.readString("Install Start Filename");
	f.readString("Install End Filename");
	f.readBoolean("Omit MSVBVM50.DLL");
	f.readBoolean("Omit Other DLL Files");
	f.readString("Dummy 4");
	f.readString("Install Wizard Language");
	f.readString("Dummy 5");

	int postInstallCount = f.readNumber("Post-Install Count");

	for (int i = 0; i < postInstallCount; i++) {
		f.readString(Common::String::format("\tPost-Install Command %d", i).c_str());
	}
};

} // namespace AdventureMaker2
