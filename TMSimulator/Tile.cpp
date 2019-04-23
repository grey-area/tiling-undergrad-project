/*
 * Tile.cpp
 *
 *  Created on: Nov 29, 2009
 *      Author: awebb
 */

#include "Tile.h"

bool operator==(Tile firstTile, Tile secondTile)
{
	if(firstTile.topLabelVector==secondTile.topLabelVector &&
	   firstTile.leftLabelVector==secondTile.leftLabelVector &&
	   firstTile.rightLabelVector==secondTile.rightLabelVector &&
	   firstTile.bottomLabelVector==secondTile.bottomLabelVector)
		return 1;
	else
		return 0;
}

Tile::Tile()
{
	isInitConfigTile=0;
	initialized=0;
	finalDecision=0;
}

Tile::~Tile()
{
	// FREE MEMORY
}
