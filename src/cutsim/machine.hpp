/*
 *  Copyright 2015      Kazuyasu Hamada (k-hamada@gifu-u.ac.jp)
*/

#ifndef MACHINE_H
#define MACHINE_H

#include <cmath>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <app/cutsim_def.hpp>

namespace cutsim {

typedef enum {
		NO_LIMIT_ERROR = 0,
		MAX_X_LIMIT			= 0x001,
		MIN_X_LIMIT			= 0x002,
		MAX_Y_LIMIT			= 0x004,
		MIN_Y_LIMIT			= 0x008,
		MAX_Z_LIMIT			= 0x010,
		MIN_Z_LIMIT			= 0x020,
		MAX_A_LIMIT			= 0x040,
		MIN_A_LIMIT			= 0x080,
		MAX_B_LIMIT			= 0x100,
		MIN_B_LIMIT			= 0x200,
		MAX_C_LIMIT			= 0x400,
		MIN_C_LIMIT			= 0x800,
}  MACHINE_LIMIT;

/// OpenGL widget for displaying 3D graphics
class Machine {

  public:
    	Machine();
    	void loadMachineSpec();
#ifdef MULTI_AXIS
    	int checkLimit(double x, double y, double z, double a, double b, double c);
#else
    	int checkLimit(double x, double y, double z);
#endif
//  private:
    	double max_x_limit, min_x_limit;
		double max_y_limit, min_y_limit;
		double max_z_limit, min_z_limit;
		double max_a_limit, min_a_limit;
		double max_b_limit, min_b_limit;
		double max_c_limit, min_c_limit;
		double z_limit_offset;
		double max_feed_rate;
		double traverse_feed_rate;
		double max_spindle_power;

        double holderradius;
        double holderlength;
        double spindleradius;
    	double spindlelength;
};

} // end of namespace

#endif
