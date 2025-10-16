#
# Half-Life mp_i386.so Makefile for Linux
#
# August 1999 by Leon Hartwig (jehannum@planethalflife.com)
#
# Based on the Q2 Makefile by Zoid
#

MODNAME=mp

ARCH=i386
CC=egcs
OBJDIR=obj

BASE_CFLAGS=-Dstricmp=strcasecmp -D_strnicmp=strncasecmp

#use these cflags to optimize it
CFLAGS=$(BASE_CFLAGS) -m486 -O6 -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations -malign-loops=2 \
	-malign-jumps=2 -malign-functions=2

#use these when debugging 
#CFLAGS=$(BASE_CFLAGS) -g

INCLUDEDIRS=-I../engine

LDFLAGS=-ldl

SHLIBEXT=so
SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) $(INCLUDEDIRS) -o $@ -c $<

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

$(OBJDIR)/%.o: %.cpp
	$(DO_CC)

SRC = \
	airtank.cpp animating.cpp animation.cpp bmodels.cpp buttons.cpp \
	cbase.cpp client.cpp combat.cpp crossbow.cpp crowbar.cpp doors.cpp \
	effects.cpp egon.cpp explode.cpp func_break.cpp func_tank.cpp \
	game.cpp gamerules.cpp gauss.cpp ggrenade.cpp globals.cpp glock.cpp \
	h_ai.cpp h_battery.cpp h_cycler.cpp h_export.cpp handgrenade.cpp \
	healthkit.cpp hornet.cpp hornetgun.cpp items.cpp lights.cpp \
	maprules.cpp mortar.cpp mp5.cpp mpstubb.cpp multiplay_gamerules.cpp \
	pathcorner.cpp plane.cpp plats.cpp player.cpp python.cpp rpg.cpp \
	satchel.cpp shotgun.cpp singleplay_gamerules.cpp skill.cpp sound.cpp \
	soundent.cpp spectator.cpp squeakgrenade.cpp subs.cpp \
	teamplay_gamerules.cpp triggers.cpp tripmine.cpp util.cpp weapons.cpp \
	world.cpp xen.cpp m4a1.cpp ak47.cpp

OBJ=$(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(SRC)))

$(MODNAME)_$(ARCH).$(SHLIBEXT) : neat $(OBJ)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) $(LDFLAGS) -o $@ $(OBJ)

neat:
	-mkdir $(OBJDIR)
clean:
	-rm -f $(OBJ)
	-rm -f $(MODNAME)_$(ARCH).$(SHLIBEXT)
spotless: clean
	-rmdir $(OBJDIR)
depend:
	$(CC) -MM $(INCLUDEDIRS) $(SRC)
