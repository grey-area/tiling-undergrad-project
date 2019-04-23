/*
 * Tile.cpp
 *
 *  Created on: Feb 3, 2010
 *      Author: awebb
 *
 *  Tile objects.. each one is a robinson tile
 */

#include "Tile.h"

// for two tiles to be not equal they must be !(equal)
bool operator!=(Tile firstTile, Tile secondTile)
{
	return(!(firstTile==secondTile));
}

// for two tiles to be equal they must match on all four edges
bool operator==(Tile firstTile, Tile secondTile)
{
	if(firstTile.top==secondTile.top &&
	   firstTile.right==secondTile.right &&
	   firstTile.bottom==secondTile.bottom &&
	   firstTile.left==secondTile.left)
		return 1;
	else
		return 0;
}

// constructor, initialises all sides as blank
// non-initialised tile
// not in error or final state
// no colour
Tile::Tile()
{
	top=0;
	right=0;
	bottom=0;
	left=0;
	initialized=0;
	error=0;
	final=0;
	colour=0;
}

Tile::~Tile()
{
}
