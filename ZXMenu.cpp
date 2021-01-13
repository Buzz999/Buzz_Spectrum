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

#include "ZXCfg.h"
#include "ZXMenu.h"
#include "ZXFiles.h"
#include "ZXSpectrum.h"
#include "ZXVPad.h"

#include <assert.h>

using namespace std;

ZXMenu::ZXMenu () : _currentMainButtonIndex(0), _currentPreviewButtonIndex(0), _actionID (A_NONE), _state (S_MAIN_INIT), _level (L_MAIN),
_sound (true), _joystick (0)
{
    loadTextures ();
    createMainButtons ();
    createPreviewButtons ();

    _previewZXSpectrum = new ZXSpectrum ("data/spectrum.rom");
    if (!_font.loadFromFile("data/zx_spectrum-7.ttf")) {
        printf ("Unable to load the font\n");
    }
}

ZXMenu::~ZXMenu ()
{
    if (_previewZXSpectrum)
    {
        delete _previewZXSpectrum;
        _previewZXSpectrum = 0;
    }
}

bool ZXMenu::loadTexture (uint32_t tileNumber, const string &filename)
{
    sf::Texture texture;
    string path = "./icons/";
    string fullname = path + filename + ".png";
    bool result = texture.loadFromFile (fullname);
    if (result)
    {
        _mapOfTextures[tileNumber] = texture;
    }
    return result;
}

bool ZXMenu::loadTextures ()
{
    loadTexture (RESET_ICON, "reset");
    loadTexture (LOAD_ICON, "load");
    loadTexture (SAVE_ICON, "save");
    loadTexture (SOUND_ICON_ON, "sound_on");
    loadTexture (SOUND_ICON_OFF, "sound_off");
    loadTexture (JOYSTICK_KEMPSTON_ICON, "joystick_kempston");
    loadTexture (JOYSTICK_SINCLAIR_1_ICON, "joystick_sinclair_1");
    loadTexture (JOYSTICK_SINCLAIR_2_ICON, "joystick_sinclair_2");
    loadTexture (JOYSTICK_CURSOR_ICON, "joystick_cursor");
    loadTexture (SCREENSHOT_ICON, "screenshot");

    loadTexture (LEFT_ICON, "left");
    loadTexture (RIGHT_ICON, "right");
    loadTexture (RUN_ICON, "run");
    loadTexture (VALID_ICON, "valid");

    loadTexture (LEAVE_ICON, "leave");
    loadTexture (QUIT_ICON, "quit");

    loadTexture (LOGO_ICON, "logo");

    return true;
}


bool ZXMenu::createMainButtons ()
{
    uint32_t x = 48;
    uint32_t y = 208;

    ZButton breset (ZGeometry (x, y, 128, 128), RESET_ICON, A_RESET);
    breset.addMessage("Reset the ZX spectrum");

    x+= 128+8;
    ZButton bload (ZGeometry (x, y, 128, 128), LOAD_ICON, A_ENTER_PREVIEW);
    bload.addMessage("Browse the existing snapshots");
    x+= 128+8;
    ZButton bsave (ZGeometry (x, y, 128, 128), SAVE_ICON, A_SAVE);
    bsave.addMessage("Save the running context to a snapshot");
    x+= 128+8;
    ZButton bsound (ZGeometry (x, y, 128, 128), SOUND_ICON_ON, A_SOUND);
    bsound.addTileID(SOUND_ICON_OFF);
    bsound.addMessage("Toggle sound ON/OFF");
    x+= 128+8;

    y+= 128 + 8;
    x = 48;
    ZButton bjoy (ZGeometry (x, y, 128, 128), JOYSTICK_KEMPSTON_ICON, A_JOYSTICK);
    bjoy.addTileID(JOYSTICK_SINCLAIR_1_ICON);
    bjoy.addTileID(JOYSTICK_SINCLAIR_2_ICON);
    bjoy.addTileID(JOYSTICK_CURSOR_ICON);
    bjoy.addMessage("Cycle through joystick interfaces");

    x+= 128+8;
    ZButton bscreen (ZGeometry (x, y, 128, 128), SCREENSHOT_ICON, A_SCREENSHOT);
    bscreen.addMessage("Take a screenshot");
    x+= 128+8;
    ZButton bleave (ZGeometry (x, y, 128, 128), LEAVE_ICON, A_LEAVE);
    bleave.addMessage("Leave the menu");

    x+= 128+8;
    ZButton bquit (ZGeometry (x, y, 128, 128), QUIT_ICON, A_QUIT);
    bquit.addMessage("Quit the application");
    x+= 128+8;

    _mainButtons.push_back (breset);
    _mainButtons.push_back (bload);
    _mainButtons.push_back (bsave);
    _mainButtons.push_back (bsound);
    _mainButtons.push_back (bjoy);
    _mainButtons.push_back (bscreen);
    _mainButtons.push_back (bleave);
    _mainButtons.push_back (bquit);
    _mainButtons.push_back (bquit);

    return true;
}

bool ZXMenu::createPreviewButtons ()
{
    uint32_t y = 208 + 128 + 8;
    uint32_t x = 48;
    ZButton bleft (ZGeometry (x, y, 128, 128), LEFT_ICON, A_LEFT);
    bleft.addMessage("Previous snapshot");
    x+= 128+8;
    ZButton bright (ZGeometry (x, y, 128, 128), RIGHT_ICON, A_RIGHT);
    bright.addMessage("Next snapshot");
    x+= 128+8;
    ZButton bload (ZGeometry (x, y, 128, 128), RUN_ICON, A_LOAD);
    bload.addMessage("Load the selected snapshot");
    x+= 128+8;
    ZButton bleave (ZGeometry (x, y, 128, 128), LEAVE_ICON, A_LEAVE_PREVIEW);
    bleave.addMessage("Go back to the main menu");
    x+= 128+8;

    _previewButtons.push_back (bleft);
    _previewButtons.push_back (bright);
    _previewButtons.push_back (bload);
    _previewButtons.push_back (bleave);

    return true;
}

void ZXMenu::restartMenu () {
    _currentMainButtonIndex = 0;
    _currentPreviewButtonIndex = 0;
    _actionID = A_NONE;
    _state = S_MAIN_INIT;
    _level = L_MAIN;
}

ACTION_ID ZXMenu::process (ZXVPad &zxpad) {
    ACTION_ID actionID = A_NONE;

    switch (_state)
    {
    // Main MENU
    case S_MAIN_INIT:
    {
        std::vector<ZButton>::iterator it;
        for (it = _mainButtons.begin(); it != _mainButtons.end(); ++it)
        {
            ZButton &b = *it;
            b.setHighlight(false);
        }
        _currentMainButtonIndex = 0;
        ZButton &cb = _mainButtons.at(_currentMainButtonIndex);
        cb.setHighlight(true);
        _message = cb.getMessage();

        _state = S_MAIN_IDLE;
        _level = L_MAIN; // MAIN MENU
    }
    break;

    case S_MAIN_IDLE: {
        if (zxpad.isRight()) _state = S_MAIN_RIGHT_PRESSED;  // RIGHT
        else if (zxpad.isLeft()) _state = S_MAIN_LEFT_PRESSED;  // LEFT
        else if (zxpad.isEnter()) _state = S_MAIN_ENTER_PRESSED; // ENTER
    }
    break;

    case S_MAIN_RIGHT_PRESSED: {
           assert (!_mainButtons.empty()); // GUARD

            _mainButtons.at(_currentMainButtonIndex).setHighlight(false);
            if (_currentMainButtonIndex + 1 < _mainButtons.size()) _currentMainButtonIndex++;
            else _currentMainButtonIndex = 0;

            ZButton &cb = _mainButtons.at(_currentMainButtonIndex);
            cb.setHighlight(true);
            _message = cb.getMessage();
            _state = S_MAIN_WAIT_RIGHT_RELEASED;
    }
        break;

    case S_MAIN_WAIT_RIGHT_RELEASED:
        if (!zxpad.isRight()) _state = S_MAIN_IDLE;
        break;

    case S_MAIN_LEFT_PRESSED: {
             assert (!_mainButtons.empty()); // GUARD
            _mainButtons.at(_currentMainButtonIndex).setHighlight(false);
            if (_currentMainButtonIndex > 0) _currentMainButtonIndex--;
            else _currentMainButtonIndex = _mainButtons.size() - 1;

            ZButton &cb = _mainButtons.at(_currentMainButtonIndex);
            cb.setHighlight(true);
            _message = cb.getMessage();

            _state = S_MAIN_WAIT_LEFT_RELEASED;
    }
        break;

    case S_MAIN_WAIT_LEFT_RELEASED:
        if (!zxpad.isLeft()) _state = S_MAIN_IDLE;
        break;

    case S_MAIN_ENTER_PRESSED: {
            _state = S_MAIN_WAIT_ENTER_RELEASED;

            ZButton &b = _mainButtons.at(_currentMainButtonIndex);
            b.cycleTile();

            actionID = b.getActionID();

            if (actionID == A_JOYSTICK) {
                _joystick = b.getTileIndex(); // Keeping the Joystick ID
                // The action is performed outside
            }
            else if (actionID == A_SOUND) {
                _sound = !(b.getTileIndex()); // ICON_0 = ON, ICON_1 = OFF
                // The action is performed outside
            }
            else if (actionID == A_ENTER_PREVIEW) _state = S_PREVIEW_INIT;  // Go to sub menu

        }

        break;

    case S_MAIN_WAIT_ENTER_RELEASED:
        if (!zxpad.isEnter()) _state = S_MAIN_IDLE;
        break;

    // Preview Menu
    case S_PREVIEW_INIT: {
        _level = L_PREVIEW; // For the Display
        _state = S_PREVIEW_IDLE;

        std::vector<ZButton>::iterator it;
        for (it = _previewButtons.begin(); it != _previewButtons.end(); ++it)
        {
            ZButton &b = *it;
            b.resetCycleTile();
            b.setHighlight(false);
        }
        _currentPreviewButtonIndex = 0;
        ZButton &cb = _previewButtons.at(_currentPreviewButtonIndex);
        cb.setHighlight(true);
        _message = cb.getMessage();

        _zxFiles.browseSnaFiles ("sna/"); // DO NOT FORGET the '/'
        _previewZXSpectrum->loadSNAFile(_zxFiles.getCurrentSnaFile());

    }
    break;

    case S_PREVIEW_IDLE: {
        ZXInputs dummy;
        _previewZXSpectrum->run(dummy);

        if (zxpad.isRight()) _state = S_PREVIEW_RIGHT_PRESSED;  // RIGHT
        else if (zxpad.isLeft()) _state = S_PREVIEW_LEFT_PRESSED;  // LEFT
        else if (zxpad.isEnter()) _state = S_PREVIEW_ENTER_PRESSED; // ENTER
    }
    break;

    case S_PREVIEW_RIGHT_PRESSED: {
        _state = S_PREVIEW_WAIT_RIGHT_RELEASED;

        assert (!_previewButtons.empty()); // GUARD

        _previewButtons.at(_currentPreviewButtonIndex).setHighlight(false);
        if (_currentPreviewButtonIndex + 1 < _previewButtons.size())  _currentPreviewButtonIndex++;
        else _currentPreviewButtonIndex = 0;

        ZButton &cb = _previewButtons.at(_currentPreviewButtonIndex);
        cb.setHighlight(true);
        _message = cb.getMessage();
    }
    break;

    case S_PREVIEW_WAIT_RIGHT_RELEASED:
        if (!zxpad.isRight()) _state = S_PREVIEW_IDLE;
        break;

    case S_PREVIEW_LEFT_PRESSED: {
       _state = S_PREVIEW_WAIT_LEFT_RELEASED;

       assert (!_previewButtons.empty()); // GUARD
       _previewButtons.at(_currentPreviewButtonIndex).setHighlight(false);
       if (_currentPreviewButtonIndex > 0) _currentPreviewButtonIndex--;
       else _currentPreviewButtonIndex = _previewButtons.size() - 1;

        ZButton &cb = _previewButtons.at(_currentPreviewButtonIndex);
        cb.setHighlight(true);
        _message = cb.getMessage();
    }
    break;

    case S_PREVIEW_WAIT_LEFT_RELEASED:
        if (!zxpad.isLeft()) _state = S_PREVIEW_IDLE;
    break;

    case S_PREVIEW_ENTER_PRESSED: {
            _state = S_PREVIEW_WAIT_ENTER_RELEASED;

            ZButton &b = _previewButtons.at(_currentPreviewButtonIndex);
            b.cycleTile();

            actionID = b.getActionID();
            if (actionID == A_LEFT) {
                _zxFiles.selectPreviousSnaFile ();
                _previewZXSpectrum->loadSNAFile(_zxFiles.getCurrentSnaFile());
            }
            else if (actionID == A_RIGHT) {
                _zxFiles.selectNextSnaFile ();
                _previewZXSpectrum->loadSNAFile(_zxFiles.getCurrentSnaFile());
            }
            else if (actionID == A_LEAVE_PREVIEW) _state = S_MAIN_INIT; // Going back to the main menu
        }
    break;

    case S_PREVIEW_WAIT_ENTER_RELEASED:
        if (!zxpad.isEnter()) _state = S_PREVIEW_IDLE;
    break;

    default: // GUARD
        _state = S_MAIN_INIT;
        break;
    }

    _actionID = actionID; // Keeping the last one
    return _actionID;
}

void ZXMenu::display (sf::RenderWindow &window)
{
    window.clear (sf::Color::Black);

    if (_level == L_MAIN){
        sf::Sprite s;
        sf::Texture t = _mapOfTextures[LOGO_ICON];
        s.setTexture(t);
        s.setPosition (0, 0);
        window.draw (s);

        uint32_t n = _mainButtons.size();
        for (uint32_t i = 0; i < n; ++i)
        {
            ZButton &b = _mainButtons.at(i);
            const ZGeometry &g = b.getGeometry ();
            uint32_t idtexture = b.getTileID ();
            sf::Sprite sp;
            sf::Texture t = _mapOfTextures[idtexture];
            sp.setTexture(t);
            sp.setPosition (g._x, g._y);
            window.draw (sp);

            if (b.isHighlighted())
            {
                sf::RectangleShape rect (sf::Vector2f(g._w, g._h));
                rect.setOutlineColor (sf::Color (255, 0, 0));
                rect.setFillColor (sf::Color::Transparent);
                rect.setOutlineThickness(1);
                rect.setPosition (g._x, g._y);
                window.draw (rect);
            }
        }
   }

    else if (_level == L_PREVIEW)
    {
        // Buttons
        uint32_t n = _previewButtons.size();
        for (uint32_t i = 0; i < n; ++i)
        {
            ZButton &b = _previewButtons.at(i);
            const ZGeometry &g = b.getGeometry ();
            uint32_t idtexture = b.getTileID ();
            sf::Sprite sp;
            sf::Texture t = _mapOfTextures[idtexture];
            sp.setTexture(t);
            sp.setPosition (g._x, g._y);
            window.draw (sp);

            if (b.isHighlighted())
            {
                sf::RectangleShape rect (sf::Vector2f(g._w, g._h));
                rect.setOutlineColor (sf::Color (255, 0, 0));
                rect.setFillColor (sf::Color::Transparent);
                rect.setOutlineThickness(1);
                rect.setPosition (g._x, g._y);
                window.draw (rect);
            }
        }

        // Spectrum Preview
        sf::Sprite zx;
        sf::Texture tzx;
        tzx.loadFromImage(_previewZXSpectrum->getOffScreen());
        zx.setTexture(tzx);
        zx.setPosition (128, 16);
        zx.scale (1.5f, 1.5f);
        window.draw (zx);
    }

    // Help info
    if (!_message.empty()) {
        sf::Text tmsg (_message, _font);
        tmsg.setCharacterSize(32);

        sf::Vector2f msgPosition;
        if (_level == L_MAIN) msgPosition = sf::Vector2f(44, 132);
        else msgPosition = sf::Vector2f(44, 304);
        tmsg.setPosition(msgPosition);
        tmsg.setFillColor(sf::Color::Yellow);
        window.draw(tmsg);
    }
    window.display();
}
