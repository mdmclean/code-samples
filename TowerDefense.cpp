/*
CS 349 Assignment 1: Tower Defense Game

Earth Defender

    towerdefense.cpp  

- - - - - - - - - - - - - - - - - - - - - -

Commands to compile and run:

    g++ -o towerdefense towerdefense.cpp -L/usr/X11R6/lib -lX11 -lstdc++
    ./towerdefense

Note: the -L option and -lstdc++ may not be needed on some machines.

*/
#include <iostream>
#include <list>
#include <cstdlib>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>

/*
 * Header files for X functions
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;
 
const int Border = 10;
const int BufferSize = 10;
const int FPS = 24;

class EnemyGenerator; // forward declaration

/*
 * Information to draw on the window.
 */
struct XInfo {
	Display	 *display;
	int		 screen;
	Window	 window;
	GC		 gc[6];
	Pixmap pixmap; 			// for double buffering
	int		width;		// size of window
	int		height;
	int pX;				// x position in frame for pixmap
	int pY;				// y position in frame for pixmap
	int pW;				// width of pixmap
	int pH;				// height of pixmap
};

/*
 * Function to put out a message on error exits.
 */
void error( string str ) {
  cerr << str << endl;
  exit(0);
}

/*
 * Function to change an integer into a string for printing on screen
 */ 
string numToString (int num)
{
	stringstream convert;

	convert << num;

	return convert.str();
}

/*
 * An abstract class representing displayable things. 
 */
class Displayable {
	public:
		virtual void paint(XInfo &xinfo) = 0;
};       

/*
 * Class used to raw text on screen. 
 */
class Text : public Displayable {
	public:
	virtual void paint (XInfo &xinfo) {
		if (displayTime != 0)
		{
			if (color == 1)	
			{
				XDrawImageString (xinfo.display, xinfo.pixmap, xinfo.gc[0], this->x, this->y, this-> s.c_str(), this->s.length());
			}
			else
			{
				XDrawImageString (xinfo.display, xinfo.pixmap, xinfo.gc[1], this->x, this->y, this-> s.c_str(), this->s.length());
			}
		}
		if (displayTime > 0)
		{
			displayTime -= 1;
		}
	}

	Text(int x, int y, string s, int t, int c):x(x), y(y), s(s), displayTime(t), color(c) {
		rememberDT = t;	
	}
	
	void changeText (string news)
	{
		s = news;
		displayTime = rememberDT;
	}

	private:
	int x;
	int y;
	string s;
	int displayTime;
	int rememberDT;	
	int color;
};

/* 
 * An abstract class for enemies.
 */

class Enemy : public Displayable {
	public:
		

                virtual void move() = 0; 

		int getX() {
			return x;
		}

		int getY(){
			return y;
		}

		int setY(int newY)
		{
			y = newY;
		}
		
		int getWidth(){
			return width;
		}

		int getHeight(){
			return height;
		}

		void setWidth(int newW)
		{
			width = newW;
		}

		void setHeight(int newH)
		{
			height = newH;
		}

		void setXSpeed(int nxspeed)
		{
			xspeed = nxspeed;
		}

  		int getXSpeed () {
    			return xspeed;
 	 	}

		void decreaseSpeed()
		{
			if (xspeed > 1) xspeed--;
		}

  Enemy (int x, int y, int speed, int height, int width): x(x), y(y), xspeed(speed), height(height), width(width){
		}

	protected:
		int x;
		int y;
  		int xspeed; 
  		int height;
  		int width;
};

/*
 * Class for Asteroid enemies. 
 */ 
class Asteroid : public Enemy {
       public:
                virtual void paint(XInfo &xinfo) {
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[1], x, y, height, width, 0, 360*64);
		}
		
		void move() {
			x = x - xspeed;
		}

  Asteroid(int x, int y, int s) : Enemy (x, y, s, 50, 50) {} 	// width = 50, height = 50														  
};

/*
 * Class that animates explosions on screen. 
 */
class Explosion : public Displayable {
	public: 
		virtual void paint(XInfo &xinfo) {
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[3], x - width/2, y - height/2, width, height, 0, 360*64);
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[4], x - (width-10)/2, y - (height-10)/2, width-10, height-10, 0, 360*64);
		}

		void decreaseTime()
		{
			timer = timer -1;
			height += 1;
			width += 1;
		}

		bool remove (){
			return timer == 0;
		}

	Explosion (int x, int y): x(x), y(y) {timer = 15; width = 10; height = 10;}

	private:
		int x, y;
		int timer;
		int width;
		int height; 
};

list<Explosion *> explosions;  // a list of active explosions

/*
 * Class that handles creating, moving, and removing enemies. 
 */
class EnemyGenerator {
	
	public:	
		
		EnemyGenerator()
		{
			delay = 90;
			numenemies = 5;
			delaycount = delay;
			types = 1;
			level = 1;
			releaseField = 100; 
			enemySpeed = 3;
		}

		void resetGenerator()
		{
			delay = 90;
			numenemies = 5;
			delaycount = delay;
			types = 1;
			level = 1;
			releaseField = 100; 
			enemySpeed = 3;
			released.clear();
			unreleased.clear();
		}
		
		void increaseESpeed ()
		{
			enemySpeed+=1;
		}
		
		int decreaseESpeed()
		{
			int ret = 1;
			if (enemySpeed > 1)
			{
				enemySpeed--;
				for(std::list<Enemy *>::iterator it = released.begin(); it != released.end(); it)
				{
					(*it)->decreaseSpeed();	
					it++;
				}
				for(std::list<Enemy *>::iterator it = unreleased.begin(); it != unreleased.end(); it)
				{
					(*it)->decreaseSpeed();	
					it++;
				}
				
			}
			else 
			{
				ret  = 2;
			}
			return ret;
		}
		
		void increaseLevel(XInfo &xinfo)
		{
			level = level+1;
			numenemies = 5 * level; 
			if (delay > 10) delay -= 10;
			if (releaseField+30 < xinfo.height - 50)
 			{
				releaseField += 30;
			}
		}

		bool testHit (int x, int y, int type)
		{
			bool hit = false;
			for (std::list<Enemy *>::iterator it = released.begin(); it != released.end(); it)
			{
				int enemyX, enemyY, enemyWidth, enemyHeight, xcentre, ycentre;

				enemyX = (*it)->getX();
				enemyY = (*it)->getY();
				enemyWidth = (*it)->getWidth();
				enemyHeight = (*it)->getHeight();
				xcentre = enemyX + enemyWidth/2;
				ycentre = enemyY + enemyHeight/2;

				// type 2 is if the enemy is hit by a bomb, which instantly destroys it
				if (type == 2 && x > enemyX + enemyWidth/2 && x < enemyX + enemyWidth && y > enemyY && y < enemyY + enemyHeight)
				{
					released.erase(it++);
					hit = true;
					explosions.push_front(new Explosion(xcentre, ycentre));
					explosions.push_front(new Explosion(xcentre+10, ycentre+10));
					explosions.push_front(new Explosion(xcentre-10, ycentre+10));
					explosions.push_front(new Explosion(xcentre+10, ycentre-10));
					explosions.push_front(new Explosion(xcentre-10, ycentre-10));
				}

				// type 1 is when the enemy is clicked by the mouse
				if (type == 1 && x > enemyX && x < enemyX + enemyWidth && y > enemyY && y < enemyY + enemyHeight)
				{
					if (enemyHeight <= 20)
					{
						released.erase(it++);
					}
					else
					{
						(*it)->setWidth(enemyWidth - 10);
						(*it)->setHeight(enemyHeight-10);
						(*it)->setY(enemyY + 2 );
						it++;
					}
					hit = true;
				}
				else
				{
					it++;
				}
			}
			return hit;
		}

		bool generatorEmpty ()
		{
			if (unreleased.empty() && released.empty() && numenemies == 0) { return true; } else { return false; } 
		}
		
		int getLevel()
		{
			return level;
		}

		int getNumEnemies()
		{
			return numenemies + unreleased.size() + released.size();
		}

		// Creates and moves enemies across the playing field
		int cycleEngine (XInfo &xinfo)
		{
			int returnCode = 0;
			delaycount -= 1;
			
			// enemies are generated at a regular rate
			if (delaycount == 0)
			{
				delaycount = delay;
				if (numenemies > 0)
				{
					if (types == 1 ) // there's only one type of enemy, but this is in case of expansion of the game
					{
						// randomly select the start height of the Asteroid
						int startheight = (rand() % (releaseField) - releaseField/2) + xinfo.pH/2 ;
						unreleased.push_back(new Asteroid(xinfo.pW + 50, startheight, enemySpeed));
						numenemies-=1;
					}
				}
			}

			// release enemies
			if (delaycount % 9 == 0)
			{
				if (!unreleased.empty())
					{
						int temp = rand() % (numenemies%5+1 ) + 1; // so enemies are releaseed at an irregular rate 
						if (temp == 1)
						{
							released.push_back(unreleased.front());
							unreleased.pop_front();
					}	
				}
			}
			// move released enemies
			for (std::list<Enemy *>::iterator it = released.begin(); it != released.end(); it)
			{
				(*it)->move();
				if ((*it)->getX() < -100)
				{
					released.erase(it++);
					returnCode = 1;
				}
				else
				{
					it++;
				}
			}
			return returnCode;
		}

	
	void paintEnemies (XInfo xinfo)
	{
		for (std::list<Enemy *>::iterator it = released.begin(); it != released.end(); it++)
		{
			(*it)->paint(xinfo);
		}
	}	


	private:
		int level; 	// what level is the player on
		int numenemies; // number of enemies for the current level
		int delay;  	// delay between the spawn of enemies
		int delaycount; // count down for the next spawn
		int types; 	// how many types of enemies are being created
		std::list<Enemy *> unreleased; // enemies that have been spawned but aren't moving yet
		std::list<Enemy *> released; // enemies that are released and moving
		int releaseField; // range of height enemies can be created in
		int enemySpeed; // speed of enemies 
};	

/*
 * Bombs the player can place on screen to destroy enemies
 */
class Bomb : public Displayable { 
	public:
	  virtual void paint(XInfo &xinfo) {
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[2], x, y, height, width, 0, 360*64); 
			XDrawArc(xinfo.display, xinfo.pixmap, xinfo.gc[3], x, y, height, width, 0, 360*64);
	}


  Bomb (int x, int y): x(x), y(y){
	height = 20;
	width = 20;
		}

	void setX(int newx) { x = newx;}
	void setY(int newy) { y = newy;}

	int getX() { return x; }
	int getY() { return y; } 

	private:
		int x;
		int y;
		int width;
		int height;
};

/*
 * Controller for managing bombs being picked up, dropped, and painted. 
 */
class BombGenerator : public Displayable { 
	public:
	  virtual void paint(XInfo &xinfo) {
			
			XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[2], x, y, height, width, 0, 360*64);
		if (carrying)
		{
			currentB->paint(xinfo);
		} 
			
		for (std::list<Bomb *>::iterator it = dropped.begin(); it != dropped.end(); it)
		{
			(*it)->paint(xinfo);
			it++;
		}
	}

	BombGenerator()
	{
		x = 5;
		y = 5;
		height = 20;
		width = 20;
		carrying = false;
		bombNum = 0;
	}

	void reset()
	{
		carrying = false;
		bombNum = 0;
		dropped.clear();
	}

		bool testHit (int tx, int ty)
		{
			if (bombNum > 0 && tx > x && tx < x + width && ty > y && ty < y + height)
			{
				currentB = new Bomb(5,5);		
				carrying = true;
				bombNum -= 1;
				return true;
			}
			return false;
		}

	void keyPickup(int x, int y)
	{
		if (bombNum > 0)
		{
			currentB = new Bomb(x,y);
			carrying = true;
			bombNum -= 1;
		}
	}

	bool testCarrying(){
		return carrying;
	}
		
	int getBombNum (){
		return bombNum;
	}

	void setBombNum (int newBN){
		bombNum = newBN;
	}
	
	void setCarryingPos(int newx, int newy) {
		currentB->setX(newx);
		currentB->setY(newy);
	 }

	void drop (int dx, int dy)	
	{
		setCarryingPos(dx, dy);
		dropped.push_back (currentB);	
		currentB = NULL;
		carrying = false;
	}

	void testBombHit (EnemyGenerator &gen)
	{
		for (std::list<Bomb *>::iterator it = dropped.begin(); it != dropped.end(); it)
		{
			int currentBX = (*it)->getX() + 10;
			int currentBY = (*it)->getY() + 10;
			bool hit = gen.testHit(currentBX, currentBY, 2);
			if (hit)
			{
				dropped.erase(it++);
			}
			else
			{
				it++;
			}
		}
	}	

	private:
		int x;
		int y;
		int height;
		int width;
		Bomb *currentB;
		std::list<Bomb *> dropped;
		bool carrying;
		int bombNum;
};

list<Displayable *> dList;  // list of displayable objects 

/*
 * Initialize X and create a window
 */
void initX(int argc, char *argv[], XInfo &xInfo) {
	XSizeHints hints;
	unsigned long white, black;

   /*
	* Display opening uses the DISPLAY	environment variable.
	* It can go wrong if DISPLAY isn't set, or you don't have permission.
	*/	
	xInfo.display = XOpenDisplay( "" );
	if ( !xInfo.display )	{
		error( "Can't open display." );
	}
	
   /*
	* Find out some things about the display you're using.
	*/
	xInfo.screen = DefaultScreen( xInfo.display );

	white = XWhitePixel( xInfo.display, xInfo.screen );
	black = XBlackPixel( xInfo.display, xInfo.screen );

	hints.x = 100;
	hints.y = 100;
	hints.width = 400;
	hints.height = 500;
	hints.flags = PPosition | PSize;

	xInfo.window = XCreateSimpleWindow( 
		xInfo.display,				// display where window appears
		DefaultRootWindow( xInfo.display ), // window's parent in window tree
		hints.x, hints.y,			// upper left corner location
		hints.width, hints.height,	// size of the window
		Border,						// width of window's border
		black,						// window border colour
		white);						// window background colour
	
	xInfo.pX=0;
	xInfo.pY=0;
	xInfo.pW = hints.width;
	xInfo.pH = hints.height;
	
	XSetStandardProperties(
		xInfo.display,		// display containing the window
		xInfo.window,		// window whose properties are set
		"EarthDefender",		// window's title
		"EarthDefender",			// icon's title
		None,				// pixmap for the icon
		argv, argc,			// applications command line args
		&hints );			// size hints for the window

	/* 
	 * Create Graphics Contexts
	 */
	int i = 0;
	xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
	XSetForeground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
	XSetBackground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
	XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
	XSetLineAttributes(xInfo.display, xInfo.gc[i],
	                     1, LineSolid, CapButt, JoinRound);

	// Reverse Video
	i = 1;
	xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
	XSetForeground(xInfo.display, xInfo.gc[i], WhitePixel(xInfo.display, xInfo.screen));
	XSetBackground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
	XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
	XSetLineAttributes(xInfo.display, xInfo.gc[i],
	                     1, LineSolid, CapButt, JoinRound);

	// colour info
	Colormap screen_colormap;
	XColor red, brown, blue, yellow, green;
	screen_colormap = DefaultColormap(xInfo.display, DefaultScreen(xInfo.display));
	XAllocNamedColor(xInfo.display, screen_colormap, "blue", &blue, &blue);
	XAllocNamedColor(xInfo.display, screen_colormap, "red", &red, &red);
	XAllocNamedColor(xInfo.display, screen_colormap, "yellow", &yellow, &yellow);
	XAllocNamedColor(xInfo.display, screen_colormap, "green", &green, &green);
	i=2;
	xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
	XSetForeground(xInfo.display, xInfo.gc[i], blue.pixel);
	XSetBackground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
	XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
	XSetLineAttributes(xInfo.display, xInfo.gc[i],
	                     1, LineSolid, CapButt, JoinRound);
	i=3;
	xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
	XSetForeground(xInfo.display, xInfo.gc[i], red.pixel);
	XSetBackground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
	XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
	XSetLineAttributes(xInfo.display, xInfo.gc[i],
	                     1, LineSolid, CapButt, JoinRound);
	i=4;
	xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
	XSetForeground(xInfo.display, xInfo.gc[i], yellow.pixel);
	XSetBackground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
	XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
	XSetLineAttributes(xInfo.display, xInfo.gc[i],
	                     1, LineSolid, CapButt, JoinRound);
	i=5;
	xInfo.gc[i] = XCreateGC(xInfo.display, xInfo.window, 0, 0);
	XSetForeground(xInfo.display, xInfo.gc[i], green.pixel);
	XSetBackground(xInfo.display, xInfo.gc[i], BlackPixel(xInfo.display, xInfo.screen));
	XSetFillStyle(xInfo.display, xInfo.gc[i], FillSolid);
	XSetLineAttributes(xInfo.display, xInfo.gc[i],
	                     1, LineSolid, CapButt, JoinRound);
	

	XSelectInput(xInfo.display, xInfo.window, 
		ButtonPressMask | KeyPressMask | 
		PointerMotionMask | 
		EnterWindowMask | LeaveWindowMask |
		StructureNotifyMask);  // for resize events

	int depth = DefaultDepth(xInfo.display, DefaultScreen(xInfo.display));
	xInfo.pixmap = XCreatePixmap(xInfo.display, xInfo.window, hints.width, hints.height, depth);
	

	XSetWindowBackgroundPixmap(xInfo.display, xInfo.window, None);
	

	/*
	 * Put the window on the screen.
	 */
	XMapRaised( xInfo.display, xInfo.window );
	
	XFlush(xInfo.display);
	sleep(2);	// let server get set up before sending drawing commands
}

/*
 * Function to repaint a display list
 */
void repaint( XInfo &xinfo, EnemyGenerator &gen, int option) {
	list<Displayable *>::const_iterator begin = dList.begin();
	list<Displayable *>::const_iterator end = dList.end();

	// background for our pixmap
	XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[0], 0, 0, xinfo.pW, xinfo.pH);
	//outline
	XDrawRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[3], 0, 1, xinfo.pW-1, xinfo.pH-2);

	// the Earth
	XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[2], -650, -175, 700, 900, 0, 360*64 );
	XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[5], -75, 130, 100, 150, 0, 360*64);
	XFillArc(xinfo.display, xinfo.pixmap, xinfo.gc[5], -175, 300, 200, 100, 0, 360*64);
	XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], 0, 0, xinfo.pW, 30);
	// draw display list
	gen.paintEnemies(xinfo);
	while( begin != end ) {
		Displayable *d = *begin;
		d->paint(xinfo);
		begin++;
	}
	
		for (std::list<Explosion *>::iterator it = explosions.begin(); it != explosions.end(); it)
		{
			(*it)->paint(xinfo);
			(*it)->decreaseTime();
			if ((*it)->remove())
			{
				explosions.erase(it++);
			}	
			else
			{
				it++;
			}
		}
		
		// splash text
		if (option == 0)
		{
			string s[17];
			s[0] = "Welcome,";
			s[2] = "It's your duty to defend the Earth";
			s[3] = "against an asteroid field the likes";
			s[4] = "of which the planet has never seen.";
			s[6] = "Controls: ";
			s[7] = "Click on the asteroids to fire";
			s[8] = "b - pick up a bomb";
			s[9] = "q - quit";
			s[10] = "**FOR TA: s - slow down asteroids";
			s[12] = "Click to start!";
			s[16] = "By Michael McLean";
			XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[1], 100, 100, xinfo.pW-175, xinfo.pH-200);
			for (int i=0; i<17; i++)
			{
			XDrawImageString (xinfo.display, xinfo.pixmap, xinfo.gc[0], 105, 120 + i*15, s[i].c_str(), s[i].length());
			}
		}

	// copy pixmap to screen (double buffering)
	XCopyArea(xinfo.display, xinfo.pixmap, xinfo.window, xinfo.gc[0], 0, 0, xinfo.pW, xinfo.pH, xinfo.pX, xinfo.pY);

	XFlush( xinfo.display );
}

// handle mouse clicks
void handleButtonPress(XInfo &xinfo, XEvent &event, EnemyGenerator &gen, BombGenerator &bgen) {

	// see if it hit the enemy
	gen.testHit(event.xbutton.x - xinfo.pX, event.xbutton.y - xinfo.pY, 1);
	
	// or dropped a bomb
	if (bgen.testCarrying())
	{
		bgen.drop(event.xbutton.x - xinfo.pX - 10, event.xbutton.y - xinfo. pY - 10);
	}
	else
	{
		//pick up a bomb 
		bool res = bgen.testHit(event.xbutton.x - xinfo.pX , event.xbutton.y - xinfo.pY);
		
		// place explosion where click happened if a bomb wasn't picked up
		if (!res && !bgen.testCarrying())
		{
			explosions.push_front(new Explosion(event.xbutton.x-xinfo.pX, event.xbutton.y-xinfo.pY));
		}

	}
}

// handle key press
int handleKeyPress(XInfo &xinfo, XEvent &event, EnemyGenerator &egen, BombGenerator &bgen) {
	KeySym key;
	char text[BufferSize];
	
	int returnVal =0;

	/*
	 * Exit when 'q' is typed.
	 */
	int i = XLookupString( 
		(XKeyEvent *)&event, 	// the keyboard event
		text, 					// buffer when text will be written
		BufferSize, 			// size of the text buffer
		&key, 					// workstation-independent key symbol
		NULL );					// pointer to a composeStatus structure (unused)
	if ( i == 1) {
		if (text[0] == 'q') {
			error("Terminating normally.");
		}
		if (text[0] == 'b' && !bgen.testCarrying())
		{
			bgen.keyPickup(event.xbutton.x - xinfo.pX - 10, event.xbutton.y- xinfo.pY -10);
		}
		if (text[0] == 's')
		{
			returnVal = egen.decreaseESpeed();
		}
		if (text[0] == 'f')
		{
			egen.increaseESpeed();
		}
	}
	return returnVal;
}

// used to make a bomb follow the mouse when it's pick up / before it's dropped
void handleMotion(XInfo &xinfo, XEvent &event, int inside, BombGenerator &bgen) {
	if (bgen.testCarrying())
	{
		bgen.setCarryingPos(event.xbutton.x - xinfo.pX - 10, event.xbutton.y - xinfo.pY - 10);
	}
	
}

int handleAnimation(XInfo &xinfo, int inside, EnemyGenerator &gen, BombGenerator &bgen) {
	
	int rcode = gen.cycleEngine(xinfo);
	bgen.testBombHit(gen);
	return rcode;
}

// update width and height when window is resized ... centre our pixmap
void handleResize(XInfo &xinfo, XEvent &event) {
	XConfigureEvent xce = event.xconfigure;
	if (xce.width != xinfo.width || xce.height != xinfo.height) {
		xinfo.width = xce.width;
		xinfo.height = xce.height;
	}
	if (xinfo.width > xinfo.pW) xinfo.pX = (xinfo.width-xinfo.pW)/2;
	else xinfo.pX = 0;

	if (xinfo.height > xinfo.pH) xinfo.pY = (xinfo.height-xinfo.pH)/2;
	else xinfo.pY = 0;

	
	XFillRectangle(xinfo.display, xinfo.window, xinfo.gc[0], 0, 0, xinfo.width, xinfo.height);
}

// get microseconds
unsigned long now() {
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec;
	//return tv.tv_sec * 1000000 + tv.tv_usec;
}

//main loop for the program
void eventLoop(XInfo &xinfo) {

	Text bombText = Text(30, 20, "Bombs: 0", -1, 1);
	dList.push_front (&bombText);

	Text levelText = Text(200, 20, "Level: 1", -1, 1);
	dList.push_front (&levelText);

	Text remainingEnemiesText = Text (260, 20, "Remaining Asteroids: ", -1, 1);
	dList.push_front (&remainingEnemiesText);

	XEvent event;
	int inside = 0;

	EnemyGenerator generator;

	BombGenerator bgen;
	dList.push_front(&bgen); 

	int pause = 0;   			// 0 if the game is paused (before playing / between games
	bool justSwitched = false; 		// used to help print instructions before game
	int ret = 0;
	
	Text info = Text (10, 50, "Click your mouse on screen to start!", 500, 2);
	dList.push_front (&info);

	bool firstGame = true;			// is it the first game since it started up
	int kret;

	while( true ) {
		
		if (XPending(xinfo.display) > 0) {
			XNextEvent( xinfo.display, &event );
			switch( event.type ) {
				case ButtonPress:
					handleButtonPress(xinfo, event, generator,bgen);
					if (pause == 0)	
					{
						justSwitched = true;
					}
					pause = 1;
					break;
				case KeyPress:
					kret = handleKeyPress(xinfo, event, generator, bgen);
				 	if (kret == 1)
					{
						info.changeText("Decreasing speed.");
					}
					else if (kret == 2)
					{
						info.changeText ("Speed at minimum!");
					}
					break;
				case MotionNotify:
					handleMotion(xinfo, event, inside, bgen);
					break;
				case EnterNotify:
					inside = 1;
					break;
				case LeaveNotify:
					inside = 0;
					break;
				case ConfigureNotify:
					handleResize(xinfo, event);
					break;	
				default: 
					usleep(1000000/FPS);
			}
		} 

		if (pause == 0 && firstGame)
		{
			info.changeText("Click your mouse on screen to start!");
		}
		
		if (pause == 1 && justSwitched)
		{
			justSwitched = false;
			info.changeText ("Click on the asteroids to destroy them.");
		}

		// increase the level if all the enemies are defeated
		if (generator.generatorEmpty()) 
		{
			generator.increaseLevel(xinfo);
			int level = generator.getLevel();
			bgen.setBombNum(bgen.getBombNum() + 2 * (level - 1));
			levelText.changeText ("Level: " + numToString(level));
			if (level % 3 == 0)
			{
				generator.increaseESpeed();
				info.changeText("Uh oh! They're getting faster!");
			}
	
			if (level == 2)
			{
				info.changeText("Press b on your keyboard to grab a bomb, click to place it.");	
			}
		}
		
		// Repaint and perform movement, FPS times a second
		if (now() % (100000/FPS) == 0)
		{
			bombText.changeText("Bombs: " + numToString( bgen.getBombNum()));
			remainingEnemiesText.changeText("Remaining Asteroids: " + numToString(generator.getNumEnemies()));
			if (pause == 1)
			{		
			int rcode = handleAnimation(xinfo, inside, generator, bgen);
		
			// do a big explosion when an asteroid hits the Earth
			if (rcode == 1)
			{
				for (int j=0; j<10; j++) {
					for (int i=10; i<50; i++)
					{
						explosions.push_front(new Explosion(j*4,i*10 ));
					}	
					repaint(xinfo,generator, 1);
				}
				// reset the game
				generator.resetGenerator();
				bgen.reset();
				levelText.changeText ("Level: " + numToString(generator.getLevel()));
				info.changeText("You let down the planet! Click to try again!");
				pause = 0;
				firstGame = false;
			}	
			}
			
			if (pause == 0 && firstGame == true) // put up the splash screen
			{
				repaint(xinfo, generator, 0);
			}
			else // paint normally 
			{
				repaint(xinfo, generator, 1);	
			}
		}
		
	}
}


/*
 * Start executing here.
 *	 First initialize window.
 *	 Next loop responding to events.
 */
int main ( int argc, char *argv[] ) {
	XInfo xInfo;

	initX(argc, argv, xInfo);
	eventLoop(xInfo);
	XCloseDisplay(xInfo.display);
}
