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

#include "ZXSpectrum.h"
#include "ZXManager.h"
#include "ZXMenu.h"

/* TODO
* Logo parameters
* Animation icone selectionnée
* Pb ON/OFF son
*/

int main (int argc, char **argv) {
  ZXManager mgr;

  // ZX
  ZXSpectrum aSpectrum ("data/spectrum.rom");

  sf::Sound soundStream;
  soundStream.setBuffer(aSpectrum.prepareSoundBuffer());
  if (aSpectrum.getSound()) soundStream.play(); // Based on the spectrum flag

  bool fin = false;

  while (!fin) {
    ZXManager::MGR_STATE state = mgr.process();

    // Actions may be needed even if we are not in menu mode
    switch (mgr.getMenu().popActionID ()) {
        case A_RESET: aSpectrum.reset(); break;
        case A_LOAD: aSpectrum.loadSNAFile(mgr.getMenu().getFilename().c_str()); break; // Load an existing SNA file
        case A_SAVE: aSpectrum.saveSNAFile(mgr.getMenu().getSnapshotFilename().c_str()); break; // Write running context to a SNA file
        case A_SOUND: aSpectrum.setSound (mgr.getMenu().getSound()); break; // Changing the state at the spectrum level with value given by the menu
        case A_JOYSTICK: aSpectrum.setJoystick(mgr.getMenu().getJoystick()); break; // Change joystick
        case A_SCREENSHOT: aSpectrum.getOffScreen().saveToFile (mgr.getMenu().getScreenshotFilename()); break; // Take a picture
        case A_NONE: break;
        default: break;
    }

    // Display things
    switch (state) {
        case ZXManager::M_SPLASH:
            mgr.displaySplashScreen();
            break;

        case ZXManager::M_MENU:
            mgr.displayMenu ();
            break;

        case ZXManager::M_RUN: {
            aSpectrum.run (mgr.getZXInputs());

            if (aSpectrum.getSound()) {
                    soundStream.setBuffer(aSpectrum.prepareSoundBuffer());
                    soundStream.play();
            } else {
                soundStream.stop ();
            }

            mgr.displaySpectrum (aSpectrum);
        }
        break;
        case ZXManager::M_END: fin=true; break;

        default: break;
    }
  }

  soundStream.stop();

  return 0;
}
