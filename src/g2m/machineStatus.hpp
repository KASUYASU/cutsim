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
#ifndef MACHINESTATUS_HH
#define MACHINESTATUS_HH

#include <map>
#include <limits.h> //to fix "error: INT_MIN was not declared in this scope"
#include "point.hpp"

namespace g2m {

/// the current plane, XY, YZ, or XZ
enum CANON_PLANE { CANON_PLANE_XY, CANON_PLANE_YZ, CANON_PLANE_XZ };
/// the status of the spindle
enum SPINDLE_STATUS { OFF = 0x100, CW = 0x200, CCW = 0x400, BRAKE = 0x800 };
/// motion type
enum MOTION_TYPE { NOT_DEFINED = 0x0, MOTIONLESS = 0x1, HELICAL = 0x2, STRAIGHT_FEED = 0x4, TRAVERSE = 0x8 };


/// coolant-status
struct coolantStruct {
    /// is flood on?
    bool flood; 
    /// is mist on?
    bool mist; 
    /// is the spindle on?
    bool spindle;
};

/**
\class machineStatus
\brief This class contains the machine's state for one canonical command.
The information stored includes the 
* coolant state, spindle speed and direction, feedrate, start and end pose, tool in use

Important: 'pose' refers to how the machine's axes are positioned, 
* while 'direction' refers to the direction of motion
*/
class machineStatus {
  public:
    machineStatus(machineStatus const& oldStatus);
    machineStatus(Pose initial);
    machineStatus(Pose initial, Pose userOrigin);
    void setMotionType(MOTION_TYPE m);
    void setEndPose(Pose newPose);
    /// set endPose
    void setEndPose(Point p);
    /// set current feedrate
    void setFeed(const double f) { F = f; };
    /// set spindle speed
    void setSpindleSpeed(const double s) { S = s; };
    /// set spindle status
    void setSpindleStatus(const SPINDLE_STATUS s) { spindleStat = s; };
    /// set coolant status
    void setCoolant(coolantStruct c) { coolant = c; };
    /// set the current plane
    void setPlane(CANON_PLANE p) { plane = p; };
    /// set the current origin
    void setOrigin(Pose newOrigin) { origin = newOrigin; };
    /// return the current feedrate
    double getFeed() const { return F; };
    /// return spindle speed
    double getSpindleSpeed() const { return S; };
    /// return the spindle-status structure
    SPINDLE_STATUS getSpindleStatus() const { return spindleStat; };
    /// return the coolant structure
    const coolantStruct getCoolant() { return coolant; };
    /// return startPose
    const Pose getStartPose() { return startPose; };
    /// return endPose
    const Pose getEndPose() { return endPose; };
    /// return the current plane
    CANON_PLANE getPlane() const { return plane; };
    /// return the current origin
    Pose getOrigin() const { return origin; };
    /// set startDir
    void setStartDir( Point d) { startDir = d; };
    /// return startDir
    const Point getStartDir() const { return startDir; };
    /// set endDir
    void setEndDir( Point d) { endDir = d; };
    /// return endDir
    const Point getEndDir() const { return endDir; };
    /// return prevEndDir
    const Point getPrevEndDir() const { return prevEndDir; };
    void clearAll(void);
    /// return first
    bool isFirst() { return first; };
    /// set the tool index
    void setTool(int n); //n is the ID of the tool to be used.
    /// return the current tool index
    int  getTool() const { return myTool; };
    /// return the current spindle status & motion type
    int  getSpindleMotionStatus() const { return (spindleStat | motionType); }

  protected:
    /// machine Pose at start of this move
    Pose startPose;
    /// machine Pose at end of this move
    Pose endPose;
    /// feed-rate
    double F;
    /// spindle speed
    double S;  
    /// coolant status
    coolantStruct coolant;
    /// the direction in which the current move starts
    Point startDir;
    /// the direction in which the current move ends
    Point endDir;
    /// endDir of the previous move
    Point prevEndDir;
    /// flag indicating very first move of g-code program(?)
    bool first;
    /// misc flag(?)
    bool lastMotionWasTraverse;
    /// index of current tool
    int myTool;
    /// the current motion type
    MOTION_TYPE motionType;
    /// the status of the spindle
    SPINDLE_STATUS spindleStat;
    /// the current plane (used e.g. for G2 and G3 moves), either XY, XZ, or YZ
    CANON_PLANE plane;
    /// the current origin
    Pose origin;

  private:
    machineStatus();  //prevent use of this ctor by making it private 
};

} // end namespace

#endif //MACHINESTATUS_HH
