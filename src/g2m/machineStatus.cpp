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

#include <iostream>
#include <sstream>

#include "machineStatus.hpp"

namespace g2m {

/// copy-constructor
machineStatus::machineStatus(const machineStatus& oldStatus) {
    spindleStat = oldStatus.spindleStat;
    F = oldStatus.F;
    S = oldStatus.S;
    coolant = oldStatus.coolant;
    plane = oldStatus.plane;
    origin = oldStatus.origin;
    endPose = startPose = oldStatus.endPose;
    myTool = oldStatus.myTool;
    endDir = Point(0,0,-1);
    prevEndDir = oldStatus.endDir;
    first = oldStatus.first;
    motionType = NOT_DEFINED;
    /// lastMotionWasTraverse gets copied from the previous machine status, 
    /// and only gets changed if the prev status was motion at feedrate 
    /// (this way, motionless cmds don't mess things up)
    lastMotionWasTraverse = oldStatus.lastMotionWasTraverse;
    if ( oldStatus.motionType == STRAIGHT_FEED || oldStatus.motionType == HELICAL) {
        lastMotionWasTraverse = false;
    }
}

/**
This constructor is only to be used when initializing the simulation; it would not be useful elsewhere.
\param initial is the initial pose of the machine, as determined by the interp from the variable file.
\sa machineStatus(machineStatus const& oldStatus)
*/
machineStatus::machineStatus(Pose initial) {
    clearAll();
    startPose = endPose = initial;
    first = true;
    setTool(0);
}

machineStatus::machineStatus(Pose initial, Pose userOrigin) {
    clearAll();
    startPose = endPose = initial;
    origin = userOrigin;
    first = true;
    setTool(0);
}

/// reset machineStatus to reasonable defaults
void machineStatus::clearAll() {
    F=S=0.0;
    plane = CANON_PLANE_XY;
    origin = Pose( Point(0,0,0), Point(0,0,0));
    coolant.flood = false;
    coolant.mist = false;
    coolant.spindle = false;
    endPose = startPose = Pose( Point(0,0,0), Point(0,0,1));
    endDir = prevEndDir = Point(0,0,-1);
    spindleStat = OFF;
    myTool = -1;
    motionType = NOT_DEFINED;
    lastMotionWasTraverse = false;
}

///sets motion type, and checks whether this is the second (or later) motion command.
void machineStatus::setMotionType(MOTION_TYPE m) {
    motionType = m;
    if (motionType == NOT_DEFINED) {
        std::cout << "motion type undef!! \n";
    } else if (motionType == TRAVERSE) {
        lastMotionWasTraverse = true;
    }
    static int count = 0;
    if ((first) && ((m == STRAIGHT_FEED) || (m == TRAVERSE) || (m == HELICAL)) ) {
        if (count == 0)
          count++;
        else
          first = false;
    }
}


///  setEndPose
void machineStatus::setEndPose( Point p) {
#ifdef MULTI_AXIS
	endPose = Pose( p, Point(0,0,0) );
#else
    endPose = Pose( p, Point(0,0,1) );
#endif
}

/// setEndPose
void machineStatus::setEndPose( Pose newPose) {
    endPose = newPose;
}

/// set the current tool index
void machineStatus::setTool(int n) {
    //std::cout << "adding tool " << n << ".\n";
    myTool = n;
    //canon::addTool(n);
}


}


