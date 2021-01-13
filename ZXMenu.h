/*
 *  Copyright (C) 2008-2018 Florent Bedoiseau (electronique.fb@free.fr)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma once

#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "ZXCfg.h"
#include "ZXInputs.h"
#include "ZXFiles.h"

class ZXFiles;
class ZXULA;
class ZXInput;
class ZXSpectrum;

#define RESET_ICON 0
#define LOAD_ICON 1
#define SAVE_ICON 2
#define SOUND_ICON_ON 30
#define SOUND_ICON_OFF 31
#define JOYSTICK_KEMPSTON_ICON 40
#define JOYSTICK_SINCLAIR_1_ICON 41
#define JOYSTICK_SINCLAIR_2_ICON 42
#define JOYSTICK_CURSOR_ICON 43
#define SCREENSHOT_ICON 5
#define LEFT_ICON 61
#define RIGHT_ICON 62
#define RUN_ICON 63
#define VALID_ICON 64

#define LEAVE_ICON 7
#define QUIT_ICON 8
#define LOGO_ICON 9

#if 0
bool createDir (const std::string &directory) {
    struct stat a_stat;
    int error = 0;
    if (! (error = stat(directory.c_str(), &a_stat))) {
      if (! (a_stat.st_mode & S_IFDIR)) {
        return false; // Exist already
      }
    } else {
      error = mkdir(directory.c_str(), 0777);
      if (error) return false;
    }

    return true;
}
#endif // 0

class ZGeometry {
public:
	ZGeometry () : _x (0), _y (0), _w(0), _h(0) {}
	ZGeometry (uint32_t x, uint32_t y, uint32_t w, uint32_t h) : _x (x), _y (y), _w(w), _h(h) { }
    ~ZGeometry () { }

	void copy (const ZGeometry &copied) {
		_x = copied._x;
		_y = copied._y;
		_w = copied._w;
		_h = copied._h;
	}

	ZGeometry (const ZGeometry& geo) {
		copy (geo);
	}

	ZGeometry &operator=(const ZGeometry &geo) {
		if (&geo == this) return *this;
		copy (geo);
		return *this;
	}

	uint32_t _x, _y;
	uint32_t _w, _h;
};

// NONE: Nothing requested
// RESET : Need to reset the spectrum
// LOAD : Load a snapshot
// SAVE : Save the running context
// SOUND :
// JOYSTICK :
// SCREENSHOT : Take a screenshot
// LEAVE : Leave the menu mode
// QUIT : Quit the emulator
enum ACTION_ID { A_NONE, A_RESET, A_ENTER_PREVIEW, A_SAVE, A_SOUND, A_JOYSTICK, A_SCREENSHOT, A_LEAVE, A_LEFT, A_RIGHT, A_LOAD, A_LEAVE_PREVIEW, A_QUIT };

class ZButton {
public:
    ZButton () : _actionID(A_NONE), _msg(""), _highlight (false), _currentTileIndex (0) { }
    ZButton (const ZGeometry &geo, uint32_t firstTileID, ACTION_ID actionID) : _actionID(actionID),  _geometry(geo), _highlight (false), _currentTileIndex (0) {
         _tilesID.push_back (firstTileID);
    }

    ~ZButton () {}

   	void copy (const ZButton &copied) {
   	    _actionID = copied._actionID;
   	    _msg = copied._msg;
        _geometry = copied._geometry;
        _tilesID = copied._tilesID;
        _highlight = copied._highlight;
        _currentTileIndex = copied._currentTileIndex;
	}

	ZButton (const ZButton& bt) { copy (bt); }

	ZButton &operator=(const ZButton &bt) {
		if (&bt == this) return *this;
		copy (bt);
		return *this;
	}

    const ZGeometry &getGeometry () const { return _geometry; }
    uint32_t getTileID () const { return _tilesID.at (_currentTileIndex); }
    uint32_t getTileIndex () const { return _currentTileIndex; }
    ACTION_ID getActionID () const { return _actionID; }
    const std::string &getMessage () const { return _msg; }

    void cycleTile () {
        if (_currentTileIndex + 1 < _tilesID.size()) _currentTileIndex++;
        else _currentTileIndex = 0;
    }

    void resetCycleTile () { _currentTileIndex = 0; }

    void setHighlight (bool value) { _highlight = value; }
    bool isHighlighted () const { return _highlight; }

    void addTileID (uint32_t tileID) { _tilesID.push_back (tileID); }
    void addMessage (const std::string &msg) { _msg = msg; }

protected:
    ACTION_ID _actionID;
    std::string _msg;

    ZGeometry _geometry;
    std::vector<uint32_t> _tilesID; // ID of the tiles

    bool _highlight;
    uint32_t _currentTileIndex;
};

class ZXMenu {
 public:
     // INIT : Restart positions and states of the buttons
     // IDLE : Manage mvts inside the menu buttons
     // An action is requested. Waiting 20ms
  enum MENU_STATE {
      S_MAIN_INIT,
      S_MAIN_IDLE,
      S_MAIN_RIGHT_PRESSED, S_MAIN_WAIT_RIGHT_RELEASED,
      S_MAIN_LEFT_PRESSED, S_MAIN_WAIT_LEFT_RELEASED,
      S_MAIN_ENTER_PRESSED, S_MAIN_WAIT_ENTER_RELEASED,

      S_PREVIEW_INIT,
      S_PREVIEW_IDLE,
      S_PREVIEW_RIGHT_PRESSED, S_PREVIEW_WAIT_RIGHT_RELEASED,
      S_PREVIEW_LEFT_PRESSED, S_PREVIEW_WAIT_LEFT_RELEASED,
      S_PREVIEW_ENTER_PRESSED, S_PREVIEW_WAIT_ENTER_RELEASED
      };

  enum MENU_LEVEL { L_MAIN, L_PREVIEW };

  ZXMenu ();
  ~ZXMenu ();

  void restartMenu ();

  ACTION_ID process (ZXVPad &zxpad);
  void display (sf::RenderWindow &window);

  ACTION_ID getActionID() const { return _actionID; }
  ACTION_ID popActionID () { ACTION_ID action = _actionID; _actionID = A_NONE; return action; }

  uint32_t getJoystick() const { return _joystick; }
  bool getSound() const { return _sound; }
  std::string getFilename () const { return _zxFiles.getCurrentSnaFile(); }
  std::string getScreenshotFilename () { return _zxFiles.getScreenshotFilename(); };
  std::string getSnapshotFilename () { return _zxFiles.getSnapshotFilename(); };
  const sf::Font &getFont() const { return _font; }

protected:
  std::vector<ZButton> _mainButtons;
  uint32_t _currentMainButtonIndex;
  std::vector<ZButton> _previewButtons;
  uint32_t _currentPreviewButtonIndex;

  ACTION_ID _actionID;
  MENU_STATE _state;
  MENU_LEVEL _level;
  std::string _message;

  ZXSpectrum *_previewZXSpectrum;

  bool _sound;
  uint32_t _joystick;

  std::map<uint32_t, sf::Texture> _mapOfTextures;

  bool loadTexture (uint32_t tileNumber, const std::string &filename);
  bool loadTextures ();
  bool createMainButtons ();
  bool createPreviewButtons ();

  ZXFiles _zxFiles;
  sf::Font _font;
};
