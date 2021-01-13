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

#include <SFML/Graphics.hpp>
#include <stdint.h>

#include "ZXSpectrum.h"
#include "ZXVKeyboard.h"
#include "ZXVPad.h"
#include "ZXSequencer.h"
#include "ZXMenu.h"
#include "ZXInputs.h"

class ZXManager {
public:
    enum MGR_STATE { M_INIT, M_SPLASH, M_MENU, M_RUN, M_END };
    ZXManager ();
    ~ZXManager ();

    void displaySplashScreen ();
    void displaySpectrum (ZXSpectrum &spectrum);
    void displayMenu ();

    MGR_STATE process ();

    sf::RenderWindow &getMainWindow () { return _mainW; }

    ZXInputs getZXInputs ();
    ZXMenu &getMenu() { return _zxMenu; }

private:
      sf::RenderWindow _mainW;
      sf::RenderWindow _keyboardW;
      sf::Texture _kbTexture;
      sf::Texture _zxTexture;

      MGR_STATE _state;
      sf::Clock _clock;

      ZXKeyboard _zxKeyboard;
      ZXVPad _zxVPad;
      ZXSequencer _zxSequencer;
      ZXMenu _zxMenu;

      void loadTextures ();
};
