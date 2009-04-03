/*
 * Froggix 
 *
 * Nicholas DeClario 2009
 * <nick@declario.com>
 *
 * This program is distributed under the GNU Public License
 *   <Insert GNU license blurb here>
 */


/*
 * Our pretty standard includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

/* 
 * Set some basic definitions
 */
#define VERSION "$Id: froggix.c,v 1.2 2009-04-03 02:48:21 nick Exp $"
#define TITLE "Froggix"
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FALSE 0
#define TRUE 1
#define LIVES 3
#define COLORKEY 255, 0, 255
#define BGCOLOR 0, 0, 0
#define FROGGER_START_X 290
#define FROGGER_START_Y 425
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define X 0
#define Y 1
#define FRAME 24
#define HFRAME 12
#define HOP_DISTANCE 30
#define HOP_SPEED 3
#define ROW_BASE 425
#define LEFT_SIDE 115
#define RIGHT_SIDE 525
#define SPLASH 1	/* Death types */
#define SPLAT 2

/* Point table */
#define SCORE_HOP 10
#define SCORE_GOAL 50
#define SCORE_LEVEL 1000
#define SCORE_FLY 150
#define SCORE_PINK 200
#define SCORE_SECONDS 10
#define HIGH_SCORE 4630
#define SCORE_FREE_FROG 200

/* The green game timer */
#define MAX_TIMER 350
#define TIMER_SIZE 150
#define TIMER_COLOR 32, 211, 0
#define TIMER_LOW_COLOR 255, 0, 0

/* baddies */
#define VEHICLE 0
#define LOG 1
#define TURTLE 2
#define GATOR 3
#define SNAKE 4
#define BEAVER 5

/* Goal areas */
#define MAX_GOALS 5 

/* logs */
#define SHORT_LOG 4
#define MEDIUM_LOG 6
#define LONG_LOG 9
#define MAX_WOOD 7

/* Turtles */
#define DIVE_START_TIME 50
#define DIVE_PHASE_TIME 20
#define MAX_TURTLES 9
#define TURTLE_ANIM_TIME 5

/* Vehicles */
#define MAX_VEHICLES 40


/*
 * Froggers dstruct
 */
typedef struct {
	int placement[2];
	int oldPlacement[2];
	int direction;
	int location; 
	int hopCount;
	int currentRow;
	int alive;
	int riding;
	int ridingIdx;
	int ridingType;
	int frogger;	/* Are we frogger or bonus frog */
	int deathType;
	int deathCount;

	SDL_Rect src;
	SDL_Rect dst;

	Mix_Chunk *s_hop;
	Mix_Chunk *s_squash;
	Mix_Chunk *s_splash;
	Mix_Chunk *s_extra;
} froggerObj;

/*
 * Goals
 */
typedef struct {
	int x, y, w, h;
	int occupied;
	int fly;
	int gator;
} goalObj;

/*
 * Vehicles
 */
typedef struct {
	int placement[2];
	int oldPlacement[2];
	int direction; 	// LEFT or RIGHT
	int row;  	// row
	int speed;	// How fast are we traveling
	int level;	// Must be >= this level to display
		
	SDL_Rect src;
} vehicleObj;

/*
 * It's Log!
 */
typedef struct {
	int placement[2];
	int oldPlacement[2];
	int row;       /* Current row we are in */
	int type;      /* SHORT, MEDIUM, or LONG */
	int speed;     /* What speed does the log move at */
	int hasPink;   // Is bonus frog riding

	SDL_Rect src;
} logObj;

/*
 * Turtles
 */
typedef struct {
	int placement[2];
	int oldPlacement[2];
	int row;	/* Which row are the turtles in? */
	int count;	/* How many turtles in this group? */
	int speed;	/* How fast are they swimming */
	int canDive;	/* Can this group dive */
	int diveStep;   /* If they can dive, what diving step are then in */
	int diveTime;	/* Current dive time */
	int animStep;	/* Current animation frame */
	int animDelay;  /* The number of ticks to wait between frames */

	SDL_Rect src;
} turtleObj;

int keyEvents( SDL_Event event );
int mySDLInit( void );
void beginGame( void );
int loadMedia( void );
int heartbeat( void );
int updateGameState( void );
void configGameScreen( void );
void drawGameScreen( void );
void drawBackground( void );
int getRowPixel ( int row );
int collisionRow ( void );
int freeFrog( int score );
int collideFrogger ( int x, int y, int h, int w );
void checkFroggerBorder( void );
void levelUp( void );
int checkGoals( void );
void froggerReset( void );
logObj setWood( int type, int speed, int row, int startX );
turtleObj setTurtle( int dive, int diveTimer, int speed, int row, int startX, int count );
vehicleObj setVehicle( int row, int startX, int speed, int level );
goalObj setGoal( int x, int y, int w, int h );
void moveFrogger( void );
void ridingFrogger( );
void drawTitleScreen( void );
void drawPauseScreen( void );
void drawGameOver( void );
int drawDeathSequence( int deathType );
int checkTimer( void );
void drawScore( int high, int score );
void drawNumbers( int num, int x, int y );
void drawGoals( void );
void drawTimer( int length );
void drawLives( int lives );
void drawLevel( int level );
void drawWood( void );
void drawTurtles( void );
void drawVehicles( void );
void drawImage(SDL_Surface *srcimg, int sx, int sy, int sw, int sh, SDL_Surface *dstimg, int dx, int dy, int alpha );
void playSound( Mix_Chunk *sound );
void setFullScreenMode( void );

int level = 0;
int playing = 0;
int lives = 0;
int players = 0;
int score = 0;
int givenFreeFrog = 0;
int hScore = HIGH_SCORE;
int redraw_all = 0;
int fullscreen = 0;
int drawBG = 0;
int goDelay;
float timeLeft;
froggerObj frogger;
logObj wood[MAX_WOOD];
turtleObj turtle[MAX_TURTLES];
vehicleObj vehicle[MAX_VEHICLES];
goalObj goals[MAX_GOALS];

Mix_Chunk  *s_freeFrog;
SDL_Surface *gfx;
SDL_Surface *background; // This is the frogger back drop
SDL_Rect backgroundRect;
SDL_Surface *titleSurface; // Title 'Froggix' image
SDL_Surface *screen; //This pointer will reference the backbuffer 
SDL_Rect leftBorderRect;
SDL_Rect rightBorderRect;
TTF_Font *font;

int debugBorder = 0;

/*
 * int mySDLInit(void);
 *
 * This starts the basic SDL initialization for everything we'll need
 * 	
 */
int mySDLInit( void ) {
	int result = 1;

	if( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
		fprintf( stderr, "Warning: Unable to initialize video: %s\n", SDL_GetError( ) );
		result--;
	}

	if( TTF_Init( ) == -1 ) {
		fprintf( stderr, "Warning: Unable to initialize font engine: %s\n", TTF_GetError( ) );
		result--;
	}

	if( SDL_Init( SDL_INIT_AUDIO ) != 0 ) {
		fprintf( stderr, "Warning: Unable to initialize audio: %s\n", SDL_GetError( ) );
		result--;
	}
	
	if( Mix_OpenAudio( 11025, AUDIO_S16, 2, 512 ) < 0 ) {
		fprintf( stderr, "Warning: Audio set failed: %s\n", SDL_GetError( ) );
		result--;
	}

	screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, 16, SDL_HWSURFACE );

	if ( screen == NULL ) {
		fprintf( stderr, "Error: Unable to set video mode: %s\n", SDL_GetError( ) );
		result--;
	}

	SDL_WM_SetCaption( TITLE, NULL );

	return result;
}

/*
 * void beginGame( void );
 *
 * Main game routine
 */
void beginGame( void ) {
	float	next_heartbeat = 0;
	SDL_Event event;
	int 	done = 0;

	printf ( "D: Starting main game loop\n" );

	if ( loadMedia( ) <= 0 ) {
		fprintf( stderr, "Error: Failed to load graphics and audio!\n" );
		return;
	}

	drawBackground( );

	while( ! done ) {
		while( SDL_PollEvent( &event ) ) {
			done = keyEvents( event );
		}

		/* Check the heartbeat to see if we're ready */
		if ( SDL_GetTicks( ) >= next_heartbeat ) {
			next_heartbeat = SDL_GetTicks( ) + heartbeat( );
		}
		SDL_Delay( 30 );
	}

	SDL_FreeSurface( gfx );
}

int loadMedia( void ) {
	int result = 1;

	/*
 	 * Load frogger's textures and sounds
 	 */
	gfx = IMG_Load( "images/frogger.png" );
		frogger.riding = FALSE;

	if ( gfx == NULL ) {
		fprintf( stderr, "Error: 'images/frogger.bmp' could not be open: %s\n", SDL_GetError( ) );
		result--;
	}

	if ( SDL_SetColorKey( gfx, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB( gfx->format, COLORKEY ) ) == -1 ) 
		fprintf( stderr, "Warning: colorkey will not be used, reason: %s\n", SDL_GetError( ) );

	background = IMG_Load( "images/gameboard.png" );

	if ( gfx == NULL ) {
		fprintf( stderr, "Error: 'images/gameboard.png' could not be open: %s\n", SDL_GetError( ) );
		result--;
	}

	titleSurface = IMG_Load( "images/froggix-title.png" );
	if ( titleSurface == NULL ) {
		fprintf( stderr, "Error: 'images/froggix-title.png' could not be open: %s\n", SDL_GetError( ) );
		result--;
	}

	font = TTF_OpenFont( "fonts/CourierNew-Bold.ttf", 22 );
	if ( font == NULL ) {
		printf( "TTF_OpenFont: %s\n", TTF_GetError( ) );
		result--;
	}

	frogger.s_hop = Mix_LoadWAV( "sounds/froggix-hop.wav" );

	if ( frogger.s_hop == NULL ) 
		fprintf( stderr, "Warning: dp_frogger_hop.wav could not be opened: %s\n", SDL_GetError( ) );

	frogger.s_squash = Mix_LoadWAV( "sounds/dp_frogger_squash.wav" );
	if ( frogger.s_squash == NULL )
		fprintf( stderr, "Warning: dp_frogger_plunk could not be opened %s\n", SDL_GetError( ));

	frogger.s_splash = Mix_LoadWAV( "sounds/dp_frogger_plunk.wav" );
	if ( frogger.s_splash == NULL )
		fprintf( stderr, "Warning: dp_frogger_splash could not be opened %s\n", SDL_GetError( ));

	s_freeFrog = Mix_LoadWAV( "sounds/dp_frogger_extra.wav" );
	if ( s_freeFrog == NULL )
		fprintf( stderr, "Warning: dp_frogger_extra could not be opened %s\n", SDL_GetError( ));

	return result;
}

/*
 * void keyEvents( void );
 *
 * Process the incoming keyboard and mouse events
 *
 */
int keyEvents( SDL_Event event ) {
	int done = 0;

	/* Always check for shutdown */
	switch( event.type ) {
		case SDL_QUIT:
			done = 1;
			break;
		case SDL_KEYDOWN:
			/* printf( "Found key: %i\n", event.key.keysym.sym );*/
			switch( event.key.keysym.sym ) {
				case SDLK_ESCAPE:
					done = 1;
					break;
				case 102:
					setFullScreenMode( );
					break;
				default: 
					break;
			}
			break;
		default: 	
			break;
	}
	/* We are playing the game */
	if ( level ) {
		/* Main game playing input */
		if ( playing ) {
			if ( event.type == SDL_KEYDOWN && frogger.alive ) {
				switch( event.key.keysym.sym ) {
					case SDLK_UP:
						if ( ! frogger.direction ) {
							frogger.hopCount = 0;
							frogger.direction = UP;
							frogger.currentRow++;
							playSound( frogger.s_hop );
						}
						break;
					case SDLK_DOWN:
						if ( ! frogger.direction ) {
							frogger.hopCount = 0;
							frogger.direction = DOWN;
							frogger.currentRow--;
							playSound( frogger.s_hop );
						}
						break;
					case SDLK_LEFT:
						if ( ! frogger.direction ) {
							frogger.hopCount = 0;
							frogger.direction = LEFT;
							playSound( frogger.s_hop );
						}
						break;
					case SDLK_RIGHT:
						if ( ! frogger.direction ) {
							frogger.hopCount = 0;
							frogger.direction = RIGHT;
							playSound( frogger.s_hop );
						}
						break;
					case 108:
						levelUp( );
						fprintf( stderr, "Increase level to %i.\n", level );
						break;
					default:
						break;
				}
				printf( "x,y,d => %i,%i,%i,%i\n", frogger.placement[X],
							       frogger.placement[Y],
							       frogger.direction,
							       frogger.currentRow );
			}
			/* Game over man, game over! */
			if ( ! lives ) {
		
			}
		}
		/* we're at the pause screen */
		else {

		}
	}
	/* Main intro screen input */
	else {
		if ( event.type == SDL_KEYUP ) {
			switch( event.key.keysym.sym ) {
				case SDLK_ESCAPE:
					done = 1;
					break;
				case SDLK_1:
					printf( "D: Starting single player game\n" );
					level = 1;
					lives = LIVES;
					playing = TRUE;
					score = 0;
					players = 1;
					redraw_all = 1;
					break;
				default:
					break;
			}
		}
	}

	return done;
}

int updateGameState( void ) {
	int i;

	if ( ! drawBG ) configGameScreen( );

	if ( lives <= 0 ) {
		goDelay++;
		drawGameOver( );
		/* Display game over screen for 50 ticks before returning
 		 * to the main screen */
		if ( goDelay > 7 ) {
			playing = 0;
			lives   = 0;
			level   = 0;
			score   = 0;
			givenFreeFrog = 0;
			drawBG  = 0;
			for ( i = 0; i < MAX_GOALS; i++ ) { goals[i].occupied = 0; }
			
		}
		return 500;
	}

	drawGameScreen( );

	return 50;
}

logObj setWood( int type, int speed, int row, int startX ) {
	logObj tempWood;
	int imgPixelSrc = 0;
	
	switch( type ) {
		case LONG_LOG:
			imgPixelSrc = 0;
			break;
		case MEDIUM_LOG:
			imgPixelSrc = FRAME * LONG_LOG;
			break;
		case SHORT_LOG:
			imgPixelSrc = FRAME * ( LONG_LOG + MEDIUM_LOG );
			break;
	}

        tempWood.row  = row;
        tempWood.type = type;
	tempWood.speed = speed;
	tempWood.hasPink = 0;
        tempWood.placement[X] = LEFT_SIDE + startX;
        tempWood.placement[Y] = getRowPixel( row );
        tempWood.oldPlacement[X] = LEFT_SIDE + startX;
        tempWood.oldPlacement[Y] = getRowPixel( row );
        tempWood.src.y = FRAME;
        tempWood.src.x = imgPixelSrc;
        tempWood.src.w = FRAME * tempWood.type;
        tempWood.src.h = FRAME;

	return tempWood;
}

turtleObj setTurtle( int dive, int diveTimer, int speed, int row, int startX, int count ) {
	turtleObj tt;

	tt.row = row;
	tt.canDive = dive;
	tt.diveStep = 0;
	tt.diveTime = diveTimer;
	tt.animStep = 0;
	tt.animDelay = 0;
	tt.speed = speed;
	tt.count = count;
	tt.placement[X] = LEFT_SIDE + startX;
	tt.placement[Y] = getRowPixel( row );
	tt.oldPlacement[X] = tt.placement[X];
	tt.oldPlacement[Y] = tt.placement[Y];
	tt.src.y = FRAME * 2;
	tt.src.x = 0;
	tt.src.w = FRAME;
	tt.src.h = FRAME;
	
	return tt;
}


vehicleObj setVehicle( int row, int startX, int speed, int level ) {
	vehicleObj v;

	v.direction = ( row % 2 ) ? LEFT : RIGHT; /* Odd rows travel left, evens go right */
	v.row = row;
	v.speed = speed;
	v.level = level;
	v.placement[X] = LEFT_SIDE + startX;
	v.placement[Y] = getRowPixel( row );
	v.oldPlacement[X] = v.placement[X];
	v.oldPlacement[Y] = v.placement[Y];
	v.src.y = FRAME * 2;
	v.src.x = FRAME * ( 4 + row );
	v.src.w = ( row == 5 ) ? FRAME * 2 : FRAME; /* Are we a truck? */
	v.src.h = FRAME;
	
	return v;
}

goalObj setGoals( int x, int y, int w, int h ) {
	goalObj g;

	g.x = x;
	g.y = y;
	g.w = w;
	g.h = h;
	g.occupied = 0;
	g.fly = 0;
	g.gator = 0;

	return g;
}

void configGameScreen( void ) {
	drawBG = 1;

	/*
 	 * Draw background map
 	 */
	//drawBackground( );
	drawImage( background, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen, 0, 0, 255 );

	/* Cars drive on rows 1 - 5
 	 * Logs are on rows 8, 9 and 11, 8 = short, 9 long, 11 medium
 	 * Turtles are on rows 7, 10
 	 * Frogger starts on Row 0, 
 	 * Sidewalk is row 6
 	 * and the goal is row 12
 	 */

	/* I MUST figure out a better way to handle the logs, turtles and cars */

	/* Set up the LONG logs on row 9 first */
	wood[0] = setWood( LONG_LOG, 3, 9, 0 );
	wood[1] = setWood( LONG_LOG, 3, 9, 305 );
	wood[2] = setWood( SHORT_LOG, 2, 8, 25 );
	wood[3] = setWood( SHORT_LOG, 2, 8, 160 );
	wood[4] = setWood( SHORT_LOG, 2, 8, 380 );
	wood[5] = setWood( MEDIUM_LOG, 4, 11, 140 );
	wood[6] = setWood( MEDIUM_LOG, 4, 11, 440 );

	drawWood( );

	/* Configure our turtles */
	turtle[0] = setTurtle( FALSE, 0, 1, 7, 0, 3 );
	turtle[1] = setTurtle( TRUE, 0, 1, 7, 125, 3 );
	turtle[2] = setTurtle( FALSE, 0, 1, 7, 250, 3 );
	turtle[3] = setTurtle( TRUE, 30, 1, 7, 375, 3 );
	turtle[4] = setTurtle( FALSE, 0, 2, 10, 100, 2 );
	turtle[5] = setTurtle( TRUE, 50, 2, 10, 200, 2 );
	turtle[6] = setTurtle( FALSE, 0, 2, 10, 300, 2 );
	turtle[7] = setTurtle( TRUE, 10, 2, 10, 400, 2 );
	turtle[8] = setTurtle( FALSE, 0, 2, 10, 500, 2 );

	drawTurtles( );

	/* Configure vehicles */
	/*			row, X, speed */
	/* Row 1 -- yellow car */
	vehicle[0] = setVehicle( 1, 0, 1, 1 );
	vehicle[1] = setVehicle( 1, 100, 1, 3 );
	vehicle[2] = setVehicle( 1, 200, 1, 1 );
	vehicle[3] = setVehicle( 1, 300, 1, 1 );
//	/* Row 2 -- tractor */
	vehicle[4] = setVehicle( 2, 0, 3, 1 );	
	vehicle[5] = setVehicle( 2, 100, 3, 2 );	
	vehicle[6] = setVehicle( 2, 200, 3, 1 );	
	vehicle[7] = setVehicle( 2, 300, 3, 3 );	
//	/* Row 3 -- pink car */
	vehicle[8] = setVehicle( 3, 75, 2,1  );	
	vehicle[9] = setVehicle( 3, 150, 2, 3 );	
	vehicle[10] = setVehicle( 3, 225, 2, 1 );	
	vehicle[11] = setVehicle( 3, 375, 2, 2 );	
//	/* Row 4 -- white car */
	vehicle[12] = setVehicle( 4, 75, 5, 1 );
	vehicle[13] = setVehicle( 4, 150, 5, 3 );
	vehicle[14] = setVehicle( 4, 225, 5, 2 );
	vehicle[15] = setVehicle( 4, 375, 5, 3 );
//	/* Row 5 -- Trucks */
	vehicle[16] = setVehicle( 5, 30, 3, 1 );
	vehicle[17] = setVehicle( 5, 150, 3, 1 );
	vehicle[18] = setVehicle( 5, 250, 3, 1 );
	vehicle[19] = setVehicle( 5, 350, 3, 3 );

	drawVehicles( );

	/* Configure the goals for frogger */
	goals[0] = setGoals( LEFT_SIDE + 3, 55, 43, 35 );
	goals[1] = setGoals( LEFT_SIDE + 91, 55, 43, 35 );
	goals[2] = setGoals( LEFT_SIDE + 179, 55, 43, 35 );
	goals[3] = setGoals( LEFT_SIDE + 267, 55, 43, 35 );
	goals[4] = setGoals( LEFT_SIDE + 355, 55, 43, 35 );

	/*
  	 * Configure the left and right side black borders to conceal logs, 
  	 * turtles, cars, etc. that go past their boundries
  	 */
	leftBorderRect.x = 0;
	leftBorderRect.y = 0;
	leftBorderRect.w = LEFT_SIDE;
	leftBorderRect.h = SCREEN_HEIGHT;

	rightBorderRect.x = RIGHT_SIDE;
	rightBorderRect.y = 0;
	rightBorderRect.w = SCREEN_WIDTH - RIGHT_SIDE;
	rightBorderRect.h = SCREEN_HEIGHT;


	/* 
 	 * Draw frogger in starting position 
 	 */
	froggerReset( );
}

void drawGameScreen( void ) {
	/* 
 	 * Update frogger
 	 */
	if ( frogger.direction ) moveFrogger( );
	if ( frogger.riding ) ridingFrogger( );

	/* Check for collisions with frogger */
	if ( frogger.alive ) {
		if ( frogger.currentRow > 6 && frogger.currentRow < 12 ) {
			if ( ( ! collisionRow( ) ) || ( frogger.riding == FALSE ) ) {
				playSound( frogger.s_splash );
				frogger.deathType = SPLASH;
				fprintf( stderr, "D: Frog in water!!\n" );
				frogger.alive = FALSE; 
			}
		}
		else if ( frogger.currentRow == 12 ) {
			if ( collisionRow( ) ) {
				playSound( frogger.s_squash );
				frogger.deathType = SPLAT;
				fprintf( stderr, "D: Frog in thorn bushes!\n" );
				frogger.alive = FALSE;
			}
		}
		else if( collisionRow( ) ) {
			playSound( frogger.s_squash );
			frogger.deathType = SPLAT;
			fprintf( stderr, "D: Frog Squashed!\n" );
			frogger.alive = FALSE;
		}
	}

	/*
 	 * Update and draw everthing else
 	 */
	drawImage( background, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen, 0, 0, 255 );
	drawScore( 0, score );
	drawScore( 1, hScore );
	drawGoals( );
	drawLives( lives );
	drawLevel( level );
	drawWood( );
	drawTurtles( );
	drawVehicles( );

	if ( frogger.alive == FALSE ) {
		frogger.riding = FALSE;
		if ( ! drawDeathSequence( frogger.deathType ) ) {
			lives--;
			if ( lives < 0 ) { drawGameOver( ); }
			else { froggerReset( ); }	
		}
	}

	if ( frogger.alive )  {
		frogger.alive = checkTimer( );
		drawImage( gfx, frogger.src.x, frogger.src.y, frogger.src.w,
			   frogger.src.h, screen, frogger.dst.x, frogger.dst.y, 255 );
	}
	if ( ! debugBorder ) {
		SDL_FillRect( screen, &leftBorderRect,  SDL_MapRGB( screen->format, BGCOLOR ) );
		SDL_FillRect( screen, &rightBorderRect, SDL_MapRGB( screen->format, BGCOLOR ) );
	}

	SDL_Flip( screen );
}

void drawBackground( void ) {
        /*
         * Draw background map
         */
        backgroundRect.x = 0;
        backgroundRect.y = 0;
        SDL_BlitSurface( background, NULL, screen, &backgroundRect );
	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

/*
 * This calculates the pixel top of the requested row 
 */
int getRowPixel ( int row ) {
	return ROW_BASE - ( row * HOP_DISTANCE );
}

/*
 * This does collision detection based on the row frogger
 * is in to help reduce overhead 
 */
int collisionRow ( void ) {
	int i;

	if ( frogger.currentRow <= 0 ) return 0;

	/* Check collision with cars */
	if ( frogger.currentRow < 6 ) {
		for( i = 0; i < MAX_VEHICLES; i++ ) {
			if ( level >= vehicle[i].level ) {
				int length = ( vehicle[i].row == 5 ) ? FRAME * 2 : FRAME; /* Trucks */
				if ( collideFrogger( vehicle[i].placement[X],
						     vehicle[i].placement[Y], FRAME,
						     length ) ) {
					return 1;
				}
			}
		}
		return 0;
	}
	/* check for collisions with turtles, logs, etc.. */
	else if ( frogger.currentRow > 6 && frogger.currentRow < 12 ) {
		/* here a collision is good, else death */
		for( i = 0; i < MAX_TURTLES; i++ ) {
			if ( collideFrogger( turtle[i].placement[X],
					     turtle[i].placement[Y], FRAME,
					     FRAME * turtle[i].count ) ) {
				frogger.riding = ( turtle[i].diveStep == 3 ) ? FALSE : LEFT;
				frogger.ridingIdx = i;
				frogger.ridingType = TURTLE;
				return 1;
			}
		}		
		for( i = 0; i < MAX_WOOD; i++ ) {
			if ( collideFrogger( wood[i].placement[X],
					     wood[i].placement[Y], FRAME,
					     FRAME * wood[i].type ) ) {
				frogger.riding = RIGHT;
				frogger.ridingIdx = i;
				frogger.ridingType = LOG;
				return 1;
			}
		}
	}
	/* We're on the path, if the snake is active, check that */
	else if ( frogger.currentRow == 6 ) {
		frogger.riding = FALSE;  /*in case we hopped off a turtle */
	}
	/* This leaves the goal area only */
	else {
		for ( i = 0; i < MAX_GOALS; i++ ) {
			if ( collideFrogger( goals[i].x, goals[i].y, goals[i].w, goals[i].h ) ) {
				if ( goals[i].occupied ) return 1;
				goals[i].occupied++;
				/* playSound( s_goal ); */
				score += SCORE_GOAL;
				score += ( ( int ) ( timeLeft / 10 ) ) * 10;
				lives += freeFrog( score );
				froggerReset( );
				if ( checkGoals( ) ) levelUp( );
				return 0;
			}
		}
		return 1;
	}		

	return 0;
}

/* If the player gets enough points, award them a free frog */
int freeFrog ( int score ) {
	if ( givenFreeFrog ) return 0;
	if ( score >= SCORE_FREE_FROG ) {
		givenFreeFrog++;
		playSound( s_freeFrog );
		return 1;
	}
	return 0;
}

/* Check what frogger is colliding with */
int collideFrogger ( int x, int y, int h, int w ) {
	h++; w++;
	
        if ( ( frogger.placement[Y] >= ( y + h ) ) ||
             ( frogger.placement[X] >= ( x + w ) ) ||
             ( y >= ( frogger.placement[Y] + FRAME ) ) ||
             ( x >= ( frogger.placement[X] + FRAME ) ) ) {
                return( 0 );
        }
        return( 1 );
}

/* Check left and right borders */
void checkFroggerBorder( void ) {
	if ( frogger.placement[Y] - 5 >= getRowPixel( 0 ) ) {
		frogger.placement[Y] = frogger.oldPlacement[Y];
		frogger.currentRow = 0;
	}

	if ( ( frogger.placement[X] <= LEFT_SIDE ) ||
	     ( frogger.placement[X] + frogger.src.w >= RIGHT_SIDE ) ) {
		if ( ( frogger.currentRow == 0 ) ||
		     ( frogger.currentRow == 6 ) ) {
			frogger.placement[X] = frogger.oldPlacement[X];
		}
		else {
			frogger.alive = FALSE;
		}
	}
}

void levelUp ( void ) {
	int i;

	fprintf ( stderr, "Level %i beat!  ", level );

	level++;
	score += SCORE_LEVEL;
	lives += freeFrog( score );
	froggerReset( );
	/* Play sounds */

	/* Empty goals */
	for ( i = 0; i < MAX_GOALS; i++ ) goals[i].occupied = 0;

	/* Speed things up */
	vehicle[0].speed = level;
	vehicle[1].speed = level;
	vehicle[2].speed = level;
	vehicle[3].speed = level;
	
	fprintf (stderr, "Starting level %i!\n", level );
}

int checkGoals ( void ) {
	int savedFrogs = 0;
	int i;

	for ( i = 0; i < MAX_GOALS; i++ ) {
		if ( goals[i].occupied ) savedFrogs++;
	}

	drawGoals( );

	return ( savedFrogs >= 5 ) ? 1 : 0;
}

void froggerReset ( void ) {
	timeLeft = MAX_TIMER;

	frogger.placement[X] = FROGGER_START_X;
	frogger.placement[Y] = FROGGER_START_Y;
	frogger.oldPlacement[X] = FROGGER_START_X;
	frogger.oldPlacement[Y] = FROGGER_START_Y;
	frogger.hopCount   = 0;
	frogger.direction  = 0;
	frogger.currentRow = 0;
	frogger.alive      = TRUE;
	frogger.riding	   = FALSE;
	frogger.deathType  = 0; /* Death type SPLAT or SPLASH */
	frogger.deathCount = 0; /* death animation timer */

	frogger.src.y = 0; 
	frogger.src.x = 0;
	frogger.src.w = FRAME;
	frogger.src.h = FRAME;
	frogger.dst.y = frogger.placement[Y];
	frogger.dst.x = frogger.placement[X];

	drawImage( gfx, frogger.src.x, frogger.src.y, frogger.src.w,
		   frogger.src.h, screen, frogger.dst.x, frogger.dst.y, 255 );
}

/*
 * This actually moves frogger...  I need to come up with a better
 * algorithm for calculating the distance and time
 */
void moveFrogger( void ) {
	int currentFrame = 0;
	int x = 0;
	int y = 0;
	int h = FRAME;
	int w = FRAME;
	int frameLow  = HOP_SPEED / 3;
	int frameHigh = frameLow * 2;

	/* Determine which frame of frogger to display */
	if ( ( frogger.hopCount >= frameLow ) && ( frogger.hopCount <= frameHigh ) ) 
		currentFrame = FRAME;
	
	frogger.oldPlacement[Y] = frogger.placement[Y];
	frogger.oldPlacement[X] = frogger.placement[X];

	switch( frogger.direction ) {
		case UP:
			x = currentFrame;
			frogger.placement[Y] -= ( HOP_DISTANCE / HOP_SPEED );
			break;
		case DOWN:
			x = currentFrame + ( 4 * FRAME );
			frogger.placement[Y] += ( HOP_DISTANCE / HOP_SPEED );
			break;
		case LEFT:
			x = currentFrame + ( 6 * FRAME );
			frogger.placement[X] -= ( HOP_DISTANCE / HOP_SPEED );
			break;
		case RIGHT:
			x = currentFrame + ( 2 * FRAME );
			frogger.placement[X] += ( HOP_DISTANCE / HOP_SPEED );
			break;	
	}

	checkFroggerBorder( );

	/* select the frame to display */
	frogger.src.y = y;
	frogger.src.x = x;
	frogger.src.w = w;
	frogger.src.h = h;

	/* Set the old place to be erased */
	frogger.dst.y = frogger.oldPlacement[Y];
	frogger.dst.x = frogger.oldPlacement[X];

	SDL_FillRect( screen, NULL, SDL_MapRGB( screen->format, BGCOLOR ) );

	/* Place the new position */
	frogger.dst.y = frogger.placement[Y];
	frogger.dst.x = frogger.placement[X];

	frogger.hopCount++;

	if ( frogger.hopCount >= HOP_SPEED ) {
		frogger.hopCount  = 0;
		frogger.direction = FALSE;
		score += SCORE_HOP;
		lives += freeFrog( score );
	}
}

void ridingFrogger( void ) {
	int speed = 0;

	if ( frogger.hopCount > 0 ) return;

	switch( frogger.ridingType ) {
		case LOG:
			speed = wood[frogger.ridingIdx].speed;
			break;
		case TURTLE:
			speed = turtle[frogger.ridingIdx].speed + 2;
			break;
	}

	switch( frogger.riding ) {
		case LEFT:
			frogger.oldPlacement[X] = frogger.placement[X];
			frogger.placement[X] -= speed;
			frogger.dst.x = frogger.placement[X];
			break;
		case RIGHT:
			frogger.oldPlacement[X] = frogger.placement[X];
			frogger.placement[X] += speed;
			frogger.dst.x = frogger.placement[X];
			break;
	}

	checkFroggerBorder( );
}

void drawTitleScreen( void ) {
	SDL_Surface *introText;
	SDL_Color fontColor = { 123, 158, 53, 255 };
	int center = ( SCREEN_WIDTH / 2 ) - ( titleSurface->w / 2 );
	int i;
	char *txt[] = { "Press 1 for single player game",
		        "Press 2 for two player games",
		        "Press F for full screen mode",
		        "Press ESC to quit" };

	// drawBackground( );

	drawImage( titleSurface, 0, 0, titleSurface->w, 
		   titleSurface->h, screen, center, 100, 255 );

	for( i = 0; i <= 3; i++ ) {
		introText = TTF_RenderText_Solid( font, txt[i], fontColor );
		drawImage( introText, 0, 0, introText->w, introText->h, screen, 
			   140, 300 + ( i * introText->h ), 255 );
	}

	SDL_Flip( screen );
}

void drawPauseScreen( void ) {
	printf( "D: Draw Pause Screen\n" );

}

void drawGameOver( void ) {
	printf( "D: Game Over\n" );
}

void playSound( Mix_Chunk *sound ) {
	Mix_PlayChannel( -1, sound, 0 );
}

void setFullScreenMode( void ) {
	/* Lets give fullscreen mode a try */
	if ( ! fullscreen ) {
		screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, 16, SDL_HWSURFACE | SDL_FULLSCREEN );
		fullscreen = TRUE;
	}
	/* Switch back to window mode */
	else {
		screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, 16, SDL_HWSURFACE );
		fullscreen = FALSE;
	}

	printf( "D: Fullscreen : %i\n", fullscreen );
}

int drawDeathSequence( int deathType ) {
	int animDelay = 55;
	int animOffset = ( deathType == SPLAT ) ? 8 : 11;
	int dFrame = 0;

	if ( frogger.deathCount < 7 ) dFrame = 0;
	if ( frogger.deathCount >= 7 ) dFrame = 1;
	if ( frogger.deathCount >= 14) dFrame = 2;
	if ( frogger.deathCount > 20 ) animOffset = 12;

	frogger.deathCount++;
	if ( frogger.deathCount >= animDelay ) 
		return 0;  /* we're done with death */

	drawImage( gfx, FRAME * ( animOffset + dFrame ), 0, FRAME, FRAME, 
		   screen, frogger.placement[X], frogger.placement[Y], 255 );
	
	return 1;
}

/* draw green timer and return 0 if out of time */
int checkTimer( void ) {
	float lvl = level;
	int step = ( int ) ( lvl / 2 );
	if ( step < 1 ) step = 1;
	if ( step > 3 ) step = 3;
	timeLeft -= step;

	drawTimer( (int) ( ( timeLeft / MAX_TIMER ) * TIMER_SIZE ) );
	
	if ( timeLeft <= 0 ) return 0;
	return 1;
}

void drawScore( int high, int score ) {
	int x = 169;
	int y = 14;

	if ( score > hScore ) hScore = score;	
	if ( high ) x = 260;
		
	drawNumbers( score, x, y );		
}

void drawNumbers( int num, int x, int y ) {
	char numStr[6] = "00000";
	int i;

	/* Assume anything less than 50 pixels location is a score and
         * pad with '0'.
         */
	if ( y <= 15 ) sprintf( numStr, "%05i", num );
	else  	       sprintf( numStr, "%i", num );

	for ( i = 0; i <= 4; i++ ) {
		char c = numStr[i];
		int n = atoi( &c );

		drawImage( gfx, n * HFRAME, FRAME * 3 , HFRAME, FRAME, 
			   screen, x + ( i * HFRAME ) + 2, y, 255 );
	}
}

/* Normally this functions manages and draws the flys, gators and saved frogs
 * but if drawDebugRect below is turned on it will draw the rectangle
 * used for collision detectiong for debuggin purposes in timer green
 */
void drawGoals( void ) {
	int drawDebugRect = 0;
	int i;
	
	for ( i = 0; i < MAX_GOALS; i++ ) {
		if ( drawDebugRect ) {
			SDL_Rect d;
			d.x = goals[i].x;	
			d.y = goals[i].y;	
			d.w = goals[i].w;	
			d.h = goals[i].h;	
	
			SDL_FillRect( screen, &d, SDL_MapRGB( screen->format, TIMER_COLOR ) );
		}

		if ( goals[i].occupied ) 
			drawImage( gfx, FRAME * 15, 0, FRAME, FRAME, 
				   screen, goals[i].x + 13, goals[i].y + 5, 255 );
	}
}

void drawTimer( int length ) {
	SDL_Rect timerRect;

	timerRect.x = RIGHT_SIDE - 60 - length;
	timerRect.y = 465;
	timerRect.w = length;
	timerRect.h = 15;

	SDL_FillRect( screen, &timerRect,  SDL_MapRGB( screen->format, TIMER_COLOR ) );
}

void drawLives( int lives ) {
	int i;
	int lifeFroggerSize = 16;

	for( i = 0; i <= lives - 2; i++ ) {
		drawImage( gfx, FRAME * 11, FRAME * 2, lifeFroggerSize, FRAME,
			   screen, LEFT_SIDE + ( lifeFroggerSize * i ), 450, 255 );
	}
}

void drawLevel( int level ) {
	int i;
	int levelImageSize = 12;

	for( i = 0; i <= level - 1; i++ ) {
		drawImage( gfx, FRAME * 12, FRAME * 2, levelImageSize, FRAME,
			   screen, RIGHT_SIDE - levelImageSize - ( levelImageSize * i ),			   450, 255 );
	}
}

void drawWood ( void ) {
	int i;

        for ( i = 0; i < MAX_WOOD; i++ ) {
		if ( wood[i].placement[X] > ( RIGHT_SIDE + 5 ) )
			wood[i].placement[X] = LEFT_SIDE - wood[i].src.w - 5;
		wood[i].placement[X] += wood[i].speed;
		drawImage( gfx, wood[i].src.x, wood[i].src.y,
			wood[i].src.w, wood[i].src.h,
			screen, wood[i].placement[X],
			wood[i].placement[Y], 255 );
        }
}

void drawTurtles ( void ) {
	int i = 0;
	int n = 0;
	int animFrame = 0;

	for ( i = 0; i < MAX_TURTLES; i++ ) {
		/* This managed the turtles basic 3 frames of animation */
		animFrame = turtle[i].animStep;
		if ( turtle[i].animDelay >= TURTLE_ANIM_TIME ) {	
			turtle[i].animDelay = 0;
			turtle[i].animStep++;
			if ( turtle[i].animStep > 2 ) turtle[i].animStep = 0;
		}
		else {
			turtle[i].animDelay++;
		}

		/* If a set of turtles have dive capability, this enables that */
		if ( turtle[i].canDive ) {
			turtle[i].diveTime++;
			/* Check if turtle is diving */
			if ( turtle[i].diveStep > 0 ) {
				switch( turtle[i].diveStep ) {
					case 1:
						animFrame = 3;
						break;
					case 2: 
						animFrame = 4;
						break;
					case 4: 
						animFrame = 4;
						break;
					case 5: 
						animFrame = 3;	
						break;
					case 6:
						turtle[i].diveStep = 0;
						turtle[i].diveTime = 0;
						break;
					default:
						animFrame = 4;
						break;
				}
			
				if ( turtle[i].diveTime > DIVE_START_TIME + ( DIVE_PHASE_TIME * turtle[i].diveStep ) )
					turtle[i].diveStep++;
	
			}
			else {
				if ( turtle[i].diveTime > DIVE_START_TIME ) 
					turtle[i].diveStep++;
			}
		}

		/* Display out turtles */
		for ( n = 0; n <= ( turtle[i].count - 1 ); n++ ) {
			turtle[i].placement[X] -= turtle[i].speed;
			if ( turtle[i].placement[X] <= LEFT_SIDE - ( turtle[i].count * FRAME ) + 10 ) 
				turtle[i].placement[X] = RIGHT_SIDE + 10;
			if ( turtle[i].diveStep != 4 ) 
				drawImage( gfx, turtle[i].src.x + ( FRAME * animFrame ), 
				   turtle[i].src.y, turtle[i].src.w, turtle[i].src.h,
				   screen, turtle[i].placement[X] + ( ( FRAME + 3 ) * n ),
				   turtle[i].placement[Y], 255 );
		}
	}
}

void drawVehicles ( void ) {
	int i;

        for ( i = 0; i < MAX_VEHICLES; i++ ) {
		if ( vehicle[i].direction == RIGHT ) {	
			if ( vehicle[i].placement[X] > ( RIGHT_SIDE + 5 ) )
				vehicle[i].placement[X] = LEFT_SIDE - vehicle[i].src.w - 5;
			vehicle[i].placement[X] += vehicle[i].speed;
		}
		else {	
			if ( vehicle[i].placement[X] < ( LEFT_SIDE - 5 ) )
				vehicle[i].placement[X] = RIGHT_SIDE + vehicle[i].src.w + 5;
			vehicle[i].placement[X] -= vehicle[i].speed;
		}

		if ( level >= vehicle[i].level )
			drawImage( gfx, vehicle[i].src.x, vehicle[i].src.y,
				vehicle[i].src.w, vehicle[i].src.h,
				screen, vehicle[i].placement[X],
				vehicle[i].placement[Y], 255 );
        }
}

void drawImage( SDL_Surface *srcimg, int sx, int sy, int sw, int sh, 
		SDL_Surface *dstimg, int dx, int dy, int alpha ) {
  if ((!srcimg) || (alpha == 0)) return; 
  SDL_Rect src, dst;        

  src.x = sx;  src.y = sy;  src.w = sw;     src.h = sh;
  dst.x = dx;  dst.y = dy;  dst.w = src.w;  dst.h = src.h;

  if (alpha != 255) SDL_SetAlpha(srcimg, SDL_SRCALPHA, alpha); 
  SDL_BlitSurface(srcimg, &src, dstimg, &dst);                
}

int heartbeat ( void ) {
	int ticks;
	if ( level ) {
		if ( playing ) {
			ticks = updateGameState( );
			if ( ticks <= 0 ) ticks = 50;
			return ticks;		
		}
		else {
			drawPauseScreen( );
			return 500;
		}
	}
	else {
		drawTitleScreen( );
		return 500;
	}

	return 50;
}

/*
 * Main program starts here.  We'll init the video and audio
 * and then begin our main program loop here
 *
 */
int main ( int argc, char **argv ) {
	if ( mySDLInit( ) <= 0 ) {
		fprintf( stderr, "Failure to start froggix\n" );
		return 255;
	}

	beginGame( );

	SDL_Quit( );
	
	return 0;
}
