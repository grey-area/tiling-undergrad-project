/*
 * Tile.h
 *
 *  Created on: Mar 30, 2010
 *      Author: awebb
 */
#include <vector>
using namespace std;
#ifndef TILE_H_
#define TILE_H_

class Tile
{
	public:
		vector <int> topVector;
		vector <int> leftVector;
		vector <int> rightVector;
		vector <int> bottomVector;
		int initialized;
		Tile();
		virtual ~Tile();
};

#endif /* TILE_H_ */
