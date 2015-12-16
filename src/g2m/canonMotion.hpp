/***************************************************************************
 *   Copyright (C) 2010 by Mark Pictor                                     *
 *   mpictor@gmail.com                                                     *
 *   modified by Kazuyasu Hamada 2015, k-hamada@gifu-u.ac.jp               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef CANONMOTION_HH
#define CANONMOTION_HH

#include <string>
#include <vector>
#include <cmath>
#include <limits.h>

#include "machineStatus.hpp"
#include "canonLine.hpp"

namespace g2m {

/**
\class canonMotion
\brief This class is for the canonical commands STRAIGHT_TRAVERSE, STRAIGHT_FEED, and ARC_FEED.
canonMotion is an ABC. Its children should only be instantiated via 
* canonLine::canonLineFactory(), which creates 
* linearMotion objects for STRAIGHT_TRAVERSE and STRAIGHT_FEED commands, 
* and helicalMotion objects for ARC_FEED commands.

Note, you may find variations in the terminology I use - I misremembered some of 
* the canonical commands issued by the interpreter. For example, I thought it 
* issued LINEAR_FEED but it's actually STRAIGHT_FEED. So the class 
* linearMotion could have been named straightMotion.

Also, rapid and traverse are used interchangeably, at least in my comments...
*/

class canonMotion: protected canonLine {

  public:
    /// return type of motion
    virtual MOTION_TYPE getMotionType() {return NOT_DEFINED;}
    /// return true
    bool isMotion() {return true;};
    /// return start
    Point getStart() const {return start;}
    /// return end
    Point getEnd() const {return end;}

  protected:
    /// create canonMotion
    canonMotion(std::string canonL, machineStatus prevStatus);
    Pose getPoseFromCmd();
    /// start of this move
    Point start;
    /// end of this move
    Point end;
#ifdef MULTI_AXIS
    /// the direction in which the current move starts
    Point startDir;
    /// the direction in which the current move ends
    Point endDir;
#endif
};

} // end namespace 

#endif //CANONMOTION_HH
