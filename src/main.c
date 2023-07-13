#include <gbdk/gbdecompress.h>
#include <gbdk/platform.h>
#include <rand.h>

#include "boardstate.h"
#if defined(GAMEBOY)
#include "sfx.h"
#endif

#include "sfxplayer.h"


#define BOARD_XOFF (DEVICE_SCREEN_WIDTH/2 - 8)
#define BOARD_YOFF (DEVICE_SCREEN_HEIGHT/2 - 8)
#define DXOFF DEVICE_SPRITE_PX_OFFSET_X
#define DYOFF DEVICE_SPRITE_PX_OFFSET_Y

extern unsigned char const cm_tiles[];
static signed char const xoff[] = {24, 12,  0, 12};
static signed char const yoff[] = {12,  0, 12, 24};
static signed char const shake_xoff[] = {0, 1, 0, -1, 0,  1, 0, -1};
static signed char const shake_yoff[] = {0, 0, 0,  0, 1, -1, 0, -1};

static unsigned char tbuf[32];
static unsigned char last_btn;
static char shake_duration;
static unsigned char rand_initted;

static void clear_screen(void);
static void do_logo(void);
static void do_title(void);
static void do_end(void);
static void draw_small_board(int);
static void draw_player(void);
static void main_update(void);
static void setup_system(void);
static unsigned char get_keys(void);

static unsigned char
get_keys(void)
{
	unsigned char keys = joypad();
#if defined(GAMEBOY)
	if (!((~keys) & (J_A | J_B | J_START | J_SELECT))) reset();
#endif
	return keys;
}

int
main(void)
{
	unsigned char i;
	setup_system();
	do_logo();
	while (1)
	{
		do_title();
		display_off();
		clear_screen();
		initialize_boards();
		shake_duration = 0;
		for (i = 16; i; --i)
		{
			draw_small_board(i-1);
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

static void
main_update(void)
{
	unsigned char keys = get_keys();
	unsigned char pressed = keys & ~last_btn;
	unsigned char x = 0;
	if (pressed & J_RIGHT) { x = 1; player_face = FACE_RIGHT; }
	if (pressed & J_UP)    { x = 1; player_face = FACE_UP; }
	if (pressed & J_LEFT)  { x = 1; player_face = FACE_LEFT; }
	if (pressed & J_DOWN)  { x = 1; player_face = FACE_DOWN; }
	if (pressed & J_B)
	{
		if (is_big)
		{
			if (set_cboard())
			{
				find_hole(boards[board[cboard] - 1]);
				is_big = 0;
			}
			else
			{
				sfx_set_sample(0, sfx00);
				shake_duration = 2;
			}
		}
		else
		{
			find_hole(board);
			is_big = 1;
		}
	}
	if ((pressed & J_A) || (x && !is_big))
	{
		x = (player_y<<2) + player_x;
		if (is_big)
		{
			if (set_cboard())
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
			else
			{
				sfx_set_sample(0, sfx00);
				shake_duration = 2;
			}
		}
		else
		{
			if (try_flip(boards[board[cboard] - 1]))
			{
				sfx_set_sample(0, sfx03);
				draw_small_board(cboard);
			}
			else
			{
				sfx_set_sample(0, sfx00);
				shake_duration = 2;
			}
		}
	}
	move_bkg(shake_xoff[shake_duration],
	         shake_yoff[shake_duration]);
	shake_duration -= !!(shake_duration);
	draw_player();
	last_btn = keys;
}

static void
update_sound(void)
{
	sfx_play_isr();
}

static void
setup_system(void)
{
#if !defined(GAMEBOY)
	unsigned char buf[4096];
#endif
	display_off();
	SPRITES_8x8;
	add_VBL(update_sound);
	sfx_sound_init();
	clear_screen();
#if defined(GAMEBOY)
	gb_decompress_bkg_data(0, cm_tiles);
#else
	gb_decompress(cm_tiles, buf);
	set_bkg_2bpp_data(0, 256, buf);
	set_sprite_2bpp_data(0, 128, buf + 2048);
#endif
}

static void
clear_screen(void)
{
	unsigned char i;
	for (i = 0; i < 4; ++i) move_sprite(i, 0, 0xF0U);
	fill_rect(0, 0, 32, 32, 0);
}

static void
draw_player(void)
{
	unsigned char pxb;
	unsigned char pyb;
#if defined(GAMEBOY)
#define PBASE 0x80U
#elif defined(MASTERSYSTEM) || defined(GAMEGEAR)
#define PBASE 0x00U
#endif
	set_sprite_tile(0, player_face | 0x50U | PBASE);
	set_sprite_tile(1, 0x00 | PBASE);
	set_sprite_tile(2, 0x20 | PBASE);
	set_sprite_tile(3, 0x10 | PBASE);
	set_sprite_tile(4, 0x30 | PBASE);
	if (is_big)
	{
		pxb = (player_x<<5) + (BOARD_XOFF<<3)+DXOFF;
		pyb = (player_y<<5) + (BOARD_YOFF<<3)+DYOFF;
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
		pxb = ((cboard&3)<<5) + (BOARD_XOFF<<3)+DXOFF;
		pyb = ((cboard>>2)<<5) + (BOARD_YOFF<<3)+DYOFF;
		pxb += player_x<<3;
		pyb += player_y<<3;
		move_sprite(0, pxb, pyb);
	}
}

static void
draw_small_board(int n)
{
	unsigned char const * b;
	unsigned char x = board[n];
	unsigned char i;
	if (!x) {
		fill_rect(4*(n&3) + BOARD_XOFF,
		          4*(n>>2) + BOARD_YOFF,
		          4, 4, 0);
		return;
	}
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
	set_bkg_tiles(4*(n&3) + BOARD_XOFF,
	              4*(n>>2) + BOARD_YOFF,
	              4, 4, tbuf);
}

static void
do_logo(void)
{
	unsigned char i;
	unsigned char j;
	unsigned char x = 0;
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
#if defined(MASTERSYSTEM)
	set_palette_entry(0, 0, 0x24U);
	set_palette_entry(0, 1, 0x18U);
	set_palette_entry(0, 2, 0x27U);
	set_palette_entry(0, 3, 0x3FU);
#elif defined(GAMEGEAR)
	set_palette_entry(0, 0, 0x840U);
	set_palette_entry(0, 1, 0x480U);
	set_palette_entry(0, 2, 0x84FU);
	set_palette_entry(0, 3, 0xFFFU);
#endif
	set_bkg_tiles(DEVICE_SCREEN_WIDTH/2 - 3,
	              DEVICE_SCREEN_HEIGHT/2 - 3,
	              6,4,tbuf);
	for (i = 12; i; --i)
	{
		tbuf[i] = " PINEBERRYF0X"[i] + 0x20;
	}
	set_bkg_tiles(DEVICE_SCREEN_WIDTH/2 - 5,
	              DEVICE_SCREEN_HEIGHT/2 + 2,
	              9,1,tbuf+1);
	set_bkg_tiles(DEVICE_SCREEN_WIDTH/2 - 2,
	              DEVICE_SCREEN_HEIGHT/2 + 3,
	              3,1,tbuf+10);
	for (i = 120; i; --i) wait_vbl_done();
}

static void
do_title(void)
{
	unsigned char keys = get_keys();
	unsigned char i;
	display_off();
	clear_screen();
	/* display stuff */
	for (i = 0; i < 16; ++i)
	{
		tbuf[i] = 0xC0 | ((i+1)&0x0F);
	}
	set_bkg_tiles(DEVICE_SCREEN_WIDTH/2 - 2,
	              DEVICE_SCREEN_HEIGHT/2 + 3,
	              4,4,tbuf);
	for (i = 0; i < 32; ++i)
	{
		tbuf[i] = ((i>>3)<<4) | (i&0x07);
	}
	set_bkg_tiles(DEVICE_SCREEN_WIDTH/2 - 4,
	              DEVICE_SCREEN_HEIGHT/2 - 5,
	              8,4,tbuf);
	set_bkg_tile_xy(DEVICE_SCREEN_WIDTH-2,
	                DEVICE_SCREEN_HEIGHT-1,0x38);
	set_bkg_tile_xy(DEVICE_SCREEN_WIDTH-1,
	                DEVICE_SCREEN_HEIGHT-1,0x39);
#if defined(MASTERSYSTEM)
	set_palette_entry(0, 0, 0x00U);
	set_palette_entry(0, 1, 0x04U);
	set_palette_entry(0, 2, 0x19U);
	set_palette_entry(0, 3, 0x2FU);
	set_palette_entry(1, 3, 0x3FU);
#elif defined(GAMEGEAR)
	set_palette_entry(0, 0, 0x000U);
	set_palette_entry(0, 1, 0x040U);
	set_palette_entry(0, 2, 0x484U);
	set_palette_entry(0, 3, 0x8FFU);
	set_palette_entry(1, 3, 0xFFFU)
#endif
	/* done making screen */
	HIDE_SPRITES;
	SHOW_BKG;
	DISPLAY_ON;
	i = 0;
	while(!(keys & (~last_btn) & (J_A | J_B | J_START)))
	{
		last_btn = keys;
		keys = get_keys();
		if (rand_initted) wait_vbl_done();
		++i;
	}
	last_btn = keys;
#if defined(GAMEBOY)
	if (!rand_initted) initrand(DIV_REG);
#else
	initrand(i);
#endif
}

static void
do_end(void)
{
	unsigned char i;
	unsigned char keys = last_btn;
	wait_vbl_done();
	sfx_set_sample(0, sfx01);
	for (i = 0; i < 15; ++i)
	{
		tbuf[i] = "C0NGRATULATI0NS"[i] + 0x20;
	}
	set_bkg_tiles(DEVICE_SCREEN_WIDTH/2-8,
	              DEVICE_SCREEN_HEIGHT/2-9,
	              15,1,tbuf);
	set_bkg_tiles(DEVICE_SCREEN_WIDTH/2-7,
	              DEVICE_SCREEN_HEIGHT/2+8,
	              15,1,tbuf);
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
