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

#ifndef ADVENTUREMAKER2_FRAME_H
#define ADVENTUREMAKER2_FRAME_H

#include "common/array.h"
#include "common/str.h"

#include "adventuremaker2/vb50file.h"

namespace AdventureMaker2 {

class AdventureMaker2Channel {
public:
	Common::String filename[8];
	long loop[8];
	bool noaction[8], stop[8], play[8];

	void readCHND(VB50File &f);
	Common::String dump();
};

class AdventureMaker2Label {
public:
	Common::String text;
	Common::String font;
	int x, y;
	int size;
	int color;
	bool bold, italic, strikethrough, underline;

	void readLBL(VB50File &f);
	Common::String dump();
};

class AdventureMaker2Area {
public:
	// Area dimensions
	int x, y, w, h;

	// Area appearance
	Common::String pictureName;
	bool hasPicture, resizeToFit;
	bool isAnimated;
	int animationDelay;

	// Display logic
	bool visibleOnlyIf;
	Common::Array<Common::String> visibleZeroVars;
	Common::Array<Common::String> visibleOneVars;

	// ON CLICK ACTIONS
	// Variables
	bool onClickChangeVariables;
	Common::Array<Common::String> varsClear;
	Common::Array<Common::String> varsSet;
	Common::Array<Common::String> varsFlip;

	// Inventory
	bool onClickGiveItem;
	Common::String onClickItemName;

	// Message
	bool onClickShowMessage;
	Common::String onClickMessage;
	int onClickMessageDest;

	// Appearance
	bool onClickHidePermanently;

	// Transitions
	bool onClickCloseGame, onClickGoBack, onClickGoToFrame;
	Common::String onClickFrame;

	// Video
	bool onClickPlayVideo, videoFullscreen, videoUseCoords;
	Common::String videoFilename;
	int videoX, videoY, videoW, videoH;

	// Audio
	bool onClickStopMIDI, onClickStopWAV, onClickStopMP3;
	bool onClickPlayMIDI, onClickPlayMP3, onClickMIDIRepeatForever;
	Common::String onClickMIDI, onClickMP3;

	AdventureMaker2Channel channels;

	void read(Common::String, int);
	Common::String dump();

private:
	void readA(VB50File &f);
	void readB(VB50File &f);
	void readC(VB50File &f);
	void readD(VB50File &f);
};

class AdventureMaker2Frame {
public:
	// sub-objects on the frame
	Common::Array<AdventureMaker2Area> areas;
	Common::Array<AdventureMaker2Label> labels;
	AdventureMaker2Channel channels;

	// Background image / color / tiling
	bool bgIsImage;
	uint32 bgColor;
	Common::String bgFilename;
	bool bgTiled;

	// MIDI music
	bool midiStop, midiLoop, midiWaitVideo;
	Common::String midiFilename;

	// MP3 playback
	bool mp3Stop;
	Common::String mp3Filename;

	// WAV file (see also CHN file)
	bool wavStop;

	// Video playback
	bool videoFullscreen, videoUseCoords;
	Common::String videoFilename;
	int videoX, videoY, videoW, videoH;

	// Auto-advancement
	bool advanceAuto;
	Common::String advanceFrame;
	double advanceDelay;

	void read(Common::String);
	Common::String dump();

private:
	void readFME(VB50File &f);
	void readLKS(VB50File &f);
	void readCHN(VB50File &f);
};

} // namespace AdventureMaker2

#endif
