/*
 *  Copyright 2015      Kazuyasu Hamada (k-hamada@gifu-u.ac.jp)
*/

#include "machine.hpp"

namespace cutsim {

Machine::Machine() {
	max_x_limit = DEFAULT_MAX_X_LIMIT;
	min_x_limit = DEFAULT_MIN_X_LIMIT;
	max_y_limit = DEFAULT_MAX_Y_LIMIT;
	min_y_limit = DEFAULT_MIN_Y_LIMIT;
	max_z_limit = DEFAULT_MAX_Z_LIMIT;
	min_z_limit = DEFAULT_MIN_Z_LIMIT;
	z_limit_offset = 0.0;

	max_a_limit = min_a_limit = max_b_limit = min_b_limit= max_c_limit = min_c_limit = NAN;

	max_feed_rate = DEFAULT_MAX_FEED_RATE;

	traverse_feed_rate = DEFAULT_TRAVERSE_FEED_RATE;

	max_spindle_power = DEFAULT_MAX_SPINDLE_POWER;

	holderradius  = DEFAULT_HOLDER_RADIUS;
	holderlength  = DEFAULT_HOLDER_LENGTH;
	spindleradius = DEFAULT_SPINDLE_RADIUS;
	spindlelength = DEFAULT_SPINDLE_LENGTH;
}

void Machine::loadMachineSpec() {

}
#ifdef MULTI_AXIS
int Machine::checkLimit(double x, double y, double z, double a, double b, double c) {
	int result = NO_LIMIT_ERROR;

	if (x > max_x_limit) result |= MAX_X_LIMIT;
	else if (x < min_x_limit) result |= MIN_X_LIMIT;
	if (y > max_y_limit) result |= MAX_Y_LIMIT;
	else if (y < min_y_limit) result |= MIN_Y_LIMIT;
	if (z > max_z_limit-z_limit_offset) result |= MAX_Z_LIMIT;
	else if (z < min_z_limit-z_limit_offset) result |= MIN_Z_LIMIT;
	if ((max_a_limit != NAN) && (a > max_a_limit)) result |= MAX_A_LIMIT;
	else if ((min_a_limit != NAN) && (a < min_a_limit)) result |= MIN_A_LIMIT;
	if ((max_b_limit != NAN) && (b > max_b_limit)) result |= MAX_B_LIMIT;
	else if ((min_b_limit != NAN) && (b < min_b_limit)) result |= MIN_B_LIMIT;
	if ((max_c_limit != NAN) && (c > max_c_limit)) result |= MAX_C_LIMIT;
	else if ((min_c_limit != NAN) && (c < min_c_limit)) result |= MIN_C_LIMIT;

	return result;
}
#else
int Machine::checkLimit(double x, double y, double z) {
	int result = NO_LIMIT_ERROR;

	if (x > max_x_limit) result |= MAX_X_LIMIT;
	else if (x < min_x_limit) result |= MIN_X_LIMIT;
	if (y > max_y_limit) result |= MAX_Y_LIMIT;
	else if (y < min_y_limit) result |= MIN_Y_LIMIT;
	if (z > max_z_limit-z_limit_offset) result |= MAX_Z_LIMIT;
	else if (z < min_z_limit-z_limit_offset) result |= MIN_Z_LIMIT;

	return result;
}
#endif

} // end namespace
// end of file machine.cpp
