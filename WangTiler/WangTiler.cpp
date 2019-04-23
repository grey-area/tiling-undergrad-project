//============================================================================
// Name        : Wang.cpp
// Author      : Andrew Webb
// Date		   : 30 Mar 2010
// Version     : 0.5
// Copyright   : Your copyright notice
//============================================================================

#include <iostream>
#include <GL/glut.h>
#include <fstream>
#include <deque>
#include <stack>
#include "Tile.h"
#include <cstdlib>
#include <unistd.h>
using namespace std;

int displayType=1;

int showLines=1;

int windowHeight = 750;
int windowWidth = 750;
int initialSize = 15;
int currentSize = initialSize;

float tileSize;

int showTileSet=0;

vector <Tile> tileSet; // set of tiles to choose from
vector <vector<Tile> > tiling; // a 2D vector.. the tiling


typedef struct
{
	int x; /* decisions, for the decision stack */
	int y;
	int choice;
} decision;

Tile blankTile;
stack <decision> decisionStack;
decision currDecision;


// displays a single tile
void displayTile(Tile tile)
{
	if(tile.initialized)
	{
		// top of tile
		glColor4f(tile.topVector[0]/2.0,tile.topVector[1]/2.0,tile.topVector[2]/2.0,0.0);
		glBegin(GL_TRIANGLES);
			glVertex3f(0.0,tileSize,0.0);
			glVertex3f(tileSize,tileSize,0.0);
			glVertex3f(tileSize/2.0,tileSize/2.0,0.0);
		glEnd();

		// right of tile
		glColor4f(tile.rightVector[0]/2.0,tile.rightVector[1]/2.0,tile.rightVector[2]/2.0,0.0);
		glBegin(GL_TRIANGLES);
			glVertex3f(tileSize,0.0,0.0);
			glVertex3f(tileSize,tileSize,0.0);
			glVertex3f(tileSize/2.0,tileSize/2.0,0.0);
		glEnd();

		// bottom of tile
		glColor4f(tile.bottomVector[0]/2.0,tile.bottomVector[1]/2.0,tile.bottomVector[2]/2.0,0.0);
		glBegin(GL_TRIANGLES);
			glVertex3f(0.0,0.0,0.0);
			glVertex3f(tileSize,0.0,0.0);
			glVertex3f(tileSize/2.0,tileSize/2.0,0.0);
		glEnd();

		// left of tile
		glColor4f(tile.leftVector[0]/2.0,tile.leftVector[1]/2.0,tile.leftVector[2]/2.0,0.0);
		glBegin(GL_TRIANGLES);
			glVertex3f(0.0,0.0,0.0);
			glVertex3f(0.0,tileSize,0.0);
			glVertex3f(tileSize/2.0,tileSize/2.0,0.0);
		glEnd();

		// the black lines/outer box

		if(showLines)
		{
		glColor4f(0.0,0.0,0.0,0.0);
		glBegin(GL_LINE_LOOP);
			glVertex3f(0.0,0.0,0.0);
			glVertex3f(tileSize,0.0,0.0);
			glVertex3f(tileSize,tileSize,0.0);
			glVertex3f(0.0,tileSize,0.0);
		glEnd();
		glBegin(GL_LINE_STRIP);
			glVertex3f(0.0,0.0,0.0);
			glVertex3f(tileSize,tileSize,0.0);
		glEnd();
		glBegin(GL_LINE_STRIP);
			glVertex3f(0.0,tileSize,0.0);
			glVertex3f(tileSize,0.0,0.0);
		glEnd();
		}
	}
}

// shows the tiles that have been created
void displayTileSet()
{
	unsigned int i;
	// loop through tile set
	for(i=0; i<tileSet.size(); i++)
	{
		glPushMatrix();
		glTranslatef((i%(initialSize/3) +0.5)*tileSize*2,(i/(initialSize/3) +0.5)*tileSize*2,0.0);
		displayTile(tileSet[i]);
		glPopMatrix();
	}
}

// shows the tiling as it's being constructed
void displayTiling()
{
	unsigned int i;
	for(i=0; i<tiling.size(); i++)
	{
		unsigned int j;
		for(j=0; j<tiling[i].size(); j++)
		{
			glPushMatrix();
			glTranslatef(i*tileSize, j*tileSize, 0.0);
			displayTile(tiling[i][j]);
			glPopMatrix();
		}
	}
}

// returns TRUE if the current decision tile matches edges of those around it
int allowedPlacement(int y, int x)
{
	int leftMatch=1;
	int topMatch=1;
	int rightMatch=1;
	int bottomMatch=1;

	if(tiling[y+1][x].initialized && tiling[y][x].rightVector!=tiling[y+1][x].leftVector)
		rightMatch=0;
	if(tiling[y][x+1].initialized && tiling[y][x].topVector!=tiling[y][x+1].bottomVector)
		topMatch=0;
	if(y-1>=0 && tiling[y-1][x].initialized && tiling[y][x].leftVector!=tiling[y-1][x].rightVector)
		leftMatch=0;
	if(x-1>=0 && tiling[y][x-1].initialized && tiling[y][x].bottomVector!=tiling[y][x-1].topVector)
		bottomMatch=0;

	return leftMatch&&topMatch&&rightMatch&&bottomMatch;
}

void display()
{
	glClearColor (1.0,1.0,1.0,1.0);	/* clear colour is white */
	glClear(GL_COLOR_BUFFER_BIT);	/* clear */

	if(showTileSet)
		displayTileSet();
	else
		displayTiling();
	glutSwapBuffers();
}

void constructTiling()
{
	if(currDecision.y==currentSize-1 || currDecision.x==currentSize-1)
	{
		currentSize++;
		tiling.resize(currentSize);
		int i;
		for(i=0; i<currentSize; i++)
			tiling[i].resize(currentSize);
		tileSize=windowWidth/(float)(currentSize-1);
	}

	if(displayType==0)
	{
		display();
		sleep(1);
	}
	else if(displayType==1)
		display();
	tiling[currDecision.y][currDecision.x]=tileSet[currDecision.choice];
	if(allowedPlacement(currDecision.y,currDecision.x))
	{
		decisionStack.push(currDecision);
		currDecision.choice=tileSet.size()-1;
		// change position
		if(currDecision.x<currDecision.y)
		{
			currDecision.x++;
		}
		else
		{
			if(currDecision.y==0)
			{
				if(displayType==2)
				{
					display();
				}
				currDecision.y=currDecision.x+1;
				currDecision.x=0;
			}
			else
			{
				currDecision.y--;
			}
		}
	}
	// if that placement is not allowed
	else
	{
		currDecision.choice--;
		//if all choices have been exhausted, then backtrack
		while(currDecision.choice == -1)
		{
			tiling[currDecision.y][currDecision.x]=blankTile;
			if(displayType<2 || currDecision.y==0)
							display();
			if(!(decisionStack.empty()))
			{
				currDecision=decisionStack.top();
				currDecision.choice--;
				decisionStack.pop();
			}
		}
	}

}

// for every glut loop...
void idleFunction()
{
	if(!showTileSet)
		constructTiling();
}

void reshape(int width, int height)
{
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (GLfloat) width, 0.0, (GLfloat) height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// does initialization
void init(int argc, char* argv[])
{
	currDecision.x=0;
	currDecision.y=0;

	string description = "description/";
	string file;
	int inputFileSpecified=0;

	ifstream reader;

	int i;
	for(i=1;i<argc;i++) /* loop through all command line options */
	{
	if(argv[i][0]=='-') /* if the 1st char is a - */
	{
		switch (argv[i][1])
		{
		case 'l':
			showLines=0;
			break;
		case 's':
			showTileSet=1;
			break;
		case 'w':
			inputFileSpecified=1;
			i++;
			if(i<argc)
				file.assign( argv[i] );
			else
			{
				cout << "Correct use: WangTiler -w <Wang Description Filename>" << endl;
				exit(0);
			}
			description.append(file);
			cout << "Wang tile description file: " << description << endl;
			reader.open(description.c_str());
			if(!reader)
			{
				cout << "Error opening input file" << endl;
				exit(0);
			}
			break;
		default: break;
		}
	}
	} // end loop through command line options

	if(!inputFileSpecified)
		reader.open("description/example");
	if(!reader)
	{
		cout << "Error opening input file" << endl;
		exit(0);
	}

	// construct the tile set
	Tile tempTile;
	vector <int> *tempVector;
	tempVector=&(tempTile.topVector);
	i=1;
	while(!reader.eof())
	{
		switch(i)
		{
		case 4:	tempVector=&(tempTile.rightVector);
				break;
		case 7:	tempVector=&(tempTile.bottomVector);
				break;
		case 10:	tempVector=&(tempTile.leftVector);
					break;
		case 13:	tempTile.initialized=1;
					tileSet.push_back(tempTile);
					i=1;
					tempTile=blankTile;
					tempVector=&(tempTile.topVector);
					break;
		default: break;
		} // end of switch
		char letter;
		reader.get(letter);
		// ignore new lines, spaces and tabs
		// so that the input can be structured to be
		// 'slightly' more readable
		if(!(letter=='\n' || letter==' ' || letter=='\t'))
		{
			int symbol=atoi(&letter);
			(*tempVector).push_back(symbol);
			i++;
		}
	}

	tileSize=windowWidth/(float)(currentSize-1);

	tiling.resize(currentSize);
	int p;
	for(p=0; p<currentSize; p++)
		tiling[p].resize(currentSize);

	currDecision.choice=tileSet.size()-1;

} // end init

void printTileSet()
{
	unsigned int i;
	// for each tile
	for(i=0; i<tileSet.size(); i++)
	{
		Tile tempTile = tileSet[i];
		cout << "Tile " << i << ": ";
		int j;
		// for each side...
		for(j=0; j<4; j++)
		{
			vector <int> tempVector;
			switch(j)
			{
			case 0:	tempVector=tempTile.topVector;
					cout << "T:";
					break;
			case 1: tempVector=tempTile.rightVector;
					cout << "R:";
					break;
			case 2:	tempVector=tempTile.bottomVector;
					cout << "B:";
					break;
			case 3: tempVector=tempTile.leftVector;
					cout << "L:";
					break;
			default:	break;
			} // end switch
			int k;
			// for each colour...
			for(k=0; k<3; k++)
			{
				cout << tempVector[k];
			} // end for each colour
			cout << " ";
		} // end for each side
		cout << endl;
	} // end for each tile
}

// special function keys
void special(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_DOWN:
		displayType--;
		if(displayType<0)
			displayType=0;
		cout << "Display type = " << displayType << endl;
		break;
	case GLUT_KEY_UP:
		displayType++;
		if(displayType>2)
			displayType=2;
		cout << "Display type = " << displayType << endl;
		break;
	}
}


int main(int argc, char* argv[])
{
	init (argc, argv);                    /* Do any other initialization */
	//printTileSet();	// prints out the tile set, neatly

	// OpenGL stuff
	glutInit(&argc, argv);                /* Initialize OpenGL */
	glutInitDisplayMode (GLUT_DOUBLE);    /* Set the display mode */
	glutInitWindowSize (windowWidth,windowHeight);         /* Set the window size */
	glutInitWindowPosition (0, 0);    /* Set the window position */
	glutCreateWindow ("Wang Tiler"); 		  /* Create the window */
	glutDisplayFunc(display);             /* Register the "display" function */
	glutReshapeFunc(reshape);             /* Register the "reshape" function */
	glutIdleFunc(idleFunction);			  /* idle function */
	//glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMainLoop();                       /* Enter the main OpenGL loop */
	return 0;
}
