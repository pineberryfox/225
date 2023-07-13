#ifndef NOTES_H
#define NOTES_H
#define NOTE(x,n) (NOTE_ ## x + 12*n)
enum {
	NOTE_C  =  0,
	NOTE_Cs =  1,
	NOTE_Db =  1,
	NOTE_D  =  2,
	NOTE_Ds =  3,
	NOTE_Eb =  3,
	NOTE_E  =  4,
	NOTE_F  =  5,
	NOTE_Fs =  6,
	NOTE_Gb =  6,
	NOTE_G  =  7,
	NOTE_Gs =  8,
	NOTE_Ab =  8,
	NOTE_A  =  9,
	NOTE_As = 10,
	NOTE_Bb = 11,
	NOTE_B  = 12,
};
#endif
