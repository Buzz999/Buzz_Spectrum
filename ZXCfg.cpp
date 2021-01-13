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

#include <fstream>
#include <sstream>

using namespace std;

ZXCfgLine::ZXCfgLine () {
  _name = "Undefined.sna";
  _joy = J_KEMPSTON;
}

ZXCfgLine::ZXCfgLine (const ZXCfgLine& cfg) {
  _name = cfg._name;
  _joy = cfg._joy;
  _sequences = cfg._sequences;
}

ZXCfgLine& ZXCfgLine::operator= (const ZXCfgLine & cfg) {
  if (&cfg == this) return *this;
  _name = cfg._name;
  _joy = cfg._joy;
  _sequences = cfg._sequences;
  return *this;
}

ZXCfgLine::ZXCfgLine (const string &name, const string joy, const deque<string> & lseq) : _joy(J_KEMPSTON) {
  _name = name;

  if (joy == "KEMPSTON" || joy == "K")_joy = J_KEMPSTON;
  else if (joy == "SINCLAIR_1" || joy == "S1") _joy = J_SINCLAIR_1;
  else if (joy == "SINCLAIR_2" || joy == "S2") _joy = J_SINCLAIR_2;
  else if (joy == "CURSOR" || joy == "C") _joy = J_CURSOR;

  _sequences = lseq;
}

ZXCfgLine::~ZXCfgLine () {
}


ZXCfg::ZXCfg (const string filename) {
  _filename = filename;
  loadZXCfg();
}

ZXCfg::~ZXCfg () {
}

ZXCfgLine &ZXCfg::getZXCfg (const std::string &sna) {
  ZXCfgLine &c = _cfgline[sna];
  c.setName(sna);
  return c;
}

bool ZXCfg::saveZXCfg () {
  if (_modified == false) return false;

  ofstream fout(_filename.c_str() );
  if( !fout ) return false;

  map<string, ZXCfgLine>::iterator it;
  for (it = _cfgline.begin(); it != _cfgline.end(); it++) {
	fout << it->second.getName ();

	ZX_JOYSTICK joy = it->second.getJoy ();
	if (joy == J_KEMPSTON) { fout << " KEMPSTON"; }
	else if (joy == J_SINCLAIR_1) { fout << " SINCLAIR_1"; }
	else if (joy == J_SINCLAIR_2) { fout << " SINCLAIR_2"; }
	else if (joy == J_CURSOR) { fout << " CURSOR"; }

	deque<string>::iterator itl;
	for (itl = it->second.getSequence().begin(); itl != it->second.getSequence().end(); itl++) {
	  fout << " " << *itl;
	}

	fout << endl;
  }
  _modified = false;
  return true;
}

bool ZXCfg::loadZXCfg () {

  ifstream fin(_filename.c_str() );
  if( !fin ) return false;

  _cfgline.clear();

  _modified = true;

  string s;
  while( getline(fin,s) ) {
	string name;
	string sjoy;
	string seq;
	deque<string> lseq;
	istringstream istr;
	istr.str(s);
	istr >> name;
	istr >> sjoy;

	while (!istr.eof()) {
		istr >> seq;
		lseq.push_back(seq);
	}

	ZXCfgLine l (name, sjoy, lseq);
	_cfgline[name]=l;
  }

  return true;
}


