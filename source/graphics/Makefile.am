
MAINTAINERCLEANFILES	= Makefile.in

AM_CXXFLAGS		= @SDL_CFLAGS@ @SIGC_CFLAGS@  @LokiInclude@

noinst_LTLIBRARIES 	= libgraphics.la

libgraphics_la_SOURCES	= surface.cpp blitter.cpp drawing.cpp surface2png.cpp colorizer.cpp
libgraphics_la_LIBADD		= @SDL_LIBS@ -lpng -lz

## AM_CPPFLAGS = -I$(top_srcdir)/source/libs/paragui/include 

EXTRA_DIST=$(srcdir)/*.h  