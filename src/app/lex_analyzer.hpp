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
#ifndef LEX_ANALIZER_HH
#define LEX_ANALIZER_HH

#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <sstream>
#include <limits.h>
#include <cassert>

namespace lex_analyzer {
/**
\class LexAnalyzer
\brief A canonLine object represents one canonical command.
Each gcode line produces one or more canonical commands. It can either be a 
* motion command (one of LINEAR_TRAVERSE LINEAR_FEED ARC_FEED), or a motionless 
* command (anything else)
You cannot create objects of this class - instead, create an object of a class 
* that inherits from this class via canonLineFactory()
*/
class LexAnalyzer {

  public:
	LexAnalyzer(std::string line): myLine(line) {
	  tokenize(myLine, tokens);
	}
    /// return the line as a string
    const std::string getLine() { return myLine; };
    /// return the n:th token
    std::string getToken(unsigned int n);

    // keyword matching
    bool wordMatch(std::string word, unsigned int n = 0);
    // get double number
    double token2d(unsigned int n);
    // get integer number
    int token2i(unsigned int n, unsigned int offset = 0);

  protected:
    // tokenizer
    void tokenize(std::string str, std::vector<std::string>& tokenV, const std::string& delimiters = "(), \t");

// DATA
    /// the line as a string
    std::string myLine; 
    /// the tokens in this line, set after tokenizing myLine
    std::vector<std::string> tokens; 
};

} // end namespace

#endif //LEX_ANALIZER_HH
