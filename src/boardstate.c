#include "boardstate.h"

#ifdef __SDCC
#include <rand.h>
#else
#include <stdlib.h>
#endif

unsigned char board[16];
unsigned char boards[15][16];
unsigned char cboard;
unsigned char is_big;
unsigned char player_face;
unsigned char player_x;
unsigned char player_y;
unsigned char solved;

unsigned char
check_solved(void)
{
	unsigned char i;
	unsigned char j;
	for (i = 15; i; --i)
	{
		if (board[i] != ((i+1)&0x0F)) return 0;
	}
	for (i = 0; i < 15; ++i)
	{
		for (j = 15; j; --j)
		{
			if (boards[i][j] != ((j+1)&0x0F)) return 0;
		}
	}
	return 1;
}

void
find_hole(unsigned char const * b)
{
	unsigned char i;
	for (i = 0; i < 16; ++i)
	{
		if (b[i]) continue;
		player_x = i & 0x03;
		player_y = i >> 2;
		break;
	}
}

void
initialize_boards(void)
{
	char i;
	mksolvable(board);
	for (i = 15; i; --i) mksolvable(boards[i-1]);
	find_hole(board);
	is_big = 1;
	player_face = FACE_RIGHT;
	cboard = (player_y<<2)+player_x+1;
	solved = 0;
}

void
mksolvable(unsigned char * board)
{
	unsigned char parity = 0;
	unsigned char i;
	unsigned char j;
	unsigned char k;
	unsigned char x;
	/* fill */
	for (i = 0; i < 16; ++i) board[i] = i;
	/* shuffle */
	for (i = 15; i; --i)
	{
		j = rand() % (i + 1);
		x = board[i];
		board[i] = board[j];
		board[j] = x;
	}
	/* check solvability */
	for (i = 16; i; --i)
	{
		k = i - 1;
		x = board[k];
		if (!x)
		{
			parity ^= !!((k>>2) & 1);
			continue;
		}
		for (j = i; j; --j) parity ^= (board[j-1] > x);
	}
	if (!parity)
	{
		i = 15;
		j = 14;
		if (!board[i]) { --i; --j; }
		if (!board[j]) { --j; }
		x = board[i];
		board[i] = board[j];
		board[j] = x;
	}
}

unsigned char
set_cboard(void)
{
	if (player_face == FACE_RIGHT && player_x == 3) return 0;
	if (player_face == FACE_UP    && player_y == 0) return 0;
	if (player_face == FACE_LEFT  && player_x == 0) return 0;
	if (player_face == FACE_DOWN  && player_y == 3) return 0;
	cboard = (((player_y
	            - (player_face == FACE_UP)
	            + (player_face == FACE_DOWN))<<2)
	          + (player_x
	             - (player_face == FACE_LEFT)
	             + (player_face == FACE_RIGHT)));
	return 1;
}

unsigned char
try_flip(char * b)
{
	unsigned char x;
	unsigned char y;
	x = (player_y<<2) + player_x;
	if (player_face == FACE_RIGHT && player_x == 3) return 0;
	if (player_face == FACE_UP    && player_y == 0) return 0;
	if (player_face == FACE_LEFT  && player_x == 0) return 0;
	if (player_face == FACE_DOWN  && player_y == 3) return 0;
	y = (((player_y
	       - (player_face == FACE_UP)
	       + (player_face == FACE_DOWN))<<2)
	     + (player_x
	        - (player_face == FACE_LEFT)
	        + (player_face == FACE_RIGHT)));
	b[x] = b[y];
	b[y] = 0;
	player_y = y>>2;
	player_x = y&0x03;
	return 1;
}
