
#include <GL/glut.h>
#include <deque>
#include <stack>
#include "TileCompiler.h"
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

int displayType=2; // display after each decision, each tile or each row?
int windowHeight = 750; // dimensions of window
int windowWidth = 750;

TileCompiler *ptr_compiler;

int showTileSet=0; // if 1 then don't show tiling, just show complete tile set

vector <Tile> tileVector;
vector <Tile>& rTileVector=tileVector;

int initialSize=40;
int displaySize=initialSize;
int xSize=initialSize;
int ySize=initialSize; // ySize will grow
int rowCount=0;
float tileSize = windowWidth/(float)displaySize;
// a 2D vector
vector <vector<Tile> > tiling;

Tile blankTile;
Tile blankAlphabetTile;

// how much have we panned?
float hPan=0;
float vPan=0;

// a vector of states in which the TM halts
vector <int> haltStates;
int halted=0; // have we reached a halting state?
int finishedHaltedRow=0;
int haltedRow=-1; // which row did we halt on?

int checkPeriodicity=0; // are we checking for periodicity?
int repeatedRow1=-1; // which row had we reached when noticed repetition?
int repeatedRow2=-1; // which row does it repeat?

char* tMachineFilename; // the filename of TM, to be passed to tile compiler

ifstream reader;

typedef struct {int x; /* decisions, for the decision stack */
                int y;
                int choice;
               } decision;

int currentX=0;
int currentY=1;
stack <decision> decisionStack;
decision currDecision;

// initialises the tiler
void init(int argc, char* argv[], TileCompiler& compiler)
{
	int i;
	string input = "input/";
	string file;
	int inputFileSpecified=0;
	for(i=1;i<argc;i++) /* loop through all command line options */
	{
		if(argv[i][0]=='-') /* if the 2nd char is a - */
		{
			switch (argv[i][1])
			{
			case 'l':
				i++;
				if(i<argc)
					sscanf(argv[i], "%d",&initialSize); /* sets initial size */
				break;
			case 'p':
				checkPeriodicity=1;
				break;
			case 's':
				i++;
				showTileSet=1;
				break;
			case 'd':
				i++;
				if(i<argc)
					sscanf(argv[i],"%d",&displayType); /* set display type */
				break;
			case 'i':
				inputFileSpecified=1;
				i++;
				if(i<argc)
					file.assign( argv[i] );
				else
				{
					cout << "Correct use: Tiler -i <Input Filename>" << endl;
					exit(0);
				}
				input.append(file);
				cout << "Input file: " << input << endl;
				reader.open(input.c_str());
				if(!reader)
				{
					cout << "Error opening input file" << endl;
					exit(0);
				}
				break;
			default: break;
			}
		}
	}

	if(!inputFileSpecified)
		reader.open("input/example");
	if(!reader)
	{
		cout << "Error opening input file" << endl;
		exit(0);
	}
	currDecision.x=0;
	currDecision.y=1;
	currDecision.choice=-1;

	glColor4f(0.0,0.0,0.0,0.0);

	vector <int> inputVector;
	while(!reader.eof())
	{
		char letter;
		reader.get(letter);
		// ignore new lines, spaces and tabs
		// so that the input can be structured to be
		// 'slightly' more readable
		if(!(letter=='\n' || letter==' ' || letter=='\t'))
		{
			int symbol;
			if(letter=='b')
				symbol=-4;
			else
				symbol=atoi(&letter);
			inputVector.push_back(symbol);
		}
	}
	inputVector.pop_back();

	cout << "Input vector size: " << inputVector.size() << endl;

	initialSize=inputVector.size();
	if(initialSize<15)
		initialSize=15;

	displaySize=initialSize;
	xSize=initialSize;
	ySize=initialSize;
	tileSize = windowWidth/((float)displaySize);
	tiling.resize(ySize);

	int p;
	for(p=0; p<ySize; p++)
		tiling[p].resize(xSize);

	unsigned int k=0; // to index the input symbol vector

	int middle = 0;
	for(i=0;i<initialSize;i++)
	{
		int currentSymbol = -4;
		if(k<inputVector.size())
		{
			currentSymbol = inputVector.at(k); // get symbol from input file
			//cout << "K: " << k << "  Current Symbol:" << currentSymbol << endl;
		}
		if(i<middle)
		{
			// left facing blank tile
			tiling[0][i]=*(compiler.ptr_leftFacingBlank);
			tiling[0][i].finalDecision=1;
		}
		else if(i==middle)
		{
			// middle tile, using input
			switch(currentSymbol)
			{
			case -4:
				tiling[0][i]=*(compiler.ptr_middleBlank);
				break;
			case 0:
				tiling[0][i]=*(compiler.ptr_middleZero);
				break;
			case 1:
				tiling[0][i]=*(compiler.ptr_middleOne);
				break;
			default:
				tiling[0][i]=*(compiler.ptr_middleBlank);
				break;
			}
			tiling[0][i].finalDecision=1;
			k++; // increment k for next loop
		}
		else
		{
			// right facing tile, using input
			switch(currentSymbol)
			{
			case -4:
				tiling[0][i]=*(compiler.ptr_rightFacingBlank);
				break;
			case 0:
				tiling[0][i]=*(compiler.ptr_rightFacingZero);
				break;
			case 1:
				tiling[0][i]=*(compiler.ptr_rightFacingOne);
				break;
			default:
				tiling[0][i]=*(compiler.ptr_rightFacingBlank);
				break;
			}
			tiling[0][i].finalDecision=1;
			k++; // increment k for next loop
		}
	}
	if(reader)
		reader.close();
}

string strToBin( unsigned long n )
{
	string result;

	do result.push_back( '0' + (n & 1) );
	while (n >>= 1);

	reverse( result.begin(), result.end() );
	return result;
}

// given a tile object, this will draw it
void drawTile(Tile currentTile)
{
	glBegin(GL_LINE_STRIP);
		glVertex3f(0.0,0.0,0.0);

		//left side
		glVertex3f(0.0,tileSize/4.0,0.0);
		switch(currentTile.leftLabelVector.at(0))
		{
		case -1: // left facing bump
			glVertex3f(-tileSize/4.0,tileSize/2.0,0.0);
			glVertex3f(0.0,3.0*tileSize/4.0,0.0);
			break;
		case -2: // right facing bump
			glVertex3f(tileSize/4.0,tileSize/2.0,0.0);
			glVertex3f(0.0,3.0*tileSize/4.0,0.0);
			break;
		default: break;
		}
		// state
		if(currentTile.leftLabelVector.size()>1)
		{
			string stateString = strToBin(currentTile.leftLabelVector.at(0));
			char *stateChars=new char[stateString.size()];
			memcpy(stateChars,stateString.c_str(),stateString.size());
			unsigned int k;
			for(k=0;k<stateString.size();k++)
			{
				int numb;
				if(stateChars[k]=='0')
					numb=-1;
				else
					numb=1;
				glVertex3f(currentTile.leftLabelVector.at(1)*numb*tileSize/5.0,tileSize/4.0+(2.0/4.0)*tileSize*(double)k/stateString.size(),0.0);
				glVertex3f(0.0,tileSize/4.0+(2.0/4.0)*tileSize*(double)(k+0.5)/stateString.size(),0.0);
			}
			//glVertex3f(currentTile.leftLabelVector.at(1)*(currentTile.leftLabelVector.at(0)+1)*tileSize/4.0,tileSize/2.0,0.0);
			glVertex3f(0.0,3.0*tileSize/4.0,0.0);
		}
		glVertex3f(0.0,tileSize,0.0);
		glVertex3f(tileSize/8.0,tileSize,0.0);
		// top side
		// symbol
		switch(currentTile.topLabelVector.at(0))
		{
		case 0: // zero symbol
			glVertex3f(tileSize/4.0,tileSize-tileSize/4.0,0.0);
			glVertex3f(3.0*tileSize/8.0,tileSize,0.0);
			break;
		case 1: // one symbol
			glVertex3f(tileSize/4.0,tileSize+tileSize/4.0,0.0);
			glVertex3f(3.0*tileSize/8.0,tileSize,0.0);
			break;
		case -4: // blank symbol
			glVertex3f(tileSize/8.0,tileSize+tileSize/8.0,0.0);
			glVertex3f(3.0*tileSize/8.0,tileSize+tileSize/8.0,0.0);
			glVertex3f(3.0*tileSize/8.0,tileSize,0.0);
			break;
		case -5: // start symbol
			glVertex3f(3.0*tileSize/8.0,tileSize+tileSize/2.0,0.0);
			glVertex3f(3.0*tileSize/8.0,tileSize,0.0);
			break;
		default: break;
		} // end of switch.  Need to put in a case for blank symbol
		// state
		if(currentTile.topLabelVector.size()>1)
		{
			string stateString = strToBin(currentTile.topLabelVector.at(1));
			char *stateChars=new char[stateString.size()];
			memcpy(stateChars,stateString.c_str(),stateString.size());
			glVertex3f(tileSize/2.0,tileSize,0.0);
			unsigned int k;
			for(k=0;k<stateString.size();k++)
			{
				int numb;
				if(stateChars[k]=='0')
					numb=-1;
				else
					numb=1;
				glVertex3f(tileSize/2.0+(2.0/4.0)*tileSize*(double)k/stateString.size(),tileSize+numb*tileSize/5.0,0.0);
				glVertex3f(tileSize/2.0+(2.0/4.0)*tileSize*(double)(k+0.5)/stateString.size(),tileSize,0.0);
			}

			//glVertex3f(3.0*tileSize/4.0,tileSize+(currentTile.topLabelVector.at(1)+1)*tileSize/4.0,0.0);
		}
		glVertex3f(tileSize,tileSize,0.0);

		//right side
		glVertex3f(tileSize,3.0*tileSize/4.0,0.0);
		switch(currentTile.rightLabelVector.at(0))
		{
		case -1: // left facing bump
			glVertex3f(tileSize-tileSize/4.0,tileSize/2.0,0.0);
			glVertex3f(tileSize,tileSize/4.0,0.0);
			break;
		case -2: // right facing bump
			glVertex3f(tileSize+tileSize/4.0,tileSize/2.0,0.0);
			glVertex3f(tileSize,tileSize/4.0,0.0);
			break;
		default: break;
		}
		// state
		if(currentTile.rightLabelVector.size()>1)
		{
			string stateString = strToBin(currentTile.rightLabelVector.at(0));
			char *stateChars=new char[stateString.size()];
			memcpy(stateChars,stateString.c_str(),stateString.size());
			//glVertex3f(tileSize/2.0,tileSize,0.0);
			unsigned int k;
			for(k=stateString.size();k>0;k--)
			{
				int numb;
				if(stateChars[k-1]=='0')
					numb=-1;
				else
					numb=1;
				glVertex3f(tileSize,tileSize/4.0+(2.0/4.0)*tileSize*(double)(k-0.5)/stateString.size(),0.0);
				glVertex3f(tileSize+currentTile.rightLabelVector.at(1)*numb*tileSize/5.0,tileSize/4.0+(2.0/4.0)*tileSize*(double)(k-1)/stateString.size(),0.0);
			}
			//glVertex3f(tileSize+currentTile.rightLabelVector.at(1)*(currentTile.rightLabelVector.at(0)+1)*tileSize/4.0,tileSize/2.0,0.0);
			glVertex3f(tileSize,tileSize/4.0,0.0);
		}
		glVertex3f(tileSize,0.0,0.0);

		// bottom side
		// state
		if(currentTile.bottomLabelVector.size()>1)
		{
			string stateString = strToBin(currentTile.bottomLabelVector.at(1));
			char *stateChars=new char[stateString.size()];
			memcpy(stateChars,stateString.c_str(),stateString.size());
			//glVertex3f(tileSize/2.0,tileSize,0.0);
			unsigned int k;
			for(k=stateString.size();k>0;k--)
			{
				int numb;
				if(stateChars[k-1]=='0')
					numb=-1;
				else
					numb=1;
				glVertex3f(tileSize/2.0+(2.0/4.0)*tileSize*(double)(k-0.5)/stateString.size(),0.0,0.0);
				glVertex3f(tileSize/2.0+(2.0/4.0)*tileSize*(double)(k-1)/stateString.size(),numb*tileSize/5.0,0.0);
			}

			glVertex3f(tileSize/2.0,0.0,0.0);
		}
		glVertex3f(3.0*tileSize/8.0,0.0,0.0);
		// symbol
		switch(currentTile.bottomLabelVector.at(0))
		{
		case 0: // zero symbol
			glVertex3f(tileSize/4.0,-tileSize/4.0,0.0);
			break;
		case 1: // one symbol
			glVertex3f(tileSize/4.0,tileSize/4.0,0.0);
			break;
		case -4: // blank symbol
			glVertex3f(3.0*tileSize/8.0,tileSize/8.0,0.0);
			glVertex3f(tileSize/8.0,tileSize/8.0,0.0);
			break;
		case -5: // start symbol
			glVertex3f(3.0*tileSize/8.0,0.0,0.0);
			glVertex3f(3.0*tileSize/8.0,tileSize/2.0,0.0);
			break;
		default: break;
		} // end of switch.  Need to put in a case for blank symbol
		glVertex3f(tileSize/8.0,0.0,0.0);
		glVertex3f(0.0,0.0,0.0);
	glEnd();
}

void displayTileSet()
{
	int i;
	int horizontalTranslationCount=0;
	int verticalTranslationCount=0;
	glPushMatrix();
	glTranslatef(tileSize/2.0,tileSize/2.0,0.0);
	for(i=0;i<(signed int)rTileVector.size();i++)
	{
		if(-horizontalTranslationCount==initialSize/3)
		{
			glPushMatrix();
			glTranslatef(tileSize*2*horizontalTranslationCount,tileSize*2,0.0);
			verticalTranslationCount++;
			horizontalTranslationCount=0;
		}
		horizontalTranslationCount--;
		glPushMatrix();
		glTranslatef(i*tileSize*2,0.0,0.0);
		drawTile(rTileVector.at(i));
		glPopMatrix();
	}
	for(i=0;i<verticalTranslationCount;i++)
	{
		glPopMatrix();
	}
	glPopMatrix();
}

void displayTiling()
{
	int i,j;
	int yStart=0;
	if(rowCount-initialSize>0)
	{
		yStart=rowCount-(windowWidth/tileSize);
		if(yStart<0)
			yStart=0;
	}
	for(i=yStart;i<ySize;i++)
	{
		if(i==haltedRow)
		{
			glColor4f(0.0,1.0,0.0,0.0);
			glLineWidth(2.0);
		}
		if(i==repeatedRow1 || i==repeatedRow2)
		{
			glColor4f(1.0,0.0,0.0,0.0);
			glLineWidth(2.0);
		}
		for(j=0;j<xSize;j++)
		{
			glPushMatrix();
			glTranslatef(hPan,vPan,0.0);
			glTranslatef(tileSize*j,tileSize*i,0.0);
			if(tiling[i][j].initialized)
			{
				drawTile(tiling[i][j]);
			}
			glPopMatrix();
		}
		if(i==haltedRow || i==repeatedRow1 || i==repeatedRow2)
		{
			glColor4f(0.0,0.0,0.0,0.0);
			glLineWidth(1.0);
		}
	}
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

void reshape(int width, int height)
{
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, (GLfloat) width, 0.0, (GLfloat) height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// a tile matches the 4 bordering tiles iff:
// they don't exist OR they are not initialized OR they actually match
bool matching(int x, int y)
{
	bool toReturn=true;
	if(y-1>=0)
	{
		if(tiling[y-1][x].initialized && !(tiling[y-1][x].topLabelVector==tiling[y][x].bottomLabelVector))
			toReturn=false;
	}
	if(y+1>=ySize)
	{
		int oldYSize=ySize;
		int increase=initialSize;
		ySize+=increase;
		tiling.resize(ySize);
		int k;
		for(k=oldYSize; k<ySize; k++)
			tiling[k].resize(xSize);
	}
		if(tiling[y+1][x].initialized && !(tiling[y+1][x].bottomLabelVector==tiling[y][x].topLabelVector))
			toReturn=false;
	if(x-1>=0)
	{
		if(tiling[y][x-1].initialized && !(tiling[y][x-1].rightLabelVector==tiling[y][x].leftLabelVector))
			toReturn=false;
	}
	else
	{
		if(tiling[y][x].leftLabelVector.at(0)!=-3)
			toReturn=false;
	}
	if(x+1<xSize)
	{
		if(tiling[y][x+1].initialized && !(tiling[y][x+1].leftLabelVector==tiling[y][x].rightLabelVector))
			toReturn=false;
	}
	/*else
	{
		if(tiling[y][x].rightLabelVector.at(0)!=-3)
			toReturn=false;
	}*/
	return toReturn;
}

int checkForHalting(int state)
{
	int check=0;
	unsigned int i;
	for(i=0; i<haltStates.size(); i++)
	{
		check=check||(state==haltStates[i]);
	}
	return check;
}

void checkForPeriodicity(int row)
{
	int j,i;
	for(j=row-1; j>=0; j--)
	{
		int checkRow=1;
		for(i=0; i<xSize; i++)
		{
			int checkTile=(tiling[row][i].topLabelVector == tiling[j][i].topLabelVector);
			checkRow=checkRow && checkTile;
		}
		if(checkRow==1)
		{
			repeatedRow1=row;
			repeatedRow2=j;
			halted=1;
			j=-1;
		}
	}
}

void idleFunction()
{
	if(!finishedHaltedRow)
	{
	if(displayType==0)
	{
		long int i;
		for(i=0;i<1000000;i++)
		{
			cout << "";
		}
	}
	if(decisionStack.empty() && currDecision.choice >=(signed int)tileVector.size())
	{
		display();
	}
	else
	{
		// for each call of this function, the tiler should attempt to place one more tile
		// or one more row?
		if (currDecision.y == ySize)
		{
		}
		else
		{
		currDecision.choice++;
		if (currDecision.choice >= (signed int)tileVector.size())
		{
			tiling[currDecision.y][currDecision.x]=blankTile;
			if(!decisionStack.empty())
			{
				currDecision=decisionStack.top();
				decisionStack.pop();
				if(displayType==1||(currDecision.x==xSize-1))
					display();
			}
		}
		else
		{

			tiling[currDecision.y][currDecision.x]=tileVector[currDecision.choice];

			if(matching(currDecision.x,currDecision.y))
			{
				if(tiling[currDecision.y][currDecision.x].topLabelVector.size()==2)
					if(checkForHalting(tiling[currDecision.y][currDecision.x].topLabelVector.at(1)))
					{
						halted=1;
						cout << "Reached halting state" << endl;
					}
				if(displayType==1)
							display();
				decisionStack.push(currDecision);
				currDecision.x++;

				// horizontal resizing
				if(currDecision.x==xSize && (tiling[currDecision.y][currDecision.x-1].rightLabelVector.at(0)!=-3))
				{
					int amountToResizeBy = xSize/3;

					int i,j;
					for(j=0; j<amountToResizeBy; j++)
						tiling[0].push_back(tiling[0][xSize-1]);
					for(i=1; i<currDecision.y; i++)
					{
						for(j=0; j<amountToResizeBy; j++)
						{
							tiling[i].push_back(blankAlphabetTile);
							tiling[i].back().initialized=1;
						}
					}
					for(i=currDecision.y; i<ySize; i++)
					{
						for(j=0; j<amountToResizeBy; j++)
						{
							tiling[i].push_back(blankTile);
							tiling[i].back().initialized=0;
						}
					}
					tiling[currDecision.y][currDecision.x]=blankTile;
					tiling[currDecision.y][currDecision.x].initialized=0;
					//vPan-=tileSize;
					tileSize=tileSize/((float)(xSize+amountToResizeBy)/(float)(xSize));

					//deconstruct decision stack
					vector <decision> oldStack;
					while(!(decisionStack.empty()))
					{
						oldStack.push_back(decisionStack.top());
						decisionStack.pop();
					}

					// the top of the decision stack is at the beginning of the vector
					// so need to reconstruct it starting at the end of the vector

					decision blankDecision;
					blankDecision.choice=-1;
					//reconstruct decision stack
					int p,k;
					for(p=0; p<ySize; p++)
					{
						for(k=0; k<xSize; k++)
						{
							decisionStack.push(oldStack[(oldStack.size()-1)-(p*xSize)+k]);
						} // at this point a single row is complete (minus the additions)
						blankDecision.y=p+1;
						if(p!=(ySize-1))
						{
							int l;
							for(l=0; l<amountToResizeBy; l++)
							{
								blankDecision.x=xSize+l;
								decisionStack.push(blankDecision);
							}
						}
					}

					xSize+=amountToResizeBy;

					display();

				}

				if(currDecision.x >= xSize)
				{
					if(checkPeriodicity)
						checkForPeriodicity(currDecision.y);
					if(halted)
					{
						finishedHaltedRow=1;
						haltedRow=currDecision.y;
					}
					currDecision.x=currDecision.x-xSize;

					currDecision.y++;
					display();
					float position;
					if(currDecision.y>7*(windowWidth/tileSize)/8.0)
						position=-currDecision.y+7*(windowWidth/tileSize)/8.0;
					else
						position=0;
					vPan=position*tileSize;
					rowCount=currDecision.y;
				}
				currDecision.choice=-1;
			}
		}
		if(displayType==0)
			display();
		}
	}
	}
}

// special function keys
void special(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_DOWN:
		displayType--;
		if(displayType<0)
			displayType=0;
		break;
	case GLUT_KEY_UP:
		displayType++;
		if(displayType>2)
			displayType=2;
		break;
	}
}

// when keys are pressed
void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 'p':
		checkPeriodicity=!checkPeriodicity;
		break;
	case 32:
		halted=0;
		finishedHaltedRow=0;
		haltedRow=-1;
		checkPeriodicity=0;
		// stop zooming
		break;
	}
}

int main(int argc, char* argv[])
{
	int i;
	for(i=1;i<argc;i++) /* loop through all command line options */
	{
		if(argv[i][0]=='-') /* if the 1st char is a - */
		{
			switch (argv[i][1])
			{
			case 't':
				i++;
				if(i<argc)
					tMachineFilename=argv[i];
				else
				{
					cout << "Correct use: Tiler -t <Turing Machine Filename>" << endl;
					exit(0);
				}
				break;
			default: break;
			}
		}
	}

	// creates a tile compiler
	TileCompiler compiler = TileCompiler(tMachineFilename);
	ptr_compiler = &compiler;

	blankAlphabetTile.topLabelVector.push_back(-4);
	blankAlphabetTile.bottomLabelVector.push_back(-4);
	blankAlphabetTile.rightLabelVector.push_back(-3);
	blankAlphabetTile.leftLabelVector.push_back(-3);
	blankAlphabetTile.initialized=1;
	blankAlphabetTile.finalDecision=0;
	// creates a set of tiles from it
	// later, the name of the file describing the Turing machine
	// should be passed to here
	tileVector = compiler.mainFunction();
	haltStates = compiler.haltState;

	// if the 2nd arg is 1, print the tile set
	compiler.printTileSet(rTileVector,0);
	TileCompiler& rCompiler=compiler;

	// OpenGL
	glutInit(&argc, argv);                /* Initialize OpenGL */
	glutInitDisplayMode (GLUT_DOUBLE);    /* Set the display mode */
	glutInitWindowSize (windowWidth,windowHeight);         /* Set the window size */
	glutInitWindowPosition (0, 0);    /* Set the window position */
	glutCreateWindow ("Tiler"); 		  /* Create the window */
    init (argc, argv, rCompiler);                    /* Do any other initialization */
    glutDisplayFunc(display);             /* Register the "display" function */
    glutReshapeFunc(reshape);             /* Register the "reshape" function */
    glutIdleFunc(idleFunction);			  /* idle function */
    glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
    glutMainLoop();                       /* Enter the main OpenGL loop */

    return 0;
}
