#
# Makefile for Polyray using GNU C.
#

NAME=polyray
FLEX=flex
BISON=bison
#X11LIB=-L/usr/X11R6/lib -lX11
#OBJ=obj
OBJ=o

O1  = alight.$(OBJ) bezier.$(OBJ) blob.$(OBJ) bound.$(OBJ) box.$(OBJ)
O2  = builder.$(OBJ) cheight.$(OBJ) color.$(OBJ) cone.$(OBJ) csg.$(OBJ)
O3  = cylinder.$(OBJ) disc.$(OBJ) eval.$(OBJ) function.$(OBJ) glyph.$(OBJ)
O4  = gridded.$(OBJ) height.$(OBJ) hypertex.$(OBJ) ibm.$(OBJ) image.$(OBJ)
O5  = intersec.$(OBJ) io.$(OBJ) jpeg.$(OBJ) lexyy.$(OBJ) light.$(OBJ)
O6  = main.$(OBJ) mcube.$(OBJ) memory.$(OBJ) mfacet.$(OBJ) parabola.$(OBJ)
O7  = parametr.$(OBJ) particle.$(OBJ) pexper.$(OBJ) pic.$(OBJ) pngread.$(OBJ)
O8  = poly.$(OBJ) polynom.$(OBJ) psupport.$(OBJ) raw.$(OBJ) revolve.$(OBJ)
O9  = roots.$(OBJ) scan.$(OBJ) screen.$(OBJ) shade.$(OBJ) sheight.$(OBJ)
O10 = simplify.$(OBJ) sphere.$(OBJ) spline.$(OBJ) subdiv.$(OBJ) superq.$(OBJ)
O11 = sweep.$(OBJ) symtab.$(OBJ) texture.$(OBJ) torus.$(OBJ) trace.$(OBJ)
O12 = tri.$(OBJ) vector.$(OBJ) wavefrnt.$(OBJ) ytab.$(OBJ) urandomdat.$(OBJ) unixcompat.$(OBJ)

OBJS = $(O1) $(O2) $(O3) $(O4) $(O5) $(O6) $(O7) $(O8) $(O9) $(O10) $(O11) $(O12)

#
# This is added to the compile line if detailed memory debugging will be used
#
# DEBUG_MEM=-DDEBUG_POINTERS

#
# Use these definitions with Symantec C
#
#CC	=sc
#MDL	=x
#BIN_FLG =-c
#CFLAGS	=-m$(MDL) -ff -A -r -DDOS386 -Id:\sc\include
#CFLAGS	=-m$(MDL) -A -r -DDOS386
#OPTFLG	=-o -4
#LINKER	=link386
#$(NAME).exe : $(OBJS)
#	$(LINKER) @symantec.lnk

#
# Use these definitions with Watcom C
# Use Watcom suggested "performance" options. /oneatx
# Use /5r in place of /4r for a Pentium optimized compile.
# Use /fp5 to optimize for Pentium FPU 
#
#LIB	=d:\WATCOM\lib386
#CC	=wcc386
#LINKER	=wlink
#CFLAGS	=/mf /fpi87 /fp5 /zp4 /5r /w5
#CFLAGS	=/mf /fpi87 /fp5 /zp4 /5r
#OPTFLG	=/oneatx
#CFLAGS	=/mf /zp4 /4r
#$(NAME).exe : $(OBJS)
#	$(LINKER) @watcom.lnk

#
# Use these definitions with GNU C++
#
CC	=gcc
CFLAGS  =-Wall -c -O3
#-DDEBUG -DDEBUG_FN_CALLS
#-DEXPERIMENTALMATRIX (needs AVX,untested)
#-fomit-frame-pointer -Wall -c
#CFLAGS  =-c 
#-pg
#-pipe
BIN_FLG =
#public compile no debugging
#LFLAGS = -lm
LFLAGS	=-lssp 
#-g
#-pg -g
# public compile, no debugging
#OPT	= -lm -O2 -minline-all-stringops -funroll-all-loops -momit-leaf-frame-pointer -fomit-frame-pointer
OPT	= -lssp -O3 -march=native -fstack-protector
#-march=pentium 
# compile with debugging
#OPT    = -lm -g
#OPT    = -lm -Os -march=athlon-xp -ftracer -fomit-frame-pointer -fno-ident -pipe -fweb -falign-functions=64 -mfpmath=sse -malign-double -mmmx -msse2 -m3dnow -g
#OPT     = -lm -O2 -march=pentium4 -mfpmath=sse -malign-double -mavx -msse2 --fast-math -g
GOPT	= 
#-funroll-loops
OPTFLG	=$(OPT) $(GOPT)
$(NAME) : $(OBJS)
	$(CC) -o polyray.exe $(OBJS) $(X11LIB) $(LFLAGS)
#
#$(NAME).exe : $(OBJS)
#	$(CC) -o plygcc @gcc.lnk $(LFLAGS)
#	aout2exe plygcc
#	copy /b \tools\gcc\bin\go32.exe+plygcc plygcc.exe
#	del plygcc

# The default compilation is to do heavy optimization
.c.$(OBJ):
	$(CC) $(OPTFLG) $(CFLAGS) $(BIN_FLG) $(DEBUG_MEM) $<

#
# If any file dependencies change, then the following rules should
# change with them
#
alight.$(OBJ): alight.c alight.h defs.h memory.h bound.h vector.h intersec.h symtab.h shade.h roots.h

bezier.$(OBJ): bezier.c bezier.h defs.h io.h memory.h builder.h vector.h eval.h symtab.h ytab.h

blob.$(OBJ): blob.c blob.h defs.h memory.h io.h intersec.h symtab.h mcube.h vector.h bound.h roots.h

bound.$(OBJ): bound.c bound.h defs.h vector.h io.h memory.h symtab.h intersec.h light.h

box.$(OBJ): box.c box.h defs.h memory.h io.h intersec.h symtab.h scan.h vector.h bound.h box.h

builder.$(OBJ): builder.c builder.h defs.h ytab.h memory.h io.h image.h spline.h
	$(CC) $(CFLAGS) $(BIN_FLG) $(DEBUG_MEM) builder.c

cheight.$(OBJ): cheight.c height.h defs.h io.h memory.h intersec.h symtab.h scan.h vector.h bound.h eval.h builder.h pic.h

color.$(OBJ): color.c defs.h symtab.h

cone.$(OBJ): cone.c cone.h defs.h memory.h io.h intersec.h symtab.h scan.h vector.h bound.h cylinder.h

csg.$(OBJ): csg.c csg.h defs.h io.h memory.h vector.h symtab.h bound.h

cylinder.$(OBJ): cylinder.c cylinder.h defs.h memory.h io.h intersec.h symtab.h scan.h vector.h bound.h

disc.$(OBJ): disc.c disc.h defs.h memory.h io.h intersec.h symtab.h scan.h vector.h bound.h

#
# May need to turn off optimization when compiling eval.c using GCC - it
# seems to crash while rendering some data files.
#
eval.$(OBJ): eval.c eval.h defs.h ytab.h io.h vector.h pic.h intersec.h symtab.h trace.h

function.$(OBJ): function.c function.h defs.h memory.h io.h intersec.h symtab.h scan.h vector.h builder.h bound.h eval.h

glyph.$(OBJ): glyph.c glyph.h defs.h io.h memory.h intersec.h symtab.h scan.h vector.h roots.h

gridded.$(OBJ): gridded.c gridded.h defs.h io.h memory.h intersec.h symtab.h scan.h vector.h bound.h pic.h

height.$(OBJ): height.c height.h defs.h io.h memory.h intersec.h symtab.h scan.h vector.h bound.h eval.h builder.h pic.h

hypertex.$(OBJ): hypertex.c hypertex.h defs.h io.h memory.h intersec.h symtab.h mcube.h vector.h builder.h bound.h function.h eval.h shade.h trace.h

ibm.$(OBJ): ibm.c display.h defs.h io.h memory.h

image.$(OBJ): image.c image.h defs.h io.h memory.h vector.h symtab.h eval.h

intersec.$(OBJ): intersec.c intersec.h defs.h bound.h vector.h csg.h symtab.h shade.h roots.h

io.$(OBJ): io.c io.h memory.h

jpeg.$(OBJ): jpeg.c jpeg.h defs.h io.h memory.h

lexyy.$(OBJ): lexyy.c parse.h defs.h ytab.h io.h memory.h symtab.h

light.$(OBJ): light.c light.h defs.h io.h memory.h vector.h scan.h pic.h builder.h eval.h symtab.h intersec.h image.h

main.$(OBJ): main.c defs.h particle.h io.h memory.h psupport.h light.h scan.h screen.h display.h pic.h symtab.h bound.h

memory.$(OBJ): memory.c memory.h defs.h io.h

mcube.$(OBJ): mcube.c mcube.h defs.h io.h memory.h scan.h vector.h

mfacet.$(OBJ): mfacet.c mfacet.h defs.h vector.h

parabola.$(OBJ): parabola.c parabola.h defs.h io.h memory.h intersec.h symtab.h scan.h vector.h bound.h

parametr.$(OBJ): parametr.c parametr.h defs.h io.h memory.h symtab.h vector.h eval.h builder.h

particle.$(OBJ): particle.c particle.h defs.h io.h memory.h symtab.h vector.h psupport.h builder.h eval.h intersec.h

pexper.$(OBJ): pexper.c defs.h ytab.h io.h memory.h builder.h psupport.h symtab.h parse.h

pic.$(OBJ): pic.c pic.h defs.h io.h memory.h display.h jpeg.h

pngread.$(OBJ): pngread.c pngread.h pngerrs.h defs.h io.h memory.h jpeg.h

poly.$(OBJ): poly.c poly.h defs.h io.h memory.h intersec.h symtab.h scan.h vector.h roots.h

polynom.$(OBJ): polynom.c polynom.h defs.h io.h memory.h intersec.h symtab.h scan.h vector.h bound.h roots.h

psupport.$(OBJ): psupport.c psupport.h defs.h ytab.h io.h parse.h memory.h builder.h symtab.h eval.h pic.h bound.h blob.h polynom.h torus.h roots.h csg.h texture.h simplify.h vector.h

raw.$(OBJ): raw.c raw.h defs.h scan.h io.h memory.h intersec.h symtab.h vector.h bound.h

revolve.$(OBJ): revolve.c revolve.h defs.h scan.h io.h memory.h intersec.h symtab.h vector.h bound.h roots.h cone.h

roots.$(OBJ): roots.c roots.h defs.h vector.h io.h

# urandomdat.c generated by randomtable (randomtable.c)
urandomdat.$(OBJ): urandomdat.c defs.h

#
# May need to turn off optimization when compiling scan.c using GCC - it
# seems to lock up for some data files.
#
scan.$(OBJ): scan.c scan.h defs.h memory.h io.h display.h vector.h eval.h csg.h symtab.h shade.h

#	$(CC) $(CFLAGS) $(BIN_FLG) $(DEBUG_MEM) scan.c

screen.$(OBJ): screen.c screen.h defs.h vector.h io.h memory.h trace.h pic.h display.h io.h symtab.h

shade.$(OBJ): shade.c shade.h defs.h light.h trace.h symtab.h intersec.h vector.h

sheight.$(OBJ): sheight.c height.h defs.h io.h memory.h intersec.h symtab.h scan.h vector.h bound.h eval.h builder.h pic.h

simplify.$(OBJ): simplify.c simplify.h defs.h ytab.h io.h memory.h builder.h roots.h eval.h

sphere.$(OBJ): sphere.c sphere.h defs.h io.h memory.h intersec.h symtab.h scan.h vector.h bound.h

spline.$(OBJ): spline.h defs.h io.h vector.h eval.h ytab.h

subdiv.$(OBJ): subdiv.c subdiv.h defs.h io.h memory.h scan.h vector.h eval.h symtab.h

superq.$(OBJ): superq.c superq.h defs.h io.h memory.h intersec.h symtab.h scan.h vector.h bound.h

sweep.$(OBJ): sweep.c sweep.h defs.h scan.h io.h memory.h intersec.h vector.h bound.h symtab.h roots.h

symtab.$(OBJ): symtab.c symtab.h defs.h particle.h bound.h glyph.h sweep.h display.h memory.h io.h eval.h builder.h csg.h scan.h texture.h

# Have to turn off unroll-loops, since gcc's memory usage explodes.
# seems to be a bug in the 2.6.4 beta I'm using
#	$(CC) $(OPT) $(CFLAGS) $(BIN_FLG) $(DEBUG_MEM) $<

texture.$(OBJ): texture.c texture.h defs.h ytab.h io.h memory.h light.h builder.h eval.h mfacet.h symtab.h vector.h shade.h

torus.$(OBJ): torus.c torus.h defs.h io.h memory.h intersec.h scan.h vector.h bound.h symtab.h roots.h

trace.$(OBJ): trace.c trace.h defs.h screen.h vector.h io.h intersec.h display.h eval.h shade.h symtab.h

tri.$(OBJ): tri.c tri.h defs.h scan.h io.h memory.h intersec.h scan.h vector.h bound.h symtab.h roots.h


vector.$(OBJ): vector.c vector.h defs.h io.h memory.h

wavefrnt.$(OBJ): wavefrnt.c wavefrnt.h defs.h io.h memory.h symtab.h subdiv.h raw.h

#
# Always turn off optimization in ytab.c - it's basically a huge case
# statement driven by data tables.  No noticible speed gain when it's
# optimized.  On the other hand, a huge amount of time is spent compiling it
# if optimization is turned on
#
#lex.yy.c: polyray.l
#	$(FLEX) -l polyray.l
#y.tab.c: polyray.y
#	$(BISON) polyray.y
ytab.$(OBJ): ytab.c defs.h particle.h io.h memory.h psupport.h symtab.h texture.h light.h parse.h builder.h eval.h vector.h bezier.h blob.h box.h cone.h csg.h cylinder.h disc.h function.h glyph.h gridded.h height.h parabola.h poly.h polynom.h revolve.h sphere.h sweep.h torus.h tri.h
	$(CC) $(CFLAGS) $(BIN_FLG) $(DEBUG_MEM) ytab.c

