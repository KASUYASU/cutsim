#ifndef FACET_H
#define FACET_H

#include <cassert>

#include "glvertex.hpp"

namespace cutsim {

class Facet {

   public:
	   Facet(GLVertex n, GLVertex p1, GLVertex p2, GLVertex p3) { normal = n; v1 = p1; v2 = p2; v3 = p3; }
	   GLVertex	normal;
	   GLVertex	v1, v2, v3;
};

}

#endif // FACET_H

