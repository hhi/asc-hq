SUBDIRS      = dialog  @MUSICDIR@
DIST_SUBDIRS = dialog music
# unitsets trrobj buildings icons sounds 
MAINTAINERCLEANFILES  = Makefile.in

EXTRA_DIST = $(srcdir)/* $(srcdir)/sounds/*

pkgdata_DATA 		= main.ascdat 
pkgdatadir 		= $(datadir)/games/@PACKAGE@

@maindatainclude@

main.ascdat: main.d
	cd $(srcdir) && touch main.d && @abs_top_builddir@/source/tools/linux/asc_mount \@files.txt @abs_builddir@/main.ascdat 

clean-local:
	rm -f main.ascdat main.d 


dist-hook:
	cp -pr $(srcdir)/buildings $(srcdir)/unitsets $(srcdir)/icons $(srcdir)/trrobj $(srcdir)/campaign $(distdir)
	cp -p `find $(srcdir) -maxdepth 1 -type f` $(distdir)
	rm -f $(distdir)/main.ascdat
	