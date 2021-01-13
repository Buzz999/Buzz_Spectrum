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
#include <map>
#include <deque>

enum ZX_JOYSTICK { J_KEMPSTON, J_SINCLAIR_1, J_SINCLAIR_2, J_CURSOR };

class ZXCfgLine {
 public:
  ZXCfgLine ();
  ZXCfgLine (const std::string &name, const std::string joy, const std::deque<std::string> &lseq);
  ~ZXCfgLine ();

  ZXCfgLine (const ZXCfgLine& cfg);
  ZXCfgLine& operator= (const ZXCfgLine & cfg);

  void setName (const std::string &name) { _name = name; }
  void setJoy (ZX_JOYSTICK joy) { _joy = joy; }
  void setSequence(const std::deque<std::string> &seq) { _sequences=seq; }

  std::string getName () const { return _name; }
  ZX_JOYSTICK getJoy () const { return _joy; }
  std::deque<std::string> &getSequence() { return _sequences; }

 private:
  std::string _name;
  ZX_JOYSTICK _joy;
  std::deque<std::string> _sequences;
};

class ZXCfg {
 public:
  ZXCfg (const std::string filename);
  ~ZXCfg ();

  ZXCfgLine &getZXCfg (const std::string &sna);
  bool saveZXCfg ();
  bool loadZXCfg ();

 private:
  bool _modified;
  std::map<std::string, ZXCfgLine> _cfgline;
  std::string _filename;
};
