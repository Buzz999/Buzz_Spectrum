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

#include <vector>
#include <stdint.h>

#include <SFML/Graphics.hpp>

#define ZX_CAPS_SHIFT sf::Keyboard::LShift
#define ZX_SYMBOL_SHIFT sf::Keyboard::RShift

class ZXKeyboard {
 public:
  ZXKeyboard () {}
  ~ZXKeyboard () {}

  void getRKeys (std::vector<uint8_t> &pressedkeys) const; // Get all pressed keys
};

class ZXVPad {
public:
    ZXVPad ();
    ~ZXVPad () { }

    bool readResourceFile (const std::string & filename);

    bool isDown() const;
    bool isUp() const;
    bool isLeft() const;
    bool isRight() const;
    bool isA() const;
    bool isB() const;
    bool isX() const;
    bool isY() const;
    bool isL() const;
    bool isR() const;

    bool isEnter() const;
    bool isSelect () const;

    sf::Keyboard::Key getUpKey() const { return _up; }
    sf::Keyboard::Key getDownKey() const { return _down; }
    sf::Keyboard::Key getLeftKey() const { return _left; }
    sf::Keyboard::Key getRightKey() const { return _right; }

    sf::Keyboard::Key getAKey() const { return _a; }
    sf::Keyboard::Key getBKey() const { return _b; }
    sf::Keyboard::Key getXKey() const { return _x; }
    sf::Keyboard::Key getYKey() const { return _y; }
    sf::Keyboard::Key getLKey() const { return _l; }
    sf::Keyboard::Key getRKey() const { return _r; }

    sf::Keyboard::Key getEnterKey() const { return _enter; }
    sf::Keyboard::Key getSelectKey() const { return _select; }

protected:
    sf::Keyboard::Key _up;
    sf::Keyboard::Key _down;
    sf::Keyboard::Key _left;
    sf::Keyboard::Key _right;
    sf::Keyboard::Key _a;
    sf::Keyboard::Key _b;
    sf::Keyboard::Key _x;
    sf::Keyboard::Key _y;
    sf::Keyboard::Key _l;
    sf::Keyboard::Key _r;

    sf::Keyboard::Key _enter;
    sf::Keyboard::Key _select;
};

class ZXInputs {
public:
    ZXInputs (const std::vector<uint8_t> &keys, bool u, bool d, bool l, bool r, bool f) : _keys(keys), _up(u), _down(d), _left(l), _right(r), _fire(f) { }
    ZXInputs () : _up(false), _down(false), _left(false), _right(false), _fire(false) { }
    ~ZXInputs () { }

    void copy (const ZXInputs &copied) {
   	    _keys = copied._keys;
        _up = copied._up;
        _down = copied._down;
        _left = copied._left;
        _right = copied._right;
        _fire = copied._fire;
	}

	ZXInputs (const ZXInputs& zi) { copy (zi); }
	ZXInputs &operator=(const ZXInputs &zi) {
		if (&zi == this) return *this;
		copy (zi);
		return *this;
	}

    const std::vector<uint8_t> &getKeys () const { return _keys; }
    bool isUp () const { return _up; }
    bool isDown() const { return _down; }
    bool isLeft () const { return _left; }
    bool isRight() const { return _right; }
    bool isFire() const { return _fire; }

private:
    std::vector<uint8_t> _keys;
    bool _up;
    bool _down;
    bool _left;
    bool _right;
    bool _fire;
};
