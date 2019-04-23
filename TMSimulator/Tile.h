/*
 * Tile.h
 *
 *  Created on: Nov 29, 2009
 *      Author: awebb
 */
#include <vector>
#include <string>
using namespace std;
#ifndef TILE_H_
#define TILE_H_

class Tile
{
	public:
		vector <int> topLabelVector;
		vector <int> leftLabelVector;
		vector <int> rightLabelVector;
		vector <int> bottomLabelVector;
		int isInitConfigTile;
		int initialized;
		int finalDecision;
		friend bool operator==(Tile firstTile, Tile secondTile);
		Tile();
		virtual ~Tile();
};

#endif /* TILE_H_ */
