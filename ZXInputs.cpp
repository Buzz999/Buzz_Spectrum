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

#include "ZXInputs.h"
#include <stdint.h>

#include <fstream>
#include <sstream>

using namespace std;

uint8_t keys[4][10] = {
  { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' },
  { 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p' },
  { 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\n' },
  { ZX_CAPS_SHIFT, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ZX_SYMBOL_SHIFT, ' '}
};

sf::Keyboard::Key sfml_keys [4][10] = {
  { sf::Keyboard::Num1, sf::Keyboard::Num2, sf::Keyboard::Num3, sf::Keyboard::Num4, sf::Keyboard::Num5, sf::Keyboard::Num6, sf::Keyboard::Num7, sf::Keyboard::Num8, sf::Keyboard::Num9, sf::Keyboard::Num0 },
  { sf::Keyboard::Q, sf::Keyboard::W, sf::Keyboard::E, sf::Keyboard::R, sf::Keyboard::T, sf::Keyboard::Y, sf::Keyboard::U, sf::Keyboard::I, sf::Keyboard::O, sf::Keyboard::P},
  { sf::Keyboard::A, sf::Keyboard::S, sf::Keyboard::D, sf::Keyboard::F, sf::Keyboard::G, sf::Keyboard::H, sf::Keyboard::J, sf::Keyboard::K, sf::Keyboard::L, sf::Keyboard::Return},
  { sf::Keyboard::LShift, sf::Keyboard::Z, sf::Keyboard::X, sf::Keyboard::C, sf::Keyboard::V, sf::Keyboard::B, sf::Keyboard::N, sf::Keyboard::M, sf::Keyboard::RShift, sf::Keyboard::Space}
};

void ZXKeyboard::getRKeys (std::vector<uint8_t> &pressedkeys) const {
    for (uint8_t j=0; j<4; ++j) {
        for (uint8_t i=0; i < 10; ++i) {
            if (sf::Keyboard::isKeyPressed(sfml_keys[j][i])) pressedkeys.push_back (keys[j][i]);
        }
    }
}


ZXVPad::ZXVPad () :
    _up(sf::Keyboard::Up),
    _down (sf::Keyboard::Down),
    _left (sf::Keyboard::Left),
    _right(sf::Keyboard::Right),

    _a (sf::Keyboard::A),
    _b (sf::Keyboard::B),
    _x (sf::Keyboard::X),
    _y (sf::Keyboard::Y),
    _l (sf::Keyboard::L),
    _r (sf::Keyboard::R),


    _enter (sf::Keyboard::Return),
    _select (sf::Keyboard::Space) {
}

bool ZXVPad::isDown () const {
    return sf::Keyboard::isKeyPressed(_down);
}

bool ZXVPad::isUp () const {
    return sf::Keyboard::isKeyPressed(_up);
}

bool ZXVPad::isLeft() const {
    return sf::Keyboard::isKeyPressed(_left);
}

bool ZXVPad::isRight() const {
    return sf::Keyboard::isKeyPressed(_right);
}

bool ZXVPad::isA() const {
    return sf::Keyboard::isKeyPressed(_a);
}

bool ZXVPad::isB() const {
    return sf::Keyboard::isKeyPressed(_b);
}
bool ZXVPad::isX() const {
    return sf::Keyboard::isKeyPressed(_x);
}

bool ZXVPad::isY() const {
    return sf::Keyboard::isKeyPressed(_y);
}

bool ZXVPad::isSelect () const {
    return sf::Keyboard::isKeyPressed(_select);
}

bool ZXVPad::isEnter () const {
    return sf::Keyboard::isKeyPressed (_enter);
}

bool ZXVPad::readResourceFile (const string &resfile) {
  ifstream filein( resfile.c_str() );
  if( filein ) {

	string line;
	while( getline (filein, line) ) {
    printf ("Reading mapper resource file ...");
    istringstream istr(line);

    string tag;
    istr >> tag;
    if (tag != "KEY") continue; // Skipping the line

    string keyDir;
    istr >> keyDir;

    string keyValue;
    istr >> keyValue;
    uint32_t key = atoi (keyValue.c_str());

	if (keyDir == "UP")         _up = (sf::Keyboard::Key) key;
	else if (keyDir == "DOWN")  _down = (sf::Keyboard::Key) key;
	else if (keyDir == "LEFT")  _left = (sf::Keyboard::Key) key;
	else if (keyDir == "RIGHT") _right = (sf::Keyboard::Key) key;
	else if (keyDir == "A")  _a = (sf::Keyboard::Key) key;
    else if (keyDir == "B")  _b = (sf::Keyboard::Key) key;
    else if (keyDir == "X")  _x = (sf::Keyboard::Key) key;
    else if (keyDir == "Y")  _y = (sf::Keyboard::Key) key;
    else if (keyDir == "L")  _l = (sf::Keyboard::Key) key;
    else if (keyDir == "R")  _r = (sf::Keyboard::Key) key;

	else if (keyDir == "ENTER") _enter = (sf::Keyboard::Key) key;
	else if (keyDir == "SELECT") _select = (sf::Keyboard::Key) key;
	}
  }
  else {
    ofstream fileout( resfile.c_str() );

    if (!fileout) {
      printf ("%s failed : Unable to open %s for writing\n", __FUNCTION__, resfile.c_str());
      return false;
    }

    printf ("Writing mapper resource file %s ...", resfile.c_str());

      fileout << "KEY " << " UP " << _up << endl;
      fileout << "KEY " << " DOWN " << _down << endl;
      fileout << "KEY " << " LEFT " << _left << endl;
      fileout << "KEY " << " RIGHT " << _right << endl;
      fileout << "KEY " << " A " << _a << endl;
    fileout << "KEY " << " B " << _b << endl;
    fileout << "KEY " << " X " << _x << endl;
    fileout << "KEY " << " Y " << _y << endl;
    fileout << "KEY " << " L " << _l << endl;
    fileout << "KEY " << " R " << _r << endl;

      fileout << "KEY " << " ENTER " << _enter << endl;
      fileout << "KEY " << " SELECT " << _select << endl;

      fileout << endl;
  }
  return true;
}

