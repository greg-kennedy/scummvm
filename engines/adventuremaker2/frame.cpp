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

#include "adventuremaker2/frame.h"

#include "common/debug.h"

namespace AdventureMaker2 {

/* ************************************************************************* */
/* CHN / D file */
void AdventureMaker2Channel::readCHND(VB50File &f) {
	f.readString("CHN/D AM2 Version");
	// 21 dummy lines
	for (int i = 0; i < 21; i++)
		f.readString("\tDummy");

	// 8 sound channels
	for (int i = 0; i < 8; i++) {
		debug("Channel %d", i);
		noaction[i] = f.readBoolean("\tNo Action");
		stop[i] = f.readBoolean("\tStop Sound");
		play[i] = f.readBoolean("\tPlay Sound");
		filename[i] = f.readString("\tSound Filename");
		loop[i] = f.readNumber("\tLoop");
	}
}

Common::String AdventureMaker2Channel::dump() {
	Common::String ret = "";
	for (int i = 0; i < 8; i++)
		ret += Common::String::format("CHANNEL %d\n\tnoaction=%s\n\tstop=%s\n\tplay=%s\n\tfilename='%s'\n\tloop=%d\n",
									  i, (noaction[i] ? "true" : "false"), (stop[i] ? "true" : "false"), (play[i] ? "true" : "false"), filename[i].c_str(), loop[i]);

	return ret;
}

/* ************************************************************************* */
/*  LABEL */
void AdventureMaker2Label::readLBL(VB50File &f) {
	text = f.readString("Text");
	x = f.readNumber("x");
	y = f.readNumber("y");
	bold = f.readBoolean("Bold");
	italic = f.readBoolean("Italic");
	font = f.readString("Font Face");
	size = f.readNumber("Font Size");
	strikethrough = f.readBoolean("Strikethrough");
	underline = f.readBoolean("Underline");
	color = f.readNumber("Color");
}

Common::String AdventureMaker2Label::dump() {
	return Common::String::format(
		"LABEL\n\ttext='%s'\n\tfont='%s'\n\tx=%d, y=%d\n\tsize=%d, font_color=%d\n\tbold=%d, italic=%d, strikethrough=%d, underline=%d",
		text.c_str(), font.c_str(), x, y, size, color, bold, italic, strikethrough, underline);
}

/* ************************************************************************* */
/*  AREA */
void AdventureMaker2Area::read(Common::String basename, int i) {
	Common::SeekableReadStream *srs;

	srs = SearchMan.createReadStreamForMember(Common::String::format("areas/%s/%da.dat", basename.c_str(), i));
	if (srs != nullptr) {
		VB50File f(srs);
		readA(f);
		delete srs;
	}
	srs = SearchMan.createReadStreamForMember(Common::String::format("areas/%s/%db.dat", basename.c_str(), i));
	if (srs != nullptr) {
		VB50File f(srs);
		readB(f);
		delete srs;
	}
	srs = SearchMan.createReadStreamForMember(Common::String::format("areas/%s/%dc.dat", basename.c_str(), i));
	if (srs != nullptr) {
		VB50File f(srs);
		readC(f);
		delete srs;
	}
	srs = SearchMan.createReadStreamForMember(Common::String::format("areas/%s/%dd.dat", basename.c_str(), i));
	if (srs != nullptr) {
		VB50File f(srs);
		readD(f);
		delete srs;
	}
}

void AdventureMaker2Area::readA(VB50File &f) {
	x = f.readNumber("x");
	y = f.readNumber("y");
	w = f.readNumber("w");
	h = f.readNumber("h");
	isAnimated = f.readNumber("isAnimated");
	hasPicture = f.readBoolean("Has Picture");
	pictureName = f.readString("Picture Filename");
	resizeToFit = f.readNumber("Resize to Fit");

	visibleOnlyIf = f.readNumber("Visible Only If...");
	int varsZeroCount = f.readNumber("Vars are False");
	visibleZeroVars.reserve(varsZeroCount);
	for (int i = 0; i < varsZeroCount; i++)
		visibleZeroVars.push_back(f.readString("\tvar"));
	int varsOneCount = f.readNumber("Vars are True");
	visibleOneVars.reserve(varsOneCount);
	for (int i = 0; i < varsZeroCount; i++)
		visibleOneVars.push_back(f.readString("\tvar"));
}

void AdventureMaker2Area::readB(VB50File &f) {
	f.readNumber("Dummy 1");
	f.readString("Dummy 2");
	f.readBoolean("Dummy 3");
	f.readBoolean("Dummy 4");
	f.readBoolean("Dummy 5");

	onClickShowMessage = f.readNumber("On Click Show Message");
	onClickMessage = f.readString("On Click Message Text");
	f.readString("Dummy 6");
	f.readString("Dummy 7");
	onClickHidePermanently = f.readNumber("On Click Hide Permanently");
	onClickCloseGame = f.readNumber("On Click Close Game");
	onClickGoToFrame = f.readNumber("On Click Go To Frame");
	onClickFrame = f.readString("On Click Frame Name");

	onClickStopMIDI = f.readNumber("On Click Stop MIDI");
	onClickStopWAV = f.readNumber("On Click Stop WAV");
	onClickPlayVideo = f.readNumber("On Click Play Video");

	videoFilename = f.readString("Video to play");
	videoFullscreen = f.readBoolean("Play video full-screen"); // bool
	videoUseCoords = f.readBoolean("Use video coordinates");   // bool
	videoX = f.readNumber("Video LEFT");                       // int
	videoY = f.readNumber("Video TOP");                        // int
	videoW = f.readNumber("Video WIDTH");                      // int
	videoH = f.readNumber("Video HEIGHT");                     // int

	onClickStopMP3 = f.readNumber("On Click Stop MP3");
	onClickPlayMIDI = f.readNumber("On Click Play MIDI");
	onClickMIDI = f.readString("On Click MIDI Name");
	onClickPlayMP3 = f.readNumber("On Click Play MP3");
	onClickMP3 = f.readString("On Click MP3 Name");
	f.readString("Dummy 8");
	f.readString("Dummy 9");
	f.readString("Dummy 10");
	onClickGoBack = f.readNumber("On Click Go Back");
	f.readString("Dummy 11");
	f.readString("Dummy 12");
	f.readString("Dummy 13");
	onClickMessageDest = f.readNumber("On Click Message Destination");
	f.readString("Dummy 14");
	f.readString("Dummy 15");
	f.readString("Dummy 16");
	onClickChangeVariables = f.readNumber("On Click Change Variables");

	int count;

	count = f.readNumber("Clear variables count");
	varsClear.reserve(count);
	for (int i = 0; i < count; i++)
		varsClear.push_back(f.readString("\tclear:"));

	count = f.readNumber("Set variables count");
	varsSet.reserve(count);
	for (int i = 0; i < count; i++)
		varsSet.push_back(f.readString("\tset:"));

	count = f.readNumber("Toggle variables count");
	varsFlip.reserve(count);
	for (int i = 0; i < count; i++)
		varsFlip.push_back(f.readString("\ttoggle:"));

	onClickMIDIRepeatForever = f.readBoolean();
}

void AdventureMaker2Area::readC(VB50File &f) {
	onClickGiveItem = f.readNumber("Give item on click?");
	onClickItemName = f.readString("Item to give when clicked");
	f.readString("Dummy 1");
	f.readString("Dummy 2");
	animationDelay = f.readNumber("GIF animation frame delay (ms)");
}

void AdventureMaker2Area::readD(VB50File &f) {
	channels.readCHND(f);
}

/* ************************************************************************* */
/*  FRAME */
void AdventureMaker2Frame::read(Common::String basename) {
	Common::SeekableReadStream *srs;

	// Populate it with data by calling read() methods
	// open the .fme file, which is the Frame definition
	srs = SearchMan.createReadStreamForMember("frames/" + basename + ".fme");
	if (srs != nullptr) {
		VB50File f(srs);
		readFME(f);
		delete srs;
	}

	// optionally, there could be a .chn and a .lks
	srs = SearchMan.createReadStreamForMember("frames/" + basename + ".chn");
	if (srs != nullptr) {
		VB50File f(srs);
		readCHN(f);
		delete srs;
	}
	srs = SearchMan.createReadStreamForMember("frames/" + basename + ".lks");
	if (srs != nullptr) {
		VB50File f(srs);
		readLKS(f);
		delete srs;
	}

	// and lastly each Frame may have 0+ Areas and 0+ Labels.
	// Areas first. We have a count from the LKS (above).
	for (uint i = 0; i < areas.size(); ++i) {
		areas[i].read(basename, i);
	}

	// Labels next.  Unfortunately there is not a count, so we will just
	//  search for matching text strings instead.
	Common::ArchiveMemberList allLabels;
	SearchMan.listMatchingMembers(allLabels, "areas/" + basename + "/text##.lbl");
	if (allLabels.size() > 0) {
		labels.resize(allLabels.size());
		uint i = 0;
		for (Common::ArchiveMemberList::const_iterator it2 = allLabels.begin(); it2 != allLabels.end(); ++it2) {
			srs = (*it2)->createReadStream();
			if (srs != nullptr) {
				VB50File f(srs);
				labels[i].readLBL(f);
				delete srs;
			}
			++i;
		}
	}
}

void AdventureMaker2Frame::readFME(VB50File &f) {
	// Parse the frame file line-by-line and store useful items into the class.
	bgIsImage = f.readNumber("Frame Background");

	// EITHER solid color 0 (int) OR image filename 1
	if (bgIsImage)
		bgFilename = f.readString("Background Filename");
	else
		bgColor = f.readNumber("Background Color");

	bgTiled = f.readBoolean("Background Tiled"); // bool
	midiStop = f.readBoolean("Stop Playing MIDI"); // bool
	midiFilename = f.readString("MIDI Filename");

	advanceAuto = f.readBoolean("Advance to Next Frame Automatically"); // bool

	Common::String delay = f.readString("Delay (seconds)"); // string, but contains a float
	// convert the result to a double (floating-point)
	errno = 0;
	advanceDelay = strtod(delay.c_str(), nullptr);
	if (errno) {
		debug("error converting to double in AdventureMaker2Frame::readFME: strtod('%s'): %s", delay.c_str(), strerror(errno));
	}

	advanceFrame = f.readString("Name of next frame");

	videoFilename = f.readString("Video to play");
	midiWaitVideo = f.readBoolean("Delay MIDI until after video"); // bool

	wavStop = f.readBoolean("Stop any WAV sound on entry"); // bool

	mp3Stop = f.readBoolean("Stop any MP3 sound on entry"); // bool
	mp3Filename = f.readString("MP3 to play");

	f.readString("Dummy 1");
	f.readString("Dummy 2");
	f.readString("Dummy 3");

	videoFullscreen = f.readBoolean("Play video full-screen"); // bool
	videoUseCoords = f.readBoolean("Use video coordinates"); // bool
	videoX = f.readNumber("Video LEFT");  // int
	videoY = f.readNumber("Video TOP");            // int
	videoW = f.readNumber("Video WIDTH");  // int
	videoH = f.readNumber("Video HEIGHT");            // int

	midiLoop = f.readBoolean("Repeat MIDI forever"); // bool
}

void AdventureMaker2Frame::readLKS(VB50File &f) {
	// There is only one thing in the .lks file: the number of areas contained inside
	int area_count = f.readNumber("Linked Areas Count"); // int
	areas.resize(area_count);
}

void AdventureMaker2Frame::readCHN(VB50File &f) {
	channels.readCHND(f);
}

} // namespace AdventureMaker2
