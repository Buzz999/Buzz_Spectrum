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

#include "ZXFiles.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sstream>
#include <iostream>

using namespace std;

ZXFiles::ZXFiles () : _currentSnaIndex (0), _shotNumber(0), _snapNumber(0) {
}

ZXFiles::~ZXFiles() {
}

bool ZXFiles::browseSnaFiles (const std::string &directory) {
    _snaFiles.clear();
    _currentSnaIndex = 0;

    DIR *dir = 0;
    struct dirent *dent = 0;

    dir = opendir (directory.c_str());
    if (!dir) {
        //--fprintf (stderr, "Unable to open the %s directory\n", directory.c_str());
        return false;
    }

    bool fin = false;
    while (!fin) {
        dent = readdir (dir);
        if (!dent) fin = true;
        else {
            string name = dent->d_name;
            if (name == "." || name == "..") continue;

            string fullname = directory + name;
            _snaFiles.push_back (fullname);
        }
    }

    closedir (dir);
    dir = 0;
    return true;
}

std::string ZXFiles::getCurrentSnaFile () const {
      if (_snaFiles.empty()) {
            //--fprintf (stderr, "NO FILE\n");
            return "";
      }

      //fprintf (stderr, "FILE:%s at %d\n", _snaFiles.at(_currentSnaIndex).c_str(), _currentSnaIndex);
      return _snaFiles.at(_currentSnaIndex);
 }

bool ZXFiles::createDir (const string &directory) {
   struct stat a_stat;
   int result = stat (directory.c_str(), &a_stat);
   if (result == 0) return true; // Exist already

    int error = mkdir(directory.c_str());
    if (error == 0) return true; // Success

    return false; // Failed
}

bool ZXFiles::isFileExists (const std::string &filename) const {
    struct stat f_stat;
    if(stat(filename.c_str(), &f_stat)) return false; // Does not exist
    return true;
}

string ZXFiles::getScreenshotFilename () {
    createDir ("screen");

    for (uint32_t i = _shotNumber; i < 100; ++i) {
        stringstream ss;
        ss << "screen/scr_" << _shotNumber << ".png";
        _shotNumber++;
        if (!isFileExists (ss.str())) return ss.str();
    }

    return "screen/scr_last.png";
}

string ZXFiles::getSnapshotFilename () {
    createDir ("sna");

    for (uint32_t i = _snapNumber; i < 100; ++i) {
        stringstream ss;
        ss << "sna/snap_" << _snapNumber << ".sna";
        _snapNumber++;
        if (!isFileExists (ss.str())) return ss.str();
    }

    return "sna/snap_last.sna";
}
