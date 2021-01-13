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

#include "ZXManager.h"
#include "ZXMenu.h"

#include <string>
using namespace std;

ZXManager::ZXManager () : _state (M_INIT) {
    _mainW.create(sf::VideoMode(640, 480), "ZX Spectrum 48k Emulator");
    _mainW.setFramerateLimit (50);

    loadTextures ();
}

ZXManager::~ZXManager () {
     if (_keyboardW.isOpen()) _keyboardW.close();
    _mainW.close();
}

void ZXManager::loadTextures () {
    string keyboardfile = "./icons/keyboard.png";
    bool res = _kbTexture.loadFromFile (keyboardfile);
    if (!res) {
        printf ("Unable to load %s file\n", keyboardfile.c_str());
    }

    string zxspectrumfile = "./icons/zxspectrum.png";
    res = _zxTexture.loadFromFile (zxspectrumfile);
    if (!res) {
        printf ("Unable to load %s file\n", zxspectrumfile.c_str());
    }
}

ZXManager::MGR_STATE ZXManager::process () {
   if (!_mainW.isOpen()) {
        _state = M_END;
        return _state;
   }

   // Main Window events
    sf::Event event;
    while (_mainW.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            _state = M_END;
            return _state;
        }
    }

    // Keyboard Window Events
    if (_keyboardW.isOpen()) {
        while (_keyboardW.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                _keyboardW.close();
            }
        }
    }

    // F1 : Menu
    // F2 : Open/Close Virtual Keyboard in a window
    // ESC : Quit

    switch (_state) {
    case M_INIT:
        _clock.restart();
        _state = M_SPLASH;
        break;

    case M_SPLASH:{
        // Skipping Splash
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return)) {
            while (sf::Keyboard::isKeyPressed(sf::Keyboard::Return));
            _zxMenu.restartMenu ();
            _state = M_MENU;
        }

        // Ending splash after 4 seconds
        sf::Time t = _clock.getElapsedTime ();
        if (t.asMilliseconds() >= 4000) _state = M_MENU;
    }
        break;

    case M_MENU: {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            while (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape));
            _state = M_RUN;
            break;
        }

        ACTION_ID action = _zxMenu.process(_zxVPad);

        // Going back to emulation directly
        if (action == A_LOAD || action == A_SAVE || action == A_LEAVE || action == A_RESET || action == A_SCREENSHOT) _state = M_RUN;

        // Leaving the emulator
        if (action == A_QUIT)  _state = M_END;
    }
        break;

    case M_RUN:
        // Virtual Keyboard
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F1)) {
            while (sf::Keyboard::isKeyPressed(sf::Keyboard::F1));
            if (_keyboardW.isOpen()) {
                _keyboardW.close();
            }
            else {
                _keyboardW.create (sf::VideoMode(600, 259), "ZX Keyboard");
                _keyboardW.setFramerateLimit (50);
            }
        }

        // Menu
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            while (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape));
            _zxMenu.restartMenu ();
            _state = M_MENU;
        }

        break;

    case M_END:
        break;
    }


    // Drawing things
    if (_keyboardW.isOpen()) {
        _keyboardW.clear(sf::Color::Black);

        sf::Sprite kb;
        kb.setTexture(_kbTexture);
        _keyboardW.draw(kb);
        _keyboardW.display();
    }

    return _state;
}

ZXInputs ZXManager::getZXInputs () {
    std::vector<uint8_t> keys;
    uint8_t k = _zxSequencer.getKey(); // Simulating pressed keys
    if (!k) { // No key
        _zxKeyboard.getRKeys (keys); // Key from Keyboard
    }
    else {
        keys.push_back(k); // only one
    }

    // Other come from Pad/Joystick
    ZXInputs inputs (keys, _zxVPad.isUp(), _zxVPad.isDown(), _zxVPad.isLeft(), _zxVPad.isRight(), _zxVPad.isA());
    return inputs;
}

void ZXManager::displaySplashScreen () {
    _mainW.clear(sf::Color::Transparent);

    // Display splash image
    sf::Sprite zx;
    zx.setTexture(_zxTexture);
    zx.setPosition (16, 48); // Xé because of scale
    zx.scale (2.0f, 2.0f);
    _mainW.draw(zx);

    // Display text
    const sf::Font &font = _zxMenu.getFont();

    sf::Text t3 ("ZX Spectrum 48k", font);
    t3.setCharacterSize(60);
    t3.setPosition(sf::Vector2f(256, -16));
    t3.setFillColor(sf::Color::Cyan);
    _mainW.draw(t3);

    sf::Text t5 ("Emulator", font);
    t5.setCharacterSize(60);
    t5.setPosition(sf::Vector2f(348, 16));
    t5.setFillColor(sf::Color::Cyan);
    _mainW.draw(t5);

    char buf [64];
    sprintf (buf, "Build - %s -", __DATE__);
    sf::Text t6 (buf, font);
    t6.setCharacterSize(20);
    t6.setPosition(sf::Vector2f(350, 96));
    t6.setFillColor(sf::Color::Cyan);
    _mainW.draw(t6);

    sf::Text f1 ("ESC : Enter the menu mode", font);
    f1.setPosition(sf::Vector2f(8, 360));
    f1.setCharacterSize(32);
    f1.setFillColor(sf::Color::Red);
    _mainW.draw(f1);

    sf::Text f2 ("F1 : Toggle the ZX key mapping", font);
    f2.setPosition(sf::Vector2f(8, 380));
    f2.setCharacterSize(32);
    f2.setFillColor(sf::Color::Yellow);
    _mainW.draw(f2);

     sf::Text t1 ("Left/Right : Navigate through the menu", font);
    t1.setPosition(sf::Vector2f(8, 400));
    t1.setCharacterSize(32);
    t1.setFillColor(sf::Color::Green);
    _mainW.draw(t1);

     sf::Text t2 ("Enter : Validate the action", font);
    t2.setPosition(sf::Vector2f(8, 420));
    t2.setCharacterSize(32);
    t2.setFillColor(sf::Color::Blue);
    _mainW.draw(t2);

    sf::Text t4 ("(c) 2018 BUZZ (e-mail:buzz.computer (at) free.fr)", font);
    t4.setPosition(sf::Vector2f(8, 440));
    t4.setCharacterSize(32);
    t4.setFillColor(sf::Color::White);
    _mainW.draw(t4);

    _mainW.display();
}

void ZXManager::displaySpectrum (ZXSpectrum &spectrum) {
    _mainW.clear (sf::Color::White);

    // Border
    sf::RectangleShape r (sf::Vector2f(320, 240));
    r.setPosition (0,0);
    r.setFillColor(spectrum.getBorderColor());
    r.scale (2.0f, 2.0f);
    _mainW.draw (r);

    // Screen
    sf::Sprite sp;
    sf::Texture t;
    t.loadFromImage(spectrum.getOffScreen());
    sp.setTexture(t);
    sp.setPosition (64, 48); // x2 because of scale
    sp.scale (2.0f, 2.0f);
    _mainW.draw (sp);

    _mainW.display();
}

void ZXManager::displayMenu () {
    _zxMenu.display (_mainW);
}
