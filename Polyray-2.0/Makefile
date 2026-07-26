#
# Makefile for Polyray using GNU C++.
#

NAME=polyray
FLEX=flex
BISON=bison
#X11LIB=-L/usr/X11R6/lib -lX11
#OBJ=obj
OBJ=o
CXXFLAGS=-no-pie
REFLEX_EXPECTED_MAJOR=4

O1  = alight.$(OBJ) bezier.$(OBJ) blob.$(OBJ) bound.$(OBJ) box.$(OBJ)
O2  = builder.$(OBJ) cheight.$(OBJ) colour.$(OBJ) cone.$(OBJ) csg.$(OBJ)
O3  = cylinder.$(OBJ) disc.$(OBJ) eval.$(OBJ) function.$(OBJ) factory.$(OBJ) glyph.$(OBJ)
O4  = gridded.$(OBJ) height.$(OBJ) hypertex.$(OBJ) display.$(OBJ) image.$(OBJ)
O5  = image_decoder.$(OBJ) intersec.$(OBJ) io.$(OBJ) jpeg.$(OBJ) lex.yy.$(OBJ) light.$(OBJ)
O6  = main.$(OBJ) mcube.$(OBJ) memory.$(OBJ) mfacet.$(OBJ) parabola.$(OBJ)
O7  = parametr.$(OBJ) particle.$(OBJ) pexper.$(OBJ) pic.$(OBJ) pngread.$(OBJ)
O8  = poly.$(OBJ) polynom.$(OBJ) psupport.$(OBJ) raw.$(OBJ) revolve.$(OBJ)
O9  = roots.$(OBJ) scan.$(OBJ) screen.$(OBJ) shade.$(OBJ) sheight.$(OBJ)
O10 = simplify.$(OBJ) sphere.$(OBJ) spline.$(OBJ) subdiv.$(OBJ) superq.$(OBJ)
O11 = sweep.$(OBJ) symtab.$(OBJ) texture.$(OBJ) torus.$(OBJ) trace.$(OBJ) 
O12 = util.$(OBJ) getopt.$(OBJ) vec2.$(OBJ) consolewin.$(OBJ) triangle.$(OBJ)
O13 = tri.$(OBJ) vector.$(OBJ) wavefrnt.$(OBJ) polyray.tab.$(OBJ) urandomdat.$(OBJ) unixcompat.$(OBJ)
O14 = ripple.$(OBJ) Enqueue3.$(OBJ) runtime_state.$(OBJ) lodepng.$(OBJ) polygonsplitcontext.$(OBJ)

OBJS = $(O1) $(O2) $(O3) $(O4) $(O5) $(O6) $(O7) $(O8) $(O9) $(O10) $(O11) $(O12) $(O13) $(O14)

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
CPP     =g++
CFLAGS  =-Wall -c -O3 -no-pie -g -DxTESTING -DxDEBUG -DixDEBUG_FN_CALLS -DxCOMPLETE_MEMORY_DEBUG -DxDEBUG_POINTERS  -Wextra -Werror=format=2 -Werror=missing-declarations -Werror=redundant-decls 
CPPFLAGS  =-Wall -c -O3 -no-pie -DxTESTING -std=c++23 -g -Iinclude/SDL3/include \
  -Iinclude/GSL/include \
  -Iinclude \
  -Wextra -Wshadow=local -Werror=format=2  -Werror=redundant-decls -fmax-errors=3 \
  -DxDEBUG -DxDEBUG_FN_CALLS -DxCOMPLETE_MEMORY_DEBUG -DxDEBUG_POINTERS 
CPPMODFLAGS =-Wall -fmodules-ts -g -DDEBUG -DDEBUG_FN_CALLS -DCOMPLETE_MEMORY_DEBUG \
 -DPOLYRAY_USE_MODULES -DDEBUG_POINTERS -Iinclude/SDL3/include -Iinclude/GSL/include \
 -std=c++23 -g -c -xc++
#-fomit-frame-pointer -Wall -c
#CFLAGS  =-c 
#-pg
#-pipe
BIN_FLG =
#public compile no debugging

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
CC     =gcc
CPP    =g++
CPPFLAGS  =-Wall -c -O3 -no-pie -std=c++23 -DUSE_SDL -g -I/usr/include/SDL3/ -Iinclude/GSL/include \
 -Iinclude \
 -Wextra -Wshadow=local -Werror=format=2 -Werror=missing-declarations -Werror=redundant-decls \
 -fsanitize=address \
# -fsanitize=integer-divide-by-zero \
# -fsanitize=leak \
# -fsanitize=undefined \
-DDEBUG -DDEBUG_FN_CALLS -DCOMPLETE_MEMORY_DEBUG -DDEBUG_POINTERS \
 
#-DUSE_SDL
CPPMODFLAGS =-Wall -fmodules-ts -g -DDEBUG -DxDEBUG_FN_CALLS -DCOMPLETE_MEMORY_DEBUG \
 -DDEBUG_POINTERS \
 -std=c++23 -g -c -I/usr/include/SDL3/ -Iinclude/GSL/include -Iinclude 
LDFLAGS=-flto -std=c++23 -L$$HOME/lib -L/usr/local/lib -L./benchmark/buildl/src \
 -fopenmp -lasan -lubsan -lSDL3 -lreflex  -g -fstack-protector \
  -fsanitize=address 
#-fsanitize=undefined -static-libasan \
#  -fsanitize=leak -fsanitize=integer-divide-by-zero
  # -lssp
else
ifeq ($(UNAME), SunOS) #only Tribblix has been tried
CC     =/opt/gcc/14.2.0/bin/gcc
CPP    =/opt/gcc/14.2.0/bin/g++
CPPFLAGS  =-Wall -c -O3 -no-pie -std=c++23 -g -Iinclude/GSL/include \
 -Iinclude \
 -I/usr/local/src/RE-flex-6.1.0/include \
 -Wextra -Wshadow=local -Werror=format=2 -Werror=missing-declarations -Werror=redundant-decls -Wfatal-errors \
-DDEBUG -DDEBUG_FN_CALLS -DCOMPLETE_MEMORY_DEBUG -DDEBUG_POINTERS \
 
#-DUSE_SDL
CPPMODFLAGS =-Wall -fmodules-ts -g -DDEBUG -DxDEBUG_FN_CALLS -DCOMPLETE_MEMORY_DEBUG \
 -DDEBUG_POINTERS \
 -std=c++23 -g -c -I/usr/include/SDL3/ -Iinclude/GSL/include -Iinclude 
LDFLAGS=-flto -std=c++23 -L$$HOME/lib -L/usr/local/lib -L./benchmark/buildl/src \
 -L/usr/local/src/RE-flex-6.1.0/lib -L/opt/gcc/14.2.0/lib \
 -R/opt/gcc/14.2.0/lib \
 -fopenmp -lreflex  -g -fstack-protector 
#-fsanitize=undefined -static-libasan \
#  -fsanitize=leak -fsanitize=integer-divide-by-zero
  # -lssp
else

ifeq ($(UNAME), Darwin)
CC     =gcc-16
CPP    =g++-16
unexport SDKROOT CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH
CPPFLAGS  =-Wall -c -O3 -no-pie -std=c++23 -DUSE_SDL -g -I/opt/homebrew/include \
 -I/opt/homebrew/include/SDL3 -Iinclude/GSL/include \
 -Iinclude -Wfatal-errors \
 -Wextra -Wshadow=local -Werror=format=2 -Werror=missing-declarations -Werror=redundant-decls \
 -I/usr/local/include -DDEBUG -DDEBUG_FN_CALLS -DCOMPLETE_MEMORY_DEBUG \
 -I${HOME}/opt/reflex-4.2.1/include \
 -DDEBUG_POINTERS -DPOLYRAY_USE_MODULES
CPPMODFLAGS =-Wall -fmodules-ts -DDEBUG -DDEBUG_FN_CALLS -DCOMPLETE_MEMORY_DEBUG \
 -DDEBUG_POINTERS -DPOLYRAY_USE_MODULES -I/opt/homebrew/include \
 -I/opt/homebrew/include/SDL3 -Iinclude/GSL/include \
 -Iinclude \
 -std=c++23 -g -c
# linking keg only using tip from https://9to5answer.com/libiconv-and-macos (solution 2)
LDFLAGS= -Wl,-ld_classic -L/opt/homebrew/opt/libiconv/lib -liconv \
  -L/Users/cmeli/opt/reflex-4.2.1/lib -L/opt/homebrew/lib -L/usr/lib \
  -liconv -L/usr/local/lib -lssp -lSDL3 -lreflex -g  
else
ifeq ($(UNAME),  MINGW64_NT-10.0-19045)
CPPFLAGS  =-Wall -c -O0 -no-pie -std=c++23 -DUSE_SDL -g -I/mingw64/include/SDL3/ -Iinclude/GSL/include \
 -Iinclude \
 -fmax-errors=1 \
 -Wextra -Wshadow=local -Werror=format=2 -Werror=missing-declarations -Werror=redundant-decls

LDFLAGS=-flto -L$$HOME/lib -L/usr/local/lib -L/mingw64/lib -lreflex -Wl,--stack,267772160 -fstack-protector \
 -msse -mavx -fopenmp -lmingw32 -static-libgcc -static-libstdc++ \
 -lSDL3 -lssp -g 
  #-mwindows -lssp -g 
else
LDFLAGS= -unrecognised_OS $(UNAME)
endif
endif
endif
endif

ifeq ($(UNAME), Darwin)
ifeq ($(findstring g++,$(CPP)),g++)
ifneq ($(findstring -lc++,$(LDFLAGS)),)
$(error Darwin build uses $(CPP) but LDFLAGS contains -lc++; remove -lc++ to avoid libc++/libstdc++ ABI mismatch)
endif
endif
endif


#-pg -g
# public compile, no debugging
#OPT	= -lm -minline-all-stringops -funroll-all-loops -momit-leaf-frame-pointer -fomit-frame-pointer
OPT	= -lssp -march=native -fstack-protector
#-march=pentium 
# compile with debugging
#OPT    = -lm -g
#OPT    = -lm -Os -march=athlon-xp -ftracer -fomit-frame-pointer -fno-ident -pipe -fweb -falign-functions=64 -mfpmath=sse -malign-double -mmmx -msse2 -m3dnow -g
#OPT     = -lm -O2 -march=pentium4 -mfpmath=sse -malign-double -mmmx -msse2 --fast-math -g
GOPT	= 
#-funroll-loops
OPTFLG	=$(OPT) $(GOPT)
$(NAME) : preflight $(OBJS)
	$(CPP) -o polyray.exe $(OBJS) $(X11LIB) $(LDFLAGS) 
#
#$(NAME).exe : $(OBJS)
#	$(CC) -o plygcc @gcc.lnk $(LDFLAGS)
#	aout2exe plygcc
#	copy /b \tools\gcc\bin\go32.exe+plygcc plygcc.exe
#	del plygcc

# The default compilation is to do heavy optimization
.c.$(OBJ):
	$(CC) $(OPTFLG) $(CFLAGS) $(BIN_FLG) $(DEBUG_MEM) $<
	
.cc.$(OBJ):
	$(CPP) $(OPTFLG) $(CPPFLAGS) $(BIN_FLG) $(DEBUG_MEM) $<
	
.cpp.$(OBJ):
	$(CPP) $(OPTFLG) $(CPPFLAGS) $(BIN_FLG) $(DEBUG_MEM) $<

#
# If any file dependencies change, then the following rules should
# change with them
#
alight.$(OBJ): alight.cc alight.h include/defs3.h memory.h bound.h vector.h intersec.h symtab.h shade.h roots.h runtime_state.h display_backend.h

bezier.$(OBJ): bezier.cc bezier.h include/defs3.h io_ply.h memory.h builder.h vector.h eval.h symtab.h polyray.tab.h

blob.$(OBJ): blob.cc blob.h include/defs3.h memory.h io_ply.h intersec.h symtab.h mcube.h vector.h bound.h roots.h

bound.$(OBJ): bound.cc bound.h include/defs3.h enqueue.h vector.h io_ply.h memory.h symtab.h intersec.h light.h  runtime_state.h display_backend.h

box.$(OBJ): box.cc box.h include/defs3.h memory.h io_ply.h intersec.h symtab.h scan.h vector.h bound.h box.h

builder.$(OBJ): builder.cc builder.h include/defs3.h polyray.tab.h memory.h io_ply.h image.h spline.h

cheight.$(OBJ): cheight.cc height.h include/defs3.h io_ply.h memory.h intersec.h symtab.h scan.h vector.h bound.h eval.h builder.h pic.h runtime_state.h display_backend.h

colour.$(OBJ): colour.cc include/defs3.h symtab.h

cone.$(OBJ): cone.cc cone.h include/defs3.h memory.h io_ply.h intersec.h symtab.h scan.h vector.h bound.h cylinder.h runtime_state.h display_backend.h

consolewin.$(OBJ): consolewin.cc consolewin.h display_backend.h include/defs3.h

csg.$(OBJ): csg.cc csg.h include/defs3.h io_ply.h memory.h vector.h symtab.h bound.h runtime_state.h display_backend.h

cylinder.$(OBJ): cylinder.cc cylinder.h include/defs3.h memory.h io_ply.h intersec.h symtab.h scan.h vector.h bound.h runtime_state.h display_backend.h

disc.$(OBJ): disc.cc disc.h include/defs3.h memory.h io_ply.h intersec.h symtab.h scan.h vector.h bound.h

#
# May need to turn off optimization when compiling eval.c using GCC - it
# seems to crash while rendering some data files.
#
eval.$(OBJ): eval.cc eval.h include/defs3.h polyray.tab.h io_ply.h vector.h pic.h intersec.h symtab.h trace.h runtime_state.h display_backend.h

factory.$(OBJ): factory.cc factory.h raw.h

function.$(OBJ): function.cc function.h include/defs3.h memory.h io_ply.h intersec.h symtab.h scan.h \
   vector.h builder.h bound.h eval.h mcube.h

getopt.$(OBJ): getopt.cc getopt.h

glyph.$(OBJ): glyph.cc glyph.h include/defs3.h io_ply.h memory.h intersec.h symtab.h scan.h vector.h roots.h runtime_state.h display_backend.h

gridded.$(OBJ): gridded.cc gridded.h include/defs3.h io_ply.h memory.h intersec.h symtab.h scan.h vector.h bound.h pic.h runtime_state.h display_backend.h

height.$(OBJ): height.cc height.h include/defs3.h io_ply.h memory.h intersec.h symtab.h scan.h vector.h bound.h eval.h builder.h \
  pic.h runtime_state.h display_backend.h

hypertex.$(OBJ): hypertex.cc hypertex.h include/defs3.h io_ply.h memory.h intersec.h symtab.h mcube.h vector.h builder.h bound.h \
  function.h eval.h shade.h trace.h runtime_state.h display_backend.h

display.$(OBJ): display.cc display.h include/defs3.h io_ply.h memory.h consolewin.h display_backend.h

image.$(OBJ): image.cc image.h include/defs3.h io_ply.h memory.h vector.h symtab.h eval.h runtime_state.h display_backend.h

image_decoder.$(OBJ): image_decoder.cc image_decoder.h include/defs3.h image.h jpeg.h pngread.h memory.h

intersec.$(OBJ): intersec.cc intersec.h include/defs3.h bound.h vector.h csg.h symtab.h shade.h roots.h runtime_state.h display_backend.h

io.$(OBJ): io.cc io_ply.h memory.h runtime_state.h util.h

jpeg.$(OBJ): jpeg.cc jpeg.h include/defs3.h io_ply.h memory.h

lex.yy.$(OBJ): lex.yy.cpp lex.yy.h include/defs3.h polyray.tab.h parse.h io_ply.h memory.h symtab.h unixcompat.h

light.$(OBJ): light.cc light.h alight.h include/defs3.h io_ply.h memory.h vector.h scan.h pic.h builder.h eval.h symtab.h \
    intersec.h image.h runtime_state.h display_backend.h

lodepng.$(OBJ): lodepng.cc lodepng.h

main.$(OBJ): main.cc include/defs3.h particle.h io_ply.h memory.h psupport.h light.h scan.h screen.h \
    display.h pic.h symtab.h bound.h getopt.h colour.h runtime_state.h display_backend.h

memory.$(OBJ): memory.cc memory.h include/defs3.h io_ply.h

mcube.$(OBJ): mcube.cc mcube.h include/defs3.h io_ply.h memory.h scan.h vector.h

mfacet.$(OBJ): mfacet.cc mfacet.h include/defs3.h vector.h

parabola.$(OBJ): parabola.cc parabola.h include/defs3.h io_ply.h memory.h intersec.h symtab.h scan.h vector.h bound.h

parametr.$(OBJ): parametr.cc parametr.h include/defs3.h io_ply.h memory.h symtab.h vector.h eval.h builder.h

particle.$(OBJ): particle.cc particle.h include/defs3.h io_ply.h memory.h symtab.h vector.h psupport.h \
  builder.h eval.h intersec.h runtime_state.h display_backend.h

pexper.$(OBJ): pexper.cc include/defs3.h polyray.tab.h io_ply.h memory.h builder.h psupport.h symtab.h parse.h

pic.$(OBJ): pic.cc pic.h include/defs3.h io_ply.h memory.h display.h jpeg.h runtime_state.h display_backend.h

pngread.$(OBJ): pngread.cc pngread.h include/defs3.h io_ply.h memory.h jpeg.h lodepng.h

poly.$(OBJ): poly.cc poly.h include/defs3.h io_ply.h memory.h intersec.h symtab.h scan.h vector.h roots.h

polynom.$(OBJ): polynom.cc polynom.h include/defs3.h io_ply.h memory.h intersec.h symtab.h scan.h vector.h \
   bound.h roots.h mcube.h

psupport.$(OBJ): psupport.cc psupport.h include/defs3.h polyray.tab.h io_ply.h parse.h memory.h builder.h symtab.h \
   eval.h pic.h bound.h blob.h polynom.h torus.h roots.h csg.h texture.h simplify.h vector.h runtime_state.h \
   display_backend.h

raw.$(OBJ): raw.cc raw.h include/defs3.h scan.h io_ply.h memory.h intersec.h symtab.h vector.h bound.h runtime_state.h runtime_state.h display_backend.h

revolve.$(OBJ): revolve.cc revolve.h include/defs3.h scan.h io_ply.h memory.h intersec.h symtab.h vector.h bound.h roots.h cone.h

ripple.$(OBJ): ripple.cc ripple.h vec3.h vector.h io_ply.h

Enqueue3.$(OBJ): Enqueue3.cc enqueue.h

#Enqueue3.cxx: Enqueue3.ixx
#	cp -f Enqueue3.ixx Enqueue3.cxx

test.$(OBJ): ../P19-GTest/test.cpp include/defs3.h
	$(CPP) -I/k/polyraysource/polyray19/Polyray-1.9 -I/usr/include/ $(CPPFLAGS) $(BIN_FLG) $(DEBUG_MEM) ../P19-GTest/test.cpp
	

#	$(CPP) $(OPTFLG) $(CPPMODFLAGS) $(BIN_FLG) $(DEBUG_MEM) Enqueue3.ixx -o enqueue.$(OBJ)

roots.$(OBJ): roots.cc roots.h include/defs3.h vector.h io_ply.h

unixcompat.$(OBJ): unixcompat.cc io_ply.h

util.$(OBJ): util.cc util.h


# urandomdat.c generated by randomtable (randomtable.c)
urandomdat.$(OBJ): urandomdat.cc urandomdat.h include/defs3.h

vec2.$(OBJ): vec2.cc vec2.h include/defs3.h 

#
# May need to turn off optimization when compiling scan.c using GCC - it
# seems to lock up for some data files.
#
scan.$(OBJ): scan.cc scan.h include/defs3.h memory.h io_ply.h display.h vector.h eval.h csg.h symtab.h shade.h runtime_state.h image.h util.h vec3.h factory.h

#	$(CC) $(CFLAGS) $(BIN_FLG) $(DEBUG_MEM) scan.c

screen.$(OBJ): screen.cc screen.h include/defs3.h vector.h io_ply.h memory.h trace.h pic.h display.h io_ply.h symtab.h runtime_state.h

shade.$(OBJ): shade.cc shade.h include/defs3.h light.h trace.h symtab.h intersec.h vector.h runtime_state.h

sheight.$(OBJ): sheight.cc height.h include/defs3.h io_ply.h memory.h intersec.h symtab.h scan.h vector.h bound.h eval.h \
   builder.h pic.h runtime_state.h

simplify.$(OBJ): simplify.cc simplify.h include/defs3.h polyray.tab.h io_ply.h memory.h builder.h roots.h eval.h

sphere.$(OBJ): sphere.cc sphere.h include/defs3.h io_ply.h memory.h intersec.h symtab.h scan.h vector.h bound.h runtime_state.h

spline.$(OBJ): spline.cc spline.h include/defs3.h io_ply.h vector.h eval.h polyray.tab.h

subdiv.$(OBJ): subdiv.cc subdiv.h include/defs3.h bound.h io_ply.h memory.h scan.h vector.h eval.h symtab.h runtime_state.h

superq.$(OBJ): superq.cc superq.h include/defs3.h io_ply.h memory.h intersec.h symtab.h scan.h vector.h bound.h

sweep.$(OBJ): sweep.cc sweep.h include/defs3.h scan.h io_ply.h memory.h intersec.h vector.h bound.h symtab.h roots.h

symtab.$(OBJ): symtab.cc symtab.h include/defs3.h particle.h bound.h glyph.h sweep.h \
  display.h memory.h util.h io_ply.h eval.h builder.h csg.h scan.h texture.h \
  display_backend.h runtime_state.h

texture.$(OBJ): texture.cc texture.h include/defs3.h polyray.tab.h io_ply.h memory.h light.h builder.h eval.h mfacet.h symtab.h vector.h shade.h runtime_state.h

torus.$(OBJ): torus.cc torus.h include/defs3.h io_ply.h memory.h intersec.h scan.h vector.h bound.h symtab.h roots.h runtime_state.h

trace.$(OBJ): trace.cc trace.h include/defs3.h screen.h vector.h io_ply.h intersec.h display.h eval.h shade.h symtab.h runtime_state.h

tri.$(OBJ): tri.cc tri.h include/defs3.h scan.h io_ply.h memory.h intersec.h scan.h vector.h bound.h symtab.h roots.h

triangle.$(OBJ): triangle.cc triangle.h io_ply.h

vector.$(OBJ): vector.cc vector.h include/defs3.h io_ply.h memory.h

wavefrnt.$(OBJ): wavefrnt.cc wavefrnt.h include/defs3.h bound.h io_ply.h memory.h symtab.h subdiv.h raw.h

runtime_state.$(OBJ): runtime_state.cc runtime_state.h include/defs3.h lex.yy.h

polygonsplitcontext.$(OBJ): polygonsplitcontext.cc include/defs3.h polygonsplitcontext.h

polyray.tab.$(OBJ): polyray.tab.cc include/defs3.h particle.h io_ply.h memory.h psupport.h symtab.h texture.h light.h \
    parse.h builder.h eval.h vector.h bezier.h blob.h box.h cone.h csg.h cylinder.h disc.h function.h glyph.h \
    gridded.h height.h parabola.h poly.h polynom.h revolve.h sphere.h sweep.h torus.h tri.h runtime_state.h

release: polyray-beta1.10.tar.bz2

polyray-beta1.10.tar.bz2:
	tar cvf polyray-beta1.10.tar ./P19Tests *.cpp *.cc Makefile *.h polyray.l polyray.y
	bzip2 polyray-beta1.10.tar
	
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
#	$(CC) $(CFLAGS) $(BIN_FLG) $(DEBUG_MEM) ytab.cc

.PHONY: preflight check-reflex-version clean

# Remove build outputs so the next `make` is a full rebuild.
# Deletes only compiled artifacts; leaves all source in place, including the
# checked-in generated lexer/parser (lex.yy.cpp, polyray.tab.cc/.h) and
# Enqueue2.cxx.
clean:
	rm -f *.$(OBJ) polyray.exe
	rm -rf gcm.cache

preflight:
ifeq ($(UNAME), Darwin)
	@if [ ! -f "lex.yy.cpp" ]; then \
		echo "ERROR: lex.yy.cpp not found."; \
		exit 1; \
	fi
	@if ! grep -q "REFLEX_VERSION \"$(REFLEX_EXPECTED_MAJOR)\." lex.yy.cpp; then \
		echo "ERROR: lex.yy.cpp REFLEX_VERSION does not match expected major $(REFLEX_EXPECTED_MAJOR)."; \
		grep -m1 "REFLEX_VERSION" lex.yy.cpp || true; \
		exit 1; \
	fi
endif

check-reflex-version:
	@echo "Scanner version in lex.yy.cpp:"
	@grep -m1 "REFLEX_VERSION" lex.yy.cpp || true
	@echo "reflex tool version on PATH:"
	@reflex --version || true
