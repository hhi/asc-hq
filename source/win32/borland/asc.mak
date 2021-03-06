# ---------------------------------------------------------------------------
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

# ---------------------------------------------------------------------------
# IDE SECTION
# ---------------------------------------------------------------------------
# The following section of the project makefile is managed by the BCB IDE.
# It is recommended to use the IDE to change any of the values in this
# section.
# ---------------------------------------------------------------------------

VERSION = BCB.06.00
# ---------------------------------------------------------------------------
PROJECT = asc.exe
OBJFILES = obj\SDL_main.obj obj\CLoadable.obj obj\getopt.obj obj\getopt1.obj \
    obj\Named.obj obj\Property.obj obj\PropertyGroup.obj obj\astar2.obj \
    obj\ascstring.obj obj\attack.obj obj\basegfx.obj obj\basestrm.obj \
    obj\building.obj obj\buildings.obj obj\buildingtype.obj \
    obj\containerbase.obj obj\containerbasetype.obj obj\controls.obj \
    obj\dashboard.obj obj\dialog.obj obj\dlg_box.obj obj\gamedlg.obj \
    obj\gamemap.obj obj\gameoptions.obj obj\graphicset.obj obj\gui.obj \
    obj\itemrepository.obj obj\loadbi3.obj obj\loaders.obj obj\loadpcxc.obj \
    obj\mapalgorithms.obj obj\mapdisplay.obj obj\messages.obj obj\misc.obj \
    obj\music.obj obj\network.obj obj\networkdata.obj obj\newfont.obj \
    obj\objecttype.obj obj\palette.obj obj\paradialog.obj obj\password.obj \
    obj\password_dialog.obj obj\pd.obj obj\replay.obj obj\research.obj \
    obj\resourcenet.obj obj\SDLStretch.obj obj\asc_IMG_jpg.obj obj\events.obj \
    obj\graphics.obj obj\loadimage.obj obj\sound.obj obj\sg.obj \
    obj\sgstream.obj obj\soundList.obj obj\spfst.obj obj\stack.obj \
    obj\stringtokenizer.obj obj\strtmesg.obj obj\terraintype.obj \
    obj\textfile_evaluation.obj obj\textfileparser.obj obj\textfiletags.obj \
    obj\typen.obj obj\unitctrl.obj obj\vehicle.obj obj\vehicletype.obj \
    obj\viewcalculation.obj obj\messagedlg.obj obj\simplestream.obj \
    obj\gameevents.obj obj\Singleton.obj obj\prehistoricevents.obj \
    obj\mappolygons.obj obj\gameeventsystem.obj obj\gameevent_dialogs.obj \
    obj\statisticdialog.obj
RESFILES = 
MAINSOURCE = asc.bpf
RESDEPEN = $(RESFILES)
LIBFILES = ..\..\..\..\Expat\Libs\exp.lib ..\..\..\..\SDL_image\bin\SDL_image.lib \
    ..\..\..\..\SDL_mixer\bin\SDL_mixer.LIB \
    ..\..\..\..\freetype2\objs\freetype.lib \
    ..\..\..\..\paragui\borland\Paragui.lib ..\..\..\..\sdl\bin\sdl.lib \
    ..\..\AI\ai.lib ..\..\libs\bzlib\win\bzlib.lib \
    ..\..\libs\jpeg-6b\libjpeg.lib ..\..\libs\sdlmm\SDLmm.lib \
    ..\..\libs\triangul\win32\triangulation.lib \
    ..\..\libs\libsigc++\libsigcpp.lib
IDLFILES = 
IDLGENFILES = 
LIBRARIES = vcl.lib rtl.lib
PACKAGES = rtl.bpi vcl.bpi vclx.bpi bcbsmp.bpi dclocx.bpi
SPARELIBS = rtl.lib vcl.lib
DEFFILE = 
OTHERFILES = 
# ---------------------------------------------------------------------------
DEBUGLIBPATH = $(BCB)\lib\debug
RELEASELIBPATH = $(BCB)\lib\release
USERDEFINES = HEXAGON;sgmain;FREEMAPZOOM;_WIN32_;NEWKEYB;_SDL_;_NOASM_;WIN32;_DEBUG
SYSDEFINES = NO_STRICT;_NO_VCL
INCLUDEPATH = ..\..\libs\loki;..\..\..\..\..\Borland\CBuilder6\Projects;..\..;..\..\LIBS\getopt;..\..\AI;..\..\..\..\sdl\include;..\..\sdl;..\..\..\..\sdl_mixer;..\..\..\..\sdl\src\main\win32;$(BCB)\include;$(BCB)\include\vcl;..\..\..\..\SDLmm\src;..\..\..\..\SDL_image;..\..\..\..\paragui\include;..\..\..\..\freetype2\include;..\..\libs\libsigc++
LIBPATH = ..\..\libs\loki;..\..\..\..\..\Borland\CBuilder6\Projects;..\..;..\..\LIBS\getopt;..\..\AI;C:\BORLAND\CBuilder5\Projects;..\..\sdl;..\..\..\..\sdl\src\main\win32;$(BCB)\lib\obj;$(BCB)\lib
WARNINGS= -w-pck -w-par -w-8027 -w-8026 -w-csu -w-big
PATHCPP = .;..\..\..\..\sdl\src\main\win32;..\..;..\..\LIBS\getopt;..\..\sdl;..\..\libs\loki
PATHASM = .;
PATHPAS = .;
PATHRC = .;
PATHOBJ = .;$(LIBPATH)
# ---------------------------------------------------------------------------
CFLAG1 = -Od -Q -Vx -Ve -X- -r- -a1 -5 -b -k -y -v -vi- -tW -tWM -c -K
IDLCFLAGS = 
PFLAGS = -N2obj -N0obj -$Y+ -$W -$O- -$A8 -v -JPHNE -M
RFLAGS = 
AFLAGS = /mx /w2 /zi
LFLAGS = -Iobj -D"" -aa -Tpe -GD -s -Gn -M -v
# ---------------------------------------------------------------------------
ALLOBJ = c0w32.obj $(OBJFILES)
ALLRES = $(RESFILES)
ALLLIB = $(LIBFILES) import32.lib cw32mt.lib
# ---------------------------------------------------------------------------
!ifdef IDEOPTIONS

[Version Info]
IncludeVerInfo=0
AutoIncBuild=0
MajorVer=1
MinorVer=0
Release=0
Build=0
Debug=0
PreRelease=0
Special=0
Private=0
DLL=0

[Version Info Keys]
CompanyName=
FileDescription=
FileVersion=1.0.0.0
InternalName=
LegalCopyright=
LegalTrademarks=
OriginalFilename=
ProductName=
ProductVersion=1.0.0.0
Comments=

[Debugging]
DebugSourceDirs=$(BCB)\source\vcl

!endif





# ---------------------------------------------------------------------------
# MAKE SECTION
# ---------------------------------------------------------------------------
# This section of the project file is not used by the BCB IDE.  It is for
# the benefit of building from the command-line using the MAKE utility.
# ---------------------------------------------------------------------------

.autodepend
# ---------------------------------------------------------------------------
!if "$(USERDEFINES)" != ""
AUSERDEFINES = -d$(USERDEFINES:;= -d)
!else
AUSERDEFINES =
!endif

!if !$d(BCC32)
BCC32 = bcc32
!endif

!if !$d(CPP32)
CPP32 = cpp32
!endif

!if !$d(DCC32)
DCC32 = dcc32
!endif

!if !$d(TASM32)
TASM32 = tasm32
!endif

!if !$d(LINKER)
LINKER = ilink32
!endif

!if !$d(BRCC32)
BRCC32 = brcc32
!endif


# ---------------------------------------------------------------------------
!if $d(PATHCPP)
.PATH.CPP = $(PATHCPP)
.PATH.C   = $(PATHCPP)
!endif

!if $d(PATHPAS)
.PATH.PAS = $(PATHPAS)
!endif

!if $d(PATHASM)
.PATH.ASM = $(PATHASM)
!endif

!if $d(PATHRC)
.PATH.RC  = $(PATHRC)
!endif

!if $d(PATHOBJ)
.PATH.OBJ  = $(PATHOBJ)
!endif
# ---------------------------------------------------------------------------
$(PROJECT): $(OTHERFILES) $(IDLGENFILES) $(OBJFILES) $(RESDEPEN) $(DEFFILE)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) -L$(LIBPATH) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB), +
    $(DEFFILE), +
    $(ALLRES)
!
# ---------------------------------------------------------------------------
.pas.hpp:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.pas.obj:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.cpp.obj:
    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }

.c.obj:
    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }

.c.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.cpp.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.asm.obj:
    $(BCB)\BIN\$(TASM32) $(AFLAGS) -i$(INCLUDEPATH:;= -i) $(AUSERDEFINES) -d$(SYSDEFINES:;= -d) $<, $@

.rc.res:
    $(BCB)\BIN\$(BRCC32) $(RFLAGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -fo$@ $<



# ---------------------------------------------------------------------------

obj\basestrm.obj: ..\..\basestrm.cpp
  $(BCB)\BIN\$(BCC32) $(CFLAG1)   -I$(INCLUDEPATH) -D$(USERDEFINES) -D$(SYSDEFINES) -n$(@D) {$** }




