/*  
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  
 *  This file is part of Cutsim / OpenCAMlib.
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

#include <iostream>
#include <cassert>
#include <set>
#include <vector>

#include <QtDebug>

#include "gldata.hpp"
#include "octnode.hpp"

namespace cutsim {

GLData::GLData() {
    // some reasonable defaults...
    renderIndex = 0;
    workIndex = 1;
    
    glp[workIndex].type = GL_TRIANGLES;
    glp[workIndex].polyVerts = 3;
    glp[workIndex].polygonMode_face = GL_FRONT_AND_BACK;
    glp[workIndex].polygonMode_mode = GL_LINE;
    
    swap(); // to intialize glp etc.. (?)
}

/// add a vertex with given position and color, return its index
unsigned int GLData::addVertex(float x, float y, float z, float r, float g, float b) {
    return addVertex( GLVertex(x,y,z,r,g,b), NULL );
}

/// add vertex, associate given Octnode with the vertex, and return index
unsigned int GLData::addVertex(GLVertex v, Octnode* n) {
    // add vertex with empty polygon-list.
    unsigned int idx = vertexArray[workIndex].size();
    vertexArray[workIndex].append(v);
    vertexDataArray.append( VertexData() );
    vertexDataArray[idx].node = n;
    assert( vertexArray[workIndex].size() == vertexDataArray.size() );
    return idx; // return index of newly appended vertex
}

/// add vertex at given position
unsigned int GLData::addVertex(float x, float y, float z, float r, float g, float b, Octnode* n) {
    unsigned int id = addVertex(x,y,z,r,g,b);
    vertexDataArray[id].node = n;
    return id;
}

/// set vertex normal
void GLData::setNormal(unsigned int vertexIdx, float nx, float ny, float nz) {
    vertexArray[workIndex][vertexIdx].setNormal(nx,ny,nz);
}

/// modify given vertex
void GLData::modifyVertex( unsigned int id, float x, float y, float z, float r, float g, float b, float nx, float ny, float nz) {
    GLVertex p = GLVertex(x,y,z,r,g,b,nx,ny,nz);
    vertexArray[workIndex][id] = p;
}

/// remove vertex with given index
void GLData::removeVertex( unsigned int vertexIdx ) {
    // i) for each polygon of this vertex, call remove_polygon:
    typedef std::set< unsigned int, std::greater<unsigned int> > PolygonSet;
    PolygonSet pset = vertexDataArray[vertexIdx].polygons;
    BOOST_FOREACH( unsigned int polygonIdx, pset ) {
        removePolygon( polygonIdx );
//std::cout << "Delete Polygon " << polygonIdx << "\n";
    }
    // ii) overwrite with last vertex:
    unsigned int lastIdx = vertexArray[workIndex].size()-1;
    if (vertexIdx != lastIdx) {
        vertexArray[workIndex][vertexIdx] = vertexArray[workIndex][lastIdx];
        vertexDataArray[vertexIdx] = vertexDataArray[lastIdx];
        // notify octree-node with new index here!
        // vertex that was at lastIdx is now at vertexIdx
        vertexDataArray[vertexIdx].node->swapIndex( lastIdx, vertexIdx );

        // request each polygon to re-number this vertex.
        BOOST_FOREACH( unsigned int polygonIdx, vertexDataArray[vertexIdx].polygons ) {
            unsigned int idx = polygonIdx*polygonVertices();
            for (int m=0;m<polygonVertices();++m) {
                if ( indexArray[workIndex][ idx+m ] == lastIdx )
                    indexArray[workIndex][ idx+m ] = vertexIdx;
            }
        }
    }
    // shorten array
    vertexArray[workIndex].resize( vertexArray[workIndex].size()-1 );
    vertexDataArray.resize( vertexDataArray.size()-1 );
    assert( vertexArray[workIndex].size() == vertexDataArray.size() );
    //std::cout << " removeVertex done.\n";
}

/// add a polygon, return its index
int GLData::addPolygon( std::vector<GLuint>& verts) {
    // append to indexArray, then request each vertex to update
    unsigned int polygonIdx = indexArray[workIndex].size()/polygonVertices();
    BOOST_FOREACH( GLuint vertex, verts ) {
        indexArray[workIndex].append(vertex);
        vertexDataArray[vertex].addPolygon(polygonIdx); // add index to vertex i1
    }
    return polygonIdx;
}

/// remove polygon at given index
void GLData::removePolygon( unsigned int polygonIdx) {
    unsigned int idx = polygonVertices()*polygonIdx; // start-index for polygon
    // i) request remove for each vertex in polygon:
    for (int m=0; m<polygonVertices() ; ++m) // this polygon has the following 3/4 vertices. we call removePolygon on them all
        vertexDataArray[ indexArray[workIndex][idx+m]   ].removePolygon(polygonIdx);
    
    unsigned int last_index = (indexArray[workIndex].size()-polygonVertices());
    // if deleted polygon is last on the list, do nothing??
    if (idx!=last_index) {
        // ii) remove from polygon-list by overwriting with last element
        for (int m=0; m<polygonVertices(); ++m)
            indexArray[workIndex][idx+m  ] = indexArray[workIndex][ last_index+m   ];
        // iii) for the moved polygon, request that each vertex update the polygon number
        for (int m=0; m<polygonVertices() ; ++m) {
            vertexDataArray[ indexArray[workIndex][idx+m   ] ].addPolygon( idx/polygonVertices() ); // this is the new polygon index
            vertexDataArray[ indexArray[workIndex][idx+m   ] ].removePolygon( last_index/polygonVertices() ); // this polygon is no longer there!
        }
    }
    indexArray[workIndex].resize( indexArray[workIndex].size()-polygonVertices() ); // shorten array
} 

/// string output
void GLData::print() {
//    std::cout << "GLData vertices: \n";
    //int n = 0;
//    for( int n = 0; n < vertexArray[workIndex].size(); ++n ) {
//        std::cout << n << " : ";
//        vertexArray[workIndex][n].str();
//        std::cout << " polys: ";
        //vertexDataArray[n].str();
//        std::cout << "\n";
 //   }
//    std::cout << "GLData polygons: \n";
//    int polygonIndex = 0;
//    for( int n=0; n< indexArray[workIndex].size(); n=n+polygonVertices() ) {
//        std::cout << polygonIndex << " : ";
//        for (int m=0;m<polygonVertices();++m)
//            std::cout << indexArray[workIndex][n+m] << " ";
//        std::cout << "\n";
//        ++polygonIndex;
 //   }
	std::cout << "GLData vertexArray(w) size: " << vertexArray[workIndex].size() << " (" << vertexArray[workIndex].size() * sizeof(GLVertex) << " bytes)\n";
	std::cout << "GLData indexArray(w) size: " << indexArray[workIndex].size() << " (" << indexArray[workIndex].size() * sizeof(GLuint) << " bytes)\n";
	std::cout << "GLData vertexArray(r) size: " << vertexArray[renderIndex].size() << " (" << vertexArray[renderIndex].size() * sizeof(GLVertex) << " bytes)\n";
	std::cout << "GLData indexArray(r) size: " << indexArray[renderIndex].size() << " (" << indexArray[renderIndex].size() * sizeof(GLuint) << " bytes)\n";
	std::cout << "GLData vertexDataArray() size: " << vertexDataArray.size() << " (" << vertexDataArray.size() * sizeof(VertexData) << " bytes)\n";
}

} // end cutsim namespace

