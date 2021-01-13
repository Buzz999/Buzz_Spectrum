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
#include <string>

class ZXFiles {
 public:
  ZXFiles ();
  ~ZXFiles ();

  bool browseSnaFiles (const std::string &directory);

  void selectNextSnaFile () {
    if (_currentSnaIndex + 1 < _snaFiles.size()) _currentSnaIndex++;
  }

  void selectPreviousSnaFile () {
      if (_currentSnaIndex >= 1) _currentSnaIndex--;
  }

  std::string getCurrentSnaFile ()const ;
  std::string getScreenshotFilename ();
  std::string getSnapshotFilename ();

 protected:
  bool isFileExists (const std::string &filename) const;
  bool createDir (const std::string &directory);

  std::vector<std::string> _snaFiles;
  uint32_t _currentSnaIndex;

  uint32_t _shotNumber; // Screenshot number
  uint32_t _snapNumber; // Snapshot number

};
