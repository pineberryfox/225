# call like "bmake MAKEOBJDIR=obj.gb" or similar
.SUFFIXES :
.SUFFIXES : .c .o .piko .bin
.PATH : $(.CURDIR)/src $(.CURDIR)/res
.PATH : $(.CURDIR)/VGM2GBSFX/src/sm83
.PATH : $(.CURDIR)/binjgb/docs

LCC  ?= /opt/toolchains/gbdk/bin/lcc
FXH = python3 $(.CURDIR)/VGM2GBSFX/utils/fxhammer2data.py -b0 -c
PORT = sm83:gb
.if $(.OBJDIR:T) == obj.gb
PORT = sm83:gb
RT = 225.gb
.elif $(.OBJDIR:T) == obj.sms
PORT = z80:sms
RT = 225.sms
.elif $(.OBJDIR:T) == obj.gg
PORT = z80:gg
RT = 225.gg
.endif
CFLAGS = --opt-code-speed -Wf--max-allocs-per-node5000
CFLAGS += -Wf--no-c-code-in-asm
CFLAGS += -Wf--fomit-frame-pointer -Wf--peep-asm
CFLAGS += -I$(.CURDIR)/VGM2GBSFX/include -I$(.OBJDIR)
LDFLAGS = -Wm-yoA -Wm-yn225
MKZIP = ditto -ck --norsrc

.MAIN : $(RT)

.c.o :
	$(LCC) -m$(PORT) $(CFLAGS) -o $(.TARGET) -c $(.IMPSRC)
.piko.bin :
	mk2bpp -C $(.IMPSRC) | nes2gb > $(.TARGET)
.bin.c :
	gbcompress --cout --varname=cm_$(.IMPSRC:T:R:C/-.*//) \
	$(.IMPSRC) $(.TARGET)

$(RT) :
	$(LCC) -m$(PORT) $(LDFLAGS) -o $(.TARGET) $(.ALLSRC:M*.o)
$(RT) : main.o boardstate.o sfxplayer.o
.if $(PORT) == sm83:gb
$(RT) : tiles.o
.else
$(RT) : tiles-semiinv.o
.endif
$(RT) : sfx00.o sfx01.o sfx02.o sfx03.o

sfx.h : sfx00.h sfx01.h sfx02.h sfx03.h
	cat $(.ALLSRC:M*.h) > $(.TARGET)
sfx00.o : sfx00.c
sfx01.o : sfx01.c
sfx02.o : sfx02.c
sfx03.o : sfx03.c
sfx00.h : hammered.sav
	$(FXH) -n0 -i $(.TARGET:T:R) -o $(.TARGET:R).c $(.ALLSRC:M*.sav)
sfx00.c : hammered.sav
	$(FXH) -n0 -i $(.TARGET:T:R) -o $(.TARGET) $(.ALLSRC:M*.sav)
sfx01.h : hammered.sav
	$(FXH) -n1 -i $(.TARGET:T:R) -o $(.TARGET:R).c $(.ALLSRC:M*.sav)
sfx01.c : hammered.sav
	$(FXH) -n1 -i $(.TARGET:T:R) -o $(.TARGET) $(.ALLSRC:M*.sav)
sfx02.h : hammered.sav
	$(FXH) -n2 -i $(.TARGET:T:R) -o $(.TARGET:R).c $(.ALLSRC:M*.sav)
sfx02.c : hammered.sav
	$(FXH) -n2 -i $(.TARGET:T:R) -o $(.TARGET) $(.ALLSRC:M*.sav)
sfx03.h : hammered.sav
	$(FXH) -n3 -i $(.TARGET:T:R) -o $(.TARGET:R).c $(.ALLSRC:M*.sav)
sfx03.c : hammered.sav
	$(FXH) -n3 -i $(.TARGET:T:R) -o $(.TARGET) $(.ALLSRC:M*.sav)
boardstate.o : boardstate.c boardstate.h
main.o : main.c boardstate.h sfx.h
tiles.o : tiles.c
tiles.c : tiles.piko
sfxplayer.o : sfxplayer.c

web.zip : simple.html simple.css simple.js binjgb.js binjgb.wasm 225.gb
	([ -d web ] || mkdir web) \
	&& cp $(.ALLSRC) web \
	&& mv web/simple.html web/index.html \
	&& sed -i '' \
	   -e "s/ROM_FILENAME =.*/ROM_FILENAME = '225.gb';/" \
	   -e 's/ENABLE_FAST_FORWARD =.*/ENABLE_FAST_FORWARD = false;/' \
	   -e 's/ENABLE_REWIND =.*/ENABLE_REWIND = false;/' \
	   -e 's/DEFAULT_PALETTE_IDX = 79/DEFAULT_PALETTE_IDX = 0/' \
	   web/simple.js \
	&& $(MKZIP) web web.zip

.PHONY : usage
usage : 225.gb
	romusage $(.ALLSRC:M*.gb) -smROM -sC -gA -sRp
