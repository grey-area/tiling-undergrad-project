Tiling Undergraduate Final Year Project
---------------------------------------

The code and report from my undergraduate final year project back in 2010.

Dependencies
------------

Requires OpenGL/GLUT.

Compiling
----------
Each folder contains a bash script named 'compile'.  Running them
should compile the programs on the Unix machines.


Penrose Tiler
-------------

For producing [Penrose tilings](https://en.wikipedia.org/wiki/Penrose_tiling). Penrose tiles are a set of two distinct tiles that can tile the plane, but only non-periodically.

File:  PenroseTiler

Flags:
- -A  starts the tiling with a thin rhomb instead of a thick one
- -e  performs the edge-completion algorithm after every deflation

Controls:
- pan using keyboard arrows
- zoom using 'q' and 'w' keys
- perform a deflation step by pressing spacebar
- press 'h' to highlight tile halfs
- press 'r' to reset the tiling
- press 'i' to toggle manual interation on/off.  (This will cause the edge-completion algorithm to run as well)
- press 'b' to run the edge-completion algorithm
- when in manual interactive mode, press 'z' to change tile type and 'a' and 's' to rotate the tile

Robinson Tiler
---------------

Produces tiling with Robinson tiles. The Robinson tiles are a set of six tiles that can tile the plane but only [non-periodically](https://en.wikipedia.org/wiki/Aperiodic_tiling).

File:	RobinsonTiler

Controls:
- press 'c' to toggle colour display on/off
- pan using arrows, zoom using 'q','w'
- press space to do an inflation step (the tiling will inflate in the direction of the current centre tile, the new centre tile will be in the same orientation as the tiles in the top bar)
- pressing 'z' or 'x' rotates the tiles in the top bar
- clicking a tile in the top bar selects it, so you can manually add it to the tiling
- clicking any of the blank area in the top bar clears the selection
- pressing 'r' resets the tiling

Wang Tiler
-----------

Attempts to produce a tiling with a given set of [Wang tiles](https://en.wikipedia.org/wiki/Wang_tile). 

File: WangTiler

Flags:
- -s  display the tile set (instead of actually running the tiling)
- -w <name of tile set>  selects the tile set to be used

The tile set descriptions are in the 'description' folder
By default it uses the 'example' one, which is aperiodic.
The 'random2' tile set makes a nice pattern.
	
Controls:
- pressing the up or down keyboard arrows changes the speed of the tiler.


Turing Machine Simulator
------------------------

This program translates a description of a Turing machine and its input into a set of tiles and an initial configuration of those tiles, such that the act of attempting to tile the plane necessarily 'simulates' the Turing machine. If the Turing machine halts, it follows that the tiles cannot tile the plane starting from the given initial configuration. If the Turing machine enters an infinite loop whereby it does not terminate and periodically revisits the same states, then the tile set (starting from the initial configuration) tiles the plane periodically. If the Turing machine does not halt and never revisits a previous state, then the tile set (starting from the initial configuration) tiles the plane non-periodically.

File:	TMSimulator

Flags:
- -t <Turing machine name>
- -i <Input file name>
- -s display tile set, do not run tiler
- -d {0,1,2} sets the starting speed of the tiler
- -p starts the tiler with periodicity checking turned on

The Turing machine description files are in the TM folder.
The interesting ones are 'complementizer' and 'palindrome_checker'.
There's also an 'infloop' TM, which loops infinitely when given a
string of '1's

The input files are in the input folder.
(Not all inputs will work for all Turing machines,
since each TM has a set of allowed symbols)
Input files palindrome1 palindrome2 etc will work with the
palindrome_checker TM
Input files complementizer1 complementizer2 will work with the
complementizer TM
Use the 'infloop' input file for the 'infloop' TM

Controls:
- pressing 'p' turns on periodicity checking
- pressing up or down arrow keys changes the speed of the tiler
