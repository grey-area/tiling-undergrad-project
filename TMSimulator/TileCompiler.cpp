//============================================================================
// Name        : Tiler.cpp
// Author      : Andrew Webb
// Version
// Description :	Takes a description of a Turing machine from a named
//					file and produces a tile set from it
// Notes	:	For the tile labels, -3 denotes blank, -2 denotes right arrow
//				-1 denotes left arrow.  All other labels are positive integers
//				where the first integer on a label is the symbol and the second
//				is the state (if it exists)
//============================================================================

#include "TileCompiler.h"
#include <cstdlib>

char* tMachineName; // filename of turing machine
int mergeTileCount=0;
// a template for each type of edge
Tile leftFacingBlank,middleBlank,middleOne,middleZero,rightFacingBlank,rightFacingOne,rightFacingZero;

// initialises the tile compiler
void initialize(ifstream& reader, int& initialState, vector <int>& haltVector, unsigned int& numberOfSymbols, vector <int>& definedSymbolVector)
{
	string stringNumberOfSymbols;
	stringstream stream;
	while ( ! reader.eof() )	// set the initial state of the machine
	{
		string temp;
		stringstream tempstream;
		getline(reader,temp,'\n');
		tempstream << temp;
		tempstream >> initialState;
		break;
	}

	stringstream stream2;
	unsigned int numberOfHaltingStates=0;
	while ( ! reader.eof() )	// set the number of halting states
	{
		string stringNumberOfHaltingStates;
		getline(reader,stringNumberOfHaltingStates,'\n');
		stream2 << stringNumberOfHaltingStates;
		stream2 >> numberOfHaltingStates;
		break;
	}

	unsigned int i=0;
	while ( !reader.eof() )		// set the halting states
	{
		string temp;
		int tempint;
		stringstream tempstream;
		if(i<(numberOfHaltingStates-1))
		{
			getline(reader,temp,'\t');
			tempstream << temp;
			tempstream >> tempint;
			haltVector.push_back(tempint);
			i++;
		}
		else
		{
			getline(reader,temp,'\n');
			tempstream << temp;
			tempstream >> tempint;
			haltVector.push_back(tempint);
			break;
		}
	}

	while ( ! reader.eof() )	// set the number of defined symbols
	{
		getline(reader,stringNumberOfSymbols,'\n');
		stream << stringNumberOfSymbols;
		stream >> numberOfSymbols;
		break;
	}

	i=0;
	while ( !reader.eof() )		// set the defined symbols
	{
		string temp;
		int tempint;
		stringstream tempstream;
		if(i<(numberOfSymbols-1))
		{
			getline(reader,temp,'\t');
			tempstream << temp;
			tempstream >> tempint;
			definedSymbolVector.push_back(tempint);
			i++;
		}
		else
		{
			getline(reader,temp,'\n');
			tempstream << temp;
			tempstream >> tempint;
			definedSymbolVector.push_back(tempint);
			break;
		}
	}

	cout << "Initial state: " << initialState << endl;
	cout << "Defined symbols: " << "[";		// some output
	for(i=0;i<definedSymbolVector.size();i++)
	{
		cout << definedSymbolVector.at(i);
		if( i < ( definedSymbolVector.size() - 1 ) )
			cout << ",";
	}
	cout << "]" << endl;	// end output
} // end initialization

// constructs a tile from given edge labels
void constructTile(vector <Tile>& tileVector,
				vector <int> top, vector <int> left, vector <int> right,
				vector <int> bottom, int isInitConfig)
{
	Tile currentTile;
	currentTile.topLabelVector=top;
	currentTile.bottomLabelVector=bottom;
	currentTile.leftLabelVector=left;
	currentTile.rightLabelVector=right;
	// decides if the tile is part of the initial configuration
	currentTile.isInitConfigTile = isInitConfig;
	currentTile.initialized=1;
	tileVector.push_back(currentTile); //pushes onto the tile vector
}

// constructed the initial configuration tiles
// (together, they form the initial row of the tiling)
void constructInitConfigTiles(int initialState,
		vector <Tile>& tileVector, vector <int>& definedSymbolVector)
{
	unsigned int i;
	// init config tiles with symbol only (left)
	for(i=0;i<definedSymbolVector.size();i++)
	{
		vector <int> top,bottom,left,right;
		top.push_back(definedSymbolVector.at(i)); // symbol at the top
		bottom.push_back(-3); // blank at the bottom
		// -1 = left facing, the 1=init tile
		left.push_back(-1);
		right.push_back(-1);
		constructTile(tileVector, top, left, right, bottom, 1);
		switch(definedSymbolVector.at(i))
				{
				case -4:
					leftFacingBlank=tileVector.at(tileVector.size()-1);
					break;
				default: break;
				}
	}
	// init config tiles with symbol only (right)
	for(i=0;i<definedSymbolVector.size();i++)
	{
		vector <int> top,bottom,left,right;
		top.push_back(definedSymbolVector.at(i)); // symbol at the top
		bottom.push_back(-3); // blank at the bottom
		// -2 = right facing, the 1=init tile
		left.push_back(-2);
		right.push_back(-2);
		constructTile(tileVector, top, left, right, bottom, 1);
		switch(definedSymbolVector.at(i))
		{
		case 0:
			rightFacingZero=tileVector.at(tileVector.size()-1);
			break;
		case 1:
			rightFacingOne=tileVector.at(tileVector.size()-1);
			break;
		case -4:
			rightFacingBlank=tileVector.at(tileVector.size()-1);
			break;
		default: break;
		}
	}
	// init config tiles with state and symbol
	for(i=0;i<definedSymbolVector.size();i++)
	{
		vector <int> top,bottom,left,right;
		top.push_back(definedSymbolVector.at(i)); // symbol at the top
		top.push_back(initialState);
		bottom.push_back(-3); // blank at the bottom
		left.push_back(-1);
		right.push_back(-2);
		constructTile(tileVector, top, left, right, bottom, 1); // middle tile
													 // the 1=init tile
		switch(definedSymbolVector.at(i))
		{
		case 0:
			middleZero=tileVector.at(tileVector.size()-1);
			break;
		case 1:
			middleOne=tileVector.at(tileVector.size()-1);
			break;
		case -4:
			middleBlank=tileVector.at(tileVector.size()-1);
			break;
		default: break;
		}
	}
	cout << "Constructed " << i*3 << " initial configuration tiles" << endl;
} // end construct init config tiles

// constructs the alphabet tiles (takes a symbol and retransmits that symbol)
void constructAlphabetTiles(vector <Tile>& tileVector,
		vector <int>& definedSymbolVector)
{
	unsigned int i;
	for(i=0;i<definedSymbolVector.size();i++)
	{
		vector <int> top,bottom,left,right;
		top.push_back(definedSymbolVector.at(i)); // symbol at the top
		bottom.push_back(definedSymbolVector.at(i)); // same at the bottom
		left.push_back(-3);
		right.push_back(-3);
		constructTile(tileVector, top, left, right, bottom, 0); // blank sides
													// NOT an init config tile
	}
	cout << "Constructed " << i << " alphabet tiles" << endl;
} // end construct alphabet tiles

// for each left and right action tile
// a number of merge tiles must be created
// (1 for each defined symbol
void constructMergeTile(vector <Tile>& tileVector,
		vector <int>& definedSymbolVector, int action, int state)
{
	unsigned int i;
	for(i=0;i<definedSymbolVector.size();i++)
	{
		vector <int> top,bottom,left,right;
		top.push_back(definedSymbolVector.at(i)); // symbol at the top
		top.push_back(state); // the state at the top
		bottom.push_back(definedSymbolVector.at(i)); // symbol at bottom
		// if action tile moves left, this tile is to the left of it
		if(action==-1)
		{
			right.push_back(state);
			right.push_back(-1);
			left.push_back(-3);
		}
		// if action tile moves right, this tile is to the right of it
		else if(action==-2)
		{
			left.push_back(state);
			left.push_back(1);
			right.push_back(-3);
		}

		constructTile(tileVector, top, left, right, bottom, 0);
		mergeTileCount++;
	} // end of for loop
} // end of merge tile construction

// constructs action tiles from the rules in the rule set
// (this function needs to read from the input file)
void constructActionTiles(ifstream& reader,
		vector <Tile>& tileVector, vector <int>& definedSymbolVector)
{
	// rules are read in the following format
	// Current State, Current Symbol, Next State, Action
	// Need to check that 'Current Symbol' is in the list of defined symbols
	// Action is either a symbol (which also must be a defined symbol)
	// or it is an instruction to move left (-1) or right (-2)
	unsigned int i=0;
	int symbol;
	int nextstate;
	int action;
	while ( !reader.eof() )
	{
		string temp;
		stringstream tempstream;
		int tempint;

		if((i%4)==0) // add a new tile for each line
		{
			Tile newTile;
			tileVector.push_back(newTile);
		}
		if((i%4)<3) // for the first 3 terms
		{
			getline(reader,temp,'\t'); // read up to the tab
			i++;
		}
		else
		{
			getline(reader,temp,'\n'); // read up to the end of the line
			i++;
		}
		tempstream << temp;
		tempstream >> tempint;
		switch((i-1)%4)
		{
		case 0:
			tileVector.at(tileVector.size()-1).bottomLabelVector.push_back(tempint);
			break;
		case 1:
			// ******** need to check that the symbol is a defined symbol
			int swap;
			symbol=tempint;
			swap=tileVector.at(tileVector.size()-1).bottomLabelVector.at(0);
			tileVector.at(tileVector.size()-1).bottomLabelVector.push_back(swap);
			tileVector.at(tileVector.size()-1).bottomLabelVector.at(0)=symbol;
			break;
		case 2:
			nextstate=tempint;
			break;
		case 3:
			action=tempint;
			if(action==-1)
			{
				// move left
				tileVector.at(tileVector.size()-1).topLabelVector.push_back(symbol);
				tileVector.at(tileVector.size()-1).leftLabelVector.push_back(nextstate);
				tileVector.at(tileVector.size()-1).leftLabelVector.push_back(-1);
				tileVector.at(tileVector.size()-1).rightLabelVector.push_back(-3);
				constructMergeTile(tileVector,definedSymbolVector,action,nextstate);
			}
			else if(action==-2)
			{
				// move right
				tileVector.at(tileVector.size()-1).topLabelVector.push_back(symbol);
				tileVector.at(tileVector.size()-1).rightLabelVector.push_back(nextstate);
				tileVector.at(tileVector.size()-1).rightLabelVector.push_back(1);
				tileVector.at(tileVector.size()-1).leftLabelVector.push_back(-3);
				constructMergeTile(tileVector,definedSymbolVector,action,nextstate);
			}
			else
			{
				// write new symbol
				tileVector.at(tileVector.size()-1).topLabelVector.push_back(action);
				tileVector.at(tileVector.size()-1).topLabelVector.push_back(nextstate);
				tileVector.at(tileVector.size()-1).leftLabelVector.push_back(-3);
				tileVector.at(tileVector.size()-1).rightLabelVector.push_back(-3);
			}
			break;
		}
		tileVector.at(tileVector.size()-1).initialized=1;
	} // end of file read
	cout << "Constructed " << i/4 << " action tiles" << endl;
	cout << "Constructed " << mergeTileCount << " merge tiles" << endl;
} // end construct action tiles

// constructs a completely blank tile
void constructBlankTile(vector <Tile>& tileVector)
{
	vector <int> top,bottom,left,right;
	top.push_back(-3);
	bottom.push_back(-3);
	left.push_back(-3);
	right.push_back(-3);
	constructTile(tileVector, top, left, right, bottom, 0); // a blank tile
	cout << "Constructed 1 blank tile" << endl;
}

// prints the tile set to the screen in a human readable way
void TileCompiler::printTileSet(vector <Tile>& tileVector, int toPrint)
{
	if(toPrint)
	{
		cout << endl;
		unsigned int i,j;
		for(i=0;i<tileVector.size();i++)
		{
			cout << "    ";
			for(j=0;j<tileVector.at(i).topLabelVector.size();j++)
			{
				if(tileVector.at(i).topLabelVector.at(j)==-3)
					cout << "B ";
				else
				{
				if(j==0)
					cout << "s";
				else
					cout << "q";
				cout << tileVector.at(i).topLabelVector.at(j) << " ";
				}
			}
			cout << endl << endl;
			for(j=0;j<tileVector.at(i).leftLabelVector.size();j++)
			{
				if(tileVector.at(i).leftLabelVector.at(j)==-3)
					cout << " B";
				else if(tileVector.at(i).leftLabelVector.at(j)==-1)
					cout << " L";
				else if(tileVector.at(i).leftLabelVector.at(j)==-2)
					cout << " R";
				else
					cout << "q" << tileVector.at(i).leftLabelVector.at(j);
				cout << " ";
			}
			cout << "      ";
			for(j=0;j<tileVector.at(i).rightLabelVector.size();j++)
			{
				if(tileVector.at(i).rightLabelVector.at(j)==-3)
					cout << " B";
				else if(tileVector.at(i).rightLabelVector.at(j)==-1)
					cout << " L";
				else if(tileVector.at(i).rightLabelVector.at(j)==-2)
					cout << " R";
				else
					cout << "q" << tileVector.at(i).rightLabelVector.at(j);
				cout << " ";
			}
			cout << endl << endl << "    ";
			for(j=0;j<tileVector.at(i).bottomLabelVector.size();j++)
			{
				if(tileVector.at(i).bottomLabelVector.at(j)==-3)
					cout << "B ";
				else
				{
				if(j==0)
					cout << "s";
				else
					cout << "q";
				cout << tileVector.at(i).bottomLabelVector.at(j) << " ";
				}
			}
			cout << endl << endl << endl;
		} // end of loop through tiles
	} // if we are printing
} // end printTileSet

// removes any duplicates from the tile vector
void removeDuplicates(vector <Tile>& tileVector)
{
	int duplicateCount=0;
	vector <int> duplicateVector (tileVector.size(),0);
	unsigned int i,j;
	for(i=0;i<tileVector.size();i++)
	{
		for(j=i;j<tileVector.size();j++)
		{
			if(tileVector.at(i)==tileVector.at(j) && i!=j)
			{
				duplicateCount++;
				duplicateVector.at(i)=1;
			} // if there is a match
		} // nestled loop
	} // loop
	for(i=0;i<tileVector.size();i++)
	{
		if(duplicateVector.at(i))
		{
			// puts the back element where the duplicate is
			// then removes the back element
			tileVector.at(i)=tileVector.back();
			tileVector.pop_back();
			duplicateVector.at(i)=duplicateVector.back();
			duplicateVector.pop_back();
		}
	}
	cout << "Removed " << duplicateCount << " duplicate 'merge' tiles" << endl;
}

TileCompiler::TileCompiler(char*filename)
{
	ptr_leftFacingBlank=&leftFacingBlank;
	ptr_rightFacingBlank=&rightFacingBlank;
	ptr_rightFacingOne=&rightFacingOne;
	ptr_rightFacingZero=&rightFacingZero;
	ptr_middleBlank=&middleBlank;
	ptr_middleOne=&middleOne;
	ptr_middleZero=&middleZero;
	tMachineName=filename;

}
TileCompiler::~TileCompiler()
{}

void setAsInitialized(vector <Tile>& tileVector)
{
	unsigned int i;
	for(i=0;i<tileVector.size();i++)
		tileVector.at(i).initialized=1;
}

vector <Tile> TileCompiler::mainFunction()
{
	int initialState; // the initial state of the machine
	unsigned int numberOfSymbols = 3;

	vector <Tile> tileVector; // the tile set
	vector <int> definedSymbolVector; // defined symbols

	// NEED TO CHANGE THIS TO A RELATIVE REFERENCE
	// AND NEED TO MAKE THE FILE NAME DEFINED WHEN PROGRAM IS RUN
	//ifstream reader("/Users/awebb/Documents/University/Third Year Project/Tile Compiler/tm");
	string tmString1 = "TMs/";
	string tmString2="";
	string tmExample="example";
	if(tMachineName!=NULL)
	{
		string tmStringTemp (tMachineName);
		tmString2=tmStringTemp;
	}
	string tmFull;
	if(tmString2.size()>0)
		tmFull = tmString1.append(tmString2);
	else
		tmFull=tmString1.append(tmExample);
	cout << "Turing Machine filename: " << tmFull << endl;
	ifstream reader(tmFull.c_str());
	if( !reader )
	{
		cout << "Error opening input file" << endl;
		exit(0);
	}

	// setting up references of anything used in functions
	ifstream& rReader = reader;
	int& rInitialState = initialState;
	unsigned int& rNumberOfSymbols = numberOfSymbols;
	vector <Tile>& rTileVector = tileVector;
	vector <int>&  rDefinedSymbolVector = definedSymbolVector;
	vector <int>& rHaltVector = haltState;

	// set up initial state and defined symbols
	initialize(rReader, rInitialState, rHaltVector, rNumberOfSymbols, rDefinedSymbolVector);

	 // init config tiles
	constructInitConfigTiles(initialState, rTileVector, rDefinedSymbolVector);
	// alphabet tile construction
	constructAlphabetTiles(rTileVector, rDefinedSymbolVector);
	// write, move and merge tiles
	constructActionTiles(rReader, rTileVector, rDefinedSymbolVector);
	//constructBlankTile(rTileVector); // adds a blank tile

	// sets all tiles as 'initialized'
	setAsInitialized(rTileVector);

	reader.close();

	cout << "Constructed " << tileVector.size() << " tiles" << endl;
	removeDuplicates(rTileVector); // remove any duplicate tiles
	cout << "Tile set size: " << tileVector.size() << endl;

	return tileVector;
}
