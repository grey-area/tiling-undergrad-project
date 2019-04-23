/*
 * TileCompiler.h
 *
 *  Created on: Dec 3, 2009
 *      Author: awebb
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include "Tile.h"
using namespace std;

#ifndef TILECOMPILER_H_
#define TILECOMPILER_H_

class TileCompiler
{
	public:
		void printTileSet(vector <Tile>&,int);
		vector <Tile> mainFunction();
		Tile* ptr_leftFacingBlank;
		Tile* ptr_middleBlank;
		Tile* ptr_middleOne;
		Tile* ptr_middleZero;
		Tile* ptr_rightFacingBlank;
		Tile* ptr_rightFacingOne;
		Tile* ptr_rightFacingZero;

		vector <int> haltState;
		TileCompiler(char*);
		virtual ~TileCompiler();
};

#endif /* TILECOMPILER_H_ */
