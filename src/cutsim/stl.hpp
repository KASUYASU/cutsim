#ifndef STL_H
#define STL_H

#include <iostream>
#include <list>
#include <cassert>

#include "facet.hpp"

namespace cutsim {

class Stl {

  public:
    	Stl() {}
    	virtual ~Stl() { facets.resize(0); }
    	void addFacet(Facet *f)	{ facets.push_back(f); }

    	int readStlFile(QString file);

    	std::vector<Facet*> facets;

  private:
};

}

#endif	// STL_H
