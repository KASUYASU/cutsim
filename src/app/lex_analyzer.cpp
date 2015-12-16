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

#include <cmath>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <cassert>

#include "lex_analyzer.hpp"

namespace lex_analyzer {

/** converts tokens[n] to double
\param n this is the token to convert
\returns token n, converted to double
*/
double LexAnalyzer::token2d(uint n) {
  if (tokens.size() < n+1 )
    return NAN;
  char* end;
  double d = strtod( tokens[n].c_str(), &end );
//  assert ( *end == 0 );
  return d;
}

/** converts tokens[n] to int
\param n this is the token to convert
\param offset skip this many chars at the beginning of the token
\returns token n, converted to integer
*/
int LexAnalyzer::token2i(unsigned int n, unsigned int offset) {
  if (tokens.size() < n+1 )
    return INT_MIN;
  char * end;
  int i = strtol( &tokens[n].c_str()[offset], &end, 10 );
  //assert ( *end != 0 );
  return i;
}

/// return the n:th token
std::string LexAnalyzer::getToken(unsigned int n) {
  if (n < tokens.size()) {
    return tokens[n];
  } else {
    std::cout << "malformed input line " << myLine << std::endl;
    std::string s = "";
    return s;
  }
}

///return true if the n:th token matches 'm'
bool LexAnalyzer::wordMatch(std::string m, unsigned int n) {
    if (tokens.size() < n+1)
        return false;
    return (m.compare(tokens[n]) == 0); //compare returns zero for a match
}

//from http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
/** Splits a string using delimiters.
\param str the string to be split
\param tokenV a vector of strings, each of which is a piece of str
\param delimiters defaults to: both parenthesis, comma, space
\sa tokenize()
*/
void LexAnalyzer::tokenize(   std::string str,
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

} // end namespace

