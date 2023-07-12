#ifndef BOARDSTATE_H
#define BOARDSTATE_H

enum {
	FACE_RIGHT = 0,
	FACE_UP,
	FACE_LEFT,
	FACE_DOWN,
};

extern unsigned char board[16];
extern unsigned char boards[15][16];
extern unsigned char cboard;
extern unsigned char is_big;
extern unsigned char player_face;
extern unsigned char player_x;
extern unsigned char player_y;
extern unsigned char solved;

unsigned char check_solved(void);
void find_hole(unsigned char *);
void initialize_boards(void);
void mksolvable(unsigned char *);
unsigned char set_cboard(void);
unsigned char try_flip(char *);

#endif
