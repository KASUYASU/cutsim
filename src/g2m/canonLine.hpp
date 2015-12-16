/***************************************************************************
 *   Copyright (C) 2010 by Mark Pictor                                     *
 *   mpictor@gmail.com                                                     *
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
#ifndef CANONLINE_HH
#define CANONLINE_HH

#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <sstream>
#include <limits.h>
#include <cassert>

#include "machineStatus.hpp"
#include "point.hpp"

namespace g2m {
/**
\class canonLine
\brief A canonLine object represents one canonical command.
Each gcode line produces one or more canonical commands. It can either be a 
* motion command (one of LINEAR_TRAVERSE LINEAR_FEED ARC_FEED), or a motionless 
* command (anything else)
You cannot create objects of this class - instead, create an object of a class 
* that inherits from this class via canonLineFactory()
*/
class canonLine {

  public:
    /// return the canon-line as a string
    const std::string getLine() { return myLine; };
    /// return Pose at start of this move
    const Pose getStart() { return status.getStartPose(); };
    /// return the Pose at end of this move
    const Pose getEnd() { return status.getEndPose(); };
    int getN(); 
    ///returns the canon line number
    int getLineNum() { return tok2i(0); }
    //const std::string getCanonType();
    /// returns the machine's status after execution of this canon line
    const machineStatus* getStatus() { return &status; }
    /// return type of motion
    virtual MOTION_TYPE getMotionType() { return NOT_DEFINED; } //= 0;
    /// return false for motion
    virtual bool isMotion() { return false; }
    /// return true if this is the end of the nc-program
    virtual bool isNCend() { return false; }
    /// return length of the motion
    virtual double length() { assert(0); return -1; }
    /// return interpolated point at position t along the motion
    virtual Point point(double t) { assert(0); return Point(); }
#ifdef MULTI_AXIS
    virtual Point angle(double t) { assert(0); return Point(); }
#endif
    // produce a canonLine based on string l, and previous machineStatus s
    static canonLine* canonLineFactory (std::string l, machineStatus s);
    
    std::string cantok(unsigned int n);
    const std::string getLnum();

  protected:
    // protected ctor, create through factory
    canonLine(std::string canonL, machineStatus prevStatus); 
    double tok2d(unsigned int n);
    int tok2i(unsigned int n, unsigned int offset=0);
    void tokenize(std::string str, std::vector<std::string>& tokenV, const std::string& delimiters = "(), ");    
    const std::string getCanonicalCommand();
    bool cmdMatch(std::string m);
// DATA
    /// the canon-line as a string
    std::string myLine; 
    /// the machine's status *after* execution of this canon line
    machineStatus status; 
    /// the tokens in this canonLine, set after tokenizing myLine
    std::vector<std::string> canonTokens; 
};

} // end namespace

#endif //CANONLINE_HH
