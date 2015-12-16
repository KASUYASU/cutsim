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
#include <string>
#include <climits>
#include <cassert>

#include "helicalMotion.hpp"
#include "machineStatus.hpp"

namespace g2m {


#define CIRCLE_FUZZ (0.000001) // from libnml/posemath/posemath.h

// example from cds.ngc:
//     231 N2250  ARC_FEED(3.5884, 1.9116, 3.5000, 2.0000, -1, 1.8437, 0.0000, 0.0000, 0.0000)
//tok: 0   1      2        3       4       5       6        7  8       9       10      11 
helicalMotion::helicalMotion(std::string canonL, machineStatus prevStatus): canonMotion(canonL,prevStatus) {
    // ( comments relate to XY-plane )
    // see the rs274 spec, www.isd.mel.nist.gov/documents/kramer/RS274NGC_22.pdf or similar
    // If rotation is positive, the arc is traversed counterclockwise as viewed from the positive end of
    // the coordinate axis perpendicular to the currently selected plane. If rotation is negative, the
    // arc is traversed clockwise.
    
    x1 = tok2d(3); // first_end   (x-coord of endpoint)
    y1 = tok2d(4); // second_end  (y-coord of endpoint
    cx = tok2d(5); // first_axis   (x-coord of centerpoint)
    cy = tok2d(6); // second_axis  (y-coord of centerpoint)
    rot = tok2d(7); //rotation (ccw if rotation==1,cw if rotation==-1), for multipple turns we can have -2, +2, etc.
    z1 = tok2d(8); // z-coord of endpoint
    a = tok2d(9);  // a
    b = tok2d(10); // b
    c = tok2d(11); // c
    start = status.getStartPose().loc + status.getOrigin().loc;
#ifdef MULTI_AXIS
//    startDir = (status.getStartPose().dir + status.getOrigin().dir) * (SIGN * PI/180.0);
    startDir.x = (status.getStartPose().dir.x + status.getOrigin().dir.x) * (SIGN_A * PI/180.0);
    startDir.y = (status.getStartPose().dir.y + status.getOrigin().dir.y) * (SIGN_B * PI/180.0);
    startDir.z = (status.getStartPose().dir.z + status.getOrigin().dir.z) * (SIGN_C * PI/180.0);
#endif
    
    /*
    switch (status.getPlane()) {
        case CANON_PLANE_XZ:
            status.setEndPose(Point(y1,z1,x1));
            break;
        case CANON_PLANE_YZ:
            status.setEndPose(Point(z1,x1,y1));
            break;
        case CANON_PLANE_XY:
            default:
            
    }*/
    
    status.setMotionType(HELICAL);
    
    
    
    // code adapted from emc2: src/emc/rs274ngc/gcodemodule.cc 
    // function rs274_arc_to_segments()
    double n[6]; // n=endpoint,
    //double o[6]; // o=origin/start-point
    // numbering of axes, depending on plane
    if ( status.getPlane() == CANON_PLANE_XY)  { // XY-plane
        X=0; Y=1; Z=2;
    } else if (status.getPlane() == CANON_PLANE_YZ) { // YZ-plane
        X=2; Y=0; Z=1;
    } else if (status.getPlane() == CANON_PLANE_XZ) {
        X=1; Y=2; Z=0; // XZ-plane
    } 
    n[X] = x1 + status.getOrigin().loc.x; // end-point, first-coord
    n[Y] = y1 + status.getOrigin().loc.y; // end-point, second-coord
    n[Z] = z1 + status.getOrigin().loc.z; // end-point, third-coord, i.e helix translation
#ifdef MULTI_AXIS
    n[3] = (a + status.getOrigin().dir.x) * (SIGN_A * PI/180.0);
    n[4] = (b + status.getOrigin().dir.y) * (SIGN_B * PI/180.0);
    n[5] = (c + status.getOrigin().dir.z) * (SIGN_C * PI/180.0);
#else
    n[3] = a;
    n[4] = b;
    n[5] = c;
#endif
    
#ifdef MULTI_AXIS
    status.setEndPose( Pose( Point( n[X], n[Y], n[Z]) - status.getOrigin().loc, Point( a, b, c ) ) );
    endDir = Point( n[3], n[4], n[5] );
#else
    status.setEndPose( Point( n[X], n[Y], n[Z]) );
#endif
    end = status.getEndPose().loc;

    o[0] = start.x;
    o[1] = start.y;
    o[2] = start.z;
#ifdef MULTI_AXIS
    o[3] = startDir.x;
    o[4] = startDir.y;
    o[5] = startDir.z;
#else
    o[3] = 0; //FIXME
    o[4] = 0; //FIXME
    o[5] = 0; //FIXME
#endif
    double theta1 = atan2( o[Y]-cy, o[X]-cx); // angle of vector from center to start
    double theta2 = atan2( n[Y]-cy, n[X]-cx); // angle of vector from center to end
    if(rot < 0) { 
        while(theta2 - theta1 > -CIRCLE_FUZZ) 
            theta2 -= 2 * PI;
    } else { 
        while(theta2 - theta1 < CIRCLE_FUZZ) 
            theta2 += 2 * PI;
    }
    // if multi-turn, add the right number of full circles
    if(rot < -1) 
        theta2 += 2 * PI * (rot+1);
    if(rot > 1) 
        theta2 += 2 * PI * (rot-1);
    // number of steps
    //int steps = std::max( 3, int(max_segments * fabs(theta1 - theta2) / PI) ); //  max_segments per PI of rotation
    //double rsteps = 1. / steps;
    
    dtheta = theta2 - theta1; // the rotation angle for this helix
    
    // n is endpoint
    // o is startpoint
    // d is diff.   x,y,z,a,b,c
    d[0]=0; d[1]=0; d[2]=0;
    d[3]=n[3]-o[3];
    d[4]=n[4]-o[4];
    d[5]=n[5]-o[5];
    d[Z] = n[Z] - o[Z]; // helix-translation diff
    tx = o[X] - cx; // center to start 
    ty = o[Y] - cy; // center to start
    radius = sqrt( tx*tx + ty*ty );
    //double dc = cos(dtheta*rsteps); // delta-cos  
    //double ds = sin(dtheta*rsteps); // delta-sin
    
    /*
    std::vector<Point> output;
    for(int i=0; i<steps; i++) {
        double f = (i+1) * rsteps; // varies from 1/rsteps..1 (?)
        double p[6]; // output point
        rotate(tx, ty, dc, ds); // rotate center-start vector by a small amount
        p[X] = tx + cx; // center + rotated point
        p[Y] = ty + cy;
        p[Z] = o[Z] + d[Z] * f; // start + helix-translation
        // simple proportional translation
        p[3] = o[3] + d[3] * f;
        p[4] = o[4] + d[4] * f;
        p[5] = o[5] + d[5] * f;
        Point pt( p[0], p[1], p[2] );
        output.push_back(pt);
    }*/
}

double helicalMotion::length() { 
#ifdef MULTI_AXIS
	double c = d[Z]/dtheta;
	double lsdir = start.Distance(Point(0.0, 0.0, 0.0));
	double ledir = end.Distance(Point(0.0, 0.0, 0.0));
	double langle = startDir.Distance(endDir);
	double lhelical = fabs(dtheta)*sqrt(radius*radius + c*c);

	langle = ((lsdir > ledir) ? lsdir : ledir) * langle;
	return lhelical > langle ? lhelical : langle;
#else
	// helix:  x= a*cos(t)     y=a*sin(t)   z=c*t
    
    // d[Z] is rise during dtheta radians
    // so d[Z]/dtheta is rise/radian
    // and (d[Z]/dtheta)
    double c = d[Z]/dtheta;
    // c= rise in one turn divided by 2pi
    // t in [0,T]
    // helix length L =T*sqrt(a^2 + c^2)
    return fabs(dtheta)*sqrt(radius*radius + c*c);
#endif
}

Point helicalMotion::point(double s) {
    // 0) relate s to t=[0...1]  and theta=[0...dtheta]
    double t= s/this->length();
    assert( t >= 0.0);  assert( t <= 1.0 + CALC_TOLERANCE );
    double theta = t*dtheta;
    // 1) rotate center-start vector
    double cos_t = cos(theta);
    double sin_t = sin(theta);
    double txr(tx), tyr(ty);
    rotate(txr,tyr,cos_t,sin_t); 
    // 2) center + center-start
    double p[6]; // the output-point
    p[X] = cx + txr;
    p[Y] = cy + tyr;
    p[Z] = o[Z] + t*d[Z]; 
    p[3] = o[3] + d[3] * t;
    p[4] = o[4] + d[4] * t;
    p[5] = o[5] + d[5] * t;
    return Point( p[0], p[1], p[2] );
}

// rotate by cos/sin. from emc2 gcodemodule.cc
void helicalMotion::rotate(double &x, double &y, double c, double s) {
    double tx = x * c - y * s;
    y = x * s + y * c;
    x = tx;
}

#ifdef MULTI_AXIS
Point helicalMotion::angle(double s) {
    double t= s/this->length();
    assert( t >= 0.0);  assert( t <= 1.0 + CALC_TOLERANCE );
    double p[6]; // the output-point
    p[3] = o[3] + d[3] * t;
    p[4] = o[4] + d[4] * t;
    p[5] = o[5] + d[5] * t;
    return Point( p[3], p[4], p[5] );;
}
#endif

} // end namespace




#ifdef EMC2_GCODEMODULE_RS274ARC_CODE
static PyObject *rs274_arc_to_segments(PyObject *self, PyObject *args) {
    //PyObject *canon;
    double x1, y1, cx, cy, z1, a, b, c, u, v, w;
    double o[9], n[9], g5xoffset[9], g92offset[9];
    int rot, plane;
    int X, Y, Z;
    double rotation_cos, rotation_sin;
    int max_segments = 128;


    
        
    if(!get_attr(canon, "plane", &plane)) return NULL;
    if(!get_attr(canon, "rotation_cos", &rotation_cos)) return NULL; // rotation-offsets
    if(!get_attr(canon, "rotation_sin", &rotation_sin)) return NULL; // rotation-offset
    
    
    if(!get_attr(canon, "g5x_offset_x", &g5xoffset[0])) return NULL;
    if(!get_attr(canon, "g5x_offset_y", &g5xoffset[1])) return NULL;
    if(!get_attr(canon, "g5x_offset_z", &g5xoffset[2])) return NULL;
    if(!get_attr(canon, "g5x_offset_a", &g5xoffset[3])) return NULL;
    if(!get_attr(canon, "g5x_offset_b", &g5xoffset[4])) return NULL;
    if(!get_attr(canon, "g5x_offset_c", &g5xoffset[5])) return NULL;
    if(!get_attr(canon, "g5x_offset_u", &g5xoffset[6])) return NULL;
    if(!get_attr(canon, "g5x_offset_v", &g5xoffset[7])) return NULL;
    if(!get_attr(canon, "g5x_offset_w", &g5xoffset[8])) return NULL;
    if(!get_attr(canon, "g92_offset_x", &g92offset[0])) return NULL;
    if(!get_attr(canon, "g92_offset_y", &g92offset[1])) return NULL;
    if(!get_attr(canon, "g92_offset_z", &g92offset[2])) return NULL;
    if(!get_attr(canon, "g92_offset_a", &g92offset[3])) return NULL;
    if(!get_attr(canon, "g92_offset_b", &g92offset[4])) return NULL;
    if(!get_attr(canon, "g92_offset_c", &g92offset[5])) return NULL;
    if(!get_attr(canon, "g92_offset_u", &g92offset[6])) return NULL;
    if(!get_attr(canon, "g92_offset_v", &g92offset[7])) return NULL;
    if(!get_attr(canon, "g92_offset_w", &g92offset[8])) return NULL;
    */
    
    // numbering of axes, depending on plane
    if(plane == 1) { // XY-plane
        X=0; Y=1; Z=2;
    } else if(plane == 3) { // YZ-plane
        X=2; Y=0; Z=1;
    } else {
        X=1; Y=2; Z=0; // XZ-plane
    }
    n[X] = x1; // end-point, first-coord
    n[Y] = y1; // end-point, second-coord
    n[Z] = z1; // end-point, third-coord, i.e helix translation
    
    n[3] = a;
    n[4] = b;
    n[5] = c;
    
    n[6] = u;
    n[7] = v;
    n[8] = w;
    
    for(int ax=0; ax<9; ax++) 
        o[ax] -= g5xoffset[ax]; // offset
        
    unrotate(o[0], o[1], rotation_cos, rotation_sin); // only rotates in XY (?)
    
    for(int ax=0; ax<9; ax++) 
        o[ax] -= g92offset[ax]; // offset

    double theta1 = atan2( o[Y]-cy, o[X]-cx); // vector from center to start
    
    double theta2 = atan2( n[Y]-cy, n[X]-cx); // vector from center to end
    
    // "unwind" theta2 ?
    if(rot < 0) { // rot = -1
        while(theta2 - theta1 > -CIRCLE_FUZZ) 
            theta2 -= 2 * PI;
    } else { // rot = 1
        while(theta2 - theta1 < CIRCLE_FUZZ) 
            theta2 += 2 * PI;
    }

    // if multi-turn, add the right number of full circles
    if(rot < -1) 
        theta2 += 2 * PI * (rot+1);
    if(rot > 1) 
        theta2 += 2 * PI * (rot-1);
        
    // number of steps
    int steps = std::max( 3, int(max_segments * fabs(theta1 - theta2) / PI) ); //  max_segments per PI of rotation
    double rsteps = 1. / steps;
    double dtheta = theta2 - theta1; // the rotation angle for this helix
    // n is endpoint
    // o is startpoint
    
    // d is diff.   x,y,z,a,b,c,u,v,w 
                // 0  1  2  3          4          5          6          7
    double d[9] = {0, 0, 0, n[4]-o[4], n[5]-o[5], n[6]-o[6], n[7]-o[7], n[8]-o[8]};
    d[Z] = n[Z] - o[Z]; // helix-translation diff

    double tx = o[X] - cx; // center to start 
    double ty = o[Y] - cy; // center to start
    double dc = cos(dtheta*rsteps);  
    double ds = sin(dtheta*rsteps);
    
    PyObject *segs = PyList_New(steps); // list of output segments
    // generate segments:
    for(int i=0; i<steps-1; i++) {
        double f = (i+1) * rsteps; // varies from 0..1 (?)
        double p[9]; // point
        rotate(tx, ty, dc, ds); // rotate center-start vector by a small amount
        p[X] = tx + cx; // center + rotated point
        p[Y] = ty + cy;
        p[Z] = o[Z] + d[Z] * f; // start + helix-translation
        
        // simple proportional translation
        p[3] = o[3] + d[3] * f;
        p[4] = o[4] + d[4] * f;
        p[5] = o[5] + d[5] * f;
        p[6] = o[6] + d[6] * f;
        p[7] = o[7] + d[7] * f;
        p[8] = o[8] + d[8] * f;
        
        // offset
        for(int ax=0; ax<9; ax++) 
            p[ax] += g92offset[ax];
        rotate(p[0], p[1], rotation_cos, rotation_sin); // rotation offset on XY
        // offset
        for(int ax=0; ax<9; ax++) 
            p[ax] += g5xoffset[ax];
        // appends segment to the segs-list (?)
        PyList_SET_ITEM(segs, i,
            Py_BuildValue("ddddddddd", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8]));
    }

// since we reversed g92, rotation-offset, and g5x, these need to be reapplied before
// adding the final point.
    // apply g92 offset
    for(int ax=0; ax<9; ax++) 
        n[ax] += g92offset[ax];
    // rotate
    rotate(n[0], n[1], rotation_cos, rotation_sin);
    // apply g5x offset
    for(int ax=0; ax<9; ax++) 
        n[ax] += g5xoffset[ax];
    
    // apply a final point at the end of the list.
    PyList_SET_ITEM(segs, steps-1,
            Py_BuildValue("ddddddddd", n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8]));
    return segs;
}

#endif
