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
#include <cmath>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <cassert>

#include "canonLine.hpp"

#include "machineStatus.hpp"
#include "canonMotionless.hpp"
#include "linearMotion.hpp"
#include "helicalMotion.hpp"

namespace g2m {

/// note, the constructor is protected!
canonLine::canonLine(std::string canonL, machineStatus prevStatus): myLine(canonL), status(prevStatus) {                       
  tokenize(myLine,canonTokens);
}

///returns the number after N on the line, -1 if none
int canonLine::getN() {
  if ( (cantok(1).compare("N.....") == 0)) 
    return -1;
  else
    return tok2i(1,1);
}

/** converts canonTokens[n] to double
\param n this is the token to convert
\returns token n, converted to double
*/
double canonLine::tok2d(uint n) {
  if (canonTokens.size() < n+1 ) 
    return NAN;
  char* end;
  double d = strtod( canonTokens[n].c_str(), &end );
//  assert ( *end == 0 );
  return d;
}

/** converts canonTokens[n] to int
\param n this is the token to convert
\param offset skip this many chars at the beginning of the token
\returns token n, converted to integer
*/
inline int canonLine::tok2i(uint n,uint offset) {
  if (canonTokens.size() < n+1 ) 
    return INT_MIN;
  char * end;
  int i = strtol( &canonTokens[n].c_str()[offset], &end, 10 );
  //assert ( *end != 0 );
  return i;
}

/// return the n:th canon-token
std::string canonLine::cantok(unsigned int n) {
  if (n < canonTokens.size()) {
    return canonTokens[n]; 
  } else {
    std::cout << "malformed input line " << myLine << std::endl;
    std::string s = ""; 
    return s;
  }
}

///return true if the canonical command for this line matches 'm'
bool canonLine::cmdMatch(std::string m) {
    if (canonTokens.size() < 3)
        return false;
    return (m.compare(canonTokens[2]) == 0); //compare returns zero for a match
}

/// return canonTokens[2] 
const std::string canonLine::getCanonicalCommand() {
  if (canonTokens.size() < 3 ) 
    return "BAD_LINE_NO_CMD";
  return canonTokens[2];
}

///return a line identifier as string: getN() if !=-1, else getLineNum()
const std::string canonLine::getLnum() {
  return ((getN()==-1) ? (cantok(0)) : (cantok(1)));
}

//from http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
//0 is canon line
//1 is gcode Nnnnnn line
//2 is canonical command
/** Splits a string using delimiters.
\param str the string to be split
\param tokenV a vector of strings, each of which is a piece of str
\param delimiters defaults to: both parenthesis, comma, space
\sa tokenize()
*/
void canonLine::tokenize(   std::string str,
                            std::vector<std::string>& tokenV,
                            const std::string& delimiters       ) {
  // Skip delimiters at beginning.
  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (std::string::npos != pos || std::string::npos != lastPos)
  {
    // Found a token, add it to the vector.
    tokenV.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
}

/**Create objects that inherit from canonLine. It determines which type 
 * of object to create, and returns a pointer to that new object
*/
canonLine * canonLine::canonLineFactory (std::string l , machineStatus s) {
    //check if canonical command is motion or something else
    //motion commands: STRAIGHT_TRAVERSE STRAIGHT_FEED ARC_FEED
    size_t lin,af,cmnt,msg;
    cmnt  = l.find("COMMENT");
    msg   = l.find("MESSAGE");
    lin   = l.find("STRAIGHT_");
    af    = l.find("ARC_FEED");
    /*
    ** check for comments first because it is not impossible
    ** for one to contain the text "STRAIGHT" or "ARC_FEED"
    */
    std::cout << l; // print the line
    if ( (cmnt!=std::string::npos) || (msg!=std::string::npos) ) {
        return new canonMotionless(l,s); // comment or message, no motion
    } else if (lin!=std::string::npos) { 
        return new linearMotion(l,s);  // straight traverse or straight feed
    } else if (af!=std::string::npos) { 
        return new helicalMotion(l,s); // arc or helix
    } else { 
        return new canonMotionless(l,s); //canonical command is not a motion command
    }
}

} // end namespace
