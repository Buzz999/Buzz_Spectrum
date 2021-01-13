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

#include "ZXSequencer.h"
#include <stdlib.h>

using namespace std;

ZXSequencer::ZXSequencer () : _wait(0) {
}

ZXSequencer::~ZXSequencer () {
}

void ZXSequencer::setSequence (const deque<string> & sequence) {
  _sequence = sequence;
  _token="";
}

uint8_t ZXSequencer::getKey() {
  if (_sequence.empty()) return 0;

  if (_wait != 0) {
	_wait--;
	return 0;
  }

  //if  (pop() == false) return 0;
  pop();

  if (_token.at(0) == '#') {
	if (_token.size() > 1) {
	  _wait = atoi (_token.substr(1).c_str()) * 30;
	}
	return 0;
  }

  return (uint8_t) (tolower (_token.at(0)));
}

void ZXSequencer::pop () {
#if 0
  if (_sequence.empty()) {
	return true;
  }
#endif

  _token = _sequence.front();
  _sequence.pop_front();
}
