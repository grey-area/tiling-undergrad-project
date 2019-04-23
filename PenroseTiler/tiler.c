/*
========================================================================

File:        tiler.c
Description: Tiler program drawing skeleton
Authors:     Andrew Webb
Version:     v0.1 15/10/09
Usage:       Menu attached to right-button.

========================================================================

*/
#include <sys/time.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


// triangle array size - should be dynamic later

int windowHeight = 700;
int windowWidth = 700;

typedef struct {long double x; /* Handy type for a vertex */
                long double y;
               } vertex;

typedef struct { vertex pos1; /* 3 vertices and which of the four half rhombs it is */
                vertex pos2;
                vertex pos3;
                char type; /*A for thin rhombs, B for thick*/
                short int half; /*1 is 'prime' half */
                short int finished; /* rhombs that need completing */
              } triangle;

typedef struct { long double angle; // in radians.. 2pi = 360
				 long double totalAngle;
				 triangle one;
				 triangle two;
			  } mouseRhomb;

mouseRhomb mouse;

int arraySize = 100;
// will point to start of triangle array
triangle *triangles;

// error margin for checking if two points are the same
// when doing edge filling
double epsilon=0.1;
// error margin for placing tiles
double epsilon2=0.35;

// 1 if we want tiler to complete unfinished rhombs on the edges
int fillInEdges=0;
int fillInOnce=0;
// when set to 1 user can place tiles
int interaction=1;

int trianglecount; /* Current number of triangles defined */
int newtrianglecount;

// details of first tile
int startType='B';
int startX;
int startY;
long double stepSize; /* size of first rhomb */

// the position of the mouse
int mouseX=0;
int mouseY=0;

// used to determine the ratio of fat:thin rhombs
// will tend to phi as the tiling gets bigger
int numberOfAHalfRhombs=1;
int numberOfBHalfRhombs=1;

int horizontalPanning=0; // are we panning h?
int verticalPanning=0; // are we panning v?
double horizontalPan=0; // how far panned h?
double verticalPan=0; // how far panned v?
float moveAmount=0.0008; // how fast to pan

int zooming=0; // are we zooming?
double zoomLevel=0; // how far have we zoomed?
float zoomAmount=0.0003; // how fast to zoom

// whether or not we should show the different halfs
// (A and A' will display differently)
int displayHalves=0;

long double phi;

// when set to 1, nothing will happen
int done=1; // user will interact to change to 0


/* displays the tile being dragged around by the user */
void displayMouseTile()
{
	if(mouse.one.type=='B')
	{
		if((!displayHalves)||mouse.one.half==0)
			glColor4f(1.0,1.0,0.0,0.0);
		else
			glColor4f(0.75,0.75,0.0,0.0);
	}
	else
	{
		if((!displayHalves)||mouse.one.half==0)
			glColor4f(0.9,0.0,0.0,0.0);
		else
			glColor4f(0.65,0.0,0.0,0.0);
	}
	glBegin(GL_TRIANGLES);
		glVertex3f(mouse.one.pos1.x,mouse.one.pos1.y,0.0);
		glVertex3f(mouse.one.pos2.x,mouse.one.pos2.y,0.0);
		glVertex3f(mouse.one.pos3.x,mouse.one.pos3.y,0.0);
	glEnd();

	if(mouse.two.type=='B')
	{
		if((!displayHalves)||mouse.two.half==0)
			glColor4f(1.0,1.0,0.0,0.0);
		else
			glColor4f(0.7,0.7,0.0,0.0);
	}
	else
	{
		if((!displayHalves)||mouse.two.half==0)
			glColor4f(0.9,0.0,0.0,0.0);
		else
			glColor4f(0.6,0.0,0.0,0.0);
	}
	glBegin(GL_TRIANGLES);
		glVertex3f(mouse.two.pos1.x,mouse.two.pos1.y,0.0);
		glVertex3f(mouse.two.pos2.x,mouse.two.pos2.y,0.0);
		glVertex3f(mouse.two.pos3.x,mouse.two.pos3.y,0.0);
	glEnd();

	glColor4f(0.0,0.0,0.0,0.0);
	glBegin(GL_LINE_LOOP);
		glVertex3f(mouse.one.pos1.x,mouse.one.pos1.y,0.0);
		glVertex3f(mouse.one.pos2.x,mouse.one.pos2.y,0.0);
		glVertex3f(mouse.one.pos3.x,mouse.one.pos3.y,0.0);
		glVertex3f(mouse.two.pos2.x,mouse.two.pos2.y,0.0);
	glEnd();
} // displayMouseTile


/******************************/

/* displays everything... */
void display(void)
{ /* Called every time OpenGL needs to update the display */

  /* Draw all the shapes */

   glClearColor (1.0,1.0,1.0,1.0);
   glClear(GL_COLOR_BUFFER_BIT);
   
   glPushMatrix(); // vvv takes pan into account
   glTranslatef(horizontalPan,verticalPan,0.0);

   int i;
   /* draw all triangles */
   for(i = trianglecount-1; i >= 0; i--)
   {
     if(triangles[i].type=='A')
     {
       if((!displayHalves)||(!triangles[i].half))
    	   glColor4f(0.9,0.0,0.0,0.0);
       else
    	   glColor4f(0.65,0.0,0.0,0.0);
     }
     else
     {
    	 if((!displayHalves)||(!triangles[i].half))
    		 glColor4f(1.0,1.0,0.0,0.0);
    	 else
    		 glColor4f(0.75,0.75,0.0,0.0);
     }
     glBegin(GL_TRIANGLES);
         glVertex3f(triangles[i].pos1.x, triangles[i].pos1.y, 0.0);
         glVertex3f(triangles[i].pos2.x, triangles[i].pos2.y, 0.0);
         glVertex3f(triangles[i].pos3.x, triangles[i].pos3.y, 0.0);
     glEnd();
   }
   glColor4f(0.0,0.0,0.0,0.0);
   for(i = 0; i <= trianglecount-1; i++)
   {
     glBegin(GL_LINE_STRIP);
         glVertex3f(triangles[i].pos1.x, triangles[i].pos1.y, 0.0);
         glVertex3f(triangles[i].pos2.x, triangles[i].pos2.y, 0.0);
         glVertex3f(triangles[i].pos3.x, triangles[i].pos3.y, 0.0);
     glEnd();
   }
   glPopMatrix();

   // only display the mouse tile if the user is interacting
   if(interaction)
	   displayMouseTile();

   glutSwapBuffers();
} // display

/******************************/

/* Sets a 2D orthographic projection with lower-left corner
 at (0,0) and upper-right corner at (width, height) */
void reshape(int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// the zoom term in the projection matrix determines zoom level
	glOrtho(0.0 + width*zoomLevel, (GLfloat) width - width*zoomLevel, 0.0 + height*zoomLevel, (GLfloat) height - height*zoomLevel, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
} // reshape

/******************************/


/* calculates the length of a vector between two vertices */
double lengthCalc(vertex first,vertex second)
{
	return sqrt(pow((first.x-second.x),2)+pow((first.y-second.y),2));
} // lengthCalc

/* gives the unit vector between two vertices */
void unitVectors(vertex first, vertex second, double length, double* x, double* y)
{
  *x=(first.x-second.x)/length;
  *y=(first.y-second.y)/length;
} // unitVectors

/******************************/


/* deflates.  This process is deterministic.  Produces new (and larger) tiling */
void deflate()
{
  // makes the error ratios finer as the tiles become smaller
  epsilon = epsilon/phi;
  epsilon2=epsilon2/phi;

  numberOfAHalfRhombs=1;
  numberOfBHalfRhombs=1;
  newtrianglecount=0;

  // pointer to new array of triangles
  triangle *newTri;
  // increase the size of the triangle array as required
  if(arraySize<trianglecount*4)
    arraySize*=16;
  newTri=(triangle*)malloc(arraySize * sizeof (triangle));

  // for all triangles...
  int j;
  for(j=0;j<trianglecount;j++)
  {
    // ... if it's of type A, deflate this way:
    if(triangles[j].type=='A')
    {
      triangle BDiff,ASame;

      // based on deflation rules
      // see diagrams in logbook
      BDiff.pos3=triangles[j].pos2;
      BDiff.pos1=triangles[j].pos1;
      ASame.pos2=BDiff.pos1;
      ASame.pos1=triangles[j].pos3;
      double length=lengthCalc(triangles[j].pos3,triangles[j].pos2);
      double unitVectorX,unitVectorY;
      unitVectors(triangles[j].pos3,triangles[j].pos2,length,&unitVectorX,&unitVectorY);
      BDiff.pos2.x=triangles[j].pos2.x+(unitVectorX*length*(phi-1));
      BDiff.pos2.y=triangles[j].pos2.y+(unitVectorY*length*(phi-1));
      ASame.pos3=BDiff.pos2;

      ASame.type='A';
      BDiff.type='B';

      ASame.half=triangles[j].half;
      BDiff.half=!(triangles[j].half);
      ASame.finished=0;
      BDiff.finished=0;

      newTri[newtrianglecount]=ASame;
      newTri[newtrianglecount+1]=BDiff;

      numberOfBHalfRhombs++;
      newtrianglecount+=2;
    } // end of IF A TYPE
    // ... if it's of type B, deflate this way:
    else
    {
      triangle BDiff,ASame,BSame;

      // based on deflation rules
      // see diagrams in logbook
      BDiff.pos3=triangles[j].pos1;
      double length=lengthCalc(triangles[j].pos2,triangles[j].pos1);
      double unitVectorX,unitVectorY;
      unitVectors(triangles[j].pos2,triangles[j].pos1,length,&unitVectorX,&unitVectorY);
      BDiff.pos2.x=triangles[j].pos1.x+(unitVectorX*length*(phi-1));
      BDiff.pos2.y=triangles[j].pos1.y+(unitVectorY*length*(phi-1));
      ASame.pos3=BDiff.pos2;
      length=lengthCalc(triangles[j].pos3,triangles[j].pos1);
      unitVectors(triangles[j].pos3,triangles[j].pos1,length,&unitVectorX,&unitVectorY);
      BDiff.pos1.x=triangles[j].pos1.x+(unitVectorX*length*(phi-1));
      BDiff.pos1.y=triangles[j].pos1.y+(unitVectorY*length*(phi-1));
      ASame.pos2=BDiff.pos1;
      BSame.pos2=BDiff.pos1;
      ASame.pos1=triangles[j].pos2;
      BSame.pos3=ASame.pos1;
      BSame.pos1=triangles[j].pos3; 

      BDiff.type='B';
      BSame.type='B';
      ASame.type='A';

      BDiff.half=!triangles[j].half;
      BSame.half=triangles[j].half;
      ASame.half=triangles[j].half;

      BDiff.finished=0;
      BSame.finished=0;
      ASame.finished=0;

      newTri[newtrianglecount]=ASame;
      newTri[newtrianglecount+1]=BSame;
      newTri[newtrianglecount+2]=BDiff;

      numberOfAHalfRhombs++;
      numberOfBHalfRhombs++;
      newtrianglecount+=3;
    } // end of IF B TYPE
  } // end of FOR ALL TRIANGLES

  // free the old memory and point to the new
  free(triangles);
  triangles=newTri;

  trianglecount=newtrianglecount;
  // so mouse tile is still the same size as other tiles
  stepSize = stepSize/phi;
} // deflate

/******************************/

/* takes a vertex and rotates it by an angle */
void vertexRotate(vertex *passedVertex, int useTotalAngle)
{
	vertex tempVertex = *(passedVertex);

	long double angle;
	// may be rotating by a small amount or by the sum of all
	// rotations so far
	if(useTotalAngle)
		angle=mouse.totalAngle;
	else
		angle=mouse.angle;

	// the rotation
	passedVertex->x = tempVertex.x*cos(angle) - tempVertex.y*sin(angle);
	passedVertex->y = tempVertex.x*sin(angle) + tempVertex.y*cos(angle);
} // vertexRotate

/******************************/

/* rotates a mouse vertex */
void recalculateMouseVertex(vertex *rotVertex, vertex firstVertex, int useTotalAngle)
{
	// rotate rotVertex about firstVertex
	// first subtract firstVertex from it
	rotVertex->x-=firstVertex.x;
	rotVertex->y-=firstVertex.y;
	// then perform a rotation
	vertexRotate(rotVertex,useTotalAngle);
	// then add firstVertex back on
	rotVertex->x+=firstVertex.x;
	rotVertex->y+=firstVertex.y;
}

/* rotates the mouse tile */
void recalculateMouseAngle(int useTotalAngle)
{
	mouseRhomb tempRhomb = mouse;

	//pos1 shouldn't change because the mouse is placed over it
	//pos2 and 3 do change
	recalculateMouseVertex(&(tempRhomb.one.pos2), mouse.one.pos1, useTotalAngle);
	recalculateMouseVertex(&(tempRhomb.one.pos3), mouse.one.pos1, useTotalAngle);
	recalculateMouseVertex(&(tempRhomb.two.pos2), mouse.two.pos1, useTotalAngle);

	// these are the same in both triangles
	tempRhomb.two.pos1=tempRhomb.one.pos1;
	tempRhomb.two.pos3=tempRhomb.one.pos3;

	mouse=tempRhomb;

	display();
}

/******************************/

/* recalculates the shape of the mouse tile when it's rotated */
void moveMouseTile()
{
	long double d; // temp variable to stand in for stepSize

	// size is dependent on tile type
	if(mouse.one.type=='B')
		d=stepSize;
	else
		d=stepSize/(phi+1);

	mouse.one.pos1.x=mouseX;
	mouse.one.pos1.y=mouseY;
	mouse.one.pos3.x=mouseX+d;
	mouse.one.pos3.y=mouseY;

	long double mouseTwoPosY; // temp variable because it's used twice..

	// if we are looking at a B rhomb then calculate the position of the
	// 2nd vertex in the following way
	if(mouse.one.type=='B')
	{
		mouseTwoPosY=(((d/2)/cos(M_PI/5))*sin(M_PI/5));
	}
	// same for A rhomb
	else
	{
		mouseTwoPosY=-(d/2)*tan(3*M_PI/5);
	}
	mouse.one.pos2.x=mouseX+d/2;
	mouse.one.pos2.y=mouseY+mouseTwoPosY;

	mouse.two=mouse.one;
	mouse.two.half=!(mouse.one.half);
	mouse.two.pos2.y=mouseY-mouseTwoPosY;
} // moveMouseTile

/******************************/

void init(int,char *[]);

/* when keys are pressed */
void keyboard(unsigned char key, int x, int y)
{
	double mouseAngleDelta=M_PI/5;
	switch (key) {
    case 32: // space
          done=0;
          break;
    case 'h':
    	// toggles the display of half tiles
    	displayHalves=!displayHalves;
    	display();
    	break;
    case 'z':
    	// changes mouse tile type
    	if(interaction)
    	{
    		if(mouse.one.type=='A')
    		{
    			mouse.one.type='B';
    			mouse.two.type='B';
    		}
    		else
    		{
    			mouse.one.type='A';
    			mouse.two.type='A';
    		}
    		moveMouseTile();
    		recalculateMouseAngle(1);
    	}
    	break;
    case 'b':
    	// fills in the edges if they weren't already filled
    	fillInOnce=1;
    	break;
    case 'i':
    	// fills in the edges if they weren't already filled
    	// then turns on user interaction
    	fillInOnce=1;
    	interaction=!interaction;
    	break;
    case 'w':
    	// zoom in
    	zooming=1;
    	break;
    case 'q':
    	// zoom out
    	zooming=-1;
    	break;
    case 'a':
    	// rotate the mouse tile anti-clockwise
    	mouse.angle=mouseAngleDelta;
    	mouse.totalAngle+=mouse.angle;
    	if(mouse.totalAngle>2*M_PI)
    		mouse.totalAngle-=2*M_PI;
    	recalculateMouseAngle(0);
    	mouse.angle=0;
    	break;
    case 's':
    	// rotates the tile clockwise
    	mouse.angle=-mouseAngleDelta;
    	mouse.totalAngle+=mouse.angle;
    	if(mouse.totalAngle<0)
    		mouse.totalAngle+=2*M_PI;
    	recalculateMouseAngle(0);
    	mouse.angle=0;
    	break;
    case 'r':
    	// reset the program to initial state
    	init(0,NULL);
    	display();
    	break;

   }
} // keyboard

/* when keys are released */
void keyboardUp(unsigned char key, int x, int y)
{
	switch (key) {
	case 'w':
		// stop zooming
		zooming=0;
		break;
	case 'q':
		// stop zooming
		zooming=0;
		break;

   }
} // keyboardUp

/* special function keys are pressed */
void special(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_RIGHT:
		// pan right
		horizontalPanning=-1;
		break;
	case GLUT_KEY_LEFT:
		// pan left
		horizontalPanning=1;
		break;
	case GLUT_KEY_DOWN:
		// pan down
		verticalPanning=1;
		break;
	case GLUT_KEY_UP:
		// pan up
		verticalPanning=-1;
		break;
	} // end switch
} // special

/* special function keys are released */
void specialUp(int key, int x, int y)
{
	// in all cases, stop panning
	switch (key) {
	case GLUT_KEY_LEFT:
		horizontalPanning=0;
		break;
	case GLUT_KEY_RIGHT:
		horizontalPanning=0;
		break;
	case GLUT_KEY_UP:
		verticalPanning=0;
		break;
	case GLUT_KEY_DOWN:
		verticalPanning=0;
		break;
	} // end switch
} // specialUp

/******************************/

/* initialize the program */
void init(int argc, char* argv[])
{
	// sets/resets navigation variables
	horizontalPan=0;
	verticalPan=0;
	zoomLevel=0;

	// set the error ranges
	epsilon=0.1;
	epsilon2=0.35;

	// the golden ratio
	phi=(1+sqrt(5))/2;
  
	glLineWidth(1.2);
  
	int i;
	for(i=1;i<argc;i++) /* loop through all command line options */
  	{
  		if(argv[i][0]=='-') /* if the 1st char is a - */
  		{
  			switch (argv[i][1])
  			{
  			case 'e':
  				// command line option 'e', forces edge filling at every step
  				fillInEdges=1;
  				break;
  			case 'h':
  				// option 'h', starts the program will half tile display on
  				displayHalves=1;
  				break;
  			case 'A':
  				// option 'A', starts program with an A tile rather than B
  				startType='A';
  				break;
  			default: break;
  			} // end switch
  		} // end if
  	} // end loop through command line options

	// initial step size
	stepSize=windowWidth/1.25;

	// initial position
	startX = windowWidth/8.0;
	startY = windowHeight/2.0;
  
	// allocate memory to triangle array
	triangles = malloc(arraySize * sizeof (triangle));
	
	// determine the shape of the first tile
	long double d; // stands in for stepSize
	long double pos2Y; // the position of the 2nd vertex
	if(startType=='B')
	{
		d=stepSize;
		pos2Y=(((d/2)/cos(M_PI/5))*sin(M_PI/5)); // refer to logbook
		numberOfBHalfRhombs+=2;
	}
	else
	{
		d=stepSize/(phi+1);
		pos2Y=-(d/2)*tan(3*M_PI/5); // refer to logbook
		numberOfAHalfRhombs+=2;
	}

	trianglecount = 2;
	// the position of the first half of the first tile
 	triangles[0].pos1.x=startX;
 	triangles[0].pos1.y=startY;
 	triangles[0].pos2.x=startX+d/2;
 	triangles[0].pos2.y=startY+pos2Y;
 	triangles[0].pos3.x=startX+d;
 	triangles[0].pos3.y=startY;
 	// A/B/A'/B'?
 	triangles[0].type=startType;
 	triangles[0].half=0;
 	triangles[0].finished=1; // both halves of this tile are present

 	// the position of the second half of the first tile
 	triangles[1].pos1.x=startX;
	triangles[1].pos1.y=startY;
	triangles[1].pos2.x=startX+d/2;
	triangles[1].pos2.y=startY-pos2Y;
	triangles[1].pos3.x=startX+d;
	triangles[1].pos3.y=startY;
	triangles[1].type=startType;
	triangles[1].half=1;
	triangles[1].finished=1;

	// if the user is interacting with the program
	if(interaction)
	{
		// set up the mouse tile
		mouse.angle=0; // start angle=horizontal
		mouse.one.finished=0;
		mouse.one.half=0;
		mouse.one.type='B'; // mouse tile starts as type B

		// the position and size of the mouse tile
		mouse.one.pos1.x=0;
		mouse.one.pos1.y=0;
		mouse.one.pos3.x=stepSize;
		mouse.one.pos3.y=0;
		mouse.one.pos2.x=stepSize/2;
		mouse.one.pos2.y=((stepSize/2)/cos(M_PI/5))*sin(M_PI/5);

		mouse.two=mouse.one;
		mouse.two.half=1;
		mouse.two.pos2.y=-mouse.one.pos2.y;
	}
	// by default, interaction is off at the start
	interaction=0;
	reshape(windowWidth,windowHeight);
	display();
} // initialize

/******************************/

// checks if two vertices are the 'same' for a given error margin
// the two float arrays are so we can return the amount we need to snap by (if necessary)
int vertexCheck(vertex one, vertex two, double error, double *diffX, double *diffY)
{
	int check1 = (fabs((one.x-two.x)/one.x) <= error) || (one.x==0&&two.x==0);
	int check2 = (fabs((one.y-two.y)/one.y) <= error) || (one.y==0&&two.y==0);
	if(check1&&check2&&diffX!=NULL&&diffY!=NULL)
	{
		*diffX=two.x-one.x;
		*diffY=two.y-one.y;
	}

	return check1&&check2;
}

// check if two triangles are touching on their 1st and 3rd vertex
// if they are, then they form a rhomb together
int touching(triangle one, triangle two)
{
	// check that they match on the first and 3rd vertices
	int firstCheck=vertexCheck(one.pos1,two.pos1,epsilon,NULL,NULL);
	int secondCheck=vertexCheck(one.pos3,two.pos3,epsilon,NULL,NULL);

	return firstCheck&&secondCheck;
}

void fillInBlanks()
{
	// a new array is first created and then copied to the old one
	// because in order to keep the time complexity low
	// the array needs to be reordered on creation
	// (to keep tiles geographically close close in the array)
	triangle *newTri=(triangle*)malloc(arraySize*sizeof(triangle));
	// the difference so far in a triangles position in each array
	int difference=0;

	int i,j;
	// loop through the array of established triangles
	// looking for any that need 'completing'
	for(i=0; i<trianglecount; i++)
	{
		// if triangle isn't already finished..
		if(!(triangles[i].finished))
		{
			// try to find one that 'completes' it
			for(j=i+1; j<trianglecount; j++)
			{
				// if the two triangles are touching on their 1st and 3rd position
				if(touching(triangles[i],triangles[j]))
				{
					// set both triangles as finished
					triangles[i].finished=1;
					triangles[j].finished=1;
				}
			} // end of INNER FOR LOOP
		} // end of IF TRIANGLE ISN'T FINISHED ALREADY
	} // end of outer for loop

	int loopstop=trianglecount;
	int numberOfUnfinishedRhombs=0;

	//loop through the triangles again, this time adding any that need finishing
	//after the triangle they finish
	for(i=0; i<loopstop; i++)
	{
		newTri[i+difference]=triangles[i];
		if(!(triangles[i].finished))
		{
			numberOfUnfinishedRhombs++;
			triangle tempTriangle = triangles[i];
			tempTriangle.half=!(tempTriangle.half);
			vertex tempVertex;
			tempVertex.x=tempTriangle.pos1.x+tempTriangle.pos3.x-tempTriangle.pos2.x;
			tempVertex.y=tempTriangle.pos1.y+tempTriangle.pos3.y-tempTriangle.pos2.y;
			tempTriangle.pos2=tempVertex;

			difference++;
			newTri[i+difference]=tempTriangle;
			trianglecount++;
		}
	}
	free(triangles);
	triangles=newTri;

	if(numberOfUnfinishedRhombs>0)
		printf("Number of unfinished rhombs = %d\n",numberOfUnfinishedRhombs);
}

/* the idle function of the GLUT loop */
void tile(void)
{
	float panModifier;
	// the more we zoom in the slower we should pan
	if(zoomLevel>=0)
		panModifier=pow(zoomLevel+1,8);
	else
		panModifier=1;

	// panning. The trianglecount term is to speed up panning when we have a lot
	// of triangles.  (because more time is taken up by the display function)
	float totalPanModifier = moveAmount * pow(trianglecount,0.2) / panModifier;
	horizontalPan += horizontalPanning * windowWidth * totalPanModifier;
	verticalPan += verticalPanning * windowHeight * totalPanModifier;

	// zooming.  When zoomed a certain amount, stop it
	if(zoomLevel<0.4999999)
		zoomLevel += zooming * zoomAmount * pow(trianglecount,0.2);
	else
		zoomLevel=0.4999998;

	// if we're zooming, call the reshape function so the projection matrix
	// is updated
	if(zooming||zooming==-1)
	{
		reshape(windowWidth,windowHeight);
		display();
	}

	if(verticalPanning||verticalPanning==-1||horizontalPanning||horizontalPanning==-1)
		display();

	// if the program is to do an iteration
	if(!done)
	{
		// time the iteration was started
		struct timeval startTime;
	 	gettimeofday(&startTime,NULL);

	 	// do the deflation step
	 	deflate();
	 	// if there are edges to be filled, fill them
	 	if(fillInEdges||interaction)
	 	{
	 		fillInBlanks();
	 	}

	 	// recalculates the position and angle of the mouse tile
	 	moveMouseTile();

	 	display();

	 	// time the iteration ended
	 	struct timeval endTime;
	 	gettimeofday(&endTime,NULL);
	 	// iteration time in millis
	 	long int timeDiff = (endTime.tv_sec - startTime.tv_sec)*1000 + (endTime.tv_usec - startTime.tv_usec) / 1000;

	 	printf("Triangles: %d\n",trianglecount);
	 	printf("Array size: %d\n",arraySize);
	 	printf("Ratio of fat to thin rhombs: %f\n", (float)numberOfBHalfRhombs/(float)numberOfAHalfRhombs);
	 	printf("Time taken to complete iteration: %ld mS\n\n",timeDiff);

	 	done=1; // so the next iteration wont happen until the user causes it
	}

	// if we are to fill in the blanks only once (because the user caused it)
	if(fillInOnce)
	{
		// fill in the blanks, set fillInOnce to false
		fillInBlanks();
		fillInOnce=0;
		display();
	}
}

/* takes panning into account when snapping the mouse tile */
void allowForPanning(triangle *checkTri, int modify)
{
	checkTri->pos1.x+=horizontalPan*modify;
	checkTri->pos2.x+=horizontalPan*modify;
	checkTri->pos3.x+=horizontalPan*modify;

	checkTri->pos1.y+=verticalPan*modify;
	checkTri->pos2.y+=verticalPan*modify;
	checkTri->pos3.y+=verticalPan*modify;
}

// checks if the mouse tile is close to an already established tile
// if it is, then snap it to that location and print it
// if it isn't, then don't print it
int edgeSnap(triangle checkTri, double *changeX, double *changeY)
{
	// moves the triangle by pan amount
	allowForPanning(&checkTri,-1);
	// to check if there is an edge we can match with
	int check=0;

	int i;
	// loop through array trying to find a tile that the mouse tile
	// can attach to
	for(i=0; i<trianglecount; i++)
	{
		if(checkTri.type==triangles[i].type)
		{
			if(checkTri.half==!(triangles[i].half))
			{
				int check1=vertexCheck(checkTri.pos1,triangles[i].pos1,epsilon2,changeX,changeY);
				int check2=vertexCheck(checkTri.pos2,triangles[i].pos2,epsilon2,changeX,changeY);
				int check3=vertexCheck(checkTri.pos3,triangles[i].pos3,epsilon2,changeX,changeY);

				check=check2&&(check1||check3);
			}
		} // end if same tile type
		else
		{
			// if looking at same 'half'
			if(checkTri.half==triangles[i].half)
			{
				int check2=vertexCheck(checkTri.pos2,triangles[i].pos2,epsilon2,changeX,changeY);

				int check3=0;
				if(checkTri.type=='A')
					check3=vertexCheck(checkTri.pos1,triangles[i].pos3,epsilon2,changeX,changeY);
				else
					check3=vertexCheck(checkTri.pos3,triangles[i].pos1,epsilon2,changeX,changeY);

				check=check2&&check3;
			}
			else
			{
				// looking at the opposite half
				int check1=0;
				int check2=0;

				if(checkTri.type=='A')
				{
					check1=vertexCheck(checkTri.pos2,triangles[i].pos1,epsilon2,changeX,changeY);
					check2=vertexCheck(checkTri.pos3,triangles[i].pos2,epsilon2,changeX,changeY);
				}
				else
				{
					check1=vertexCheck(checkTri.pos1,triangles[i].pos2,epsilon2,changeX,changeY);
					check2=vertexCheck(checkTri.pos2,triangles[i].pos3,epsilon2,changeX,changeY);
				}

				check=check1&&check2;
			}

		} // end if different tile type
		if(check)
			i=trianglecount;
	}
	return check;
}

/* move triangle
 * takes a triangle and moves all of it's vertices by a given
 * x and y amount */
void moveTriangle(triangle *tri, double changeX, double changeY)
{
	tri->pos1.x+=changeX;
	tri->pos2.x+=changeX;
	tri->pos3.x+=changeX;

	tri->pos1.y+=changeY;
	tri->pos2.y+=changeY;
	tri->pos3.y+=changeY;
}

/* checks if the triangle already exists */
/* returns 1 if there is a duplicate */
int checkForDuplicates(triangle tri)
{
	int i;
	for(i=0; i<trianglecount; i++)
	{
		int check1 = vertexCheck(tri.pos1, triangles[i].pos1, epsilon, NULL, NULL);
		int check2 = vertexCheck(tri.pos2, triangles[i].pos2, epsilon, NULL, NULL);
		int check3 = vertexCheck(tri.pos3, triangles[i].pos3, epsilon, NULL, NULL);
		if(check1&&check2&&check3)
			return 1;
	}
	return 0;
}

/* when the mouse button is clicked */
void mouseFunction(int button, int state, int x, int y)
{
	if(interaction&&state==GLUT_DOWN&&button==GLUT_LEFT_BUTTON)
	{
		// these two will tell us how far to snap the mouse tile
		double changeX=0;
		double changeY=0;
		// check if the first half of the mouse tile will edge snap
		int edgeSnapCheck = edgeSnap(mouse.one,&changeX,&changeY);
		// if that fails, check the 2nd
		if(!edgeSnapCheck)
			edgeSnapCheck = edgeSnap(mouse.two,&changeX,&changeY);

		// if either of them can snap into place
		if(edgeSnapCheck)
		{
			// move the triangle by the required amount
			moveTriangle(&(mouse.one),changeX,changeY);
			moveTriangle(&(mouse.two),changeX,changeY);

			// take panning into account
			allowForPanning(&(mouse.one),-1);
			allowForPanning(&(mouse.two),-1);

			if(arraySize<trianglecount*4)
			{
			  arraySize*=16;
			  triangle *newTri=(triangle*)malloc(arraySize * sizeof (triangle));
			  int i;
			  for(i=0; i<trianglecount; i++)
				  newTri[i]=triangles[i];
			  free(triangles);
			  triangles=newTri;
			}

			// is 1 if there is a duplicate
			int duplicateCheck = checkForDuplicates(mouse.one);

			if(!duplicateCheck) // if it's not a duplicate
			{
				// add the mouse tile to the array
				triangles[trianglecount]=mouse.one;
				triangles[trianglecount+1]=mouse.two;
				printf("Added tile\n");
				trianglecount+=2;
			}

			// reverse the panning effect
			// so the mouse tile doesn't jerk
			allowForPanning(&(mouse.one),1);
			allowForPanning(&(mouse.two),1);
			moveTriangle(&(mouse.one),-changeX,-changeY);
			moveTriangle(&(mouse.two),-changeX,-changeY);
			display();
		} // end if edge snap check
	} // end if interaction and left button click down
}

/* mouse_motion */
void mouse_motion(int x, int y)
{

  // calculates the position of the mouse tile
  // the zoomLevel term accounts for the position of the camera
  mouseX=(x-windowWidth/2) * (1-zoomLevel*2) + windowWidth/2;
  mouseY=(windowHeight/2-y) * (1-zoomLevel*2) + windowHeight/2;

  moveMouseTile();
  recalculateMouseAngle(1);

} // mouse_motion()


/******************************/

int main(int argc, char* argv[])
{
   glutInit(&argc, argv);                /* Initialise OpenGL */
   glutInitDisplayMode (GLUT_DOUBLE);    /* Set the display mode */
   glutInitWindowSize (windowWidth,windowHeight);         /* Set the window size */
   glutInitWindowPosition (100, 100);    /* Set the window position */
   glutCreateWindow ("Penrose Tiler");  /* Create the window */
   init (argc, argv);                              /* Do any other initialisation */
   glutDisplayFunc(display);             /* Register the "display" function */
   glutReshapeFunc(reshape);             /* Register the "reshape" function */
   glutKeyboardFunc (keyboard);
   glutKeyboardUpFunc (keyboardUp);
   glutSpecialFunc (special);
   glutSpecialUpFunc (specialUp);
   glutMouseFunc(mouseFunction);
   glutPassiveMotionFunc (mouse_motion);
   glutIdleFunc(tile);
   // need to introduce a main loop function (plus keyboard and mouse functions
   glutMainLoop(); /* Enter the main OpenGL loop */
   free(triangles);

   // free the allocated memory
   triangles=NULL;
   return 0;
}


