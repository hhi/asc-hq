dnl Process this file with autoconf to produce a configure script.

AC_INIT(asc,2.6.3.0)

AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_CONFIG_MACRO_DIR([m4])

ACLOCAL_AMFLAGS="-I m4"

if test -f $srcdir/source/strtmesg.cpp; then
   dnl extracting the version number from the source code
   dnl (it has to be stored there to be accessible by the windows build too)
   asc_version=`cat $srcdir/source/strtmesg.cpp | awk -F\" '/asc_release=/{print \$2}' | sed 's/ASC//g'`
   PACKAGE_VERSION=$asc_version
else
   asc_version=$PACKAGE_VERSION
fi


AC_MSG_NOTICE([ASC version is $asc_version]) 

dnl the following would be better - if it was working...
dnl Why is abs_top_srcdir not available here? Is there an alternative?
dnl ac_version=`cat $(as_top_srcdir)/source/strtmesg.cpp | awk -F\" '/asc_release=/{print \$2}' | sed 's/ASC//g'`


AC_DISABLE_SHARED
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

dnl *************************************************************************
dnl Checks for programs.
dnl
AC_PROG_CC
AC_PROG_CXX

AC_CHECK_PROG(PERLFOUND,perl,yes,no)
if test "$PERLFOUND" = no; then
     AC_MSG_ERROR([Perl not found. It is required for this configure script.])
fi

AdditionalIncludeDir=""

gcc296test=yes
AC_ARG_ENABLE( gcc296-detection, AS_HELP_STRING([--disable-gcc296-detection],[Do not perform gcc 2.96 detection test]), gcc296test=no )
if test "$GCC" = yes; then
   gcc_version=`gcc --version`
   if test "X$gcc296test" = "Xyes"; then
      gcc_version296=`echo $gcc_version | grep 2.96`
      if test "X$gcc_version296" != "X"; then
         AC_MSG_ERROR(
              [[ GCC $gcc_version detected. This version of gcc is not an official release ]
               [ and does produce incorrect code. You may try to build ASC, but it will]
               [ most likely NOT run.]
               [ To build ASC, we recommend GCC >= 3.2 ]
               [ You can bypass this test with --disable-gcc296-detection, so you can build ]
               [ ASC and see for yourself. And perhaps even rewrite some parts of ASC to ]
               [ make them Redhat compatible ;) ]])
      fi

      gcc_version29=`echo $gcc_version | grep " 2.9"`
      if test "X$gcc_version29" != "X"; then
         AC_MSG_WARN([ GCC $gcc_version detected. We don't support GCC prior to version 3.0 any more. ])
      fi
   fi
fi



AC_SUBST(abs_top_srcdir)
AC_SUBST(abs_top_builddir)
AC_SUBST(top_builddir)
AC_SUBST(top_srcdir)

AC_LANG_SAVE
AC_LANG_CPLUSPLUS


AC_MSG_CHECKING(for working C++ compiler)
AC_TRY_COMPILE( , [ return 0; ],, compilerBroken=yes)

if test "x$compilerBroken" = xyes ; then
  AC_MSG_RESULT(no)
  AC_MSG_ERROR( No working C++ compiler found. Please install it! Typical package name is 'g++' )
else  
  AC_MSG_RESULT(yes)
fi


AC_CHECK_HEADERS( bitset,, 
              AC_MSG_ERROR( 
               [[ Header <bitset> not found. This is probably caused by having an outdated ] 
                [ C++ compiler / standard library. Please upgrade to a compiler that is ]
                [ ISO/IEC 14882 conformant (for example gcc 3.2) ]]))
AC_LANG_RESTORE


CFLAGS="$CFLAGS -D_UNIX_ -D_SDL_ "
CXXFLAGS="$CXXFLAGS -Wno-sign-compare -D_UNIX_ -D_SDL_"


AC_DEFUN([AC_LIB_MISSING], [
   AC_MSG_ERROR(
      [[ ASC requires the $1 library$3. ]
      [ If you have a distribution supplied $1 package installed,]
      [ perhaps the $1-devel package is missing. ] 
      [ If your distribution does not include the $1 library, $4you can get it from] 
      [ $2 ]])
])



dnl This requires a tweaked version of genparse - but if the command line options change at some point in time
dnl we should rather switch to a different library (like libpopt) 
AC_ARG_ENABLE( genparse, AS_HELP_STRING([--enable-genparse],[enables recreation of the command line parsers]), ENABLEGENPARSE=1 )
if test "X$ENABLEGENPARSE" = "X1"; then
   AC_CHECK_PROG(GENCLL,genparse,genparse)
   if test "X$GENCLL" = "Xgenparse"; then
      GENCL=clparser
   fi
   AC_SUBST([GENCL])
fi

dnl *************************************************************************
dnl Checks for libraries.
dnl

dnl now save current FLAGS - they are modified to allow SDL test progs to
dnl be compiled correctly
SAVE_CXXFLAGS=$CXXFLAGS
SAVE_CFLAGS=$CFLAGS
SAVE_LIBS=$LIBS



AC_ARG_ENABLE( serverinterface, AS_HELP_STRING([--enable-serverinterface],[enables usage of the server interface (experimental)]), SERVERINTERFACE=1 )
if test "X$SERVERINTERFACE" = "X1"; then
 LIBCURL_CHECK_CONFIG(yes,7.10.0,,AC_LIB_MISSING( curl, [http://curl.haxx.se/libcurl/] ))
else
 LIBCURL=""
 LIBCURL_CPPFLAGS=""
fi

AM_CONDITIONAL( [CURLCLIENT], [test "X$SERVERINTERFACE" = "X1"] )


AC_SUBST(LIBCURL)
AC_SUBST(LIBCURL_CPPFLAGS)
AC_SUBST(NETWORKLIB)


XVIDAVAIL=1

AC_CHECK_LIB( xvidcore,xvid_encore,,XVIDAVAIL=0)


if test "x$XVIDAVAIL" = x1; then 
   AC_MSG_CHECKING(XviD usage)
   AC_TRY_COMPILE( [#include <stdio.h>
                  #include <xvid.h> ], [
      void *m_xvidEncoderHandle;
      xvid_encore(m_xvidEncoderHandle, XVID_ENC_DESTROY, NULL, NULL);
   ],, XVIDAVAIL=0)
fi


dnl check for libSDL
SDL_VERSION=1.2.2
AM_PATH_SDL($SDL_VERSION,:, AC_LIB_MISSING( SDL, http://www.libsdl.org/, [ (version >= $SDL_VERSION)])) 

dnl set up new FLAGS
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

AC_CHECK_LIB( SDL_image,
              IMG_LoadJPG_RW,,
              AC_LIB_MISSING( SDL_image, http://www.libsdl.org/projects/SDL_image/ )) 

#AC_MSG_CHECKING("SDL_image.h")
#AC_TRY_COMPILE( [#include <SDL_image.h>],[return 0;],
#	      AC_MSG_ERROR([[ 
#ASC requires the SDL_image library. If you have a distribution supplied 
#SDL_image.RPM installed, perhaps the SDL_image-devel.RPM is missing. 
#If your distribution does not include the SDL_image library, you can get it 
#from http://www.libsdl.org/projects/SDL_image/ ]]) )
#AC_MSG_RESULT("yes")


AC_CHECK_PROG(pkgconfigfound,pkg-config,yes,no)
if test $pkgconfigfound != yes; then 
   AC_MSG_ERROR( You need pkg-config to build ASC. Please install it )
fi


PKG_CHECK_MODULES(LUA, [lua >= 5.1 ],, LUA_MISSING=1 )

if test "x$LUA_MISSING" = "x1"; then
   PKG_CHECK_MODULES(LUA, [lua5.1 >= 5.1 ],, AC_LIB_MISSING( LUA 5.1, http://www.lua.org/ ) )
fi

LUA_FLAGS=$LUA_CFLAGS

AC_SUBST(LUA_FLAGS)
AC_SUBST(LUA_LIBS)

AC_CHECK_LIB(SDL_mixer, 
             Mix_PlayChannelTimed,, 
             AC_LIB_MISSING( SDL_mixer, http://www.libsdl.org/projects/SDL_mixer/ ))

AC_CHECK_LIB( SDL_sound, Sound_Init,,SDLsoundmissing=y)


dnl checking for wxwidgets

AM_OPTIONS_WXCONFIG
reqwx=2.4.0
AM_PATH_WXCONFIG($reqwx, wxWin=1)
if test "$wxWin" != 1; then
	AC_MSG_ERROR([
		wxWidgets $reqwx or later must be installed on your system.
		
		If you have a distribution supplied wxWidgets package installed,
		perhaps the wxWidgets-devel package is missing. 
		
		The wxWidgets packages are sometimes also called wxGTK, if you can't
		find wxWidgets packages in your distribution, search for wxGTK 
		
		If your distribution really does not contain the wxWidgets library,
		you can download it from www.wxwidgets.org
 
		If you have it installed, but it isn't working, check that wx-config is in path, 
		the directory where wxWidgets libraries are installed (returned by
		'wx-config --libs' or 'wx-config --static --libs' command)
		is in LD_LIBRARY_PATH or equivalent variable and
		wxWidgets version is $reqwx or above.
		])
fi
 
CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
LIBS="$LIBS $WX_LIBS"
AC_SUBST(WX_LIBS)


OLDCPPFLAGS=$CPPFLAGS

CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"


AC_ARG_ENABLE( internal_SDL_sound, AS_HELP_STRING([--enable-internal_SDL_sound], [Don't use the system SDL_sound lib ]), forceinternalSDLsound=yes )
if test "x$forceinternalSDLsound" = x; then
   AC_CHECK_HEADER( SDL_sound.h,,SDLSOUNDHEADER=no )
   if test "X$SDLSOUNDHEADER" = "Xno" ; then
      CFLAGS="$CFLAGS -I/usr/local/include/SDL"
      CPPFLAGS="$CPPFLAGS -I/usr/local/include/SDL"
      
      dnl AC_CHECK_HEADER caches the result, so we must make it forget to do a new test 
      unset ac_cv_header_SDL_sound_h
      
      AC_CHECK_HEADER( SDL_sound.h,,SDLSOUNDHEADER2=no )
      if test "X$SDLSOUNDHEADER2" = "Xno" ; then
         SDLsoundmissing=y
      else
         AdditionalIncludeDir="$AdditionalIncludeDir -I/usr/local/include/SDL/"
         CFLAGS="$CFLAGS $AdditionalIncludeDir"
      fi
   fi
else
   SDLsoundmissing=y
fi
   
     
if test "X$SDLsoundmissing" = "Xy" ; then
   AC_MSG_WARN( SDL_sound not found. Using stripped down sound support, which only supports OGG )

   XIPH_PATH_OGG([OGGFOUND=Y])
   if test "x$OGGFOUND" = "x" ; then 
      AC_LIB_MISSING( Ogg, http://xiph.org/ogg/ )
   fi


   dnl Check for ogg
   XIPH_PATH_VORBIS
   if test "x$no_vorbis" = "x" ; then
      AdditionalIncludeDir="$AdditionalIncludeDir -I\$(abs_top_srcdir)/source/libs/sdl_sound "
      
      SDL_SOUND_LIB="\$(abs_top_builddir)/source/libs/sdl_sound/libSDL_sound_int.la"
      
      SdlSoundDir=sdl_sound
      AC_SUBST(SdlSoundDir)
   else
      AC_LIB_MISSING( Vorbis, http://xiph.org/vorbis/ )
   fi
else
   SDL_SOUND_LIB=-lSDL_sound
fi

AC_SUBST(SDL_SOUND_LIB)
AC_SUBST(VORBIS_LIBS)
AC_SUBST(VORBISFILE_LIBS)
AC_SUBST(VORBISENC_LIBS)
AC_SUBST(VORBIS_CFLAGS)
     
     
CPPFLAGS=$OLDCPPFLAGS
     


#AC_MSG_CHECKING("SDL_mixer.h")
#AC_TRY_COMPILE( [#include <SDL_mixer.h>],[return 0;],
#	      AC_MSG_ERROR( [[
#ASC requires the SDL_mixer library. If you have a distribution supplied 
#SDL_mixer.RPM installed, perhaps the SDL_mixer-devel.RPM is missing. 
#If your distribution does not include the SDL_mixer library, you can get it 
#from http://www.libsdl.org/projects/SDL_mixer/ ]]))
#AC_MSG_RESULT("yes")


#AC_CHECK_LIB( SDL_ttf, 
#              TTF_RenderText_Solid,, 
#	      AC_MSG_ERROR( [[
#ASC requires the SDL_ttf library. If you have a distribution supplied 
#SDL_ttf.RPM installed, perhaps the SDL_ttf-devel.RPM is missing. 
#If your distribution does not include the SDL_ttf library, you can get it 
#from http://www.libsdl.org/projects/SDL_ttf/ ]]))
#SDL_TTF_LDADD="-lSDL_ttf"
#AC_SUBST(SDL_TTF_LDADD)

#AC_MSG_CHECKING("SDL_ttf.h")
#AC_TRY_COMPILE( [#include <SDL_ttf.h>],[return 0;],
#	      AC_MSG_ERROR( [[
#ASC requires the SDL_ttf library. If you have a distribution supplied 
#SDL_ttf.RPM installed, perhaps the SDL_ttf-devel.RPM is missing. 
#If your distribution does not include the SDL_ttf library, you can get it 
#from http://www.libsdl.org/projects/SDL_ttf/ ]]))
#AC_MSG_RESULT("yes")


AC_LANG_CPLUSPLUS

AC_CHECK_HEADERS([boost/unordered_map.hpp],,AC_LIB_MISSING( Boost version >= 1.49, http://www.boost.org/, [(which may be splitted into several distinct libraries)]))

AC_CHECK_LIB( boost_regex-mt, main, BOOST_LIBS="-lboost_regex-mt", BOOSTERR=yes)
if test x$BOOSTERR = xyes ; then

  BOOSTERR=none
  AC_CHECK_LIB( boost_regex-gcc-mt, main, BOOST_LIBS="-lboost_regex-gcc-mt", BOOSTERR=yes)
  if test x$BOOSTERR = xyes ; then

    BOOSTERR=none
    AC_CHECK_LIB( boost_regex, main, BOOST_LIBS="-lboost_regex", BOOSTERR=yes)
    if test x$BOOSTERR = xyes ; then

      BOOSTERR=none
      AC_CHECK_LIB( boost_regex-gcc, main, BOOST_LIBS="-lboost_regex-gcc", BOOSTERR=yes)
      if test x$BOOSTERR = xyes ; then
         AC_LIB_MISSING( Boost, http://www.boost.org/, [( which may be splitted into several distinct libraries)])
      fi
    fi
  fi
fi


AC_SUBST(BOOST_LIBS)




dnl SigC++
PKG_CHECK_MODULES(SIGC, sigc++-2.0)

AC_SUBST(SIGC_LIBS)
AC_SUBST(SIGC_CFLAGS)





dnl ***********************************
dnl CHECKS FOR INCLUDED PARAGUI LIBRARY

ft2test=yes
AC_ARG_ENABLE(ft2test,
   AS_HELP_STRING([--disable-ft2test],[disable the freetype check]),
   ft2test=no
)

if test "$ft2test" = "yes"; then

   dnl Check for freetype2 with a reasonable fallback in case
   dnl freetype-config is missing.

   have_freetype=no
   AC_PATH_PROG(freetype_config,freetype-config,no)
   if test "$ac_cv_path_freetype_config" != "no"; then
      have_freetype=yes
      FREETYPE_CFLAGS="`$ac_cv_path_freetype_config --cflags`"
      FREETYPE_LIBS="`$ac_cv_path_freetype_config --libs`"
   else
      PKG_CHECK_MODULES([FREETYPE],freetype2,have_freetype=yes)
      
      if test "$have_freetype" != "yes"; then
         
         AC_CHECK_LIB(freetype,FT_Init_FreeType,have_freetype=yes)
         AC_CHECK_HEADER(freetype/ft2build.h,have_ft2build=yes)
         if test "$have_ft2build" != "yes"; then
            have_freetype="no"
         elif test "$have_freetype" = "yes"; then
            FREETYPE_LIBS="-lfreetype"
            ft2inc="no"
            AC_MSG_CHECKING(freetype2 include directory)
            if test -d /usr/local/include/freetype2; then
               ft2inc="/usr/local/include/freetype2"
            elif test -d /usr/include/freetype2; then
               ft2inc="/usr/include/freetype2"
            fi
            AC_MSG_RESULT($ft2inc)
            FREETYPE_CFLAGS="-I$ft2inc"
         fi
      fi
   fi

   if test x$have_freetype = xno; then
      AC_LIB_MISSING( FreeType2, http://www.freetype.org/)
   fi
fi


AC_CHECK_HEADER(expat.h,,AC_LIB_MISSING( Expat, http://expat.sourceforge.net))
AC_SEARCH_LIBS(XML_ParserCreate,expat,,AC_LIB_MISSING( Expat, http://expat.sourceforge.net))

AC_CHECK_PROG(zipfound,zip,yes,no)
if test $zipfound != yes; then 
   AC_MSG_ERROR( You need the zip program to build ASC. Please install it )
fi

EXPAT_INCLUDE=""
EXPAT_LIB="-lexpat"
EXPAT_SUBDIR=""
LIBEXPAT="-lexpat"

AC_SUBST(EXPAT_INCLUDE)
AC_SUBST(EXPAT_LIB)
AC_SUBST(EXPAT_SUBDIR)
AC_SUBST(LIBEXPAT)


dnl Check for physfs

PHYSFS_INCLUDE=""
PHYSFS_LIB="-lphysfs"
PHYSFS_SUBDIR=""
LIBPHYSFS="-lphysfs"

AC_CHECK_HEADER(physfs.h,,AC_LIB_MISSING( physfs, http://www.icculus.org/physfs))
AC_CHECK_LIB(physfs, PHYSFS_init,,AC_LIB_MISSING( physfs, http://www.icculus.org/physfs))

AC_CONFIG_SUBDIRS(src/physfs)

AC_SUBST(PHYSFS_INCLUDE)
AC_SUBST(PHYSFS_LIB)
AC_SUBST(PHYSFS_SUBDIR)
AC_SUBST(LIBPHYSFS)



dnl PARAGUI_CFLAGS="-I$prefix/include/paragui $SIGC_CFLAGS $STL_CFLAGS $FREETYPE_CFLAGS $SDL_CFLAGS $RTTI_FLAGS $EXTRA_FLAGS"
dnl PARAGUI_LIBS="-lparagui $SIGC_LIBS $SDL_LIBS $LIBEXPAT $FREETYPE_LIBS $EXTRA_LIBS $LIBSTDCPP"

AC_SUBST(FREETYPE_LIBS)


dnl ***********************************


AC_ARG_WITH(win32-build-path, AS_HELP_STRING([--with-win32-build-path=dir],[Path to the win32 build dir (for installer target only)]), inst_win32_build_dir="$withval", inst_win32_build_dir="")
AC_ARG_WITH(win32-DLL-path,   AS_HELP_STRING([--with-win32-DLL-path=dir],  [Path to the win32 DLL dir (for installer target only)]),   inst_win32_DLL_dir="$withval",   inst_win32_DLL_dir="")
AC_SUBST(inst_win32_build_dir)
AC_SUBST(inst_win32_DLL_dir)


dnl ***********************************


dnl restore old flags settings
CXXFLAGS="$SAVE_CXXFLAGS $FREETYPE_CFLAGS $AdditionalIncludeDir"


CFLAGS="$SAVE_CFLAGS $AdditionalIncludeDir"
LIBS="$SAVE_LIBS "


AC_CHECK_LIB( bz2, BZ2_bzReadOpen,, AC_LIB_MISSING(libbz2,http://www.bzip.org/,,[ which is VERY unlikely,] ))
AC_CHECK_HEADER( bzlib.h,, AC_LIB_MISSING(libbz2,http://www.bzip.org/,,[ which is VERY unlikely,] ))

BZIP2_LIBS=-lbz2
AC_SUBST(BZIP2_LIBS)




AC_CHECK_LIB( png, png_create_write_struct,, AC_LIB_MISSING(png,http://www.cdrom.com/pub/png/,,[ which is VERY unlikely,] ), -lz )

dnl *************************************************************************
dnl Checks for header files.
dnl


dnl Checks for some STL header files (which aren't available for gcc 2.95.3 ).
AC_CHECK_HEADERS( exception limits )

dnl Checks for other header files.
AC_HEADER_DIRENT
AC_HEADER_TIME


AC_DEFUN([AC_DEFINE_DIR], [
  prefix_NONE=
  exec_prefix_NONE=
  test "x$prefix" = xNONE && prefix_NONE=yes && prefix=$ac_default_prefix
  test "x$exec_prefix" = xNONE && exec_prefix_NONE=yes && exec_prefix=$prefix
dnl In Autoconf 2.60, ${datadir} refers to ${datarootdir}, which in turn
dnl refers to ${prefix}.  Thus we have to use `eval' twice.
  eval ac_define_dir="\"$2\""
  eval ac_define_dir="\"$ac_define_dir\""
  eval ac_define_dir="\"$ac_define_dir\""
  AC_SUBST($1, "$ac_define_dir")
  AC_DEFINE_UNQUOTED($1, "$ac_define_dir", [$3])
  test "$prefix_NONE" && prefix=NONE
  test "$exec_prefix_NONE" && exec_prefix=NONE
])

AC_DEFINE_DIR(GAME_DATADIR,$datadir/games/$PACKAGE, [The directory in which the data files are going to be installed])
dnl AC_DEFINE_UNQUOTED( GAME_DATADIR, "$game_datadir",  [The directory in which the data files are going to be installed])

dnl *************************************************************************
dnl Checks for typedefs, structures, and compiler characteristics.
dnl 
dnl We use our own AC_UNP_CHECK_TYPE macro, instead of AC_CHECK_TYPE,
dnl to #include more headers.  Our macro is defined in "aclocal.m4".
dnl
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME

dnl *************************************************************************
dnl Checks for library functions.
AC_CHECK_FUNCS(select)
AC_CHECK_FUNCS(itoa)
AC_CHECK_FUNCS(stricmp)

dnl c++ tests
AC_LANG_CPLUSPLUS

dnl GCC 2.95.3 came with a broken standard library

AC_MSG_CHECKING(broken std::string compare)
AC_TRY_COMPILE( [#include <string>], [
      std::string s = "Hello World";
      s.compare  ( 0, 0, "Hello");
],, string_compare_broken=yes)

if test "x$string_compare_broken" = x ; then
  AC_MSG_RESULT(no)
else
  CXXFLAGS="$CXXFLAGS -D_string_compare_broken_"
  AC_MSG_RESULT(yes)
fi

AC_MSG_CHECKING(broken c_str Unicode support)
AC_TRY_COMPILE( [#include <string>], [
   std::basic_string<wchar_t> test;
   test.c_str();
],, Unicode_broken=yes)

if test "x$Unicode_broken" = x ; then
  AC_MSG_RESULT(no)
else
  CXXFLAGS="$CXXFLAGS -D_UNICODE_BROKEN_"
  AC_MSG_RESULT(yes)
fi


AC_MSG_CHECKING(broken std::vector)
AC_TRY_COMPILE( [#include <vector>], [
      std::vector<int> s;
      s.push_back(1);
      s.at(0);
],, stdvector_broken=yes)

if test "x$stdvector_broken" = x ; then
  AC_MSG_RESULT(no)
else
  CXXFLAGS="$CXXFLAGS -D_vector_at_broken_"
  AC_MSG_RESULT(yes)
fi


AC_MSG_CHECKING(distinct size_t type)
AC_COMPILE_IFELSE( [AC_LANG_SOURCE( [
    #include <string>
   void foo( int i ) {};
   void foo( size_t i ) {};
   void foo( unsigned int i ) {};
   void foo( bool b ) {};
   int main() {
     size_t t = 7;
     foo(t);
     return 0;
   }  
])], size_t_type=distinct )

if test "x$size_t_type" = x ; then
  AC_MSG_RESULT(no)

  AC_MSG_CHECKING(size_t conversion)
  AC_COMPILE_IFELSE([AC_LANG_SOURCE( [
    #include <string>
    void foo( int i ) {};
    void foo( unsigned int i ) {};
    void foo( bool b ) {};
    int main() {
       size_t t = 7;
       foo(t);
       return 0;
    }  
   ])], AC_MSG_RESULT("yes"),AC_MSG_ERROR("Failed to compile size_t distinction check. Please contact bugs@asc-hq.org ")  )

else
  CXXFLAGS="$CXXFLAGS -DSIZE_T_not_identical_to_INT"
  AC_MSG_RESULT("yes")
fi

AC_CHECK_FILE( "$srcdir/source/unix/pbpedit/pbpeditor.cpp", PBPED=pbpedit )
AC_SUBST(PBPED)


AC_SUBST(asc_version)

dnl this is a hack to prevent automake from interpreting the include statement; it would do so if it was directly in data/Makefile.am
maindatainclude="include main.mak"
AC_SUBST(maindatainclude)
extradatainclude="include pbp.mak"
AC_SUBST(extradatainclude)



AC_CHECK_FILES( $srcdir/data/music/frontiers.ogg $srcdir/data/music/time_to_strike.ogg $srcdir/data/music/machine_wars.ogg,,missingmusic=x )
if test "x$missingmusic" != xx ; then
   MUSICDIR=music
else
   MUSICDIR=""
fi

AC_SUBST(MUSICDIR)


AC_ARG_ENABLE( internal-loki, AS_HELP_STRING([--enable-internal-loki], [Don't use the system libloki]), forceinternallibloki=yes )
AC_CHECK_HEADER(loki/Functor.h,lokifound=yes,lokifound=no) 

AC_MSG_CHECKING(for Loki library to use)
if test x$lokifound != xyes -o x$forceinternallibloki = xyes  ; then 
   AC_MSG_RESULT(internal)
   LokiDir="libs/loki-0.1.6/include/"
   LokiInclude="-I\${top_srcdir}/source/libs/loki-0.1.6/include/"
   LOKI_LIBS=\${top_builddir}/source/libs/loki-0.1.6/libloki.la
else   
   AC_MSG_RESULT(system)
   LokiInclude=
   LOKI_LIBS="-lloki"
fi
AC_SUBST(LokiInclude)
AC_SUBST(LOKI_LIBS)


if test "x$XVIDAVAIL" = x1; then 
   CXXFLAGS="$CXXFLAGS -DXVIDEXPORT"
   VIDEOCODER="-lxvidcore"
   revellib=revel
   revellibfile='$(top_builddir)/source/libs/revel/librevel.la'
fi

AC_SUBST(VIDEOCODER)
AC_SUBST(revellib)
AC_SUBST(revellibfile)
   
AC_ARG_ENABLE( extradata, AS_HELP_STRING([--enable-extradata], [Build extra data]), extradata=yes )
   

AC_MSG_NOTICE( Creating Makefile for main.ascdat )
mkdir -p data
perl -e 'print "main.d: "; foreach(<>) { chomp; @a = glob("'$srcdir/data/'$_"); $l=@a; print "\$(srcdir)/$_ " if ($l > 0 ) ; }  print "\n\ttouch main.d\n"; '< $srcdir/data/files.txt > data/main.mak

if test x$extradata = xyes ; then 
   mkdir -p extra-data
   perl -e 'print "pbp.d: "; foreach(<>) { chomp; @a = glob("'$srcdir/data/'$_"); $l=@a; print "\$(srcdir)/$_ " if ($l > 0 ) ; }  print "\n\ttouch pbp.d\n"; '< $srcdir/extra-data/files_pbp.txt > extra-data/pbp.mak

   extradatadir=extra-data
fi
AC_SUBST(extradatadir)

AC_CONFIG_FILES( [ Makefile \
       asc.spec \
       asc.suse.spec \
       asc.appdata.xml \
       getmusic \
       doc/Makefile \
       doc/manpages/Makefile \
       doc/unitguide/Makefile \
       doc/unitguide/output/Makefile \
       source/Makefile \
       source/ai/Makefile \
       source/clparser/Makefile \
       source/docs/Makefile \
       source/docs/en/Makefile \
       source/graphics/Makefile \
       source/network/Makefile \
       source/dialogs/Makefile \
       source/util/Makefile \
       source/widgets/Makefile \
       source/actions/Makefile \
       source/tasks/Makefile \
       source/video/Makefile \
       source/lua/Makefile \
       source/unittests/Makefile \
       source/doc/Makefile \
       source/libs/Makefile \
       source/libs/sdlmm/Makefile \
       source/libs/sdlmm/src/Makefile \
       source/libs/triangul/Makefile \
       source/libs/loki-0.1.6/Makefile \
       source/libs/sdl_sound/Makefile \
       source/libs/revel/Makefile \
       source/libs/paragui/Makefile \
       source/libs/paragui/src/Makefile \
       source/libs/paragui/src/widgets/Makefile \
       source/libs/paragui/src/core/Makefile \
       source/libs/paragui/src/draw/Makefile \
       source/libs/paragui/src/font/Makefile \
       source/libs/paragui/src/themes/Makefile \
       source/libs/paragui/src/ystring/Makefile \
       source/sdl/Makefile \
       source/tools/Makefile \
       source/tools/lib/Makefile \
       source/tools/linux/Makefile \
       source/unix/Makefile \
       source/unix/mapeditor/Makefile \
       source/unix/pbpedit/Makefile \
       source/unix/asc/Makefile \
       source/win32/Makefile \
       source/win32/msvc/Makefile \
       data/Makefile \
       data/icons/Makefile \
       data/sounds/Makefile \
       data/dialog/Makefile \
       data/music/Makefile \
       extra-data/Makefile \
       installer/Makefile \
       installer/asc.nsi \
       unittester/Makefile ]) 
       
AC_OUTPUT

chmod +x getmusic


AC_ARG_ENABLE( debug, AS_HELP_STRING([--enable-debug],[enables some debugging options and output]), ENABLEDEBUG=1 )
if test "X$ENABLEDEBUG" = "X1"; then
   CXXFLAGS="$CXXFLAGS -O0 -g3 -Wall"
   echo SDL_CFLAGS $SDL_CFLAGS
   echo SDL_LIBS $SDL_LIBS
dnl   echo SDLMM_CFLAGS $SDLMM_CXXFLAGS
dnl   echo SDLMM_LIBS $SDLMM_LIBS
   echo PARAGUI_CFLAGS $PARAGUI_CFLAGS
   echo PARAGUI_LIBS $PARAGUI_LIBS
   echo CFLAGS $CFLAGS
   echo CXXFLAGS $CXXFLAGS
   echo LIBS $LIBS
   echo LDFLAGS $LDFLAGS
   echo SIGC_CFLAGS $SIGC_CFLAGS
   echo SIGC_LIBS $SIGC_LIBS
fi

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

AC_SUBST(PARAGUI_CFLAGS)
AC_SUBST(PARAGUI_LIBS)
   
   
   
if test "x$missingmusic" == xx ; then
AC_MSG_NOTICE( [ 
The music files could not be found. These are not distributed together with the 
source code because of their size. 
ASC can run without the music files being present, this message
is just to inform you about the existence of the music.

You can download the music files by calling the ./getmusic script and 
run configure again, which will properly install them.
Alternatively, you can download and install them manually anytime later.
] )
fi

if test "x$XVIDAVAIL" = x0; then 
AC_MSG_NOTICE( [ 
XviD was not found. ASC will run fine without it, but you will not be
able to export replays as MPEG4 movie. 
] )
fi


AC_MSG_NOTICE( [ configure completed successfully. Now run "make" do build ASC.
If the build process fails for any reason, take a look at
http://asc-hq.sourceforge.net/browse.php?topic=linux
] )

