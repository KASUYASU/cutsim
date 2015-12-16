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
#include "linearMotion.hpp"

#include <string>
#include <cassert>

#include "machineStatus.hpp"
#include "canonMotion.hpp"
#include "canonLine.hpp"

namespace g2m {

linearMotion::linearMotion(std::string canonL, machineStatus prevStatus): canonMotion(canonL,prevStatus) {
  status.setMotionType(getMotionType());
  status.setEndPose(getPoseFromCmd());
  //Point start, end;
  start = status.getStartPose().loc + status.getOrigin().loc;
  end = status.getEndPose().loc + status.getOrigin().loc;
#ifdef MULTI_AXIS
  //Point startDir, endDir;
//  startDir = (status.getStartPose().dir + status.getOrigin().dir) * (SIGN * PI/180.0);
  startDir.x = (status.getStartPose().dir.x + status.getOrigin().dir.x) * (SIGN_A * PI/180.0);
  startDir.y = (status.getStartPose().dir.y + status.getOrigin().dir.y) * (SIGN_B * PI/180.0);
  startDir.z = (status.getStartPose().dir.z + status.getOrigin().dir.z) * (SIGN_C * PI/180.0);
//  endDir = (status.getEndPose().dir + status.getOrigin().dir) * (SIGN * PI/180.0);
  endDir.x = (status.getEndPose().dir.x + status.getOrigin().dir.x) * (SIGN_A * PI/180.0);
  endDir.y = (status.getEndPose().dir.y + status.getOrigin().dir.y) * (SIGN_B * PI/180.0);
  endDir.z = (status.getEndPose().dir.z + status.getOrigin().dir.z) * (SIGN_C * PI/180.0);
#endif
}

Point linearMotion::point(double s) {
    if ( length() == 0.0 ) {
        return start;
    } else {
        double t = s/this->length();
        if ( !(t >= 0.0) || !(t <= 1.0 + CALC_TOLERANCE) )
            std::cout << "linearMotion::point() ERROR at s= " << s << " length= " << length() << " t evaluates to t= " << t << "\n";
        assert( t >= 0.0);  assert( t <= 1.0 + CALC_TOLERANCE );
        return start + (end-start)*t;
    }
}

double linearMotion::length() {
#ifdef MULTI_AXIS
	double llinear = start.Distance(end);
	double lsdir = start.Distance(Point(0.0, 0.0, 0.0));
	double ledir = end.Distance(Point(0.0, 0.0, 0.0));
	double langle = startDir.Distance(endDir);

	langle = ((lsdir > ledir) ? lsdir : ledir) * langle;
	return llinear > langle ? llinear : langle;
#else
    return start.Distance(end);
#endif
}

#ifdef MULTI_AXIS
Point linearMotion::angle(double s) {
    if ( length() == 0.0 ) {
        return startDir;
    } else {
        double t = s/this->length();
        if ( !(t >= 0.0) || !(t <= 1.0 + CALC_TOLERANCE) )
            std::cout << "linearMotion::angle() ERROR at s= " << s << " length= " << length() << " t evaluates to t= " << t << "\n";
        assert( t >= 0.0);  assert( t <= 1.0 + CALC_TOLERANCE );
        return startDir + (endDir-startDir)*t;
    }
}
#endif

//need to return RAPID for rapids...
MOTION_TYPE linearMotion::getMotionType() {
  bool traverse = cmdMatch("STRAIGHT_TRAVERSE");
  if (traverse) {
    return TRAVERSE;
  } else {
    return STRAIGHT_FEED;
  }
}

} // end namespace
