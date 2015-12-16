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

#ifndef HELICALMOTION_HH
#define HELICALMOTION_HH

#include <string>
#include <vector>
#include <cmath>
#include <limits.h>

#include "point.hpp"
#include "canonMotion.hpp"
#include "canonLine.hpp"
#include "machineStatus.hpp"

namespace g2m {

/**
\class helicalMotion
\brief For the canonical command ARC_FEED.
This class handles both planar arcs and helical arcs. Inherits from canonMotion.
*/

class helicalMotion: protected canonMotion {
  friend canonLine* canonLine::canonLineFactory(std::string l, machineStatus s);

  public:
    /// create helical motion
    helicalMotion(std::string canonL, machineStatus prevStatus);
    MOTION_TYPE getMotionType() {return HELICAL;};
    /// return interpolated point along helix, a distance s from the start
    Point point(double s);
#ifdef MULTI_AXIS
    Point angle(double s);
#endif
    /// return the length of this helix move
    double length();     

  private:    
    void rotate(double &x, double &y, double c, double s);
    
// DATA, this corresponds to the "tokens" on the ARC_FEED canon-line
    double x1,y1,z1; // endpoint for this move
    double a,b,c;    // abc axis endpoints
    double rot;      // rotation for this move
    double cx,cy;    // center-point for this move

// these are the parameters calculated from the canon-params x1,y1,z1,a,b,c,rot,cx,cy
    unsigned int X, Y, Z; // for shuffling around coordinate-indexes
    double d[6]; // 6-axis delta for this move, applies to linear interpolation
    double o[6]; // o=origin/start-point
    double dtheta; // change in angle for this move
    double tx,ty; // vector from start to center
    double radius;
};

} // end namespace 

#endif //HELICALMOTION_HH
