#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <stdio.h>

#define UPDATE 10
#define FIELD_WIDTH	10
#define FIELD_HEIGHT 20
#define BLOCK_SIZE 20
#define PIECE_SIZE 5
#define PIECE_DATA_SIZE (sizeof(int) * PIECE_SIZE * PIECE_SIZE)

typedef enum TColor_{
	NONE = 0,
	RED, 
	GREEN,
	YELLOW,
	BLUE,
	ORANGE,
	CYAN,
	PURPLE
}TColor;

typedef struct TPiece_{
	TColor color;
	int rotate;
	int data[PIECE_SIZE][PIECE_SIZE];
}TPiece;

TPiece spawnpieces[] = { 
					 { CYAN, 
					   1,
					  {{ 0, 0, 0, 0, 0},
					   { 0, 0, 1, 0, 0},
					   { 0, 0, 1, 0, 0},
					   { 0, 0, 1, 0, 0},
					   { 0, 0, 1, 0, 0}}
					 },

					{ BLUE,
					  1,
					  {{ 0, 0, 0, 0, 0},
					   { 0, 0, 0, 0, 0},
					   { 0, 1, 0, 0, 0},
					   { 0, 1, 1, 1, 0},
					   { 0, 0, 0, 0, 0}}
					},
					{ ORANGE,
					  1,
					  {{ 0, 0, 0, 0, 0},
					   { 0, 0, 0, 0, 0},
					   { 0, 0, 0, 1, 0},
					   { 0, 1, 1, 1, 0},
					   { 0, 0, 0, 0, 0}}
					},
					{ YELLOW,
					  0,
					  {{ 0, 0, 0, 0, 0},
					   { 0, 1, 1, 0, 0},
					   { 0, 1, 1, 0, 0},
					   { 0, 0, 0, 0, 0},
					   { 0, 0, 0, 0, 0}}
					},
					{ RED,
					  1,
					  {{ 0, 0, 0, 0, 0},
					   { 0, 1, 1, 0, 0},
					   { 0, 0, 1, 1, 0},
					   { 0, 0, 0, 0, 0},
					   { 0, 0, 0, 0, 0}}
					},
					{ GREEN,
					  1,
					  {{ 0, 0, 0, 0, 0},
					   { 0, 0, 1, 1, 0},
					   { 0, 1, 1, 0, 0},
					   { 0, 0, 0, 0, 0},
					   { 0, 0, 0, 0, 0}}
					},	
					{ PURPLE,
					  1,
					  {{ 0, 0, 0, 0, 0},
					   { 0, 0, 1, 0, 0},
					   { 0, 1, 1, 1, 0},
					   { 0, 0, 0, 0, 0},
					   { 0, 0, 0, 0, 0}}
					},							 
				  };

int big9logo[FIELD_HEIGHT][FIELD_WIDTH] = 
			   {{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 1, 1, 1, 1, 0, 0, 0 },
				{ 0, 1, 1, 0, 0, 0, 1, 1, 0, 0 },
				{ 0, 1, 0, 0, 0, 0, 0, 1, 1, 0 },
				{ 1, 1, 0, 0, 0, 0, 0, 0, 1, 0 },
				{ 1, 1, 0, 0, 0, 0, 0, 0, 1, 0 },
				{ 0, 1, 0, 0, 0, 0, 0, 0, 1, 0 },
				{ 0, 1, 1, 0, 0, 0, 0, 1, 1, 0 },
				{ 0, 0, 1, 1, 1, 1, 1, 1, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
				{ 0, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
				{ 0, 1, 1, 1, 1, 1, 1, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }};

/* Global variables */

Image* colorimages[8];
int field[FIELD_HEIGHT][FIELD_WIDTH];
TPiece *currentpiece;
Point currentpiecepos;

int quit = 0;
vlong score = 0;
int level = 1;
int isgamepaused = 0;
int isgamerunning = 0;
int isgameover = 0;

/* Forward declartions */

int iscurrentpiececolliding(void);
void currentpiecestuck(void);
void clearrows(void);
void startgame(void);



void
rotatepieceleft(TPiece *p){
	/* Don't rotate yellow piece */
	if(!p->rotate)
		return;

	/* Rotate the data right around center */
	int tmp[PIECE_SIZE][PIECE_SIZE];
	memset(tmp, 0, sizeof(int) * PIECE_SIZE * PIECE_SIZE);
	for(int i = 0; i < PIECE_SIZE; i++)
		for(int j = 0; j < PIECE_SIZE; j++)
			tmp[PIECE_SIZE - j - 1][i] = p->data[i][j];
	
	memcpy(p->data, tmp, sizeof(int) * PIECE_SIZE * PIECE_SIZE);
}

void
rotatepieceright(TPiece *p){
	/* Don't rotate yellow piece */
	if(!p->rotate)
		return;

	/* Rotate the data left around center */
	int tmp[PIECE_SIZE][PIECE_SIZE];
	memset(tmp, 0, sizeof(int) * PIECE_SIZE * PIECE_SIZE);
	for(int i = 0; i < PIECE_SIZE; i++)
		for(int j = 0; j < PIECE_SIZE; j++)
			tmp[j][PIECE_SIZE - i - 1] = p->data[i][j];
	
	memcpy(p->data, tmp, sizeof(int) * PIECE_SIZE * PIECE_SIZE);
}

void
rotateleft(void){
	if(!currentpiece) return;
	if(!isgamerunning) return;

	rotatepieceleft(currentpiece);
	if(iscurrentpiececolliding())
		rotatepieceright(currentpiece);
}

void 
rotateright(void){
	if(!currentpiece) return;
	if(!isgamerunning) return;

	rotatepieceright(currentpiece);
	if(iscurrentpiececolliding())
		rotatepieceleft(currentpiece);
}

void
teleportdown(void){
	if(!currentpiece) return;
	if(!isgamerunning) return;

	/* Move it down util it is stuck */
	while(!iscurrentpiececolliding()) currentpiecepos.y++;
	currentpiecepos.y--;
	currentpiecestuck();
}

void
moveleft(void){
	if(!currentpiece) return;
	if(!isgamerunning) return;

	currentpiecepos.x--;
	if(iscurrentpiececolliding())
		currentpiecepos.x++;
}

void
moveright(void){
	if(!currentpiece) return;
	if(!isgamerunning) return;

	currentpiecepos.x++;
	if(iscurrentpiececolliding())
		currentpiecepos.x--;
}

void
handlekbd(int kbdc){
	static i = 0;
	switch(kbdc){
		case 'q':
			quit = 1;
			break;
		case 'w':
			rotateleft();
			break;
		case 's':
			rotateright();
			break;
		case 'a':
			moveleft();
			break;
		case 'd':
			moveright();
			break;
		case 'e':
			teleportdown();
			break;
		case 'p':
			isgamepaused = !isgamepaused;
			isgamerunning = !isgamepaused;
			break;
		case 'n':
			startgame();
			break;
		default:
			break;
	}
}

Image*
eallocimage(Rectangle r, int repl, uint color){
	Image *tmp;
	tmp = allocimage(display, r, screen->chan, repl, color);
	if(tmp == nil)
		sysfatal("allocimage failed");

	return tmp;
}

void
setupcolors(void){
	Rectangle r = Rect(0, 0, 1, 1);
	colorimages[NONE] = eallocimage(r, 1, DWhite);
	colorimages[RED] = eallocimage(r, 1, DRed);
	colorimages[GREEN] = eallocimage(r, 1, DGreen);
	colorimages[YELLOW] = eallocimage(r, 1, DYellow);
	colorimages[BLUE] = eallocimage(r, 1, DBlue);
	colorimages[ORANGE] = eallocimage(r, 1, DDarkyellow);
	colorimages[CYAN] = eallocimage(r, 1, DCyan);
	colorimages[PURPLE] = eallocimage(r, 1, DPurpleblue);
}

void
clearscreen(void){
	draw(screen, screen->r, display->white, nil, ZP);
}

void
drawblock(int row, int col, int color){
	int y = row * BLOCK_SIZE;
	int x = col * BLOCK_SIZE;
	/* Block exclusive border */
	Rectangle r = Rect(x, y, x + BLOCK_SIZE, y + BLOCK_SIZE);
	Rectangle r2 = rectaddpt(r, screen->r.min);
	draw(screen, r2, colorimages[color], nil, ZP);
}

void 
drawgrid(void){
	/* Horizontal lines */
	for(int i = 0; i <= FIELD_HEIGHT; i++){
		Point p0 = Pt(0, i * BLOCK_SIZE);
		Point p1 = Pt(FIELD_WIDTH * BLOCK_SIZE, i * BLOCK_SIZE);
		line(screen, addpt(p0, screen->r.min), addpt(p1, screen->r.min), 0, 0, 0, display->black, ZP);
	}
	/* Vertial lines */
	for(int j = 0; j <= FIELD_WIDTH; j++){
		Point p0 = Pt(j * BLOCK_SIZE, 0);
		Point p1 = Pt(j * BLOCK_SIZE, FIELD_HEIGHT * BLOCK_SIZE);
		line(screen, addpt(p0, screen->r.min), addpt(p1, screen->r.min), 0, 0, 0, display->black, ZP);
	}
}

void
drawfield(void){
	for(int i = 0; i < FIELD_HEIGHT; i++){
		for(int j = 0; j < FIELD_WIDTH; j++){
			int color = field[i][j];
			if(color != NONE)
				drawblock(i, j, color);
		}
	}
}

void 
drawcurrentpiece(void){
	for(int i = 0; i < PIECE_SIZE; i++){
		for(int j = 0; j < PIECE_SIZE; j++){
			if(currentpiece->data[i][j])
				drawblock(currentpiecepos.y + i,
						  currentpiecepos.x + j,
						  currentpiece->color);
		}
	}
}

void
drawinfo(void){
	Point scorept = Pt(FIELD_WIDTH * BLOCK_SIZE + 10, 10);
	char scorestr[100];
	memset(scorestr, 0, 100);
	snprint(scorestr, 100, "Score: %d", score);
	
	if(isgamerunning)
		string(screen, addpt(screen->r.min, scorept), display->black, ZP, font, scorestr);

	Point lvlpt = addpt(scorept, Pt(0, 20));

	char levelstr[100];
	memset(levelstr, 0, 100);
	snprint(levelstr, 100, "Lvl: %d", level);

	if(isgamerunning)
		string(screen, addpt(screen->r.min, lvlpt), display->black, ZP, font, levelstr);

	char helpstr[100];
	memset(helpstr, 0, 100);
	if(isgamepaused)
		snprint(helpstr, 100, "** PAUSED **");
	else if(isgameover)
		snprint(helpstr, 100, "** GAME OVER ** Press n for new game");
	else if(!isgamerunning)
		snprint(helpstr, 100, "Rotate: w/s, Move: a/d, Fast: e, Pause: p, Press n to start.");
	
	Point helppt = Pt(10, FIELD_HEIGHT * BLOCK_SIZE + 10);
	
	string(screen, addpt(screen->r.min, helppt), display->black, ZP, font, helpstr);
}

void
drawscreen(void){
	drawfield();
	if(currentpiece)
		drawcurrentpiece();
	drawgrid();
	drawinfo();
}

void
currentpiecestuck(void){
	/* Copy piece to field */
	for(int i = 0; i < PIECE_SIZE; i++)
		for(int j = 0; j < PIECE_SIZE; j++)
			if(currentpiece->data[i][j])
				field[currentpiecepos.y + i][currentpiecepos.x + j] = 
							currentpiece->color;
		
	
	free(currentpiece);
	currentpiece = nil;
}

int
ispiececolliding(TPiece *p, Point pos){
	/* Check for collisions */
	for(int i = 0; i < PIECE_SIZE; i++)
		for(int j = 0; j < PIECE_SIZE; j++){
			int a = 0;

			int b = ( pos.y + i >= 0 &&
					  pos.x + j >= 0 &&
				      pos.y + i < FIELD_HEIGHT &&
				      pos.x + j < FIELD_WIDTH);

			/* Moved outside field? */
			a = p->data[i][j] && !b;

			/* Collision with other piece? */
			if(b)
				a |= p->data[i][j] &&
					field[pos.y + i][pos.x + j];
			
			if(a){
				return 1;
			}
		}

	return 0;
}

int
iscurrentpiececolliding(void){
	return ispiececolliding(currentpiece, currentpiecepos);
}


int 
isrowfull(int row){
	int b = 1;
	for(int i = 0; i < FIELD_WIDTH; i++)
		b &= (field[row][i] != 0);
	return b;
}

void
falldownrow(int row){
	for(int j = 0; j < FIELD_WIDTH; j++)
		field[0][j] = 0;

	for(int i = row; i > 0; i--)
		for(int j = 0; j < FIELD_WIDTH; j++)
			field[i][j] = field[i - 1][j];
}

void
clearrows(void){
	int ncleared = 0;
	int clearedrow = 1;
	while(clearedrow){
		clearedrow = 0;
		for(int i = FIELD_HEIGHT - 1; i >= 0; i--){
			if(isrowfull(i)){
				falldownrow(i);
				score += ++ncleared * 100;
				clearedrow = 1;
				break;
			}
		}
	}
}

void
movedownpiece(void){
	/* Try to move piece down */
	currentpiecepos.y++;
	if(iscurrentpiececolliding()){
		currentpiecepos.y--;
		currentpiecestuck();
	}
}

void
gameover(void){
	isgameover = 1;
	isgamerunning = 0;

	free(currentpiece);
	currentpiece = nil;
}

void
spawnnewpiece(void){
	clearrows();

	currentpiecepos = Pt(1,-1);
	currentpiece = malloc(sizeof(TPiece));
	memcpy(currentpiece, &spawnpieces[rand() % 7], sizeof(TPiece));

	/* If the piece is already stuck then it is game over */
	if(iscurrentpiececolliding())
		gameover();
}

void
dologic(vlong ticks){
	static vlong sincelastmove = 0;

	if(!isgamerunning) return;

	sincelastmove += ticks;

	if(score >= level * 500)
		level++;

	if(sincelastmove > 1e9/level){
		if(currentpiece)
			movedownpiece();
		else
			spawnnewpiece();

		sincelastmove = 0;
	}
	
}

void
showlogo(void){
	memset(field, 0, sizeof(int) * FIELD_HEIGHT * FIELD_WIDTH);
	for(int i = 0; i < FIELD_HEIGHT; i++)
		for(int j = 0; j < FIELD_WIDTH; j++)
			if(big9logo[i][j])
				field[i][j] = BLUE;
}

void
startgame(void){
	isgamepaused = 0;
	isgamerunning = 1;
	isgameover = 0;

	score = 0;
	level = 0;

	memset(field, 0, sizeof(int) * FIELD_HEIGHT * FIELD_WIDTH);
	spawnnewpiece();
}

void
main(int argc, char *argv[]){
	if(initdraw(0, 0, "9Tetris") < 0) 
		sysfatal("initdraw");
	einit(Emouse | Ekeyboard);

	setupcolors();
	showlogo();

	vlong currtime = nsec();
	vlong prevtime = 0;

	etimer(0, 10);
	Event ev;
	int e;
	while(!quit){
		clearscreen();
		drawscreen();
		flushimage(display, 1);

		e = event(&ev);
		switch(e){
			case Emouse:
				break;
			case Ekeyboard:
				handlekbd(ev.kbdc);
				break;
			default:
				break;
		}

		prevtime = currtime; 
		currtime = nsec();

		dologic(currtime - prevtime);
	}

	if(currentpiece)
		free(currentpiece);

	for(int i = 0; i < 8; i++)
		freeimage(colorimages[i]);

	closedisplay(display);

	exits(0);
}

void
eresized(int new){
	if(new && getwindow(display, Refmesg) < 0)
		fprint(2, "can't reattach to window");
}
