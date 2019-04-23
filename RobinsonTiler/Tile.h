/*
 * Tile.h
 *
 *  Created on: Nov 29, 2009
 *      Author: awebb
 */
#include <string>
#include <time.h>
using namespace std;
#ifndef TILE_H_
#define TILE_H_

class Tile
{
	public:
		// the four edges
		int top;
		int right;
		int bottom;
		int left;
		// wether or not the tile is doing anything interesting
		int initialized;
		// is the tile in error state (due to incorrect placement)
		int error;
		// the time at which the error started (so we can make it temporary)
		time_t errorStart;
		// was the tile placed by the program or the user? (1=program)
		int final;
		// 0=black, 1=different colours based on how the tile was placed
		int colour;
		// overloaded operators for equal and not equal tiles
		friend bool operator==(Tile firstTile, Tile secondTile);
		friend bool operator!=(Tile firstTile, Tile secondTile);
		Tile();
		virtual ~Tile();
};

#endif /* TILE_H_ */
