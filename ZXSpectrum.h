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

#include <stdint.h>
#include <string>
#include <vector>
#include <utility> // pair
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class ZXKeyboard;
class ZXMenu;
class ZXSequencer;
class ZXIO;

#include "Z80.h"
#include "ZXCfg.h"
#include "ZXManager.h"
#include "ZXInputs.h"
#include <vector>

class ZXSounds {
public:
    // A sample = Index + Value
    typedef std::vector<std::pair<uint32_t, int16_t> > VectorPairOfSamples;
    ZXSounds () { }
    ~ZXSounds () { }

    void addSample (uint32_t index, int16_t value) { _samples.push_back (std::make_pair(index, value)); }
    size_t getNbSamples () const { return _samples.size(); }
    void clearSamples () { _samples.clear(); }
    const VectorPairOfSamples &getSamples () const { return _samples; }

protected:
    VectorPairOfSamples _samples;
};

class ZXMemory {
 public:
  ZXMemory ();
  ~ZXMemory ();

  void reset ();


  bool LoadRomFromFile (const char* aFile);
  bool LoadFromBin (const uint8_t* aBin);
  bool SaveToFile (const char* aFile);

  inline void WriteToZXMemory (uint16_t adr, uint8_t data) {
      if ((adr < 16384) && (_locked == true)) {
        printf ("Write access to ROM @%d DENIED\n", adr);
        return;
      }
      _memory [adr] = data;
  }

  inline void ReadFromZXMemory (uint16_t adr, uint8_t& data) const { data = _memory [adr]; }

 protected:
  bool _locked;
  uint8_t *_memory;

  void lock(bool tolock) { _locked = tolock; }
};

class ZXIO {
 public:
  ZXIO ();
  ~ZXIO ();

  void reset ();

  // Access to IO with time-stamp
  void CPUWriteToZXIO (uint16_t Port, uint8_t data, uint32_t current_cycle);
  void CPUReadFromZXIO (uint16_t Port, uint8_t& data) const;

   // Access to IO without time-stamp
  inline void WriteToZXIO (uint16_t Port, uint8_t data) { io [Port] = data; }
  inline void ReadFromZXIO (uint16_t Port, uint8_t& data) const { data = io [Port]; }

  const ZXSounds &getZXSounds () const { return _zxsounds; }
  void clearSamples () { _zxsounds.clearSamples(); }

  uint8_t getBorderColorIndex() const { return _border_color; }

private:
  uint8_t *io;
  uint8_t _border_color;

  ZXSounds _zxsounds;
};


class ZXULA {
 public:
  ZXULA (ZXMemory *aZXMemory, ZXIO *aZXIO);
  ~ZXULA ();

  void reset ();

  void refreshScreen ();
  void refreshKeyboard (const ZXInputs &zxinputs);
  void refreshSound ();

  void setJoystick (ZX_JOYSTICK joy);

  void changeJoystick ();
  void blink ();
  void WriteToVideo (uint16_t adr, uint8_t data);

  sf::Image &getOffScreen () { return _offscreen; }
  sf::Color getBorderColor () const { return _colorTable[theZXIO->getBorderColorIndex()]; }

  const int16_t * getAudioBuffer () const { return _audioBuffer; }
  inline void clearAudioBuffer() { for (uint32_t i=0; i < 256; ++i) _audioBuffer[i] = 0; }

 private:
  inline void Draw (uint16_t x, uint16_t y, uint8_t data, uint8_t paper, uint8_t ink, bool inv);
  inline void ZXVideoAdr2DSCoord (uint16_t adr, uint16_t &x, uint16_t &y) const;
  inline void ZXAttrAdr2VideoAdr (uint16_t adr, uint16_t &v_adr) const;

  void GetAttributes (uint8_t x, uint8_t y, uint8_t &paper, uint8_t &ink, bool &blink);

  sf::Image _offscreen;
  sf::Color _colorTable [16];

  int _flash;
  bool _flash_inv;

  ZX_JOYSTICK _joystick;

  ZXMemory *theZXMemory;
  ZXIO *theZXIO;

  // Sound
  int16_t _audioBuffer[300]; // At least 256
};

class ZXSpectrum {
 public:

  ZXSpectrum (const char* aRomFile);
  ~ZXSpectrum ();

  bool loadSNAFile (const std::string &aSNAFile);
  bool saveSNAFile (const std::string &aSNAFile) const;

  void setJoystick (uint32_t index);
  void setSound (bool value) { _sound = value; printf ("SET SOUND %d\n", _sound); }
  bool getSound () const { return _sound; }

  void run (const ZXInputs &zxinputs);
  void reset ();
  sf::SoundBuffer & prepareSoundBuffer ();

  sf::Image &getOffScreen () { return theZXULA->getOffScreen(); }
  sf::Color getBorderColor () const { return theZXULA->getBorderColor();}

  // DEBUG
  void DumpRegs();
  void TestAttVideo();

 private:
  bool _sound;
  uint32_t _soundBufferIndex;
  sf::SoundBuffer _soundBuffer[2];

  Z80 *theProcessor;
  ZXMemory *theZXMemory; // RAM & ROM access
  ZXULA *theZXULA; // ULA access
  ZXIO *theZXIO; // IO access
  ZXKeyboard *theKeyboard;
};
