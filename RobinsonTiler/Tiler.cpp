/*
 * Tiler.cpp
 *
 *  Created on: Feb 3, 2010
 *      Author: awebb
 *
 *  Robinson Tiler program:
 *  This program automatically generates robinson tilings
 *  And also allows the user to attempt to place tiles
 */

#include <GL/glut.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "Tile.h"
#include <time.h>
#include <deque>
#include <stack>
#include <cmath>
#include <unistd.h>
#include <cstdlib>

int wHeight = 750; // dimensions of the display window
int wWidth = 750;
// used to adjust the height due to the selection bar at the top of the window
float heightAdjust=0.9;

int waiting=0;

// to decide on the size of tiles on screen
const int initialSize=25;
int currentSize=initialSize;
// the size of tiles as they are displayed - this will change due to zoom
float tileSize = wWidth/(float)currentSize;
// the size of the selection tiles at the top of the screen
float choiceTileSize = wWidth/(float)initialSize;

int hPanning=0; // are we panning horizontally? -1, 0, 1
int vPanning=0; // are we panning vertically? -1, 0, 1
float totalHPan=0; // how much have we panned by?
float totalVPan=0; // ... including panning to counteract zoom drift
float moveAmount=0.001; // how fast to pan

float hPan=0; // how much have we panned by?
float vPan=0; // ... not including panning to counteract zoom drift

// where the centre of the current supercell is
int centreX=currentSize/2;
int centreY=currentSize/2;

int zooming=0; // are we zooming? -1, 0, 1
float zoom=0; // how far have we zoomed?
float zoomAmount=0.003; // how fast to zoom

int baseColours=1; // the colour scheme used when turned on
int showColours=0; // will a colour scheme be used?
int alternateColours=3; // for use in the alternating colour display
int tileCount=0; // number of tiles currently placed
// flag so that the program only does one iteration of inflation
// for every time the user presses the space bar
int halt=0;

// a tile the user can place
Tile mouseTile;
int mouseTileType=0;
int mouseTileRotation=0;
int mouseX=0;
int mouseY=0;

// used to keep the current time
// (timeout for error tile flashing red)
time_t currentTime;

// the cross tile, one element for each rotation
std::vector<Tile> crossTiles(4);
// the other 6 robinson tiles, with 4 elements, one for each rotation
std::vector <std::vector<Tile> > rTiles(7, vector<Tile> (4));

// the area to be tiled (all uninitialized blank tiles)
Tile **tiling;
Tile blankTile;
// to determine size of the (2^squarePower)-1 square
int squarePower=-1;

stack <int> decisionStack; // stack of rotations chosen in inflation

// initialize a tile array
void initCrossTiles()
{

	// file reader (opens cross tile description file)
	ifstream reader("./tiles1");
	if(!reader)
		cout << "Error opening input file" << endl;

	int rotation=0; // the rotation of the current tile
	int side=0; // top,right,bottom,left?
	int changeRotation=0;
	// read the file
	while ( !reader.eof() )
	{
		string temp;
		stringstream tempstream;
		int tempint;

		if((side%4)<3) // for the first 3 terms
			getline(reader,temp,'\t'); // read up to the tab
		else
		{
			getline(reader,temp,'\n'); // read up to the end of the line
			changeRotation=1;
		}

		tempstream << temp;
		tempstream >> tempint;

		// which side are we looking at?
		switch(side%4)
		{
		case 0:
			crossTiles[rotation].top=tempint;
			break;
		case 1:
			crossTiles[rotation].right=tempint;
			break;
		case 2:
			crossTiles[rotation].bottom=tempint;
			break;
		case 3:
			crossTiles[rotation].left=tempint;
			break;
		} // end switch

		// the tile has been initialised
		crossTiles[rotation].initialized=1;

		if(changeRotation)
		{
			changeRotation=0;
			rotation++;
		}
		side++;
	} // while (for each element)
}


// initialize another tile array
void initOtherTiles()
{

	// file reader (opens other tile description file)
	ifstream reader("./tiles2");
	if(!reader)
		cout << "Error opening input file" << endl;

	int tileType=0;
	int rotation=0; // the rotation of the current tile
	int side=0; // top,right,bottom,left?
	int changeRotation=0;
	// read the file
	while ( !reader.eof() )
	{
		if(rotation==4)
		{
			rotation=0;
			tileType++;
		}
		//sleep(1);
		//printf("Rotation=%d\n",rotation);
		string temp;
		stringstream tempstream;
		int tempint;

		if((side%4)<3) // for the first 3 terms
			getline(reader,temp,'\t'); // read up to the tab
		else
		{
			getline(reader,temp,'\n'); // read up to the end of the line
			changeRotation=1;
		}

		tempstream << temp;
		tempstream >> tempint;

		switch(side%4)
		{
		case 0:
			rTiles[tileType][rotation].top=tempint;
			break;
		case 1:
			rTiles[tileType][rotation].right=tempint;
			break;
		case 2:
			rTiles[tileType][rotation].bottom=tempint;
			break;
		case 3:
			rTiles[tileType][rotation].left=tempint;
			break;
		} // end switch

		rTiles[tileType][rotation].initialized=1;

		if(changeRotation)
		{
			changeRotation=0;
			rotation++;
		}
		side++;
	} // while (for each element)
}


// prints out initialized tiles
void printAvailableTiles()
{
	cout << "Cross Tiles:" << endl;
	vector<Tile>::iterator i;
	for ( i=crossTiles.begin() ; i < crossTiles.end(); i++ )
	{
		cout << (*i).top << " " << (*i).right << " " << (*i).bottom << " " << (*i).left << endl;
	}

	cout << endl << "Robinson Tiles Tiles:" << endl;
	vector<vector<Tile> >::iterator j;
	for ( j=rTiles.begin() ; j < rTiles.end() ; j++ )
	{
		for( i=(*j).begin() ; i < (*j).end() ; i++ )
		{
			cout << (*i).top << " " << (*i).right << " " << (*i).bottom << " " << (*i).left << endl;
		} // end inner for loop
	} // end outer loop
} // end print function


// inflates to next sized supercell
void inflate()
{
	// for everything within the current supercell
	// create a copy in a direction determined by the current rotation
	// with the relevent rotation
	squarePower++;
	if(squarePower!=0)
	{
		// length of the next square
		int size = pow(2.0,squarePower)-1;

		// whay area around the centre are we interested in copying?
		int xMin=centreX-(size-1)/2;
		int xMax=centreX+(size-1)/2;
		int yMin=centreY-(size-1)/2;
		int yMax=centreY+(size-1)/2;

		int oldCurrentRotation=decisionStack.top();

		// x and y modifiers (because of rotation)
		int yMod=1;
		int xMod=1;
		if(oldCurrentRotation==1)
			xMod=-1;
		else if(oldCurrentRotation==3)
			yMod=-1;
		else if(oldCurrentRotation==2)
		{
			xMod=-1;
			yMod=-1;
		}

		// the centre is now the centre of the new supercell
		centreX += (((size-1)/2)+1) * xMod;
		centreY += (((size-1)/2)+1) * yMod;

		// loop through 'interesting area' (2D loop)
		int xIndex,yIndex;
		for(xIndex=xMin ; xIndex<=xMax ; xIndex++)
		{
			for(yIndex=yMin ; yIndex<=yMax ; yIndex++)
			{
				int rX, rY;
				int rXFinal=-1, rYFinal=-1;
				int newColour=-1;
				for(rX=0; rX < 7; rX++)
					for(rY=0;rY < 4; rY++)
						if(rTiles[rX][rY]==tiling[xIndex][yIndex])
						{
							rXFinal=rX;
							rYFinal=rY;
							newColour=tiling[xIndex][yIndex].colour;
						}

				if(rXFinal!=-1&&rYFinal!=-1)
				{
					// do we duplicate colours or not?
					if(baseColours==1)
						newColour=1;
					// temp variables to avoid recalculation
					int pos1,pos2;

					// vertical duplicate
					pos1=centreX-(yIndex-centreY);
					pos2=centreY+(xIndex-centreX);
					tiling[pos1][pos2]=rTiles[rXFinal][(rYFinal+1)%4];
					tiling[pos1][pos2].final=1;
					tiling[pos1][pos2].colour=newColour;

					// horizontal duplicate
					pos1=centreX+(yIndex-centreY);
					pos2=centreY-(xIndex-centreX);
					tiling[pos1][pos2]=rTiles[rXFinal][(rYFinal+3)%4];
					tiling[pos1][pos2].final=1;
					tiling[pos1][pos2].colour=newColour;

					// diagonal duplicate
					pos1=2*centreX-xIndex;
					pos2=2*centreY-yIndex;
					tiling[pos1][pos2]=rTiles[rXFinal][(rYFinal+2)%4];
					tiling[pos1][pos2].final=1;
					tiling[pos1][pos2].colour=newColour;
				} // end if
			} // end inner loop
		} // end outer loop
		tileCount*=4; // made 3 copies, increase tileCount accordingly
	} // if not first time round
}


// decides on the orientation of the centre cross tile
// and records the decision on the decision stack (for backtracking)
void decideOrientation()
{
	tiling[centreX][centreY]=crossTiles[mouseTileRotation];
	tiling[centreX][centreY].final=1;
	if(baseColours==1)
		tiling[centreX][centreY].colour=2;
	else
		tiling[centreX][centreY].colour=alternateColours;
	decisionStack.push(mouseTileRotation);
	tileCount++;
}


// IMPORTANT:  loop boundaries are inclusive
void patternMatch(int xMin, int xMax, int yMin, int yMax)
{
	int xDiff,yDiff,xCount,yCount,xCountMax,yCountMax;
	xCount=0;
	yCount=0;
	xCountMax = abs(xMax-xMin);
	yCountMax = abs(yMax-yMin);
	if(xMax>=xMin)
		xDiff=1;
	else
		xDiff=-1;
	if(yMax>=yMin)
		yDiff=1;
	else
		yDiff=-1;
	int x,y; // loop through interesting area (should actually be a line)
	for(x=xMin ; xCount<=xCountMax ; x+=xDiff)
	{
		for(y=yMin ; yCount<=yCountMax ; y+=yDiff)
		{
		// Inside here must loop through available tiles and pick one
		// that matches on all four sides
		// (also allowing when adjecent tiles are blank)
			int type,rotation;
			// loop through available tiles and try to pattern patch
			for(type=0 ; type < 6 ; type++)
				for(rotation=0; rotation < 4 ; rotation++)
				{
					bool eastCheck=(!tiling[x][y+1].final||!tiling[x][y+1].initialized||tiling[x][y+1].left+rTiles[type][rotation].right==0);
					bool westCheck=(!tiling[x][y-1].final||!tiling[x][y-1].initialized||tiling[x][y-1].right+rTiles[type][rotation].left==0);
					bool northCheck=(!tiling[x+1][y].final||!tiling[x+1][y].initialized||tiling[x+1][y].bottom+rTiles[type][rotation].top==0);
					bool southCheck=(!tiling[x-1][y].final||!tiling[x-1][y].initialized||tiling[x-1][y].top+rTiles[type][rotation].bottom==0);
					if(northCheck && southCheck && eastCheck && westCheck)
					{
						tiling[x][y]=rTiles[type][rotation];
						tiling[x][y].final=1;
						tiling[x][y].colour=alternateColours;
						tileCount++;
					}
				} // end of loop through available tiles
			yCount++;
		} // end of loop through tiling
		xCount++;
		yCount=0;
	} // end of outer loop through tiling
}


// fills in any blanks if necessary
// filling in blanks here is deterministic
// so no need for backtracking on decisions made here
void fillInBlanks()
{
	int size = pow(2.0,squarePower+1)-1;

	// fills in all the blanks by matching edges
	patternMatch(centreX,centreX,centreY+1,centreY+(size-1)/2);
	patternMatch(centreX,centreX,centreY-1,centreY-(size-1)/2);
	patternMatch(centreX+1,centreX+(size-1)/2,centreY,centreY);
	patternMatch(centreX-1,centreX-(size-1)/2,centreY,centreY);
}


// draw the top arrows of a tile
void drawTop(Tile currentTile)
{
	if(currentTile.top>0)
	{
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/2.0,tileSize/3.0,0.0);
			glVertex3f(tileSize/2.0,tileSize,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize/2.0-tileSize/12.0,tileSize-tileSize/8.0,0.0);
			glVertex3f(tileSize/2.0,tileSize,0.0);
			glVertex3f(tileSize/2.0+tileSize/12.0,tileSize-tileSize/8.0,0.0);
		glEnd();
	}
	else if(currentTile.top<0)
	{
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/2.0,tileSize,0.0);
			glVertex3f(tileSize/2.0,tileSize/1.14,0.0);
			glVertex3f(tileSize/2.0+tileSize/12.0,tileSize/1.14,0.0);
			glVertex3f(tileSize/2.0-tileSize/12.0,tileSize/1.14,0.0);
		glEnd();
	}

	switch(currentTile.top)
	{
	case -2:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/3.0,tileSize,0.0);
			glVertex3f(tileSize/3.0,tileSize/1.14,0.0);
			glVertex3f(tileSize/3.0+tileSize/12.0,tileSize/1.14,0.0);
			glVertex3f(tileSize/3.0-tileSize/12.0,tileSize/1.14,0.0);
		glEnd();
		break;
	case -3:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/1.5,tileSize,0.0);
			glVertex3f(tileSize/1.5,tileSize/1.14,0.0);
			glVertex3f(tileSize/1.5+tileSize/12.0,tileSize/1.14,0.0);
			glVertex3f(tileSize/1.5-tileSize/12.0,tileSize/1.14,0.0);
		glEnd();
		break;
	case 2:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/3.0,tileSize/3.0,0.0);
			glVertex3f(tileSize/3.0,tileSize,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize/3.0-tileSize/12.0,tileSize-tileSize/8.0,0.0);
			glVertex3f(tileSize/3.0,tileSize,0.0);
			glVertex3f(tileSize/3.0+tileSize/12.0,tileSize-tileSize/8.0,0.0);
		glEnd();
		break;
	case 3:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/1.5,tileSize/3.0,0.0);
			glVertex3f(tileSize/1.5,tileSize,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize/1.5-tileSize/12.0,tileSize-tileSize/8.0,0.0);
			glVertex3f(tileSize/1.5,tileSize,0.0);
			glVertex3f(tileSize/1.5+tileSize/12.0,tileSize-tileSize/8.0,0.0);
		glEnd();
		break;
	} // end switch
} // END DRAW TOP

// draw the right arrows of a tile
void drawRight(Tile currentTile)
{
	if(currentTile.right>0)
	{
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/3.0,tileSize/2.0,0.0);
			glVertex3f(tileSize,tileSize/2.0,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize-tileSize/8.0,tileSize/2.0-tileSize/12.0,0.0);
			glVertex3f(tileSize,tileSize/2.0,0.0);
			glVertex3f(tileSize-tileSize/8.0,tileSize/2.0+tileSize/12.0,0.0);
		glEnd();
	}
	else if(currentTile.right<0)
	{
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize,tileSize/2.0,0.0);
			glVertex3f(tileSize/1.14,tileSize/2.0,0.0);
			glVertex3f(tileSize/1.14,tileSize/2.0+tileSize/12.0,0.0);
			glVertex3f(tileSize/1.14,tileSize/2.0-tileSize/12.0,0.0);
		glEnd();
	}

	switch(currentTile.right)
	{
	case -2:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize,tileSize/1.5,0.0);
			glVertex3f(tileSize/1.14,tileSize/1.5,0.0);
			glVertex3f(tileSize/1.14,tileSize/1.5+tileSize/12.0,0.0);
			glVertex3f(tileSize/1.14,tileSize/1.5-tileSize/12.0,0.0);
			glEnd();
		break;
	case -3:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize,tileSize/3.0,0.0);
			glVertex3f(tileSize/1.14,tileSize/3.0,0.0);
			glVertex3f(tileSize/1.14,tileSize/3.0+tileSize/12.0,0.0);
			glVertex3f(tileSize/1.14,tileSize/3.0-tileSize/12.0,0.0);
			glEnd();
		break;
	case 2:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/3.0,tileSize/1.5,0.0);
			glVertex3f(tileSize,tileSize/1.5,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize-tileSize/8.0,tileSize/1.5-tileSize/12.0,0.0);
			glVertex3f(tileSize,tileSize/1.5,0.0);
			glVertex3f(tileSize-tileSize/8.0,tileSize/1.5+tileSize/12.0,0.0);
		glEnd();
		break;
	case 3:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/3.0,tileSize/3.0,0.0);
			glVertex3f(tileSize,tileSize/3.0,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize-tileSize/8.0,tileSize/3.0-tileSize/12.0,0.0);
			glVertex3f(tileSize,tileSize/3.0,0.0);
			glVertex3f(tileSize-tileSize/8.0,tileSize/3.0+tileSize/12.0,0.0);
		glEnd();
		break;
	} // end switch
} // END DRAW RIGHT

// draw the bottom arrows of a tile
void drawBottom(Tile currentTile)
{
	if(currentTile.bottom>0)
	{
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/2.0,tileSize/1.5,0.0);
			glVertex3f(tileSize/2.0,0.0,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize/2.0-tileSize/12.0,tileSize/8.0,0.0);
			glVertex3f(tileSize/2.0,0.0,0.0);
			glVertex3f(tileSize/2.0+tileSize/12.0,tileSize/8.0,0.0);
		glEnd();
	}
	else if(currentTile.bottom<0)
	{
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/2.0,0.0,0.0);
			glVertex3f(tileSize/2.0,tileSize/8.0,0.0);
			glVertex3f(tileSize/2.0+tileSize/12.0,tileSize/8.0,0.0);
			glVertex3f(tileSize/2.0-tileSize/12.0,tileSize/8.0,0.0);
		glEnd();
	}

	switch(currentTile.bottom)
	{
	case -2:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/3.0,0.0,0.0);
			glVertex3f(tileSize/3.0,tileSize/8.0,0.0);
			glVertex3f(tileSize/3.0+tileSize/12.0,tileSize/8.0,0.0);
			glVertex3f(tileSize/3.0-tileSize/12.0,tileSize/8.0,0.0);
		glEnd();
		break;
	case -3:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/1.5,0.0,0.0);
			glVertex3f(tileSize/1.5,tileSize/8.0,0.0);
			glVertex3f(tileSize/1.5+tileSize/12.0,tileSize/8.0,0.0);
			glVertex3f(tileSize/1.5-tileSize/12.0,tileSize/8.0,0.0);
		glEnd();
		break;
	case 2:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/3.0,tileSize/1.5,0.0);
			glVertex3f(tileSize/3.0,0.0,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize/3.0-tileSize/12.0,tileSize/8.0,0.0);
			glVertex3f(tileSize/3.0,0.0,0.0);
			glVertex3f(tileSize/3.0+tileSize/12.0,tileSize/8.0,0.0);
		glEnd();
		break;
	case 3:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/1.5,tileSize/1.5,0.0);
			glVertex3f(tileSize/1.5,0.0,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize/1.5-tileSize/12.0,tileSize/8.0,0.0);
			glVertex3f(tileSize/1.5,0.0,0.0);
			glVertex3f(tileSize/1.5+tileSize/12.0,tileSize/8.0,0.0);
		glEnd();
		break;
	} // end switch
} // END DRAW BOTTOM

// draw the left arrows of a tile
void drawLeft(Tile currentTile)
{
	if(currentTile.left>0)
	{
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/1.5,tileSize/2.0,0.0);
			glVertex3f(0.0,tileSize/2.0,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize/8.0,tileSize/2.0-tileSize/12.0,0.0);
			glVertex3f(0.0,tileSize/2.0,0.0);
			glVertex3f(tileSize/8.0,tileSize/2.0+tileSize/12.0,0.0);
		glEnd();
	}
	else if(currentTile.left<0)
	{
		glBegin(GL_LINE_STRIP);
			glVertex3f(0.0,tileSize/2.0,0.0);
			glVertex3f(tileSize/8.0,tileSize/2.0,0.0);
			glVertex3f(tileSize/8.0,tileSize/2.0+tileSize/12.0,0.0);
			glVertex3f(tileSize/8.0,tileSize/2.0-tileSize/12.0,0.0);
		glEnd();
	}

	switch(currentTile.left)
	{
	case -2:
		glBegin(GL_LINE_STRIP);
			glVertex3f(0.0,tileSize/1.5,0.0);
			glVertex3f(tileSize/8.0,tileSize/1.5,0.0);
			glVertex3f(tileSize/8.0,tileSize/1.5+tileSize/12.0,0.0);
			glVertex3f(tileSize/8.0,tileSize/1.5-tileSize/12.0,0.0);
		glEnd();
		break;
	case -3:
		glBegin(GL_LINE_STRIP);
			glVertex3f(0.0,tileSize/3.0,0.0);
			glVertex3f(tileSize/8.0,tileSize/3.0,0.0);
			glVertex3f(tileSize/8.0,tileSize/3.0+tileSize/12.0,0.0);
			glVertex3f(tileSize/8.0,tileSize/3.0-tileSize/12.0,0.0);
		glEnd();
		break;
	case 2:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/1.5,tileSize/1.5,0.0);
			glVertex3f(0.0,tileSize/1.5,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize/8.0,tileSize/1.5-tileSize/12.0,0.0);
			glVertex3f(0.0,tileSize/1.5,0.0);
			glVertex3f(tileSize/8.0,tileSize/1.5+tileSize/12.0,0.0);
		glEnd();
		break;
	case 3:
		glBegin(GL_LINE_STRIP);
			glVertex3f(tileSize/1.5,tileSize/3.0,0.0);
			glVertex3f(0.0,tileSize/3.0,0.0);
		glEnd();
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize/8.0,tileSize/3.0-tileSize/12.0,0.0);
			glVertex3f(0.0,tileSize/3.0,0.0);
			glVertex3f(tileSize/8.0,tileSize/3.0+tileSize/12.0,0.0);
		glEnd();
		break;
	} // end switch
} // END DRAW LEFT


// given a tile object, this will draw it
void drawTile(Tile currentTile)
{
	// if there is an error, display the tile as red
	if(currentTile.error)
	{
		glLineWidth(3.0);
		glColor4f(1.0,0.0,0.0,0.0);
	}
	// if it's a user placed tile, make it blue
	else if(!(currentTile.final))
		glColor4f(0.0,0.0,1.0,0.0);
	else
	{
		// else make it black...
		glColor4f(0.0,0.0,0.0,0.0);
		// unless we have the colour scheme on, in which case...
		if(showColours==1)
		{
			switch(currentTile.colour)
			{
			case 1:
				// 'duplicated' tiles should be orange
				glColor4f(1.0,0.6,0.6,0.0);
				break;
			case 2:
				// 'decision' tiles should be purple
				glColor4f(0.7,0.0,1.0,0.0);
				break;
			case 4:
			case 3:
				// 'filled in' tiles should be green
				glColor4f(0.0,1.0,0.0,0.0);
				break;
			}
		} // if showColours
		else if(showColours==2)
		{
			switch(currentTile.colour)
			{
			case 3:
				glColor4f(0.0,1.0,0.0,0.0);
				break;
			case 4:
				glColor4f(1.0,0.0,0.0,0.0);
				break;
			}
		} // if showColours==2
	}

	// draws the outer box
	glBegin(GL_LINE_LOOP);
		glVertex3f(0.0,0.0,0.0);
		glVertex3f(0.0,tileSize,0.0);
		glVertex3f(tileSize,tileSize,0.0);
		glVertex3f(tileSize,0.0,0.0);
	glEnd();
	glLineWidth(1.0);

	// draw all of the arrows
	drawTop(currentTile);
	drawBottom(currentTile);
	drawRight(currentTile);
	drawLeft(currentTile);
}


// draws the tiles in the selection box at the top
void drawChoiceTiles()
{
	glPushMatrix();
	// translation to put the tiles in place
	glTranslatef(wWidth/20.0,wHeight*(1+heightAdjust)/2,0.0);
	int i;
	float tempSize=tileSize;
	tileSize=choiceTileSize;
	// for all kinds of tile
	for(i=0;i<7;i++)
	{
		glPushMatrix();
		// another translation, puts each tile in position
		glTranslatef(tileSize*1.5*i,0.0,0.0);
		Tile tempTile=rTiles[i][mouseTileRotation];
		// they should be displayed black, so make them 'final'
		tempTile.final=1;
		drawTile(tempTile); // draw it
		glPopMatrix();
	}
	glPopMatrix();
	tileSize=tempSize;
}


// display everything
void displayTiling()
{
	int i,j;
	// loop through all tiles
	glPushMatrix();
	// takes panning into account
	glTranslatef(totalHPan,totalVPan,0.0);
	for(i=0;i<currentSize;i++)
	{
		for(j=0;j<currentSize;j++)
		{
			// if it's worth drawing something...
			// (there is a tile there or a mistake)
			if(tiling[i][j].initialized||tiling[i][j].error)
			{
				// if there's an error check if it's TTL has expired
				if(tiling[i][j].error)
				{
					time(&currentTime);
					// .. in which case the error should be removed
					if(difftime(currentTime,tiling[i][j].errorStart)>1)
						tiling[i][j].error=0;
				}
				glPushMatrix();
				// adjust because of the selection box at the top
				glTranslatef(0.0,-wHeight*(1-heightAdjust),0.0);
				// the position of each tile
				glTranslatef(tileSize*j,tileSize*i,0.0);
				drawTile(tiling[i][j]);
				glPopMatrix();
			} // end if
		} // end inner loop
	} // end outer loop
	glPopMatrix();


	// draws the selection box (white)
    glColor3f(1.0,1.0,1.0);
    glBegin(GL_QUADS);
		glVertex3f(0.0,wHeight,0.0);
		glVertex3f(wWidth,wHeight,0.0);
		glVertex3f(wWidth,wHeight*heightAdjust,0.0);
		glVertex3f(0.0,wHeight*heightAdjust,0.0);
    glEnd();
	glLineWidth(2.0);
	glColor3f(0.0,0.0,0.0);
	// .. and the black separation line
	glBegin(GL_LINE_STRIP);
		glVertex3f(0.0,wHeight*heightAdjust,0.0);
		glVertex3f(wWidth,wHeight*heightAdjust,0.0);
	glEnd();
	glLineWidth(1.0);

	drawChoiceTiles(); // draws the selection tiles in the selection box
} // end display tiling function


// when a mouse button is clicked
void mouse(int button, int state, int x, int y)
{
	if(state==GLUT_DOWN) // only when clicked, not released
	{
		// the tile position we're hovering over
		int tileY = (int)(((wHeight*(2-heightAdjust)-y)-totalVPan)/tileSize);
		int tileX = (int)((x-totalHPan)/tileSize);
		// if we're looking at the right bit (outside of the selection box)
		if (y>wHeight*(1-heightAdjust)&&tileY<currentSize&&tileX<currentSize && mouseTile!=blankTile)
		{
			if(button==GLUT_LEFT_BUTTON) // left mouse button
			{
				// must have at least one neighbour
				bool notFloating = (tiling[tileY+1][tileX].initialized||tiling[tileY-1][tileX].initialized||tiling[tileY][tileX+1].initialized||tiling[tileY][tileX-1].initialized);
				// matching rules for east/west/north/south
				bool eastMatch = (tiling[tileY+1][tileX].initialized==0||tiling[tileY+1][tileX].bottom+mouseTile.top==0);
				bool westMatch = (tiling[tileY-1][tileX].initialized==0||tiling[tileY-1][tileX].top+mouseTile.bottom==0);
				bool northMatch = (tiling[tileY][tileX+1].initialized==0||tiling[tileY][tileX+1].left+mouseTile.right==0);
				bool southMatch = (tiling[tileY][tileX-1].initialized==0||tiling[tileY][tileX-1].right+mouseTile.left==0);

				// if all above rules are true, add the user tile to the tiling
				if(notFloating&&eastMatch&&westMatch&&northMatch&&southMatch&&!(tiling[tileY][tileX].final))
				{
					tiling[tileY][tileX]=mouseTile;
				}
				// otherwise...
				else
				{
					// put the tile in an error state and set its TTL
					tiling[tileY][tileX].error=1;
					time(&(tiling[tileY][tileX].errorStart));
				}
			}
			// if we click the right mouse button...
			else if(button==GLUT_RIGHT_BUTTON)
				// ...and we're clicking a tile placed by the user...
				if(!(tiling[tileY][tileX].final))
					// ...delete it
					tiling[tileY][tileX]=blankTile;
	} // end IF MOUSE WITHIN SPECIFIED AREA

    // IF IN THE MOUSE TILE SELECTION AREA
    else if (y<=wHeight*(1-heightAdjust))
    {
    	int i = (int)(x-wWidth/20.0)/(int)(choiceTileSize*1.5);
    	int j = (int)(y - (wHeight*(1-heightAdjust))/2.0);
    	// if the y position of the mouse click looks about right
    	// and the x position is in the right place to click a tile
    	if(i<=6&&(int)(x-wWidth/20.0)%(int)(choiceTileSize*1.5)<=choiceTileSize && j<=0 && j>=-choiceTileSize)
    	{
    		// select the tile that is clicked
    		mouseTile=rTiles[i][mouseTileRotation];
    		mouseTileType=i;
    	}
    	// if we click anywhere else in the selection box
    	else
    	{
    		// remove the current selection
    		mouseTile=blankTile;
    	}
    } // END IF CLICKED IN SELECTION AREA
	} // END IF MOUSE_DOWN
} // end mouse function

// when the mouse is moved keep track of x and y
void mouse_motion(int x, int y)
{
  mouseX=x;
  mouseY=y;
} // mouse_motion()


// initialise everything
void initializeTiling()
{
	mouseTile=blankTile;
	squarePower=-1;
	currentSize=initialSize;
	tileSize=wWidth/(float)currentSize;
	centreX=currentSize/2;
	centreY=currentSize/2;
	totalHPan=0;
	totalVPan=0;
	hPan=0;
	vPan=0;
	tileCount=0;
	tiling=new Tile*[currentSize];
	int k;
	for(k=0;k<currentSize;k++)
		tiling[k]=new Tile[currentSize];

	Tile* blankTile = new Tile;

	int i,j;
	for(i=0 ; i<currentSize ; i++)
		for(j=0 ; j<currentSize ; j++)
			tiling[i][j]=*blankTile;
}

// special function keys
void special(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_RIGHT:
		hPanning=-1; // pan right
		break;
	case GLUT_KEY_LEFT:
		hPanning=1; // pan left
		break;
	case GLUT_KEY_DOWN:
		vPanning=1; // pan down
		break;
	case GLUT_KEY_UP:
		vPanning=-1; // pan up
		break;
	}
}

// special function keys
void specialUp(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT:
		hPanning=0;  // stop panning VVVVVV
		break;
	case GLUT_KEY_RIGHT:
		hPanning=0;
		break;
	case GLUT_KEY_UP:
		vPanning=0;
		break;
	case GLUT_KEY_DOWN:
		vPanning=0;
		break;
	}
}

// when keys are pressed
void keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 's':
	  waiting=!waiting;
	  break;
	case 'c':  // show colour scheme
		if(showColours==baseColours)
			showColours=0;
		else
			showColours=baseColours;
		break;
    case 'z':  // rotate selection tiles anti-clockwise
      mouseTileRotation=(mouseTileRotation-1)%4;
      if (mouseTileRotation<0)
    	  mouseTileRotation=3;
      break;
    case 'x':  // rotate selection tiles clockwise
      mouseTileRotation=(mouseTileRotation+1)%4;
      break;
    case 'r':  // reset
    	int k;
    	for(k=0;k<currentSize;k++)
    		delete[] tiling[k];
    	delete[] tiling;
    	tiling=NULL;
    	initializeTiling();
    	break;
    case 'w': // zoom in
    	zooming=1;
    	break;
    case 'q': // zoom out
    	zooming=-1;
    	break;
    case 32: // c
          halt=0;
          break;
  }

  if(mouseTile!=blankTile)
	  mouseTile=rTiles[mouseTileType][mouseTileRotation];
} // keyboard()

// when keys are released
void keyboardUp(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 'w':
		zooming=0;  // stop zooming
		break;
	case 'q':
		zooming=0;
		break;
	}
}

// display function
void display()
{
	glClearColor (1.0,1.0,1.0,1.0);	/* clear colour is white */
	glClear(GL_COLOR_BUFFER_BIT);	/* clear */

	displayTiling();  // display everything

	if(mouseTile!=blankTile)
	{
		glPushMatrix();
			glTranslatef(mouseX-5*tileSize/6.0,(wHeight-mouseY)-tileSize/6.0,0.0);
			drawTile(mouseTile);
		glPopMatrix();
	}

	glutSwapBuffers();
}

// GLUT reshape function
void reshape(int width, int height)
{
	wHeight=height;
	wWidth=width;
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// CHANGE THE 0.0s to height(or width)/1.1 to get a good view
	// of the upper right quadrant
	glOrtho(0.0, (GLfloat) width, 0.0, (GLfloat) height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// resizes the tile array if necessary
void resizeArray()
{
	// proximity = how far the centre should be from the edge
	int proximity = pow((float)2,(float)(squarePower+1)) + 10;
	int check=0;
	int check2=0;
	int difference=0;
	int difference2=0;
	Tile temp=tiling[centreX][centreY];
	// check left distance
	if(abs(temp.left)==3||abs(temp.left==2))
	{
		check = centreY-proximity;
		if(check<0)
		{
			difference=-check;
			check=1;
		}
		else
			check=0;
	}
	// check right distance
	else if(abs(temp.right==3)||abs(temp.right==2))
	{
		check = centreY+proximity;
		if(check>currentSize)
		{
			difference=check-currentSize;
			check=1;
		}
		else
			check=0;
	}
	// check bottom distance
	if(abs(temp.bottom)==3||abs(temp.bottom==2))
	{
		check2 = centreX-proximity;
		if(check2<0)
		{
			difference2=-check2;
			check2=1;
		}
		else
			check2=0;
	}
	// check top distance
	else if(abs(temp.top==3)||abs(temp.top==2))
	{
		check2 = centreX+proximity;
		if(check2>currentSize)
		{
			difference2=check2-currentSize;
			check2=1;
		}
		else
			check2=0;
	}
	check = check || check2;
	// which is the biggest difference?
	if(difference2>difference)
		difference=difference2;
	// check will be 1 if the next square will be out of bounds
	// check will be 0 if there is no need to resize

	if(check) // if we need to resize
	{
		int oldSize=currentSize;
		currentSize = currentSize + difference * 2;
		cout << "Array resizing.  Old size = " << oldSize*oldSize << ", new size = " << currentSize*currentSize << endl;

		// points to new tiling
		Tile **newTiling = new Tile*[currentSize];
		int k;
		for(k=0; k<currentSize; k++)
			newTiling[k]=new Tile[currentSize];

		int changeX=0;
		int changeY=0;

		// if pointing left
		if(abs(temp.left==3)||abs(temp.left==2))
			changeY = currentSize-oldSize;
		// if pointing down
		if(abs(temp.bottom==3)||abs(temp.bottom==2))
			changeX = currentSize-oldSize;

		// how much to move by
		hPan = hPan - changeY * tileSize;
		vPan = vPan - changeX * tileSize;

		int i; // copy across from old tiling
		for(i=0; i<oldSize; i++)
		{
			int j;
			for(j=0; j<oldSize; j++)
			{
				newTiling[i+changeX][j+changeY]=tiling[i][j];
			}
		}

		// free memory of old tiling and correct the pointer
		for(k=0; k<oldSize; k++)
			delete [] tiling[k];
		delete [] tiling;

		tiling=newTiling;
		centreX+=changeX;
		centreY+=changeY;
		totalHPan-=changeY*tileSize;
		totalVPan-=changeX*tileSize;
	}

}

// the idle function of the GLUT loop
void idleFunction()
{
	// Do panning
	float tileTerm = pow((float)tileCount,(float)0.15);
	totalHPan += hPanning * wWidth * moveAmount * tileTerm;
	totalVPan += vPanning * wHeight * moveAmount * tileTerm;
	hPan += hPanning * wWidth * moveAmount * (wWidth/(float)initialSize)/tileSize * tileTerm;
	vPan += vPanning * wHeight * moveAmount * (wHeight/(float)initialSize)/tileSize * tileTerm;

	// takes account of zoom and view position
	totalHPan -= (zooming * zoomAmount * tileTerm * (tileSize/(wWidth/(float)initialSize) * (wWidth/2.0 - hPan)));
	totalVPan -= (zooming * zoomAmount * tileTerm * (tileSize/(wHeight/(float)initialSize) * (wWidth/2.0 - vPan)));

	// increases tile size (part of the zoom)
	tileSize *= (zooming * zoomAmount * tileTerm) + 1;

	// Main loop
	display();
	if(!halt) // if it's time to do an iteration
	{
		inflate(); // inflate to the next super cell
		display();
		if(waiting)
			sleep(1);
		if(alternateColours==3)
			alternateColours=4;
		else
			alternateColours=3;
		decideOrientation(); // place the middle tile
		display();
		if(waiting)
			sleep(1);
		if(squarePower>0)
		{
			fillInBlanks(); // fill in the blank spaces by edge matching
			display();
			if(waiting)
				sleep(1);
		}
		 resizeArray(); // resize the array if necessary
		halt=1; // stop until user input
	}
}

// get command line options to set flags
void commandLineOptions(int argc, char* argv[])
{
	int i;
		for(i=1;i<argc;i++) /* loop through all command line options */
		{
			if(argv[i][0]=='-') /* if the 1st char is a - */
			{
				switch (argv[i][1])
				{
				case 'c': // -c 1 = colour scheme on
					i++;
					if(i<argc)
					{
						baseColours=atoi(argv[i]);
						showColours=baseColours;
					}
					break;
				case 's':
					waiting=1;
					break;
				default: break;
				}
			}
		}
}


// main function
int main(int argc, char* argv[])
{

	commandLineOptions(argc, argv); // set flags from command line

	time(&currentTime); // get the current time (used for error TTL)
	// initialize available tiles
	// separated into cross tiles and the others, since we know where the cross
	// tiles will appear, and must decide on their orientation
	initCrossTiles();
	initOtherTiles();

	initializeTiling(); // initializes everything

	// DEBUGGING
	// prints out tiles
	printAvailableTiles();

	glutInit(&argc, argv);                /* Initialize OpenGL */
	glutInitDisplayMode (GLUT_DOUBLE);    /* Set the display mode */
	glutInitWindowSize (wWidth,wHeight);         /* Set the window size */
	glutInitWindowPosition (0, 0);    /* Set the window position */
	glutCreateWindow ("Tiler"); 		  /* Create the window */
	glutDisplayFunc(display);             /* Register the "display" function */
	glutReshapeFunc(reshape);             /* Register the "reshape" function */
	glutIdleFunc(idleFunction);			  /* idle function */
	glutPassiveMotionFunc (mouse_motion);
	glutMouseFunc(mouse);
	glutKeyboardFunc (keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);
	glutMainLoop();                       /* Enter the main OpenGL loop */
}
