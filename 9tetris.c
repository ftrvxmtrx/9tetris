#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include <stdio.h>

enum {
	UPDATE = 10,
	FIELD_WIDTH	= 10,
	FIELD_HEIGHT = 20,
	BLOCK_SIZE = 20,
	PIECE_SIZE = 5,
	PIECE_DATA_SIZE = sizeof(int)*PIECE_SIZE*PIECE_SIZE,

	NONE = 0,
	RED,
	GREEN,
	YELLOW,
	BLUE,
	ORANGE,
	CYAN,
	PURPLE,
};

typedef struct Piece Piece;

struct Piece {
	int color;
	int rotate;
	int data[PIECE_SIZE][PIECE_SIZE];
};

Piece spawnpieces[] = {
	 { CYAN, 1, {
	 		{ 0, 0, 0, 0, 0},
			{ 0, 0, 1, 0, 0},
			{ 0, 0, 1, 0, 0},
			{ 0, 0, 1, 0, 0},
			{ 0, 0, 1, 0, 0},
		},
	},
	{ BLUE, 1, {
			{ 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0},
			{ 0, 1, 0, 0, 0},
			{ 0, 1, 1, 1, 0},
			{ 0, 0, 0, 0, 0},
		},
	},
	{ ORANGE, 1, {
			{ 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0},
			{ 0, 0, 0, 1, 0},
			{ 0, 1, 1, 1, 0},
			{ 0, 0, 0, 0, 0},
		},
	},
	{ YELLOW, 0, {
			{ 0, 0, 0, 0, 0},
			{ 0, 1, 1, 0, 0},
			{ 0, 1, 1, 0, 0},
			{ 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0},
		},
	},
	{ RED, 1, {
			{ 0, 0, 0, 0, 0},
			{ 0, 1, 1, 0, 0},
			{ 0, 0, 1, 1, 0},
			{ 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0},
		},
	},
	{ GREEN, 1, {
			{ 0, 0, 0, 0, 0},
			{ 0, 0, 1, 1, 0},
			{ 0, 1, 1, 0, 0},
			{ 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0},
		},
	},
	{ PURPLE, 1, {
			{ 0, 0, 0, 0, 0},
			{ 0, 0, 1, 0, 0},
			{ 0, 1, 1, 1, 0},
			{ 0, 0, 0, 0, 0},
			{ 0, 0, 0, 0, 0},
		},
	},
};

int big9logo[FIELD_HEIGHT][FIELD_WIDTH] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
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
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

Image *colorimages[8];
int field[FIELD_HEIGHT][FIELD_WIDTH];
Piece *currentpiece;
Point currentpiecepos;
Rectangle center;

vlong score;
int level = 1;
int isgamepaused;
int isgamerunning;
int isgameover;

/* Forward declarations */
int iscurrentpiececolliding(void);
void currentpiecestuck(void);
void clearrows(void);
void startgame(void);

void
rotatepieceleft(Piece *p)
{
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
rotatepieceright(Piece *p)
{
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
rotateleft(void)
{
	if(!currentpiece) return;
	if(!isgamerunning) return;

	rotatepieceleft(currentpiece);
	if(iscurrentpiececolliding())
		rotatepieceright(currentpiece);
}

void
rotateright(void)
{
	if(!currentpiece) return;
	if(!isgamerunning) return;

	rotatepieceright(currentpiece);
	if(iscurrentpiececolliding())
		rotatepieceleft(currentpiece);
}

void
teleportdown(void)
{
	if(!currentpiece) return;
	if(!isgamerunning) return;

	/* Move it down util it is stuck */
	while(!iscurrentpiececolliding()) currentpiecepos.y++;
	currentpiecepos.y--;
	currentpiecestuck();
}

void
moveleft(void)
{
	if(!currentpiece) return;
	if(!isgamerunning) return;

	currentpiecepos.x--;
	if(iscurrentpiececolliding())
		currentpiecepos.x++;
}

void
moveright(void)
{
	if(!currentpiece) return;
	if(!isgamerunning) return;

	currentpiecepos.x++;
	if(iscurrentpiececolliding())
		currentpiecepos.x--;
}

void
handlekbd(int kbdc)
{
	switch(kbdc){
	case Kdel:
		exits(nil);
		break;
	case 'q':
		rotateleft();
		break;
	case 'w':
		rotateright();
		break;
	case 'o':
		moveleft();
		break;
	case 'p':
		moveright();
		break;
	case ' ':
		teleportdown();
		break;
	case '\n':
		if(!isgamepaused && !isgamerunning){
			startgame();
		}else{
			isgamepaused = !isgamepaused;
			isgamerunning = !isgamepaused;
		}
		break;
	}
}

Image*
eallocimage(Rectangle r, int repl, uint color)
{
	Image *tmp;
	tmp = allocimage(display, r, screen->chan, repl, color);
	if(tmp == nil)
		sysfatal("allocimage failed");

	return tmp;
}

void
setupcolors(void)
{
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
clearscreen(void)
{
	draw(screen, screen->r, display->white, nil, ZP);
}

void
drawblock(int row, int col, int color)
{
	int y = row * BLOCK_SIZE;
	int x = col * BLOCK_SIZE;
	/* Block exclusive border */
	Rectangle r = Rect(x, y, x + BLOCK_SIZE, y + BLOCK_SIZE);
	Rectangle r2 = rectaddpt(r, center.min);
	draw(screen, r2, colorimages[color], nil, ZP);
}

void
drawgrid(void)
{
	/* Horizontal lines */
	for(int i = 0; i <= FIELD_HEIGHT; i++){
		Point p0 = Pt(0, i * BLOCK_SIZE);
		Point p1 = Pt(FIELD_WIDTH * BLOCK_SIZE, i * BLOCK_SIZE);
		line(screen, addpt(p0, center.min), addpt(p1, center.min), 0, 0, 0, display->black, ZP);
	}
	/* Vertial lines */
	for(int j = 0; j <= FIELD_WIDTH; j++){
		Point p0 = Pt(j * BLOCK_SIZE, 0);
		Point p1 = Pt(j * BLOCK_SIZE, FIELD_HEIGHT * BLOCK_SIZE);
		line(screen, addpt(p0, center.min), addpt(p1, center.min), 0, 0, 0, display->black, ZP);
	}
}

void
drawfield(void)
{
	for(int i = 0; i < FIELD_HEIGHT; i++){
		for(int j = 0; j < FIELD_WIDTH; j++){
			int color = field[i][j];
			if(color != NONE)
				drawblock(i, j, color);
		}
	}
}

void
drawcurrentpiece(void)
{
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
drawinfo(void)
{
	Point scorept = Pt(FIELD_WIDTH * BLOCK_SIZE + 10, 10);
	char scorestr[100];
	memset(scorestr, 0, 100);
	snprint(scorestr, 100, "Score: %zd", score);

	if(isgamerunning)
		string(screen, addpt(center.min, scorept), display->black, ZP, font, scorestr);

	Point lvlpt = addpt(scorept, Pt(0, 20));

	char levelstr[100];
	memset(levelstr, 0, 100);
	snprint(levelstr, 100, "Lvl: %d", level);

	if(isgamerunning)
		string(screen, addpt(center.min, lvlpt), display->black, ZP, font, levelstr);

	char helpstr[100];
	memset(helpstr, 0, 100);
	if(isgamepaused)
		snprint(helpstr, 100, "** PAUSED **");
	else if(isgameover)
		snprint(helpstr, 100, "** GAME OVER ** Press enter for new game");
	else if(!isgamerunning)
		snprint(helpstr, 100, "Rotate: q/w, Move: o/p, Fast: space, Start/Pause: enter");

	Point helppt = Pt(10, FIELD_HEIGHT * BLOCK_SIZE + 10);

	string(screen, addpt(center.min, helppt), display->black, ZP, font, helpstr);
}

void
drawscreen(void)
{
	drawfield();
	if(currentpiece)
		drawcurrentpiece();
	drawgrid();
	drawinfo();
}

void
currentpiecestuck(void)
{
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
ispiececolliding(Piece *p, Point pos)
{
	/* Check for collisions */
	for(int i = 0; i < PIECE_SIZE; i++)
		for(int j = 0; j < PIECE_SIZE; j++){
			int a;
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
iscurrentpiececolliding(void)
{
	return ispiececolliding(currentpiece, currentpiecepos);
}


int
isrowfull(int row)
{
	int b = 1;
	for(int i = 0; i < FIELD_WIDTH; i++)
		b &= (field[row][i] != 0);
	return b;
}

void
falldownrow(int row)
{
	for(int j = 0; j < FIELD_WIDTH; j++)
		field[0][j] = 0;

	for(int i = row; i > 0; i--){
		for(int j = 0; j < FIELD_WIDTH; j++)
			field[i][j] = field[i - 1][j];
	}
}

void
clearrows(void)
{
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
movedownpiece(void)
{
	/* Try to move piece down */
	currentpiecepos.y++;
	if(iscurrentpiececolliding()){
		currentpiecepos.y--;
		currentpiecestuck();
	}
}

void
gameover(void)
{
	isgameover = 1;
	isgamerunning = 0;

	free(currentpiece);
	currentpiece = nil;
}

void
spawnnewpiece(void)
{
	clearrows();

	currentpiecepos = Pt(1,-1);
	currentpiece = malloc(sizeof(Piece));
	memcpy(currentpiece, &spawnpieces[rand() % 7], sizeof(Piece));

	/* If the piece is already stuck then it is game over */
	if(iscurrentpiececolliding())
		gameover();
}

void
dologic(vlong ticks)
{
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
	for(int i = 0; i < FIELD_HEIGHT; i++){
		for(int j = 0; j < FIELD_WIDTH; j++){
			if(big9logo[i][j])
				field[i][j] = BLUE;
		}
	}
}

void
startgame(void)
{
	isgamepaused = 0;
	isgamerunning = 1;
	isgameover = 0;

	score = 0;
	level = 0;

	memset(field, 0, sizeof(int) * FIELD_HEIGHT * FIELD_WIDTH);
	spawnnewpiece();
}

void
main(int argc, char **argv)
{
	USED(argc, argv);

	if(initdraw(nil, nil, "9Tetris") < 0)
		sysfatal("initdraw");
	einit(Emouse | Ekeyboard);
	eresized(0);

	setupcolors();
	showlogo();

	vlong currtime = nsec();
	vlong prevtime;

	etimer(0, 10); /* FIXME: wat. */
	Event ev;
	int e;
	for(;;){
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
}

void
eresized(int new)
{
	int w, h;

	if(new && getwindow(display, Refmesg) < 0)
		sysfatal("can't reattach to window");

	w = (Dx(screen->r) - FIELD_WIDTH*BLOCK_SIZE) / 2;
	h = (Dy(screen->r) - FIELD_HEIGHT*BLOCK_SIZE) / 3;
	center = screen->r;
	center.min.x += w;
	center.max.x -= w;
	center.min.y += h;
	center.max.y -= h;
}
