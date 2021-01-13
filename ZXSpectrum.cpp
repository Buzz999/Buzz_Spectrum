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
#include "ZXVKeyboard.h"
#include "Z80.h"
#include "ZXMenu.h"
#include "ZXFiles.h"
#include <iostream>
#include <string>
#include <sys/unistd.h>

using namespace std;

const uint32_t ZX_VIDEO_START=16384;
const uint32_t ZX_ATTRIBUTE_START=22528;
const uint32_t ZX_ATTRIBUTE_END=23296;

//
// MEMORY
//
ZXMemory::ZXMemory () : _locked (true) {
  _memory = new uint8_t [65536];
   reset ();
}

ZXMemory::~ZXMemory () {
  if (_memory) {
    delete [] _memory;
    _memory = 0;
  }
}

bool ZXMemory::LoadRomFromFile (const char* aFile) {
  FILE *fp = fopen (aFile, "rb");
  if (!fp) {
    printf ("%s not found\n", aFile);
    return false;
  }

  uint8_t data;
  uint32_t address = 0;

  lock(false);
  while (fread (&data, sizeof (uint8_t), 1, fp) != 0) {
	if (address>=65536) {
	  fclose (fp);
      return false;
	}


    WriteToZXMemory ((uint16_t) address, data);
    address++;
  }
  lock(true);

  fclose (fp);
  return true;
}

bool ZXMemory::LoadFromBin (const uint8_t* aBin) {
  for (uint32_t address = 0; address < 16384; address++) {
    WriteToZXMemory ((uint16_t) address, (uint8_t) aBin[address]);
  }

  return true;
}

bool ZXMemory::SaveToFile (const char* aFile) {
  FILE *fp = fopen (aFile, "wb");
  if (!fp) return false;

  fwrite (_memory, 1, 65536, fp);

  fclose (fp);
  return true;
}

void ZXMemory::reset () {
    for (uint32_t i = ZX_VIDEO_START; i < 65536; i++) _memory [i] = rand() % 256;
}
//
// ZXULA
//
ZXULA::ZXULA (ZXMemory *aZXMemory, ZXIO *aZXIO) {
  theZXMemory = aZXMemory;
  theZXIO = aZXIO;

  // Not bright colors
  _colorTable [0] = sf::Color (0,0,0); // Black
  _colorTable [1] = sf::Color (0,0,200); // Blue
  _colorTable [2] = sf::Color (200,0,0); // Red
  _colorTable [3] = sf::Color (200,0,200); // Magenta
  _colorTable [4] = sf::Color (0,200, 0); // Green
  _colorTable [5] = sf::Color (0,200, 200);// Cyan
  _colorTable [6] = sf::Color (200,200, 0); // Yellow
  _colorTable [7] = sf::Color (200,200,200); // White

  // Bright colors
  _colorTable [8] = sf::Color (0,0,0); // Black
  _colorTable [9] = sf::Color (0,0,255); // Blue
  _colorTable [10] = sf::Color (255,0,0); // Red
  _colorTable [11] = sf::Color (255,0,255); // Magenta
  _colorTable [12] = sf::Color (0,255, 0); // Green
  _colorTable [13] = sf::Color (0,255, 255);// Cyan
  _colorTable [14] = sf::Color (255,255, 0); // Yellow
  _colorTable [15] = sf::Color (255,255,255); // White



  _joystick = J_KEMPSTON;
  _offscreen.create (256, 192);

  reset ();
}

void ZXULA::reset () {
  _flash = 0;
  _flash_inv = false;
  clearAudioBuffer ();
}

ZXULA::~ZXULA () {
}

void ZXULA::changeJoystick () {
  switch (_joystick) {
  case J_KEMPSTON:
	 _joystick = J_SINCLAIR_1;
    break;
  case J_SINCLAIR_1:
	_joystick = J_SINCLAIR_2;
    break;
  case J_SINCLAIR_2:
	_joystick = J_CURSOR;
    break;
  case J_CURSOR:
	_joystick = J_KEMPSTON;
    break;
  default:
   break;
  }
}

void ZXULA::setJoystick (ZX_JOYSTICK joy) {
  _joystick = joy;
}

// Acces en ecriture du Z80 dans la zone graphique de la memoire
void ZXULA::WriteToVideo (uint16_t adr, uint8_t data) {
  uint16_t x,y;
  uint8_t paper, ink;
  bool blink;
  // Partie graphique
  if (adr >= ZX_VIDEO_START && adr < ZX_ATTRIBUTE_START) {
	ZXVideoAdr2DSCoord(adr, x, y);
	GetAttributes (x, y, paper, ink, blink);
	Draw (x, y, data, paper, ink, blink);
  }
  else if (adr >= ZX_ATTRIBUTE_START && adr < ZX_ATTRIBUTE_END) { // Partie attribut
	uint16_t zx_video_base, zx_video_adr;
	uint8_t zx_video_data;
	uint16_t i;
	ZXAttrAdr2VideoAdr (adr, zx_video_base);
	ZXVideoAdr2DSCoord(zx_video_base, x, y);
	GetAttributes (x, y, paper, ink, blink);

	for (i=0; i<8; i++) {
	  zx_video_adr = zx_video_base + (i << 8);
	  theZXMemory->ReadFromZXMemory (zx_video_adr, zx_video_data);
	  ZXVideoAdr2DSCoord(zx_video_adr, x, y);
	  Draw (x, y, zx_video_data, paper, ink, blink);
	}
  }
}

void ZXULA::ZXAttrAdr2VideoAdr (uint16_t adr, uint16_t &v_adr) const {
  uint16_t ab, b, b_r;
  ab = adr - ZX_ATTRIBUTE_START;
  b = ab >> 8;
  b_r = ab & 0xFF;
  v_adr = (b << 11) + b_r + ZX_VIDEO_START;
}

void ZXULA::ZXVideoAdr2DSCoord (uint16_t adr, uint16_t &x, uint16_t &y) const {
  uint16_t y1, y1_r, y2, y2_r, y3, y3_r, ab;
  ab = adr-ZX_VIDEO_START;
  y1 = ab >> 11; // / 2048
  y1_r = ab & 0x07FF; // %2048;

  y2 = y1_r >> 8; // / 256;
  y2_r = y1_r & 0x00FF; // %256;

  y3 = y2_r >> 5; // / 32;
  y3_r = y2_r & 0x001F; // %32;

  y=(y1 << 6) + (y3 << 3) + y2; // y=(y1*64)+y2+(y3*8);
  x=y3_r;
}

void ZXULA::refreshScreen () {

  // Change border color

  // Fill the sheet
  uint16_t adr;
  uint8_t data;
  uint8_t paper, ink;
  bool blink;
  for (adr=ZX_VIDEO_START; adr < ZX_ATTRIBUTE_START; adr++) {
	theZXMemory->ReadFromZXMemory ((uint16_t) adr, data);
	uint16_t x,y;
	ZXVideoAdr2DSCoord(adr, x, y);
	GetAttributes (x, y, paper, ink, blink);
	Draw (x, y, data, paper, ink, blink);
  }
}

void ZXULA::blink () {
  _flash++;

  if (_flash < 13) return;//
  _flash = 0;


  if (_flash_inv) _flash_inv=false;
  else _flash_inv=true;

  uint8_t paper, ink;
  bool blink;
  uint16_t x,y;

  for (uint16_t adr = ZX_ATTRIBUTE_START; adr < ZX_ATTRIBUTE_END; adr++) {
	uint16_t zx_video_base, zx_video_adr;
	uint8_t zx_video_data;
	ZXAttrAdr2VideoAdr (adr, zx_video_base);
	ZXVideoAdr2DSCoord(zx_video_base, x, y);
	GetAttributes (x, y, paper, ink, blink);

	if (blink) {
	  for (uint16_t i=0; i<8; i++) {
		zx_video_adr = zx_video_base + (i << 8);
		theZXMemory->ReadFromZXMemory (zx_video_adr, zx_video_data);
		ZXVideoAdr2DSCoord(zx_video_adr, x, y);
		Draw (x, y, zx_video_data, paper, ink, blink);
	  }
	}
  }
}

  void ZXULA::GetAttributes (uint8_t x, uint8_t y, uint8_t &paper, uint8_t &ink, bool &blink) {
  // Get color attributs
  uint8_t attr = 0;
  uint16_t att_adr = ((y << 2) & 0xFFE0) + x;
  theZXMemory->ReadFromZXMemory ((uint16_t) (ZX_ATTRIBUTE_START + att_adr), attr); // Read color attributes

  // Extracts paper, ink
  paper = (attr >> 3) & 0x07; // D5-D3
  ink = attr & 0x07; // D2-D0
  if (attr & 0x40) { // D6 : Bright bit
    ink+=8; // Select ink color from 8 to 15 instead of 0 to 7
    paper+=8;// Select paper color from 8 to 15 instead of 0 to 7
  }

  if (attr & 0x80) blink=true;
  else blink=false;
}

// 0 <= x <= 31
// 0 <= y <= 191
void ZXULA::Draw (uint16_t x, uint16_t y, uint8_t data, uint8_t paper, uint8_t ink, bool blink) {
  uint8_t c1, c2;
  if (blink == false) { // Cas normal
	c1=ink;
	c2=paper;
  }
  else { // Clignotant
	if (_flash_inv) { // inversion
	  c1=paper;
	  c2=ink;
	}
	else { // normal
	  c1=ink;
	  c2=paper;
	}
  }

  for (uint8_t i=0; i < 8; i++) {
    uint8_t theData = (data << i) & 0x80;
	sf::Color color;

    if (theData) { // ink color (foreground)
      color = _colorTable [c1];
    }
    else { // paper color (background)
      color=_colorTable [c2];
    }

    _offscreen.setPixel ((x << 3)+i, y, color);
  }
}

void ZXULA::refreshSound () {
    // This function takes all samples from the ZXIO ans expand them into a fixed size buffer

    // Clearing internal buffer (_audioBuffer) to avoid glitches
    clearAudioBuffer();

    // Get samples stored into the ZXIO during execution
    const ZXSounds::VectorPairOfSamples &vos = theZXIO->getZXSounds().getSamples();
    for (ZXSounds::VectorPairOfSamples::const_iterator it = vos.begin(); it != vos.end(); ++it) {
        uint32_t aSampleIndex = (*it).first; // Corresponds to a processor cycle
        int16_t aSampleValue = (*it).second; // The sample with max range (-32768 or 32767)
        // The sound stream size is 256
        // The sample index is between 0 and 50000 (processor cycles)
        // So, index 0 corresponds to 0 and sample stamped 50000 (if exists) corresponds to 256
        for (uint32_t i = aSampleIndex/200; i < 256; ++i) _audioBuffer[i]=aSampleValue; // Dirty but fine
    }
    theZXIO->clearSamples();
}

void ZXULA::refreshKeyboard (const ZXInputs &zxinputs) {
    // Clearing all bits first (all key released)
    uint8_t portF7FE=0xFF;
    uint8_t portFBFE=0xFF;
    uint8_t portFDFE=0xFF;
    uint8_t portFEFE=0xFF;
    uint8_t portEFFE=0xFF;
    uint8_t portDFFE=0xFF;
    uint8_t portBFFE=0xFF;
    uint8_t port7FFE=0xFF;

    const std::vector<uint8_t> &keys = zxinputs.getKeys();

    std::vector<uint8_t>::const_iterator itk;
    for (itk = keys.begin(); itk < keys.end(); ++itk) {
        uint8_t key = *itk;

        if (key==ZX_CAPS_SHIFT) portFEFE &= 0x1E; // Bit 0 // CAPS SHIFT
        if (key=='z') portFEFE &= 0x1D; // Bit 1
        if (key=='x') portFEFE &= 0x1B; // Bit 2
        if (key=='c') portFEFE &= 0x17; // Bit 3
        if (key=='v') portFEFE &= 0x0F; // Bit 4
        if (key=='a') portFDFE &= 0x1E; // Bit 0
        if (key=='s') portFDFE &= 0x1D; // Bit 1
        if (key=='d') portFDFE &= 0x1B; // Bit 2
        if (key=='f') portFDFE &= 0x17; // Bit 3
        if (key=='g') portFDFE &= 0x0F; // Bit 4
        if (key=='q') portFBFE &= 0x1E; // Bit 0
        if (key=='w') portFBFE &= 0x1D; // Bit 1
        if (key=='e') portFBFE &= 0x1B; // Bit 2
        if (key=='r') portFBFE &= 0x17; // Bit 3
        if (key=='t') portFBFE &= 0x0F; // Bit 4
        if (key=='1') portF7FE &= 0x1E; // Bit 0
        if (key=='2') portF7FE &= 0x1D; // Bit 1
        if (key=='3') portF7FE &= 0x1B; // Bit 2
        if (key=='4') portF7FE &= 0x17; // Bit 3
        if (key=='5') portF7FE &= 0x0F; // Bit 4

        if (key=='0') portEFFE &= 0x1E; // Bit 0
        if (key=='9') portEFFE &= 0x1D; // Bit 1
        if (key=='8') portEFFE &= 0x1B; // Bit 2
        if (key=='7') portEFFE &= 0x17; // Bit 3
        if (key=='6') portEFFE &= 0x0F; // Bit 4
        if (key=='p') portDFFE &= 0x1E; // Bit 0
        if (key=='o') portDFFE &= 0x1D; // Bit 1
        if (key=='i') portDFFE &= 0x1B; // Bit 2
        if (key=='u') portDFFE &= 0x17; // Bit 3
        if (key=='y') portDFFE &= 0x0F; // Bit 4
        if (key=='\n') portBFFE &= 0x1E; // Bit 0
        if (key=='l') portBFFE &= 0x1D; // Bit 1
        if (key=='k') portBFFE &= 0x1B; // Bit 2
        if (key=='j') portBFFE &= 0x17; // Bit 3
        if (key=='h') portBFFE &= 0x0F; // Bit 4
        if (key==' ') port7FFE &= 0x1E; // Bit 0
        if (key==ZX_SYMBOL_SHIFT) port7FFE &= 0x1D; // Bit 1 // SYMBOL SHIFT
        if (key=='m') port7FFE &= 0x1B; // Bit 2
        if (key=='n') port7FFE &= 0x17; // Bit 3
        if (key=='b') port7FFE &= 0x0F; // Bit 4
    }

  // Hack for BackSpace/Up/Down/Left/Right
  // Mapping for the arrows
  // 0 : None
  // 2 : Sinclair 2 interface Joystick 1
  // 3 : Sinclair 2 interface Joystick 2
  // 4 : Cursor interface
  // 5 : Kempston interface

  if (_joystick == J_SINCLAIR_1) { // Sinclair 2 interface : Joystick 1
    if (zxinputs.isLeft()) portEFFE &= 0xEF; // Bit 4
    if (zxinputs.isRight()) portEFFE &= 0xF7; // Bit 3
    if (zxinputs.isDown()) portEFFE &= 0xFB; // Bit 2
    if (zxinputs.isUp()) portEFFE &= 0xFD; // Bit 1
    if (zxinputs.isFire()) portEFFE &= 0xFE; // Bit 0
  }
  else if (_joystick == J_SINCLAIR_2) { // Sinclair 2 interface : Joystick 2
    if (zxinputs.isLeft()) portF7FE &= 0xFE; // Bit 0
    if (zxinputs.isRight()) portF7FE &= 0xFD;// Bit 1
    if (zxinputs.isDown()) portF7FE &= 0xFB; // Bit 2
    if (zxinputs.isUp()) portF7FE &=  0xF7; // Bit 3
    if (zxinputs.isFire()) portF7FE &=0xEF; // Bit 4
  }
  else if (_joystick == J_CURSOR) { // Cursor interface
    if (zxinputs.isLeft()) portF7FE &= 0x0F;
    if (zxinputs.isRight()) portEFFE &= 0x1B;
    if (zxinputs.isUp()) portEFFE &= 0x17;
    if (zxinputs.isDown()) portEFFE &= 0x0F;
    if (zxinputs.isFire()) portEFFE &= 0x1E;
  }
  else if (_joystick == J_KEMPSTON) { // Kempston interface
     uint8_t port001F; // Kempston port
     theZXIO->ReadFromZXIO (0x001F, port001F);

     if (zxinputs.isLeft()) port001F |= 0x02; // Bit 1
     else port001F &= 0xFD;
     if (zxinputs.isRight()) port001F |=0x01; // Bit 0
     else port001F &= 0xFE;
     if (zxinputs.isUp()) port001F |= 0x08; // Bit 3
     else port001F &= 0xF7;
     if (zxinputs.isDown()) port001F |= 0x04; // Bit 2
     else port001F &= 0xFB;
     if (zxinputs.isFire())port001F  |= 0x10; // Bit 4
     else port001F &= 0xEF;

     theZXIO->WriteToZXIO (0x001F, port001F);
  }

  // Write all
  theZXIO->WriteToZXIO (0xF7FE, portF7FE);
  theZXIO->WriteToZXIO (0xFBFE, portFBFE);
  theZXIO->WriteToZXIO (0xFDFE, portFDFE);
  theZXIO->WriteToZXIO (0xFEFE, portFEFE);
  theZXIO->WriteToZXIO (0xEFFE, portEFFE);
  theZXIO->WriteToZXIO (0xDFFE, portDFFE);
  theZXIO->WriteToZXIO (0xBFFE, portBFFE);
  theZXIO->WriteToZXIO (0x7FFE, port7FFE);
}

//
// ZXIO
//
ZXIO::ZXIO () {
    // A map could be better than an array ...
    io = new uint8_t [65536];
    reset();
}

ZXIO::~ZXIO () {
  if (io) {
    delete [] io;
    io = 0;
  }
}

void ZXIO::reset () {
    for (uint32_t i = 0; i < 65536; i++) {
        io [i] = 0xFF;
    }
    // Kempston port
    io [0x001F] = 0;
    _border_color = 2;
}

void ZXIO::CPUWriteToZXIO (uint16_t Port, uint8_t data, uint32_t current_cycle) {
#if 0
  switch (Port) {
  case 65278:
  case 65022:
  case 64510:
  case 63486:
  case 61438:
  case 57342:
  case 49150:
  case 32766:
    break;
  default:
    printf ("ZXIO::WriteToZXIO : Port %04X, Data %02X\n", Port, data);
    break;
  }
  printf ("ZXIO::CPUWriteToZXIO : Port %04X, Data %02X\n", Port, data);
#endif
  // Manage port 254
  // D2-D1-D0 : border color
  // D3 : MIC
  // D4 : Speaker
  if ((Port & 0xE0FE) == 0x00FE) { // A7-A5 must be stuck at 0 if it is not the keyboard
    _border_color = data & 0x07; // D2-D1-D0
    // mic = data & 0x08; // D3

	int16_t speaker = 0; // Bit D4

    if (data & 0x10) speaker = 32767; // Max int16_t value
    else speaker = -32768; // Min int16_t value

	// Adding the sample
    _zxsounds.addSample(50000-current_cycle, speaker); // 50000 cycles executed every 20ms
  }

  io [Port] = data; // usefull ?
}

void ZXIO::CPUReadFromZXIO (uint16_t Port, uint8_t& data) const {
#if 0
  switch (Port) {
  case 65278:
  case 65022:
  case 64510:
  case 63486:
  case 61438:
  case 57342:
  case 49150:
  case 32766:
    break;
  default:
    printf ("ZXIO::CPUReadFromZXIO : Port %04X, Data %02X\n", Port, data);
    break;
  }
#endif

  // hack for Kempston. Sometimes port is 001F (SABRE) and sometimes 021F (PSSST)
  if ((Port & 0x001F) == 0x001F) data = io [0x001F];
  else data = io [Port];
}

//
// SPECTRUM
//
ZXSpectrum::ZXSpectrum (const char* aRomFile) : _sound(true), _soundBufferIndex(0) {
  theZXMemory = new ZXMemory ();
  theZXIO = new ZXIO ();
  theZXULA = new ZXULA (theZXMemory, theZXIO);
  theProcessor = new Z80 (theZXMemory, theZXULA, theZXIO);

  bool res = theZXMemory->LoadRomFromFile (aRomFile); // ROM at @0000
  if (!res) {
    printf ("Unable to find ROM file %s\n", aRomFile);
  }
  // TODO
#if 0
  if (res == false) { // On charge la rom depuis le binaire
	theZXMemory->LoadFromBin (rom);
  }
#endif // 0
}

sf::SoundBuffer &ZXSpectrum::prepareSoundBuffer () {
    if (_soundBufferIndex) _soundBufferIndex = 0;
    else _soundBufferIndex = 1;

    _soundBuffer[_soundBufferIndex].loadFromSamples (theZXULA->getAudioBuffer (), 256, 2, 11025); // 2 channels @ 11025Khz
    return _soundBuffer[_soundBufferIndex];
}

ZXSpectrum::~ZXSpectrum () {
  if (theZXMemory) {
    delete theZXMemory;
    theZXMemory = 0;
  }

  if (theZXULA) {
    delete theZXULA;
    theZXULA = 0;
  }

  if (theZXIO) {
    delete theZXIO;
    theZXIO = 0;
  }

  if (theProcessor) {
    delete theProcessor;
    theProcessor = 0;
  }
}

void ZXSpectrum::reset () {
  theZXMemory->reset ();
  theZXIO->reset ();
  theZXULA->reset();
  theProcessor->Reset ();
}

void ZXSpectrum::run (const ZXInputs &inputs) {
    // Run
	theProcessor->Execute (50000);
	theZXULA->refreshKeyboard (inputs);
	theProcessor->Z80_Cause_Interrupt (0);
	theZXULA->refreshSound ();
	theZXULA->blink();
    theZXULA->refreshScreen();
}

/*
 *      Load a 48K  .SNA file.
 *
 *      48K Format as follows:
 *      Offset  Size    Description (all registers stored with LSB first)
 *      0       1       I
 *      1       18      HL',DE',BC',AF',HL,DE,BC,IY,IX
 *      19      1       Interrupt (bit 2 contains IFF2 1=EI/0=DI
 *      20      1       R
 *      21      4       AF,SP
 *      25      1       Interrupt Mode (0=IM0/1=IM1/2=IM2)
 *      26      1       Border Colour (0..7)
 *      27      48K     RAM dump 0x4000-0xffff
 *      PC is stored on stack. */

bool ZXSpectrum::loadSNAFile (const string &aSNAFile) {
  if (aSNAFile.empty()) return false;

  FILE *fp = fopen (aSNAFile.c_str(), "rb");
  if (!fp) {
    printf ("ZXSpectrum::LoadSNAFile : unable to open SNA file %s\n", aSNAFile.c_str());
    return false;
  }

  Z80_Regs regs;
  uint8_t dummy;

  fread (&regs.I, sizeof (uint8_t), 1, fp); // I
  regs.I &= 0xFF;

  fread (&regs.HL2.B.l, sizeof (uint8_t), 1, fp); // HL'
  fread (&regs.HL2.B.h, sizeof (uint8_t), 1, fp);
  regs.HL2.B.l &= 0xFF;
  regs.HL2.B.h &= 0xFF;

  fread (&regs.DE2.B.l, sizeof (uint8_t), 1, fp); // DE'
  fread (&regs.DE2.B.h, sizeof (uint8_t), 1, fp);
  regs.DE2.B.l &= 0xFF;
  regs.DE2.B.h &= 0xFF;

  fread (&regs.BC2.B.l, sizeof (uint8_t), 1, fp); // BC'
  fread (&regs.BC2.B.h, sizeof (uint8_t), 1, fp);
  regs.BC2.B.l &= 0xFF;
  regs.BC2.B.h &= 0xFF;

  fread (&regs.AF2.B.l, sizeof (uint8_t), 1, fp); // AF'
  fread (&regs.AF2.B.h, sizeof (uint8_t), 1, fp);
  regs.AF2.B.l &= 0xFF;
  regs.AF2.B.h &= 0xFF;

  fread (&regs.HL.B.l, sizeof (uint8_t), 1, fp); // HL
  fread (&regs.HL.B.h, sizeof (uint8_t), 1, fp);
  regs.HL.B.l &= 0xFF;
  regs.HL.B.h &= 0xFF;

  fread (&regs.DE.B.l, sizeof (uint8_t), 1, fp); // DE
  fread (&regs.DE.B.h, sizeof (uint8_t), 1, fp);
  regs.DE.B.l &= 0xFF;
  regs.DE.B.h &= 0xFF;

  fread (&regs.BC.B.l, sizeof (uint8_t), 1, fp); // BC
  fread (&regs.BC.B.h, sizeof (uint8_t), 1, fp);
  regs.BC.B.l &= 0xFF;
  regs.BC.B.h &= 0xFF;

  fread (&regs.IY.B.l, sizeof (uint8_t), 1, fp); // IY
  fread (&regs.IY.B.h, sizeof (uint8_t), 1, fp);
  regs.IY.B.l &= 0xFF;
  regs.IY.B.h &= 0xFF;

  fread (&regs.IX.B.l, sizeof (uint8_t), 1, fp); // IX
  fread (&regs.IX.B.h, sizeof (uint8_t), 1, fp);
  regs.IX.B.l &= 0xFF;
  regs.IX.B.h &= 0xFF;

  fread (&dummy, sizeof (uint8_t), 1, fp); // IFF2
  regs.IFF2 = (dummy & 0x04) >> 2;
  regs.IFF1 = (dummy & 0x04) >> 2;

  fread (&dummy, sizeof (uint8_t), 1, fp); // R
  regs.R = dummy & 127;
  regs.R2 = dummy & 128;

  fread (&regs.AF.B.l, sizeof (uint8_t), 1, fp); // AF
  fread (&regs.AF.B.h, sizeof (uint8_t), 1, fp);
  regs.AF.B.l &= 0xFF;
  regs.AF.B.h &= 0xFF;

  fread (&regs.SP.B.l, sizeof (uint8_t), 1, fp); // SP
  fread (&regs.SP.B.h, sizeof (uint8_t), 1, fp);
  regs.SP.B.l &= 0xFF;
  regs.SP.B.h &= 0xFF;

  fread (&regs.IM, sizeof (uint8_t), 1, fp); // IM
  regs.IM &= 0xFF;

  fread (&dummy, sizeof (uint8_t), 1, fp); // Border color
  //theZXIO->SetBorderColor (dummy & 0x07);

  // Fill the Memory
  uint32_t address = 0x4000;
  uint8_t data;

  while (fread (&data, sizeof (uint8_t), 1, fp) != 0) {
	if (address>=65536) {
      fclose (fp);
      return false;
	}

    theZXMemory->WriteToZXMemory ((uint16_t)address, data);
    address++;
  }

  /* get pc from stack */
  address = regs.SP.W.l & 0xFFFF; // Get SP
  theZXMemory->ReadFromZXMemory ((uint16_t) address, regs.PC.B.l);
  theZXMemory->ReadFromZXMemory ((uint16_t) (address+1), regs.PC.B.h);

  address += 2;
  regs.SP.W.l = address & 0xFFFF; // New stack pointer

  regs.pending_irq = 0;
  regs.pending_nmi = 0;

  // Set the real Registers
  theProcessor->Z80_SetRegs (regs);
  fclose (fp);
  return true;
}


/*
 *      Save a 48K  .SNA file.
 *
 *      48K Format as follows:
 *      Offset  Size    Description (all registers stored with LSB first)
 *      0       1       I
 *      1       18      HL',DE',BC',AF',HL,DE,BC,IY,IX
 *      19      1       Interrupt (bit 2 contains IFF2 1=EI/0=DI
 *      20      1       R
 *      21      4       AF,SP
 *      25      1       Interrupt Mode (0=IM0/1=IM1/2=IM2)
 *      26      1       Border Colour (0..7)
 *      27      48K     RAM dump 0x4000-0xffff
 *      PC is stored on stack. */

bool ZXSpectrum::saveSNAFile (const string &aSNAFile) const {
  FILE *fp = fopen (aSNAFile.c_str(), "wb");
  if (!fp) {
    printf ("ZXSpectrum::SaveSNAFile : unable to open SNA file %s\n", aSNAFile.c_str());
    return false;
  }

  Z80_Regs regs= theProcessor->Z80_GetRegs ();
  uint8_t dummy;

  /* set pc to stack */
  uint16_t adr;
  adr = regs.SP.W.l & 0xFFFF; // Get SP
  theZXMemory->WriteToZXMemory ((uint16_t) (adr-2), regs.PC.B.l);
  theZXMemory->WriteToZXMemory ((uint16_t) (adr-1), regs.PC.B.h);
  adr -= 2;
  regs.SP.W.l = adr & 0xFFFF; // New stack pointer

  regs.I &= 0xFF;
  fwrite (&regs.I, sizeof (uint8_t), 1, fp); // I

  regs.HL2.B.l &= 0xFF;
  fwrite (&regs.HL2.B.l, sizeof (uint8_t), 1, fp); // HL'
  regs.HL2.B.h &= 0xFF;
  fwrite (&regs.HL2.B.h, sizeof (uint8_t), 1, fp);

  regs.DE2.B.l &= 0xFF;
  regs.DE2.B.h &= 0xFF;
  fwrite (&regs.DE2.B.l, sizeof (uint8_t), 1, fp); // DE'
  fwrite (&regs.DE2.B.h, sizeof (uint8_t), 1, fp);

  regs.BC2.B.l &= 0xFF;
  regs.BC2.B.h &= 0xFF;
  fwrite (&regs.BC2.B.l, sizeof (uint8_t), 1, fp); // BC'
  fwrite (&regs.BC2.B.h, sizeof (uint8_t), 1, fp);

  regs.AF2.B.l &= 0xFF;
  regs.AF2.B.h &= 0xFF;
  fwrite (&regs.AF2.B.l, sizeof (uint8_t), 1, fp); // AF'
  fwrite (&regs.AF2.B.h, sizeof (uint8_t), 1, fp);

  regs.HL.B.l &= 0xFF;
  regs.HL.B.h &= 0xFF;
  fwrite (&regs.HL.B.l, sizeof (uint8_t), 1, fp); // HL
  fwrite (&regs.HL.B.h, sizeof (uint8_t), 1, fp);

  regs.DE.B.l &= 0xFF;
  regs.DE.B.h &= 0xFF;
  fwrite (&regs.DE.B.l, sizeof (uint8_t), 1, fp); // DE
  fwrite (&regs.DE.B.h, sizeof (uint8_t), 1, fp);

  regs.BC.B.l &= 0xFF;
  regs.BC.B.h &= 0xFF;
  fwrite (&regs.BC.B.l, sizeof (uint8_t), 1, fp); // BC
  fwrite (&regs.BC.B.h, sizeof (uint8_t), 1, fp);

  regs.IY.B.l &= 0xFF;
  regs.IY.B.h &= 0xFF;
  fwrite (&regs.IY.B.l, sizeof (uint8_t), 1, fp); // IY
  fwrite (&regs.IY.B.h, sizeof (uint8_t), 1, fp);

  regs.IX.B.l &= 0xFF;
  regs.IX.B.h &= 0xFF;
  fwrite (&regs.IX.B.l, sizeof (uint8_t), 1, fp); // IX
  fwrite (&regs.IX.B.h, sizeof (uint8_t), 1, fp);

  dummy = (regs.IFF1 & 0x01) << 2;
  fwrite (&dummy, sizeof (uint8_t), 1, fp); // IFF2

  dummy = (regs.R & 127) | (regs.R2 & 128);
  fwrite (&dummy, sizeof (uint8_t), 1, fp); // R

  regs.AF.B.l &= 0xFF;
  regs.AF.B.h &= 0xFF;
  fwrite (&regs.AF.B.l, sizeof (uint8_t), 1, fp); // AF
  fwrite (&regs.AF.B.h, sizeof (uint8_t), 1, fp);

  regs.SP.B.l &= 0xFF;
  regs.SP.B.h &= 0xFF;
  fwrite (&regs.SP.B.l, sizeof (uint8_t), 1, fp); // SP
  fwrite (&regs.SP.B.h, sizeof (uint8_t), 1, fp);

  regs.IM &= 0xFF;
  fwrite (&regs.IM, sizeof (uint8_t), 1, fp); // IM

  //dummy = theZXIO->GetBorderColor ();
  dummy = 0;
  fwrite (&dummy, sizeof (uint8_t), 1, fp); // Border color

  // Dump the Memory
  uint8_t data;
  for (int address = 0x4000; address <= 0xFFFF; address++) {
    theZXMemory->ReadFromZXMemory ((uint16_t) address, data);
    fwrite (&data, sizeof (uint8_t), 1, fp);
  }

  fclose (fp);

  return true;
}

void ZXSpectrum::setJoystick (uint32_t index) {
    ZX_JOYSTICK j = J_KEMPSTON;
    switch (index) {
    case 0: j = J_KEMPSTON; break;
    case 1: j = J_SINCLAIR_1; break;
    case 2: j = J_SINCLAIR_2; break;
    case 3: j = J_CURSOR; break;
    default: break;
    }
    theZXULA->setJoystick(j);
}

void ZXSpectrum::DumpRegs() {
#ifdef DEBUG
  const Z80_Regs & r = theProcessor->Z80_GetRegs();
  PA_Print (0, "A:%02d\n", r.AF.B.l);
  PA_Print (0, "B:%02d\n", r.BC.B.l);
  PA_Print (0, "C:%02d\n", r.BC.B.h);
  PA_Print (0, "D:%02d\n", r.DE.B.l);
  PA_Print (0, "E:%02d\n", r.DE.B.h);
  PA_Print (0, "PC:%04d\n", r.PC.W.l);
#endif
}

void ZXSpectrum::TestAttVideo () {
  theZXMemory->WriteToZXMemory (ZX_ATTRIBUTE_START, 0x0E);
  theZXMemory->WriteToZXMemory (ZX_ATTRIBUTE_START+1, 0x1C);

  uint16_t adr = ZX_VIDEO_START;
  uint8_t data = 0xF0;
  for (uint8_t i=0; i < 8; i++) {
	theZXMemory->WriteToZXMemory (adr, data);
	theZXULA->WriteToVideo (adr, data);
	adr = adr + 256;
  }
  adr = ZX_VIDEO_START+1;
  data = 0x3C;
  for (uint8_t i=0; i < 8; i++) {
	theZXMemory->WriteToZXMemory (adr, data);
	theZXULA->WriteToVideo (adr, data);
	adr = adr + 256;
  }
  theZXMemory->WriteToZXMemory (ZX_ATTRIBUTE_START, 0x14);
  theZXULA->WriteToVideo (ZX_ATTRIBUTE_START, 0x14);
}
