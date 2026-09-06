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

#include "adventuremaker2/adventuremaker2.h"
#include "adventuremaker2/detection.h"
#include "adventuremaker2/console.h"

#include "adventuremaker2/cursor.h"
#include "adventuremaker2/resource.h"

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "common/timer.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/palette.h"
#include "graphics/wincursor.h"

namespace AdventureMaker2 {

// Timer management for a one-shot timer.
//  Sets a timer value to TRUE and removes itself.
struct timerStruct {
	bool val;
	Common::TimerManager *tm;
};

static void setBoolTrue(void *s) {
	((timerStruct *)s)->val = true;
	((timerStruct *)s)->tm->removeTimerProc(&setBoolTrue);
}

AdventureMaker2Engine *g_engine;

AdventureMaker2Engine::AdventureMaker2Engine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("AdventureMaker2") {
	g_engine = this;
}

AdventureMaker2Engine::~AdventureMaker2Engine() {
	delete _screen;
}

uint32 AdventureMaker2Engine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String AdventureMaker2Engine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error AdventureMaker2Engine::run() {

	/*
	// print to debug all files we found
	Common::ArchiveMemberList list;
	SearchMan.listMembers(list);
	for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it) {
		debug("%s", (*it)->getName().c_str());
	}
	*/

	// Read the General Config File
	debug("Going to open general config file in Config/General...");
	am2_global.read();

	// Read every Cursor (.ico, .cur) from the Icons/ folder
	debug("Going to read all Icons...");
	// these may be renamed as .002 or .003 as well
	Common::ArchiveMemberList allIcons;
	SearchMan.listMatchingMembers(allIcons, "icons/*.ico");
	SearchMan.listMatchingMembers(allIcons, "icons/*.002");
	for (Common::ArchiveMemberList::const_iterator it = allIcons.begin(); it != allIcons.end(); ++it) {
		const Common::String basename = (*it)->getName().substr(0, (*it)->getName().size() - 4) + ".ico";
		debug(("Reading icon '" + (*it)->getName() + "'").c_str());
		Common::SeekableReadStream *srs = (*it)->createReadStream();
		if (srs != nullptr) {
			am2_icons.setVal(basename, CursorGroup(*srs));
			delete srs;
		}
	}

	debug("Going to read all Cursors...");
	Common::ArchiveMemberList allCursors;
	SearchMan.listMatchingMembers(allCursors, "icons/*.cur");
	SearchMan.listMatchingMembers(allCursors, "icons/*.003");

	// load them all into an internal hash for access later
	for (Common::ArchiveMemberList::const_iterator it = allCursors.begin(); it != allCursors.end(); ++it) {
		const Common::String basename = (*it)->getName().substr(0, (*it)->getName().size() - 4) + ".cur";
		debug(("Reading cursor '" + (*it)->getName() + "'").c_str());
		Common::SeekableReadStream *srs = (*it)->createReadStream();
		if (srs != nullptr) {
			am2_icons.setVal(basename, CursorGroup(*srs));
			delete srs;
		}
	}

	// Read every Frame (.fme) from the Frames/ folder
	debug("Going to read all Frames...");

	Common::ArchiveMemberList allFrames;
	SearchMan.listMatchingMembers(allFrames, "frames/*.fme");

	for (Common::ArchiveMemberList::const_iterator it = allFrames.begin(); it != allFrames.end(); ++it) {
		const Common::String basename = (*it)->getName().substr(0, (*it)->getName().size() - 4);

		debug(("Reading frame '" + basename + "'").c_str());

		// create a new am2 frame
		AdventureMaker2Frame am2_frame;
		am2_frame.read(basename);

		// All done loading the frame.  Ready to place it in the hashmap.
		am2_frames.setVal(basename, am2_frame);
	}

	// 
	// Initialize graphics mode
	//  nullptr here means "use the native format of the backend"
	initGraphics(am2_global.frame_width, am2_global.frame_height, nullptr);
	_screen = new Graphics::Screen();

	// Set the engine's debugger console
	setDebugger(new Console(this));

	// Get the format the system was able to provide
	// in case it cannot support that format at our requested resolution
	Graphics::PixelFormat format = _system->getScreenFormat();

	if (format.bytesPerPixel == 1) {
		// Initialize palette to simulate RGB332, it's better than nothing

		byte palette[1024];
		byte *dst = palette;
		for (byte r = 0; r < 8; r++) {
			for (byte g = 0; g < 8; g++) {
				for (byte b = 0; b < 4; b++) {
					dst[0] = r << 5;
					dst[1] = g << 5;
					dst[2] = b << 6;
					dst[3] = 0;
					dst += 4;
				}
			}
		}

		g_system->getPaletteManager()->setPalette(palette, 0, 256);
	}

	// Create a mask to limit the color from exceeding the bitdepth
	// The result is equivalent to:
	// 	uint32 mask = 0;
	// 	for (int i = ourFormat.bytesPerPixel; i > 0; i--) {
	// 		mask <<= 8;
	// 		mask |= 0xFF;
	// 	}
	//uint32 mask = (1 << (format.bytesPerPixel << 3)) - 1;

	// the background buffer - in screen format
	Graphics::ManagedSurface background(am2_global.frame_width, am2_global.frame_height, _screen->format);

	// inventory window
	Graphics::ManagedSurface *invBackground = nullptr;
	Graphics::ManagedSurface *invButton = nullptr;
	if (am2_global.invFancyEnabled) {
		if (am2_global.invFancyCustomGfx && am2_global.invFancyBackground != "DEFAULT") {
			// this game uses custom inventory background (default: solid black)
			invBackground = loadImage("inventory/" + am2_global.invFancyBackground);
		}

		if (am2_global.invFancyCustomGfx && am2_global.invFancyButton != "DEFAULT") {
			// this game uses custom inventory button (default: default button)
			invButton = loadImage("inventory/" + am2_global.invFancyButton);
		} else {
			// TODO: put into .dat file
			invButton = loadImage("defaultInventory.png");
		}
	}

	// Additional setup.
//	debug("QuuxEngine::init");

	// Your main even loop should be (invoked from) here.
//	debug("QuuxEngine::go: Hello, World!");

	// This test will show up if -d1 and --debugflags=example are specified on the commandline
//	debugC(1, 1, "Example debug call");

	// This test will show up if --debugflags=example or --debugflags=example2 or both of them and -d3 are specified on the commandline
//	debugC(3, 3, "Example debug call two");

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	frameName = am2_global.initial_frame;

    Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
	CursorMan.replaceCursor(cursor);
	delete cursor;
	CursorMan.showMouse(true);

	while (!shouldQuit()) {
		// Transition into the scene
		AdventureMaker2Frame frame;

		if (!am2_frames.tryGetVal(frameName, frame)) {
			// Error!  Can't load this frame
			return Common::kReadingFailed;
		}

		// FRAME LOAD - Create background
		if (frame.bgIsImage) {
			// load the background image
			Graphics::ManagedSurface *bg = loadImage("frames/" + frame.bgFilename);
			if (bg) {
				// image loaded OK! neat.
				//  if image is Tiled it repeats over all bg
				// if not, it is stretched
				if (frame.bgTiled) {
					for (int y = 0; y < am2_global.frame_height; y += bg->h)
						for (int x = 0; x < am2_global.frame_width; x += bg->w)
							background.blitFrom(*bg, Common::Point(x, y));
				} else {
					background.blitFrom(*bg, Common::Rect(0, 0, bg->w, bg->h), Common::Rect(0, 0, am2_global.frame_width, am2_global.frame_height));
				}
				delete bg;
			}
		} else {
			// set every pixel to bgcolor
			background.clear(frame.bgColor);
		}

		// BLIT TEXT
		for (Common::Array<AdventureMaker2Label>::const_iterator it = frame.labels.begin(); it != frame.labels.end(); ++it)
		{
			const Graphics::Font *font = FontMan.getFontByName(it->font);
			// if we failed to find the font, fall back to the GUI Font at least.
			if (font == nullptr)
				font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
			if (font != nullptr)
				font->drawString(&background, it->text, it->x / 20, it->y / 20, am2_global.frame_width, it->color);
		}

		// LOAD AREA IMAGES
		Common::Array<Graphics::ManagedSurface *> areaGfx;
		for (Common::Array<AdventureMaker2Area>::const_iterator it = frame.areas.begin(); it != frame.areas.end(); ++it)
		{
			areaGfx.push_back(loadImage(it->pictureName));
		}

		//
		// The background is always displayed though it may be drawn over by the video.
		_screen->copyFrom(background);

		// Video plays at frame entry, if there is a filename.
		if (frame.videoFilename != "") {
			Video::AVIDecoder *avi = loadVideo("frames/" + frame.videoFilename);
			if (avi != nullptr) {
				avi->start();
				while (!avi->endOfVideo()) {
					const Graphics::Surface * aviFrame = avi->decodeNextFrame();

					// TODO: This incurs a copy from Surface -> cSurf::_innerSurface.
					//  Would be nice if we could directly set _innerSurface to aviFrame
					Graphics::ManagedSurface cSurf(aviFrame);
					//  also, we can't keep Palette unchanged, bummer
					cSurf.setPalette(avi->getPalette(), 0, 256);

					if (frame.videoFullscreen)
						_screen->blitFrom(cSurf);
					else
						_screen->blitFrom(cSurf, Common::Rect(frame.videoW, frame.videoH), Common::Point(frame.videoX, frame.videoY));

					_screen->update();
					g_system->delayMillis(avi->getTimeToNextFrame());
				}
				delete avi;
			}

			// reset background
			_screen->copyFrom(background);
		}

		// draw area images
//		for (Common::Array<AdventureMaker2Area>::const_iterator it = frame.areas.begin(); it != frame.areas.end(); ++it) {
		for (unsigned int i = 0; i < frame.areas.size(); i ++) {
			if (areaGfx[i] != nullptr)
				_screen->blitFrom(*(areaGfx[i]),
					Common::Rect(0, 0, areaGfx[i]->w, areaGfx[i]->h),
					Common::Rect(frame.areas[i].x, frame.areas[i].y, frame.areas[i].x + frame.areas[i].w, frame.areas[i].y + frame.areas[i].h));
		}
		_screen->update();

		// Once the video is done, we can install the frame advance timer if present.
		struct timerStruct frameTimer = { false, this->getTimerManager() };
		if (frame.advanceAuto) {
			frameTimer.tm->installTimerProc(&setBoolTrue, frame.advanceDelay * 1'000'000, &frameTimer, "AdventureMaker2::FrameTimer");
		}

		// Main loop
		bool loop = ! shouldQuit();
		while (loop)
		{

			if (frameTimer.val) {
				frameName = frame.advanceFrame;
				loop = false;
			} else
				loop = !shouldQuit();

			Common::Event e;
			while (g_system->getEventManager()->pollEvent(e)) {
				switch (e.type) {
					case Common::EVENT_QUIT:
					case Common::EVENT_RETURN_TO_LAUNCHER:
						loop = false;
						break;

					case Common::EVENT_LBUTTONDOWN:
						/*
						if (_leftButtonDownFl) {
							_curChoice = getMouseHiLite();

							if (_curChoice >= 0 && _curChoice < _scenes[_curSceneIdx]._decisionChoices) {
								debugC(5, kDebugGeneral, "Accepting mouse click with choice = %d", _curChoice);
								_totScore += _scenes[_curSceneIdx]._choices[_curChoice]._points;
								_actions.push(ChangeScene);
								_leftButtonDownFl = false;
							}
						} else if (_console->_allowSkip && _timerInstalled) {
							// Allows to skip speech by skipping wait delay
							onTimer(this);
						}
						*/
						break;
					case Common::EVENT_KEYDOWN:
						if (e.kbd.keycode == Common::KEYCODE_q) {
						}
						break;
					default:
						break;
				}
			}

			// Delay for a bit. All events loops should have a delay
			// to prevent the system being unduly loaded
			// 60fps
			_screen->update();

			g_system->delayMillis(1000 / 60);
		}
	}

	// clean up inventory graphics
	delete invButton;
	delete invBackground;

	//	delete background;

	return Common::kNoError;
}

Common::Error AdventureMaker2Engine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // namespace AdventureMaker2
