/*  
 *  Copyright 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  Copyright 2015      Kazuyasu Hamada (k-hamada@gifu-u.ac.jp)
 *  
 *  This file is part of OpenCAMlib.
 *
 *  OpenCAMlib is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenCAMlib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenCAMlib.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <cassert>
#include <cmath>

#include "volume.hpp"

namespace cutsim {

//************* Sphere **************/

/// sphere at center
SphereVolume::SphereVolume() {
    center = GLVertex(0,0,0);
    radius = 1.0;
    calcBB();
}

double SphereVolume::dist(const GLVertex& p ) const {
    double d = (center-p).norm();
    return radius-d; // positive inside. negative outside.
}

/// set the bounding box values
void SphereVolume::calcBB() {
    bb.clear();
    GLVertex maxpt = GLVertex(center.x + radius, center.y + radius, center.z + radius);
    GLVertex minpt = GLVertex(center.x - radius, center.y - radius, center.z - radius);
    bb.addPoint( maxpt );
    bb.addPoint( minpt );
}

//************* Rectangle **************/

RectVolume::RectVolume() {
    corner = GLVertex(0,0,0); 
    v1 = GLVertex(1,0,0); 
    v2 = GLVertex(0,1,0);
    v3 = GLVertex(0,0,1);
}

// FIXME??
void RectVolume::calcBB() {
    bb.clear();
    GLVertex maxp;
    GLVertex minp;
    double max_x, max_y, max_z;
    double min_x, min_y, min_z;

    max_x = fmax(fmax(fmax(corner.x, v1.x), v2.x), v3.x);
    max_y = fmax(fmax(fmax(corner.y, v1.y), v2.y), v3.y);
    max_z = fmax(fmax(fmax(corner.z, v1.z), v2.z), v3.z);
    maxp = GLVertex(max_x, max_y, max_z);
    min_x = fmin(fmin(fmin(corner.x, v1.x), v2.x), v3.x);
    min_y = fmin(fmin(fmin(corner.y, v1.y), v2.y), v3.y);
    min_z = fmin(fmin(fmin(corner.z, v1.z), v2.z), v3.z);
    minp = GLVertex(min_x, min_y, min_z);
    bb.addPoint( maxp  );
    bb.addPoint( minp );
}

double RectVolume::dist(const GLVertex& p) const {
    // translate to origo
    double max_x = corner.x + v1.x;
    double min_x = corner.x;
    double max_y = corner.y + v2.y;
    double min_y = corner.y;
    double max_z = corner.z + v3.z;
    double min_z = corner.z;
    double dOut = 0.0;

    if ( (min_x <= p.x) && (p.x <= max_x) && (min_y <= p.y) && (p.y <= max_y) && (min_z <= p.z) && (p.z <= max_z) )   {
        double xdist,ydist,zdist;
        if ( (p.x-min_x) > (max_x-p.x) )
            xdist = max_x-p.x;                    
        else
            xdist = p.x-min_x;

        if ( (p.y-min_y) > (max_y-p.y) )
            ydist = max_y-p.y;                    
        else
            ydist = p.y-min_y;

        if ( (p.z-min_z) > (max_z-p.z) )
            zdist = max_z-p.z;                    
        else
            zdist = p.z-min_z;

        if ( xdist <= ydist && xdist <= zdist )
            dOut = -xdist;
        else if ( ydist < xdist && ydist < zdist )
            dOut = -ydist;
        else if ( zdist < xdist && zdist< xdist )
            dOut = -zdist;
        else {
            assert(0);
            return -1;
        }
    }else if ( (min_y <= p.y) && (p.y <= max_y) && (min_z <= p.z) && (p.z <= max_z) )   {
        if (p.x < min_x) {
            dOut = min_x - p.x;
        } else if ( p.x > max_x ) {
            dOut = p.x-max_x;
        }
    }else if ( (min_x <= p.x) && (p.x <= max_x) && (min_z <= p.z) && (p.z <= max_z) )   {
        if (p.y < min_y) {
            dOut = min_y - p.y;
        } else if ( p.y > max_y ) {
            dOut = p.y-max_y;
        }
    }else if ( (min_x <= p.x) && (p.x <= max_x) && (min_y <= p.y) && (p.y <= max_y) )   {
        if (p.z < min_z) {
            dOut = min_z - p.z;
        }else if ( p.z > max_z ) {
            dOut = p.z-max_z;
        }
    }else if ( (p.x > max_x) && (p.y > max_y))
        dOut = sqrt((p.x - max_x)*(p.x - max_x)+(p.y - max_y)*(p.y - max_y));
    else if ( (p.x > max_x) && (p.z < min_z))
        dOut = sqrt((p.x - max_x)*(p.x - max_x)+(min_z - p.z)*(min_z - p.z));
    else if ( (p.x < min_x) && (p.y > max_y))
        dOut = sqrt((min_x - p.x)*(min_x - p.x)+(p.y - max_y)*(p.y - max_y));
    else if ( (p.y > max_y) && (p.z > max_z))
        dOut = sqrt((p.y - max_y)*(p.y - max_y)+(p.z - max_z)*(p.z - max_z));
    else if ( (p.x > max_x) && (p.z > max_z))
        dOut = sqrt((p.x - max_x)*(p.x - max_x)+(p.z - max_z)*(p.z - max_z));
    else if ( (p.x > max_x) && (p.y < min_y))
        dOut = sqrt((p.x - max_x)*(p.x - max_x)+(min_y - p.y)*(min_y - p.y));
    else if ( (p.x < min_x) && (p.y < min_y))
        dOut = sqrt((min_x - p.x)*(min_x - p.x)+(p.y - max_y)*(p.y - max_y));
    else if ( (p.y < min_y) && (p.z > max_z))
        dOut = sqrt((min_y - p.y)*(min_y - p.y)+(p.z - max_z)*(p.z - max_z));
    else if ( (p.x < min_x) && (p.z < min_z) )
        dOut = sqrt((p.x - max_x)*(p.x - max_x)+(min_z - p.z)*(min_z - p.z));
    else if ( (p.y > max_y) && (p.z < min_z))
        dOut = sqrt((p.y - max_y)*(p.y - max_y)+(min_y - p.y)*(min_y - p.y));
    else if ( (p.x < min_x) && (p.z > max_z))
        dOut = sqrt((min_x - p.x)*(min_x - p.x)+(p.z - max_z)*(p.z - max_z));
    else if ( (p.y < min_y) && (p.z < min_z))
        dOut = sqrt((min_y - p.y)*(min_y - p.y)+(min_z - p.z)*(min_z - p.z));
        
    return -dOut;
}
    
RectVolume2::RectVolume2() {
    corner = GLVertex(0,0,0);
    width = 1.0;
    length = 1.0;
    hight = 1.0;
    center = GLVertex(corner.x+width*0.5, corner.y+length*0.5, corner.z+hight*0.5);	// center is located at the center of box
    rotationCenter = GLVertex(0, 0, 0);
    angle = GLVertex(0, 0, 0);
}

void RectVolume2::calcBB() {
	GLVertex p[8] = { center + GLVertex( width * 0.5,  length * 0.5,    0.0 ),
					  center + GLVertex(-width * 0.5,  length * 0.5,    0.0 ),
					  center + GLVertex(-width * 0.5, -length * 0.5,    0.0 ),
					  center + GLVertex( width * 0.5, -length * 0.5,    0.0 ),
					  center + GLVertex( width * 0.5,  length * 0.5,  hight ),
					  center + GLVertex(-width * 0.5,  length * 0.5,  hight ),
					  center + GLVertex(-width * 0.5, -length * 0.5,  hight ),
					  center + GLVertex( width * 0.5, -length * 0.5,  hight ) };

	for (int n=0; n<8; n++) {
		p[n].rotate(rotationCenter, GLVertex(1,0,0), angle.x);
		p[n].rotate(rotationCenter, GLVertex(0,0,1), angle.z);
	}

	GLVertex maxpt;
	GLVertex minpt;
	maxpt.x = fmax(fmax(fmax(fmax(fmax(fmax(fmax(p[0].x, p[1].x), p[2].x), p[3].x), p[4].x), p[5].x), p[6].x), p[7].x) + TOLERANCE;
	maxpt.y = fmax(fmax(fmax(fmax(fmax(fmax(fmax(p[0].y, p[1].y), p[2].y), p[3].y), p[4].y), p[5].y), p[6].y), p[7].y) + TOLERANCE;
	maxpt.z = fmax(fmax(fmax(fmax(fmax(fmax(fmax(p[0].z, p[1].z), p[2].z), p[3].z), p[4].z), p[5].z), p[6].z), p[7].z) + TOLERANCE;
	minpt.x = fmin(fmin(fmin(fmin(fmin(fmin(fmin(p[0].x, p[1].x), p[2].x), p[3].x), p[4].x), p[5].x), p[6].x), p[7].x) - TOLERANCE;
	minpt.y = fmin(fmin(fmin(fmin(fmin(fmin(fmin(p[0].y, p[1].y), p[2].y), p[3].y), p[4].y), p[5].y), p[6].y), p[7].y) - TOLERANCE;
	minpt.z = fmin(fmin(fmin(fmin(fmin(fmin(fmin(p[0].z, p[1].z), p[2].z), p[3].z), p[4].z), p[5].z), p[6].z), p[7].z) - TOLERANCE;
	bb.clear();
    bb.addPoint( maxpt );
    bb.addPoint( minpt );
}

double RectVolume2::dist(const GLVertex& p) const {
    GLVertex rotated_p = p - rotationCenter;
    rotated_p = rotated_p.rotateAC(angle.x, angle.z) + rotationCenter;
    double max_x = corner.x + width;
    double min_x = corner.x;
    double max_y = corner.y + length;
    double min_y = corner.y;
    double max_z = corner.z + hight;
    double min_z = corner.z;
    double dOut = 0.0;

    if ( (min_x <= rotated_p.x) && (rotated_p.x <= max_x) && (min_y <= rotated_p.y) && (rotated_p.y <= max_y) && (min_z <= rotated_p.z) && (rotated_p.z <= max_z) )   {
        double xdist,ydist,zdist;
        if ( (rotated_p.x-min_x) > (max_x-rotated_p.x) )
            xdist = max_x-rotated_p.x;
        else
            xdist = rotated_p.x-min_x;

        if ( (rotated_p.y-min_y) > (max_y-rotated_p.y) )
            ydist = max_y-rotated_p.y;
        else
            ydist = rotated_p.y-min_y;

        if ( (rotated_p.z-min_z) > (max_z-rotated_p.z) )
            zdist = max_z-rotated_p.z;
        else
            zdist = rotated_p.z-min_z;

        if ( xdist <= ydist && xdist <= zdist )
            dOut = -xdist;
        else if ( ydist < xdist && ydist < zdist )
            dOut = -ydist;
        else if ( zdist < xdist && zdist< xdist )
            dOut = -zdist;
        else {
            assert(0);
            return -1;
        }
    } else if ( (min_y <= rotated_p.y) && (rotated_p.y <= max_y) && (min_z <= rotated_p.z) && (rotated_p.z <= max_z) )   {
        if (rotated_p.x < min_x) {
            dOut = min_x - rotated_p.x;
        } else if ( rotated_p.x > max_x ) {
            dOut = rotated_p.x-max_x;
        }
    } else if ( (min_x <= rotated_p.x) && (rotated_p.x <= max_x) && (min_z <= rotated_p.z) && (rotated_p.z <= max_z) )   {
        if (rotated_p.y < min_y) {
            dOut = min_y - rotated_p.y;
        } else if ( rotated_p.y > max_y ) {
            dOut = rotated_p.y-max_y;
        }
    } else if ( (min_x <= rotated_p.x) && (rotated_p.x <= max_x) && (min_y <= rotated_p.y) && (rotated_p.y <= max_y) )   {
        if (rotated_p.z < min_z) {
            dOut = min_z - rotated_p.z;
        } else if ( rotated_p.z > max_z ) {
            dOut = rotated_p.z-max_z;
        }
    } else if ( (rotated_p.x > max_x) && (rotated_p.y > max_y))
        dOut = sqrt((rotated_p.x - max_x)*(rotated_p.x - max_x)+(rotated_p.y - max_y)*(rotated_p.y - max_y));
    else if ( (rotated_p.x > max_x) && (rotated_p.z < min_z))
        dOut = sqrt((rotated_p.x - max_x)*(rotated_p.x - max_x)+(min_z - rotated_p.z)*(min_z - rotated_p.z));
    else if ( (rotated_p.x < min_x) && (rotated_p.y > max_y))
        dOut = sqrt((min_x - rotated_p.x)*(min_x - rotated_p.x)+(rotated_p.y - max_y)*(rotated_p.y - max_y));
    else if ( (rotated_p.y > max_y) && (rotated_p.z > max_z))
        dOut = sqrt((rotated_p.y - max_y)*(rotated_p.y - max_y)+(rotated_p.z - max_z)*(rotated_p.z - max_z));
    else if ( (rotated_p.x > max_x) && (rotated_p.z > max_z))
        dOut = sqrt((rotated_p.x - max_x)*(rotated_p.x - max_x)+(rotated_p.z - max_z)*(rotated_p.z - max_z));
    else if ( (rotated_p.x > max_x) && (rotated_p.y < min_y))
        dOut = sqrt((rotated_p.x - max_x)*(rotated_p.x - max_x)+(min_y - rotated_p.y)*(min_y - rotated_p.y));
    else if ( (rotated_p.x < min_x) && (rotated_p.y < min_y))
        dOut = sqrt((min_x - rotated_p.x)*(min_x - rotated_p.x)+(rotated_p.y - max_y)*(rotated_p.y - max_y));
    else if ( (rotated_p.y < min_y) && (rotated_p.z > max_z))
        dOut = sqrt((min_y - rotated_p.y)*(min_y - rotated_p.y)+(rotated_p.z - max_z)*(rotated_p.z - max_z));
    else if ( (rotated_p.x < min_x) && (rotated_p.z < min_z) )
        dOut = sqrt((rotated_p.x - max_x)*(rotated_p.x - max_x)+(min_z - rotated_p.z)*(min_z - rotated_p.z));
    else if ( (rotated_p.y > max_y) && (rotated_p.z < min_z))
        dOut = sqrt((rotated_p.y - max_y)*(rotated_p.y - max_y)+(min_y - rotated_p.y)*(min_y - rotated_p.y));
    else if ( (rotated_p.x < min_x) && (rotated_p.z > max_z))
        dOut = sqrt((min_x - rotated_p.x)*(min_x - rotated_p.x)+(rotated_p.z - max_z)*(rotated_p.z - max_z));
    else if ( (rotated_p.y < min_y) && (rotated_p.z < min_z))
        dOut = sqrt((min_y - rotated_p.y)*(min_y - rotated_p.y)+(min_z - rotated_p.z)*(min_z - rotated_p.z));

    return -dOut;
}

//************* Cylinder **************/

CylinderVolume::CylinderVolume() {
    radius = 1.0;
    length = 1.0;
    center = GLVertex(0, 0, 0);	// center is located at the bottom of cylinder
    rotationCenter = GLVertex(0, 0, 0);
    angle  = GLVertex(0, 0, 0);
}

void CylinderVolume::calcBB() {
	GLVertex p[8] = { center + GLVertex( radius,  radius,    0.0 ),
					  center + GLVertex(-radius,  radius,    0.0 ),
					  center + GLVertex(-radius, -radius,    0.0 ),
					  center + GLVertex( radius, -radius,    0.0 ),
					  center + GLVertex( radius,  radius, length ),
					  center + GLVertex(-radius,  radius, length ),
					  center + GLVertex(-radius, -radius, length ),
					  center + GLVertex( radius, -radius, length ) };

	for (int n=0; n<8; n++) {
		p[n].rotate(rotationCenter, GLVertex(1,0,0), angle.x);
		p[n].rotate(rotationCenter, GLVertex(0,0,1), angle.z);
	}

	GLVertex maxpt;
	GLVertex minpt;
	maxpt.x = fmax(fmax(fmax(fmax(fmax(fmax(fmax(p[0].x, p[1].x), p[2].x), p[3].x), p[4].x), p[5].x), p[6].x), p[7].x) + TOLERANCE;
	maxpt.y = fmax(fmax(fmax(fmax(fmax(fmax(fmax(p[0].y, p[1].y), p[2].y), p[3].y), p[4].y), p[5].y), p[6].y), p[7].y) + TOLERANCE;
	maxpt.z = fmax(fmax(fmax(fmax(fmax(fmax(fmax(p[0].z, p[1].z), p[2].z), p[3].z), p[4].z), p[5].z), p[6].z), p[7].z) + TOLERANCE;
	minpt.x = fmin(fmin(fmin(fmin(fmin(fmin(fmin(p[0].x, p[1].x), p[2].x), p[3].x), p[4].x), p[5].x), p[6].x), p[7].x) - TOLERANCE;
	minpt.y = fmin(fmin(fmin(fmin(fmin(fmin(fmin(p[0].y, p[1].y), p[2].y), p[3].y), p[4].y), p[5].y), p[6].y), p[7].y) - TOLERANCE;
	minpt.z = fmin(fmin(fmin(fmin(fmin(fmin(fmin(p[0].z, p[1].z), p[2].z), p[3].z), p[4].z), p[5].z), p[6].z), p[7].z) - TOLERANCE;
	bb.clear();
    bb.addPoint( maxpt );
    bb.addPoint( minpt );
}

double CylinderVolume::dist(const GLVertex& p) const {
    GLVertex rotated_p = p - rotationCenter;
    rotated_p = rotated_p.rotateAC(angle.x, angle.z) + rotationCenter;
    GLVertex tb = rotated_p - center;
    GLVertex tt = rotated_p - (center + GLVertex(0.0, 0.0, length));
    double d = (rotated_p - GLVertex(center.x, center.y, rotated_p.z)).norm();

    if (tb.z >= 0 && tt.z <= 0)
    	return ((radius - d < tb.z) && (radius - d < -tt.z)) ? radius - d : (tb.z < -tt.z) ? tb.z : -tt.z;  // positive inside. negative outside.
    else if (tb.z < 0) {
		// if we are under the cylinder, then return distance to flat cylinder bottom
    	if (d < radius)
    		return tb.z;
    	else {
    		// outside the cylinder, return a distance to the outer lower "ring" of the cylinder
    		GLVertex n = GLVertex(tb.x, tb.y, 0.0);
    		n = n * (radius / d);  // 1/d means normalization
			return -((tb - n).norm());
    	}
    } else {
		 // if we are above the cylinder, then return distance to flat cylinder top
    	if (d < radius)
			    return -tt.z;
    	else {
			  // outside the cylinder, return a distance to the outer upper "ring" of the cylinder
  		    GLVertex n = GLVertex(tt.x, tt.y, 0.0);
  		    n = n * (radius / d);  // 1/d means normalization
  		    return -((tt - n).norm());
  	   }
   }
}

//************* STL **************/

StlVolume::StlVolume() {
    center = GLVertex(0, 0, 0);	// center is treated as the origin's offset of STL
    rotationCenter = GLVertex(0, 0, 0);
    angle  = GLVertex(0, 0, 0);
}

void StlVolume::calcBB() {
    GLVertex maxpt;
    GLVertex minpt;
    for (int i=0; i < (int)facets.size(); i++) {
    	facets[i]->v1 += center; facets[i]->v2 += center; facets[i]->v3 += center;
    	facets[i]->normal = facets[i]->normal.rotateAC(angle.x, angle.z);
    	GLVertex v1p = facets[i]->v1 - rotationCenter;
    	facets[i]->v1 = v1p.rotateAC(angle.x, angle.z) + rotationCenter;
    	GLVertex v2p = facets[i]->v2 - rotationCenter;
    	facets[i]->v2 = v2p.rotateAC(angle.x, angle.z) + rotationCenter;
    	GLVertex v3p = facets[i]->v3 - rotationCenter;
    	facets[i]->v3 = v3p.rotateAC(angle.x, angle.z) + rotationCenter;
    }
    if (facets.size()) {
        maxpt.x = fmax(fmax(facets[0]->v1.x, facets[0]->v2.x),facets[0]->v3.x);
        maxpt.y = fmax(fmax(facets[0]->v1.y, facets[0]->v2.y),facets[0]->v3.y);
        maxpt.z = fmax(fmax(facets[0]->v1.z, facets[0]->v2.z),facets[0]->v3.z);
        minpt.x = fmin(fmin(facets[0]->v1.x, facets[0]->v2.x),facets[0]->v3.x);
        minpt.y = fmin(fmin(facets[0]->v1.y, facets[0]->v2.y),facets[0]->v3.y);
        minpt.z = fmin(fmin(facets[0]->v1.z, facets[0]->v2.z),facets[0]->v3.z);
    }
    for (int i=0; i < (int)facets.size(); i++) {
        maxpt.x = fmax(fmax(fmax(facets[i]->v1.x, facets[i]->v2.x),facets[i]->v3.x), maxpt.x);
        maxpt.y = fmax(fmax(fmax(facets[i]->v1.y, facets[i]->v2.y),facets[i]->v3.y), maxpt.y);
        maxpt.z = fmax(fmax(fmax(facets[i]->v1.z, facets[i]->v2.z),facets[i]->v3.z), maxpt.z);
        minpt.x = fmin(fmin(fmin(facets[i]->v1.x, facets[i]->v2.x),facets[i]->v3.x), minpt.x);
        minpt.y = fmin(fmin(fmin(facets[i]->v1.y, facets[i]->v2.y),facets[i]->v3.y), minpt.y);
        minpt.z = fmin(fmin(fmin(facets[i]->v1.z, facets[i]->v2.z),facets[i]->v3.z), minpt.z);
        V21.push_back(facets[i]->v2 - facets[i]->v1);
        invV21dotV21.push_back(1.0/(facets[i]->v2 - facets[i]->v1).dot(facets[i]->v2 - facets[i]->v1));
        V32.push_back(facets[i]->v3 - facets[i]->v2);
        invV32dotV32.push_back(1.0/(facets[i]->v3 - facets[i]->v2).dot(facets[i]->v3 - facets[i]->v2));
        V13.push_back(facets[i]->v1 - facets[i]->v3);
        invV13dotV13.push_back(1.0/(facets[i]->v1 - facets[i]->v3).dot(facets[i]->v1 - facets[i]->v3));
    }
    bb.clear();
    maxpt += GLVertex(TOLERANCE, TOLERANCE, TOLERANCE);
    minpt -= GLVertex(TOLERANCE, TOLERANCE, TOLERANCE);
std::cout << "STL maxpt x:" << maxpt.x << " y: " << maxpt.y << " z:" << maxpt.z  << "\n";
std::cout << "STL minpt x:" << minpt.x << " y: " << minpt.y << " z:" << minpt.z  << "\n";
    bb.addPoint( maxpt );
    bb.addPoint( minpt );
}

double StlVolume::dist(const GLVertex& p) const {
	GLVertex q, r;
	GLVertex n1, n2, n3;
	double s12, s23, s31;
	double min = 1.0e+3, d, ret = -1.0, u, abs_d;
	for (int i=0; i < (int)facets.size(); i++) {
		u = (p - facets[i]->v1).dot(V21[i]) * invV21dotV21[i];
		q = facets[i]->v1 + V21[i] * u;
		d = (q - p).dot(facets[i]->normal);
		if ((abs_d = fabs(d)) > min) continue;
		r = p + facets[i]->normal * d;
		n1 = (r - facets[i]->v1).cross(V13[i]);
		n2 = (r - facets[i]->v2).cross(V21[i]);
		n3 = (r - facets[i]->v3).cross(V32[i]);
		s12 = n1.dot(n2); s23 = n2.dot(n3); s31 = n3.dot(n1);

		if ((s12 * s31 > 0.0) && (s12 * s23 > 0.0) && (s23 * s31 > 0.0))
			if (abs_d < min) {
				min = abs_d;
				ret = d;
				continue;
			}

		if (s12 <= 0.0 && s31 >= 0.0) {
			if (u > 0.0 && u < 1.0)
				/*q21 = q*/;
			else if (u <= 0.0)
				q = facets[i]->v1;
			else
				q = facets[i]->v2;
			abs_d = (q - p).norm();
			if (abs_d < min) {
				d = (q - p).dot(facets[i]->normal);
				if (d > 0.0 + TOLERANCE) {
					min = abs_d + TOLERANCE;
					ret = abs_d;
				} else {
					min = abs_d;
					ret = -abs_d;
				}
			}
		} else if (s31 <= 0.0 && s23 >= 0.0) {
			u = (p - facets[i]->v3).dot(V13[i]) * invV13dotV13[i];
			if (u > 0.0 && u < 1.0)
				q = facets[i]->v3 + V13[i] * u;
			else if (u <= 0.0)
				q = facets[i]->v3;
			else
				q = facets[i]->v1;
			abs_d = (q - p).norm();
			if (abs_d < min) {
				d = (q - p).dot(facets[i]->normal);
				if (d > 0.0 + TOLERANCE) {
					min = abs_d + TOLERANCE;
					ret = abs_d;
				} else {
					min = abs_d;
					ret = -abs_d;
				}
			}
		} else if (s23 <= 0.0 && s12 >= 0.0) {
			u = (p - facets[i]->v2).dot(V32[i]) * invV32dotV32[i];
			if (u > 0.0 && u < 1.0)
				q = facets[i]->v2 + V32[i] * u;
			else if (u <= 0.0)
				q = facets[i]->v2;
			else
				q = facets[i]->v3;
			abs_d = (q - p).norm();
			if (abs_d < min) {
				d = (q - p).dot(facets[i]->normal);
				if (d > 0.0 + TOLERANCE) {
					min = abs_d + TOLERANCE;
					ret = abs_d;
				} else {
					min = abs_d;
					ret = -abs_d;
				}
			}
		}
	}
	return ret;		// positive inside. negative outside.
}


//************* CutterVolume **************/

CutterVolume::CutterVolume() {
    radius = 0.0;
    length = 0.0;
    enableholder = false;
    holderradius = 0.0;
    holderlength = 0.0;
}

void CutterVolume::calcBBHolder() {
    bbHolder.clear();
    GLVertex maxpt = GLVertex(center.x + holderradius + TOLERANCE, center.y + holderradius + TOLERANCE, center.z + length + holderlength + TOLERANCE);
    GLVertex minpt = GLVertex(center.x - holderradius - TOLERANCE, center.y - holderradius - TOLERANCE, center.z + length - TOLERANCE);
    bbHolder.addPoint( maxpt );
    bbHolder.addPoint( minpt );
}

//************* CylCutterVolume **************/

CylCutterVolume::CylCutterVolume() {
    radius = 0.0;
    length = 0.0;
}

void CylCutterVolume::calcBB() {
    bb.clear();
    GLVertex maxpt = GLVertex(center.x + maxradius + TOLERANCE, center.y + maxradius + TOLERANCE, center.z + length + TOLERANCE);
    GLVertex minpt = GLVertex(center.x - maxradius - TOLERANCE, center.y - maxradius - TOLERANCE, center.z - TOLERANCE);
    bb.addPoint( maxpt );
    bb.addPoint( minpt );
    if (enableholder)
        calcBBHolder();
}

double CylCutterVolume::dist(const GLVertex& p) const {
#ifdef MULTI_AXIS
	GLVertex rotated_p = p;
	rotated_p = rotated_p.rotateAC(angle.x, angle.z);
	GLVertex t = rotated_p - center;
	double d = (rotated_p - GLVertex(center.x, center.y, rotated_p.z)).norm();
#else
	GLVertex t = p - center;
	double d =(p - GLVertex(center.x, center.y, p.z)).norm();
#endif

    if (t.z >= 0) {
    	  return t.z > length ? holderradius - d : radius - d;  // positive inside. negative outside.
    } else {
		 // if we are under the cutter, then return distance to flat cutter bottom
		if (d < radius)
			return t.z;
		else {
			// outside the cutter, return a distance to the outer "ring" of the cutter
			GLVertex n = GLVertex(t.x, t.y, 0.0);
			n = n * (radius / d);  // 1/d means normalization
			return -((t - n).norm());
		}
	}
}

Cutting CylCutterVolume::dist_cd(const GLVertex& p) {
#ifdef MULTI_AXIS
    GLVertex rotated_p = p;
    rotated_p = rotated_p.rotateAC(angle.x, angle.z);
    GLVertex t = rotated_p - center;
    double d = (rotated_p - GLVertex(center.x, center.y, rotated_p.z)).norm();
#else
    GLVertex t = p - center;
    double d = (p - GLVertex(center.x, center.y, p.z)).norm();
#endif
    Cutting result = { t.z, NO_COLLISION };
    double rdiff = radius - d;

    if (t.z >= 0) {
    	  result.collision |= ((t.z > flutelength) && ((rdiff = neckradius  - d) > COLLISION_TOLERANCE))  ? NECK_COLLISION   : NO_COLLISION;
    	  result.collision |= ((t.z > reachlength) && ((rdiff = shankradius - d) > COLLISION_TOLERANCE))  ? SHANK_COLLISION  : NO_COLLISION;
    	  result.collision |= ((t.z > length)      && ((rdiff = holderradius - d) > COLLISION_TOLERANCE)) ? HOLDER_COLLISION : NO_COLLISION;
    	  result.f = rdiff < t.z ? rdiff : t.z;  // positive inside. negative outside.
    	  return result;
    } else {
		    // if we are under the cutter, then return distance to flat cutter bottom
    	  if (d < radius)
			   return result;
    	  else {
			  // outside the cutter, return a distance to the outer "ring" of the cutter
    		   GLVertex n = GLVertex(t.x, t.y, 0.0);
    		   n = n * (radius / d);  // 1/d means normalization
    		   result.f = -((t - n).norm());
    		   return result;
    	  }
   }
}

//************* BallCutterVolume **************/

BallCutterVolume::BallCutterVolume() {
    radius = 0.0;
    length = 0.0;
}

void BallCutterVolume::calcBB() {
    bb.clear();
    GLVertex maxpt = GLVertex(center.x + maxradius + TOLERANCE, center.y + maxradius + TOLERANCE, center.z + length + TOLERANCE);
    GLVertex minpt = GLVertex(center.x - maxradius - TOLERANCE, center.y - maxradius - TOLERANCE, center.z - radius - TOLERANCE);
    bb.addPoint( maxpt );
    bb.addPoint( minpt );
    if (enableholder)
        calcBBHolder();
}

double BallCutterVolume::dist(const GLVertex& p) const {
#ifdef MULTI_AXIS
    GLVertex rotated_p = p;
    rotated_p = rotated_p.rotateAC(angle.x, angle.z);
    GLVertex t = rotated_p - center;
#else
    GLVertex t = p - center;
#endif

    if (t.z < 0)
#ifdef MULTI_AXIS
      return radius - (rotated_p - GLVertex(center.x, center.y, center.z)).norm(); // positive inside. negative outside.
#else
      return radius - (center - p).norm(); // positive inside. negative outside.
#endif
    else
#ifdef MULTI_AXIS
    return radius - (rotated_p - GLVertex(center.x, center.y, rotated_p.z)).norm();
#else
    return radius - (p - GLVertex(center.x, center.y, p.z)).norm();
#endif
}

Cutting BallCutterVolume::dist_cd(const GLVertex& p) {
#ifdef MULTI_AXIS
    GLVertex rotated_p = p;
    rotated_p = rotated_p.rotateAC(angle.x, angle.z);
    GLVertex t = rotated_p - center;
#else
    GLVertex t = p - center;
#endif
    Cutting result = { 0.0, NO_COLLISION };

    if (t.z < 0) {
#ifdef MULTI_AXIS
    	  result.f = radius - (rotated_p - GLVertex(center.x, center.y, center.z)).norm();
#else
    	  result.f = radius - (center - p).norm();
#endif
    	  return result;
    } else {
#ifdef MULTI_AXIS
  	  double d = (rotated_p - GLVertex(center.x, center.y, rotated_p.z)).norm();
#else
  	  double d = (p - GLVertex(center.x, center.y, p.z)).norm();
#endif
  	  result.f = radius - d;
  	  result.collision |= ((t.z > flutelength) && ((result.f = neckradius  - d) > COLLISION_TOLERANCE))  ? NECK_COLLISION   : NO_COLLISION;
  	  result.collision |= ((t.z > reachlength) && ((result.f = shankradius - d) > COLLISION_TOLERANCE))  ? SHANK_COLLISION  : NO_COLLISION;
  	  result.collision |= ((t.z > length)      && ((result.f = holderradius - d) > COLLISION_TOLERANCE)) ? HOLDER_COLLISION : NO_COLLISION;
  	  return result;
    }
}

//************* BullCutterVolume **************/
// TOROID 

/*
BullCutterVolume::BullCutterVolume() {
    r1 = 1.0;
    r2 = r1/3.0;
    radius = r1+r2;
    length = 1.0;
    pos = Point(0,0,0);
}

void BullCutterVolume::setPos(Point& p) {
    pos = p;
    calcBB();
}

void BullCutterVolume::calcBB() {
    bb.clear();
    double safety = 1;
    bb.addPoint( pos + Point(safety*radius,safety*radius,safety*length) );
    bb.addPoint( pos + Point(-safety*radius,-safety*radius,-safety*length) );
}

double BullCutterVolume::dist(Point& p) const {
    Point t = p - pos - Point(0,0,r2); // shift up by tube radius
    if (t.z >= 0.0 ) // cylindrical part, above toroid
        return std::max( fabs(t.z)-length , square(t.x) + square(t.y) - square(r1+r2) );
    else if ( square(t.x)+square(t.y) <= square(r1) ) // cylindrical part, inside toroid
                     //was fabs(t.z)-r2
        return std::max( square(t.z)-square(r2) , square(t.x) + square(t.y) - square( r1 ) );
    else // toroid
        return square( square(t.x) + square(t.y) + square(t.z) + square( r1 ) - square( r2 ) ) - 
               4*square(r1)*(square(t.x)+square(t.y));
}
*/

} // end namespace
// end of file volume.cpp
