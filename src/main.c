#include "boardstate.h"
#include "sfx.h"

#include "sfxplayer.h"

#include <gbdk/gbdecompress.h>
#include <gb/gb.h>
#include <rand.h>

enum {
	FACE_RIGHT = 0,
	FACE_UP,
	FACE_LEFT,
	FACE_DOWN,
};
extern unsigned char const cm_tiles[];
static signed char const xoff[] = {24, 12,  0, 12};
static signed char const yoff[] = {12,  0, 12, 24};
static signed char const shake_xoff[] = {0, 1, 0, -1, 0,  1, 0, -1};
static signed char const shake_yoff[] = {0, 0, 0,  0, 1, -1, 0, -1};

static char tbuf[32];
static char board[16];
static char boards[15][16];
static char cboard;
static char is_big;
static unsigned char last_btn;
static char player_face;
static char player_x;
static char player_y;
static char shake_duration;
static char solved;
static char rand_initted;

static char check_solved(void);
static char set_cboard(void);
static char try_flip(char *);
static void clear_screen(void);
static void do_logo(void);
static void do_title(void);
static void do_end(void);
static void draw_empty_board(int);
static void draw_small_board(int);
static void draw_player(void);
static void find_hole(char const *);
static void initialize(void);
static void main_update(void);
static void setup_system(void);
static unsigned char get_keys(void);

static unsigned char
get_keys(void)
{
	unsigned char keys = joypad();
	if (!((~keys) & (J_A | J_B | J_START | J_SELECT))) reset();
	return keys;
}

int
main(void)
{
	char i;
	setup_system();
	do_logo();
	while (1)
	{
		do_title();
		display_off();
		clear_screen();
		initialize();
		for (i = 0; i < 16; ++i)
		{
			draw_small_board(i);
		}
		move_bkg(0,0);
		draw_player();
		SHOW_BKG;
		SHOW_SPRITES;
		DISPLAY_ON;
		do {
			main_update();
			solved = check_solved();
			wait_vbl_done();
		} while (!solved);
		do_end();
	}
}

static char
check_solved(void)
{
	char i;
	char j;
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

static void
main_update(void)
{
	unsigned char keys = get_keys();
	unsigned char pressed = keys & ~last_btn;
	unsigned char x;
	if (pressed & J_RIGHT) player_face = FACE_RIGHT;
	if (pressed & J_UP)    player_face = FACE_UP;
	if (pressed & J_LEFT)  player_face = FACE_LEFT;
	if (pressed & J_DOWN)  player_face = FACE_DOWN;
	if (pressed & J_B)
	{
		if (is_big && set_cboard())
		{
			find_hole(boards[board[cboard] - 1]);
			is_big = 0;
		}
		else if (!is_big)
		{
			find_hole(board);
			is_big = 1;
		}
	}
	if (pressed & J_A)
	{
		x = (player_y<<2) + player_x;
		if (is_big && set_cboard())
		{
			board[x] = board[cboard];
			board[cboard] = 0;
			mksolvable(boards[board[x] - 1]);
			draw_small_board(cboard);
			draw_small_board(x);
			find_hole(board);
			shake_duration = 4;
			sfx_set_sample(0, sfx02);
		}
		else if (!is_big)
		{
			try_flip(boards[board[cboard] - 1]);
			draw_small_board(cboard);
		}
	}
	move_bkg(shake_xoff[shake_duration],
	         shake_yoff[shake_duration]);
	shake_duration -= !!(shake_duration);
	draw_player();
	last_btn = keys;
}

static char
set_cboard(void)
{
	char ok = 1;
	if (player_face == FACE_RIGHT && player_x == 3) ok = 0;
	if (player_face == FACE_UP    && player_y == 0) ok = 0;
	if (player_face == FACE_LEFT  && player_x == 0) ok = 0;
	if (player_face == FACE_DOWN  && player_y == 3) ok = 0;
	if (!ok)
	{
		sfx_set_sample(0, sfx00);
		shake_duration = 2;
		return 0;
	}
	cboard = (((player_y
	            - (player_face == FACE_UP)
	            + (player_face == FACE_DOWN))<<2)
	          + (player_x
	             - (player_face == FACE_LEFT)
	             + (player_face == FACE_RIGHT)));
	return 1;
}

static char
try_flip(char * b)
{
	char x;
	char y;
	char ok = 1;
	x = (player_y<<2) + player_x;
	if (player_face == FACE_RIGHT && player_x == 3) ok = 0;
	if (player_face == FACE_UP    && player_y == 0) ok = 0;
	if (player_face == FACE_LEFT  && player_x == 0) ok = 0;
	if (player_face == FACE_DOWN  && player_y == 3) ok = 0;
	if (!ok)
	{
		sfx_set_sample(0, sfx00);
		shake_duration = 2;
		return 0;
	}
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
	sfx_set_sample(0, sfx03);
	return 1;
}

static void
find_hole(char const * b)
{
	char i;
	for (i = 0; i < 16; ++i)
	{
		if (b[i]) continue;
		player_x = i & 0x03;
		player_y = i >> 2;
		break;
	}
}

static void
update_sound(void)
{
	sfx_play_isr();
}

static void
setup_system(void)
{
	display_off();
	add_VBL(update_sound);
	sfx_sound_init();
	clear_screen();
	gb_decompress_sprite_data(0, cm_tiles);
}

static void
clear_screen(void)
{
	unsigned char i;
	unsigned char j;
	for (i = 0; i < 32; ++i)
	{
		for (j = 0; j < 32; ++j)
		{
			set_bkg_tile_xy(i, j, 0);
		}
	}
}

static void
initialize(void)
{
	char i;
	mksolvable(board);
	for (i = 15; i; --i)
	{
		mksolvable(boards[i-1]);
	}
	find_hole(board);
	is_big = 1;
	player_face = FACE_RIGHT;
	cboard = (player_y<<2)+player_x+1;
	shake_duration = 0;
	solved = 0;
}

static void
draw_player(void)
{
	unsigned char pxb;
	unsigned char pyb;
	set_sprite_tile(0, player_face);
	set_sprite_tile(1, 0x10);
	set_sprite_tile(2, 0x11);
	set_sprite_tile(3, 0x20);
	set_sprite_tile(4, 0x21);
	if (is_big)
	{
		pxb = (player_x<<5) + 24;
		pyb = (player_y<<5) + 24;
		move_sprite(1, pxb +  8, pyb + 8);
		move_sprite(2, pxb + 16, pyb + 8);
		move_sprite(3, pxb +  8, pyb + 16);
		move_sprite(4, pxb + 16, pyb + 16);
		move_sprite(0,
		            pxb + xoff[player_face],
		            pyb + yoff[player_face]);
	}
	else
	{
		pxb = ((cboard&3)<<5) + 24;
		pyb = ((cboard>>2)<<5) + 24;
		pxb += player_x<<3;
		pyb += player_y<<3;
		move_sprite(0, pxb, pyb);
	}
}

static void
draw_empty_board(int n)
{
	char i;
	for (i = 0; i < 16; ++i)
	{
		tbuf[i] = 0;
	}
	set_bkg_tiles(4*(n&3) + 2, 4*(n>>2) + 1, 4, 4, tbuf);
}

static void
draw_small_board(int n)
{
	char const * b;
	char x = board[n];
	char i;
	if (!x) { draw_empty_board(n); return; }
	b = boards[x - 1];
	for (i = 0; i < 16; ++i)
	{
		switch (b[i])
		{
		case 0:
			tbuf[i] = 0;
			break;
		case 6:
			tbuf[i] = 0x80 | x;
			break;
		case 7:
			tbuf[i] = 0xA0 | x;
			break;
		case 10:
			tbuf[i] = 0x90 | x;
			break;
		case 11:
			tbuf[i] = 0xB0 | x;
			break;
		default:
			tbuf[i] = 0xC0 | b[i];
			break;
		}
	}
	set_bkg_tiles(4*(n&3) + 2, 4*(n>>2) + 1, 4, 4, tbuf);
}

static void
do_logo(void)
{
	char i;
	char j;
	char x = 0;
	for (i = 0; i < 4; ++i)
	{
		for (j = 0; j < 6; ++j)
		{
			tbuf[x++] = (i<<4) + j + 10;
		}
	}
	wait_vbl_done();
	SHOW_BKG;
	HIDE_SPRITES;
	DISPLAY_ON;
	set_bkg_tiles(7,6,6,4,tbuf);
	for (i = 12; i; --i)
	{
		tbuf[i] = " PINEBERRYF0X"[i] + 0x20;
	}
	set_bkg_tiles(5,11,9,1,tbuf+1);
	set_bkg_tiles(8,12,3,1,tbuf+10);
	for (i = 120; i; --i) wait_vbl_done();
}

static void
do_title(void)
{
	unsigned char keys = get_keys();
	char i;
	display_off();
	clear_screen();
	/* display stuff */
	for (i = 0; i < 16; ++i)
	{
		tbuf[i] = 0xC0 | ((i+1)&0x0F);
	}
	set_bkg_tiles(8,12,4,4,tbuf);
	for (i = 0; i < 32; ++i)
	{
		tbuf[i] = ((i>>3)<<4) | (i&0x07);
	}
	set_bkg_tiles(6,4,8,4,tbuf);
	/* done making screen */
	HIDE_SPRITES;
	SHOW_BKG;
	DISPLAY_ON;
	while(!(keys & (~last_btn) & (J_A | J_B | J_START)))
	{
		last_btn = keys;
		keys = get_keys();
		if (rand_initted) wait_vbl_done();
	}
	last_btn = keys;
	if (!rand_initted) initrand(DIV_REG);
}

static void
do_end(void)
{
	char i;
	unsigned char keys = last_btn;
	wait_vbl_done();
	sfx_set_sample(0, sfx01);
	for (i = 0; i < 15; ++i)
	{
		tbuf[i] = "C0NGRATULATI0NS"[i] + 0x20;
	}
	set_bkg_tiles(2, 0,15,1,tbuf);
	set_bkg_tiles(3,17,15,1,tbuf);
	for (char i = 0; i < 30; ++i)
	{
		wait_vbl_done();
	}
	while (!(keys & (~last_btn) & (J_A | J_B | J_START)))
	{
		last_btn = keys;
		keys = get_keys();
		wait_vbl_done();
	}
	last_btn = keys;
}
