#ifndef SFXPLAYER_H
#define SFXPLAYER_H

extern unsigned char const sfx00[];
extern unsigned char const sfx01[];
extern unsigned char const sfx02[];
extern unsigned char const sfx03[];

void sfx_sound_init(void);
void sfx_set_sample(unsigned char, unsigned char const *);
unsigned char sfx_play_isr(void);
#endif
