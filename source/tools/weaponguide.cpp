/*
    This file is part of Advanced Strategic Command; http://www.asc-hq.de
    Copyright (C) 1994-2001  Martin Bickel, Marc Schellenberger and
    Steffen Froehlich
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING. If not, write to the 
    Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
    Boston, MA  02111-1307  USA
*/
// Version v1.0 , change also GENERAL


#include <stdio.h>

#include "../tpascal.inc"
#include "../typen.h"
#include "../basestrm.h"
#include "../misc.h"
#include "../sgstream.h"
#include "../buildingtype.h"
#include "../vehicletype.h"
#include "../errors.h"
#include "../graphicset.h"
#include "../ascstring.h"
#include "../itemrepository.h"
#include "../strtmesg.h"


// including the command line parser, which is generated by genparse
#include "../clparser/weaponguide.cpp"

int main(int argc, char *argv[] )
{
   Cmdline cl ( argc, argv );

   if ( cl.v() ) {
      cout << argv[0] << " " << getVersionString() << endl;
      exit(0);
   }

   verbosity = cl.r();

   initFileIO( cl.c().c_str() );  // passing the filename from the command line options

   try {

      loadpalette();
      loadbi3graphics();

      loadallobjecttypes();
      loadallbuildingtypes();
      loadallvehicletypes();

      char* wildcard;



      if ( cl.next_param() < argc ) {
         wildcard = argv[cl.next_param()];
         // if a command line parameter is specified, use it as wildcard
         // for example: weaponguide s*.veh
      }
      else {
         wildcard =  "*.*";
         // else use *.veh
      }



      FILE* overview = fopen ( "overview.html", "w" );
      FILE* overview1 = fopen ( "overview1.html", "w" );
      // opens a file for writing and assigns the file pointer overview to it

      // Beginn des HTML Files HEAD und BODY
      //  \n is the sequence to start a new line

      fprintf ( overview , "<html>\n"
                "<HEAD>\n"
                "<TITLE>UNITGUIDE OVERVIEW LEFT</TITLE>\n"
                "<base target=\"base\">\n"
                "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"../ug.css\">\n"
                "</HEAD>\n"
                "\n"
                "<BODY bgcolor=\"#447744\" text=\"#eabc1a\" link=\"#EABC1A\" vlink=\"#EABC1A\" alink=\"#EABC1A\" background=\"../ug-hin.gif\" leftmargin=\"5\" topmargin=\"2\">\n"                "<table width=\"100%\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\">\n"
                "<tr><td><a href=\"overview1.html\">SEE PICTURES</a></td></tr><tr><td></td></tr>\n" );

      fprintf ( overview1 , "<html>\n"
                "<HEAD>\n"
                "<TITLE>WEAPONGUIDE OVERVIEW PICTURES</TITLE>\n"
                "<base target=\"base\">\n"
                "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"../../asc/asc.css\">\n"
                "</HEAD>\n"
                "\n"
                "<BODY bgcolor=\"#447744\" text=\"#eabc1a\" link=\"#EABC1A\" vlink=\"#EABC1A\" alink=\"#EABC1A\" background=\"../ug-hin.gif\" leftmargin=\"5\" topmargin=\"0\">\n" );

      for ( int unit = 0; unit < vehicletypenum; unit++ ) {
         pvehicletype  ft = getvehicletype_forpos ( unit );
         ASCString fn = extractFileName_withoutSuffix( ft->filename );
         if ( patimat ( wildcard, ft->filename.c_str() )) {
            ASCString cn = fn;
            cn.toLower();
            // now we are cycling through all files that match wildcard

            printf(" processing unit %s , ID %d ... ", ft->description.c_str(), ft->id );


            string s, s1, s2, s3, s4, s5, s6, b1, b2;
            s = s1 = s2 = s3 = s4 = s5 = s6 = b1 = b2 = cn;

            // this is a C++ string which is much more powerful than the standard C strings ( char* )

            s += ".html";    // frame
            s1 += "1.html";  // general
            s2 += "2.html";  // movement
            s3 += "3.html";  // weapon
            s4 += "4.html";  // functions
            s5 += "5.html";  // loading
            s6 += "6.html";  // description
            b1 += ".gif";    // little pic
            b2 += ".jpg";    // big pic

            FILE* detailed = fopen ( s.c_str(), "w" );
            FILE* detailed1 = fopen ( s1.c_str(), "w" );
            FILE* detailed2 = fopen ( s2.c_str(), "w" );
            FILE* detailed3 = fopen ( s3.c_str(), "w" );
            FILE* detailed4 = fopen ( s4.c_str(), "w" );
            FILE* detailed5 = fopen ( s5.c_str(), "w" );
            FILE* detailed6 = fopen ( s6.c_str(), "w" );
            // c_str() converts a c++ string back to a c string which fopen requires


            // Beginn Einzelfiles
            // UNIT FRAME
            fprintf ( detailed, "<html>\n"
                      "<HEAD>\n"
                      "<TITLE>UNITGUIDE FRAME</TITLE>\n"
                      "<frameset  rows=\"207,*\" border=0 >\n"
                      "<frame name=\"over\" src=\"%s\" marginheight=\"0\">\n"
                      "<frame name=\"under\" src=\"%s\" marginheight=\"2\">\n"
                      "<noframes><body><p>Diese Seite verwendet Frames. Frames werden von Ihrem Browser aber nicht unterst�tzt.</p></body></noframes>\n"
                      "</frameset>\n"
                      "</html>\n", s1.c_str() , s6.c_str() );

            // UNIT GENERAL
            fprintf ( detailed1, "<html>\n"
                      "<HEAD>\n"
                      "<TITLE>UNITGUIDE GENERAL</TITLE>\n"
                      "<base target=\"under\"> \n"
                      "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"../ug.css\">\n"
                      "</HEAD>\n"
                      "\n"
                      "<BODY bgcolor=\"#447744\" text=\"#eabc1a\" link=\"#EABC1A\" vlink=\"#EABC1A\" alink=\"#EABC1A\" background=\"../ug-hin.gif\">\n" );

            // UNIT TERRAIN
            fprintf ( detailed2, "<html>\n"
                      "<HEAD>\n"
                      "<TITLE>UNITGUIDE MOVEMENT</TITLE>\n"
                      "<base target=\"under\"> \n"
                      "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"../ug.css\">\n"
                      "</HEAD>\n"
                      "\n"
                      "<BODY bgcolor=\"#447744\" text=\"#eabc1a\" link=\"#EABC1A\" vlink=\"#EABC1A\" alink=\"#EABC1A\" background=\"../ug-hin.gif\">\n" );

            // UNIT WEAPONS
            fprintf ( detailed3, "<html>\n"
                      "<HEAD>\n"
                      "<TITLE>UNITGUIDE TERRAIN</TITLE>\n"
                      "<base target=\"under\"> \n"
                      "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"../ug.css\">\n"
                      "</HEAD>\n"
                      "\n"
                      "<BODY bgcolor=\"#447744\" text=\"#eabc1a\" link=\"#EABC1A\" vlink=\"#EABC1A\" alink=\"#EABC1A\" background=\"../ug-hin.gif\">\n" );

            // UNIT FUNCTIONS
            fprintf ( detailed4, "<html>\n"
                      "<HEAD>\n"
                      "<TITLE>UNITGUIDE FUNCTIONS</TITLE>\n"
                      "<base target=\"under\"> \n"
                      "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"../ug.css\">\n"
                      "</HEAD>\n"
                      "\n"
                      "<BODY bgcolor=\"#447744\" text=\"#eabc1a\" link=\"#EABC1A\" vlink=\"#EABC1A\" alink=\"#EABC1A\" background=\"../ug-hin.gif\">\n" );

            // UNIT LOADING
            fprintf ( detailed5, "<html>\n"
                      "<HEAD>\n"
                      "<TITLE>UNITGUIDE LOADING</TITLE>\n"
                      "<base target=\"under\"> \n"
                      "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"../ug.css\">\n"
                      "</HEAD>\n"
                      "\n"
                      "<BODY bgcolor=\"#447744\" text=\"#eabc1a\" link=\"#EABC1A\" vlink=\"#EABC1A\" alink=\"#EABC1A\" background=\"../ug-hin.gif\">\n" );

            // UNIT DESCRIPTION
            fprintf ( detailed6, "<html>\n"
                      "<HEAD>\n"
                      "<TITLE>UNITGUIDE DESCRIPTION</TITLE>\n"
                      "<base target=\"under\"> \n"
                      "<LINK REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"../ug.css\">\n"
                      "</HEAD>\n"
                      "\n"
                      "<BODY bgcolor=\"#447744\" text=\"#eabc1a\" link=\"#EABC1A\" vlink=\"#EABC1A\" alink=\"#EABC1A\" background=\"../ug-hin.gif\">\n" );

            // OVERVIEW LEFT
            fprintf ( overview, " <tr><td><A HREF=\"%s\" target\"base\">", s.c_str() );
            fprintf ( overview, "%s", ft->getName().c_str() );
            fprintf ( overview, " </A></td></tr>\n" );

            // OVERVIEW RIGHT
            fprintf ( overview1, "<table align=\"center\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\">\n"
                      " <tr><td rowspan=\"2\" width=\"50\">" );
            if ( exist ( b1.c_str() ))
               fprintf ( overview1, "<img src=\"%s\" border=\"0\">", b1.c_str() );
            fprintf ( overview1, "</td><td width=\"140\"><A HREF=\"%s\">%s</A></td></tr><tr><td><a href=\"%s\">%s</a></td></tr></table>\n", s.c_str(), ft->name.c_str(), s.c_str(), ft->description.c_str() );

            // END OVERVIEW RIGHT

            // we are adding a link to the overview file.
            // to put a singile " into a string we must use double quotes ( "" ), because a single quote is interpreted as the end of the string by C
            // %s tells C to insert a string there. The strings are appended at the end of the command
            // at the first %s the filename in s is inserted, at the second %s the unit variable 'description'

            // UNIT GENERAL
            fprintf ( detailed1, "<table width=\"100%\" id=\"H2\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\"> \n"
                      "<tr><td colspan=\"2\"></td><td id=\"H9\" align=\"right\">UNIT GUIDE v1.7 </td></tr>"
                      "<tr><td width=\"50\">" );
            if ( exist ( b1.c_str() ))
               fprintf ( detailed1, "<img src=\"%s\">", b1.c_str() );
            fprintf ( detailed1, "</td>\n<td>" );

            fprintf ( detailed1, "<table id=\"H2\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\"> \n" );
            fprintf ( detailed1, "<tr><td bgcolor=\"#20483f\">Name</td>         <td align=\"center\" colspan=\"4\">%s</td> </tr>\n", ft->name.c_str() );
            if ( !ft->description.empty() )
               fprintf ( detailed1, "<tr><td bgcolor=\"#20483f\">Description</td>  <td align=\"center\" colspan=\"4\">%s</td> </tr>\n", ft->description.c_str() );
            else
               fprintf ( detailed1, "<tr><td bgcolor=\"#20483f\">Description</td>  <td align=\"center\" colspan=\"4\">--</td> </tr>\n" );
            fprintf ( detailed1, "<tr><td bgcolor=\"#20483f\">ID</td>           <td align=\"center\">%i</td> <td></td><td bgcolor=\"#20483f\">Weight</td> <td align=\"center\">%d</td> <td bgcolor=\"#20483f\">Type</td><td>%s</td> </tr>\n", ft->id, ft->weight, cmovemalitypes[ft->movemalustyp]  );
            fprintf ( detailed1, "<tr><td bgcolor=\"#20483f\">Armor</td>        <td align=\"center\">%i</td> <td></td><td bgcolor=\"#20483f\">View</td>   <td align=\"center\">%d</td> </tr>\n", ft->armor, (ft->view/10) );
            fprintf ( detailed1, "<tr><td bgcolor=\"#20483f\">Cost energy</td>  <td align=\"center\">%i</td> <td></td><td bgcolor=\"#20483f\">Jamming</td><td align=\"center\">%d</td> </tr>\n", ft->productionCost.material, (ft->jamming/10) );
            fprintf ( detailed1, "<tr><td bgcolor=\"#20483f\">Cost material</td><td align=\"center\">%i</td> </tr>\n", ft->productionCost.energy );

            if ( ft->wait )
               fprintf ( detailed1, "<tr><td bgcolor=\"#20483f\" colspan=\"4\">Attack after move</td> <td>No</td> </tr>\n" );
            else
               fprintf ( detailed1, "<tr><td bgcolor=\"#20483f\" colspan=\"4\">Attack after move</td> <td>Yes</td> </tr>\n" );

            if ( ft->functions & cf_moveafterattack )
               fprintf ( detailed1, "<tr><td bgcolor=\"#20483f\" colspan=\"4\">Move after Attack</td> <td>Yes</td></tr>\n" );
            else
               fprintf ( detailed1, "<tr><td bgcolor=\"#20483f\" colspan=\"4\">Move after Attack</td> <td>No</td></tr>\n" );

            fprintf ( detailed1, "</table>\n" );
            fprintf ( detailed1, "</td>\n\n<td width=\"150\">" );
            if ( exist ( b2.c_str() ))
               fprintf ( detailed1, "<img src=\"%s\">", b2.c_str() );
            fprintf ( detailed1, "</td></tr></table>\n" );
            fprintf ( detailed1, "<table width=\"100%\" id=\"H2\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\">\n<tr align=\"center\">" );
            fprintf ( detailed1, "<td><a href=\"%s\">Movement</a></td>", s2.c_str() );
            fprintf ( detailed1, "<td><a href=\"%s\">Weapons</a></td>" , s3.c_str() );
            fprintf ( detailed1, "<td><a href=\"%s\">Functions</a></td>" , s4.c_str() );
            fprintf ( detailed1, "<td><a href=\"%s\">Loading</a></td>" , s5.c_str() );
            fprintf ( detailed1, "<td><a href=\"%s\">Description</a></td>" , s6.c_str() );
            //         fprintf ( detailed1, "<td><a href=\"%s\">Research</a></td>" , s4.c_str() );
            fprintf ( detailed1, "</tr></table>\n" );
            // END UNIT GENERAL

            // some details about the unit; %d tells C to insert a decimal number there
            // take a look at the vehicletype class in vehicletype.h for the names of all variables that make a vehicletype
            // be carefuel not to make a , at the end of the first lines, since this would seperate the string in to several independant strings

            // choehenstufen is a global array that contains the names of the height levels


            // BEGIN MOVEMENT

            // Hoehenstufen
            int i,w;
            // Tabellenbeginn
            fprintf( detailed2, "<TABLE align=\"left\" rules=\"rows\" id=\"H9\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\">"
                     "<TR><td colspan=\"9\" bgcolor=\"#20483f\" id=\"H2\">Reachable levels of height:</td></tr>\n<tr>");
            // Spaltentitel
            fprintf ( detailed2, "<td></td>" );
            for ( i = 0; i < 8; i++ )
               fprintf ( detailed2, " <TD><IMG src=\"../hoehe%d.gif\" ></TD>", i);
            fprintf( detailed2, "</TR>\n<TR>");
            // Spaltenwerte Hacken
            fprintf ( detailed2, "<td></td>" );
            for ( i = 0; i < 8; i++ )
               if ( ft->height & ( 1 << i ))
                  fprintf ( detailed2, " <TD><img src=\"../hacken.gif\"></TD>" );
               else
                  fprintf ( detailed2, " <TD></TD>"  );
            fprintf( detailed2, "</TR>\n<TR>");
            // Spaltenwerte pro Runde
            fprintf ( detailed2, "<td>Round</td>" );
            for ( i = 0; i < 8; i++ )
               if ( ft->height & ( 1 << i ))
                  fprintf ( detailed2, " <TD align=\"center\">%d</TD>", (ft->movement[i]/10) );
               else
                  fprintf ( detailed2, " <TD></TD>"  );
            fprintf( detailed2, "</TR>\n<TR>\n");

            /* ????
                     // Spaltenwerte Maximum
                     fprintf ( detailed2, "<td>Max.</td>" );
                     for ( i = 0; i < 8; i++ )
                        if ( (ft->height & ( 1 << i )) && ft->fuelConsumption )
                           fprintf ( detailed2, " <TD align=\"center\">%d</TD>", ft->tank.fuel/ft->fuelConsumption );
                        else
                           fprintf ( detailed2, " <TD></TD>"  );
            */
            fprintf( detailed2, "\n</TR>\n</TABLE>\n");

            // Einzelne Werte
            // Tabellenbeginn
            fprintf( detailed2, "<TABLE id=\"H2\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\">\n" );
            fprintf( detailed2, "<TR><td bgcolor=\"#20483f\">Fuel tank</td>    <td align=\"center\">%d</td></tr>\n", ft->tank.fuel );
            fprintf( detailed2, "<TR><td bgcolor=\"#20483f\">Material tank</td><td align=\"center\">%d</td></tr>\n", ft->tank.material );
            fprintf( detailed2, "<TR><td bgcolor=\"#20483f\">Energy tank</td>  <td align=\"center\">%d</td></tr>\n", ft->tank.energy );
            fprintf( detailed2, "<TR><td bgcolor=\"#20483f\">Consumption</td>  <td align=\"center\">%d</td></tr>\n", ft->fuelConsumption );
            if ( ft->fuelConsumption > 0 )
               fprintf( detailed2, "<tr><td bgcolor=\"#20483f\">Max.Movem.</td>   <TD align=\"center\">%d</TD></tr>\n", (ft->tank.fuel/ft->fuelConsumption) );
            else
               fprintf( detailed2, "<tr><td bgcolor=\"#20483f\">Max.Movem.</td>   <TD align=\"center\">--</TD></tr>\n" );
            fprintf( detailed2, "<TR><td bgcolor=\"#20483f\">*Steigung*</td>   <td align=\"center\">%d</td></tr>\n", ft->steigung );
            fprintf( detailed2, "\n</TABLE>\n");


            // Bodentypen global
            fprintf( detailed2,"<br><br>\n<table rules=\"rows\" id=\"H2\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\"><tr><td colspan=\"34\" bgcolor=\"#20483f\">" );
            fprintf( detailed2,"Terraintypes for not flying units:</td></tr>\n<tr>" );

            // Spalten gfx
            fprintf ( detailed2, "<td></td>" );
            for ( int i = 0; i < cbodenartennum ; i++ )
               fprintf ( detailed2, " <TD><IMG src=\"../gfx%d.gif\" alt=\"%s\"></TD>", i, cbodenarten[i]);
            fprintf( detailed2, "</TR>\n<TR>\n");

            // Spaltenwerte befahrbare Bodentypen
            fprintf ( detailed2, "<td>can&nbsp;drive&nbsp;on</td>" );
            for ( int i = 0; i < cbodenartennum ; i++) {
               if ( ft->terrainaccess.terrain.test(i) )
                  fprintf ( detailed2, "<td><img src=\"../hacken.gif\"></td>" );
               else
                  fprintf ( detailed2, "<td></td>" );
            } /* endfor */
            fprintf( detailed2, "</TR>\n<TR>\n");

            // Spaltenwerte stirbt auf
            fprintf ( detailed2, "<td>dies on</td>" );
            for ( i = 0; i < cbodenartennum ; i++) {
               if ( ft->terrainaccess.terrainkill.test(i))
                  fprintf ( detailed2, "<td><img src=\"../hacken.gif\"></td>" );
               else
                  fprintf ( detailed2, "<td></td>" );
            } /* endfor */
            fprintf( detailed2, "</TR>\n<TR>\n");

            // Spaltenwerte can not drive on
            fprintf ( detailed2, "<td>not drive on</td>" );
            for ( i = 0; i < cbodenartennum ; i++) {
               if ( ft->terrainaccess.terrainnot.test(i) )
                  fprintf ( detailed2, "<td><img src=\"../hacken.gif\"></td>" );
               else
                  fprintf ( detailed2, "<td></td>" );
            } /* endfor */

            fprintf ( detailed2, "</tr><tr>\n" );
            // Spaltenwerte can need terrain
            fprintf ( detailed2, "<td>required</td>" );
            for ( i = 0; i < cbodenartennum ; i++) {
               if ( ft->terrainaccess.terrainreq.test(i) )
                  fprintf ( detailed2, "<td><img src=\"../hacken.gif\"></td>" );
               else
                  fprintf ( detailed2, "<td></td>" );
            } /* endfor */
            fprintf ( detailed2, "</tr></table>\n" );
            // ENDE MOVEMENT

            // BEGIN WEAPONS
            // Waffen NR-AMMO-DISTANCE-STRENGS-SHOT FROM-ATTACK TO-TYP
            fprintf ( detailed3, "<table id=\"H10\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\"> \n" );
            // �berschriften
            fprintf ( detailed3, "<tr><td colspan=\"2\"></td>"
                      "    <td bgcolor=\"#20483f\" colspan=\"4\" align=\"center\">Distance</td>"
                      "    <td bgcolor=\"#20483f\" colspan=\"8\" align=\"center\"> can shot from: </td>"
                      "    <td></td>"
                      "    <td bgcolor=\"#20483f\" colspan=\"8\" align=\"center\"> attack to: </td>"
                      "    <td bgcolor=\"#20483f\">Type</td>"
                      "</tr>\n" );
            // GFX
            fprintf ( detailed3, "<tr><td></td>"
                      "    <td><img src=\"../ammo.gif\"</td>"
                      "    <td><img src=\"../dis1.gif\"</td>"
                      "    <td><img src=\"../str1.gif\"</td>"
                      "    <td><img src=\"../dis2.gif\"</td>"
                      "    <td><img src=\"../str2.gif\"</td>" );
            // H�henstufenzeichen einf�gen f�r shoot from und target
            for ( i = 0; i < 8; i++ )
               fprintf ( detailed3, " <TD><IMG src=\"../hoehe%d.gif\" ></TD>", i);
            fprintf (detailed3, "<td></td> ");
            for ( i = 0; i < 8; i++ )
               fprintf ( detailed3, " <TD><IMG src=\"../hoehe%d.gif\" ></TD>", i);
            fprintf ( detailed3, "    <td></td>"
                      "</tr>\n" );

            // Werte der Waffen
            for ( w = 0; w < ft->weapons.count ; w++) {
               fprintf ( detailed3, "<tr><td>#%d</td>", w+1 );
               fprintf ( detailed3, "    <td align=\"center\">%d</td>", ft->weapons.weapon[w].count );
               fprintf ( detailed3, "    <td align=\"center\">%d</td>", (ft->weapons.weapon[w].mindistance+9)/10 );
               fprintf ( detailed3, "    <td align=\"center\">%d</td>", ft->weapons.weapon[w].maxstrength );
               fprintf ( detailed3, "    <td align=\"center\">%d</td>", (ft->weapons.weapon[w].maxdistance)/10 );
               fprintf ( detailed3, "    <td align=\"center\">%d</td>", ft->weapons.weapon[w].minstrength );
               // H�henstufenzeichen einf�gen f�r shoot from und target
               for ( i = 0; i < 8; i++ )
                  if ( ft->weapons.weapon[w].sourceheight & ( 1 << i ) )
                     fprintf ( detailed3, "<td><img src=\"../hacken.gif\"></td> " );
                  else
                     fprintf ( detailed3, "<td></td>" );
               fprintf ( detailed3, "    <td></td>");
               for ( i = 0; i < 8; i++ )
                  if ( ft->weapons.weapon[w].targ & ( 1 << i ) )
                     fprintf ( detailed3, "<td><img src=\"../hacken.gif\"></td>" );
                  else
                     fprintf ( detailed3, "<td></td>" );

               fprintf ( detailed3, "    <td nowrap>" );
               for ( int i = 0; i < cwaffentypennum; i++ )
                  if ( ft->weapons.weapon[w].typ & ( 1 << i ) )
                     fprintf ( detailed3, "%s.", cwaffentypen[i] );
               fprintf ( detailed3, "</td></tr>\n" );
            }
            fprintf ( detailed3, "</table>\n" );

            fprintf ( detailed3, "<br>\n\n" );
            // Die entfernung wird durch 10 dividiert, um die Anzahl der Felder zu erhalten
            // Die normale Division rundet IMMER ab, also 1,9 / 2 = 0
            // Aber die minimale Entfernung mu� aufgerundet werden, deshalb benutze ich einen kleinen Trick: Ich addiere vor der Division 9 (also Quotient-1) dazu

            //Weapon can hit:
            fprintf ( detailed3, "<table id=\"H9\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\"> \n" );
            // �berschrift
            fprintf ( detailed3, "<tr><td></td><td align=\"center\" colspan=\"16\" bgcolor=\"#20483f\">The weapon can hit: </td></tr>" );
            // Spalten GFX
            fprintf ( detailed3, "<tr><td></td>" );
            for ( i = 0; i < cmovemalitypenum; i++ )
               fprintf ( detailed3, " <TD><IMG src=\"../typ%d.gif\"></TD>", i);
            fprintf ( detailed3, "</TR>\n");
            // Spaltenwerte
            for ( int w = 0; w < ft->weapons.count ; w++) {
               fprintf ( detailed3, "<TR><td>#%d</td>", w+1 );
               for ( i = 0; i < cmovemalitypenum; i++ )
                  if ( !(ft->weapons.weapon[w].targets_not_hittable & ( 1 << i )) )
                     fprintf ( detailed3, "<td><img src=\"../hacken.gif\"></td>" );
                  else
                     fprintf ( detailed3, "<td></td>" );
               fprintf ( detailed3, "</TR>\n");
            }
            //Ende tabelle
            fprintf ( detailed3, "</table><br> \n" );

            // Effizienz
            fprintf ( detailed3, "<table id=\"H9\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\"> \n" );
            // �berschrift
            fprintf ( detailed3, "<tr><td></td><td align=\"center\" colspan=\"14\" bgcolor=\"#20483f\">Efficiency over high difference: </td></tr>\n" );
            // Spalten �berschrift
            fprintf ( detailed3, "<tr><td></td>" );
            for ( i = 0; i < 13; i++ )
               fprintf(detailed3, "<td>%d</td>", i-6 );
            fprintf ( detailed3, "</TR>\n");
            // Spaltenwerte
            for ( int w = 0; w < ft->weapons.count ; w++) {
               fprintf ( detailed3, "<TR><td>#%d</td>", w+1 );
               for ( i = 0; i < 13; i++ )
                  fprintf ( detailed3, "<td>%d%%</td>", ft->weapons.weapon[w].efficiency[i] );
               fprintf ( detailed3, "</TR>\n");
            }
            //Ende tabelle
            fprintf ( detailed3, "</table> \n" );






            //ENDE WAFFEN

            //BEGINN FUNCTIONS
            fprintf ( detailed4, "<table align=\"left\" id=\"H9\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\">" );
            // �berschrift
            fprintf ( detailed4, "<tr><td align=\"center\" bgcolor=\"#20483f\">Special Unit functions</td></tr>" );
            for ( int i = 0; i<cvehiclefunctionsnum; i++)
               if ( ft->functions & ( 1 << i ))
                  fprintf ( detailed4, "<tr><td>%s</td></tr>", cvehiclefunctions[i] );
            //Ende tabelle
            fprintf ( detailed4, "</table>\n" );
            //Einzelwerte
            fprintf ( detailed4, "<table id=\"H10\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\">" );
            if ( ft->maxwindspeedonwater )
               fprintf ( detailed4, "<tr><td align=\"center\" bgcolor=\"#20483f\">Max. windspeed on water</td> <td>%d</td> </tr>", ft->maxwindspeedonwater );
            else
               fprintf ( detailed4, "<tr><td align=\"center\" bgcolor=\"#20483f\">Max. windspeed on water</td> <td>--</td> </tr>" );
            fprintf ( detailed4, "<tr><td align=\"center\" bgcolor=\"#20483f\">Ressource search range</td><td>%d</td> </tr>", ft->digrange );
            fprintf ( detailed4, "<tr><td align=\"center\" bgcolor=\"#20483f\">Auto repair rate</td> <td>%d</td> </tr>", ft->autorepairrate );
            //         fprintf ( detailed4, "<tr><td align=\"center\" bgcolor=\"#20483f\">Max. unit weight</td><td>%d</td> </tr>/n", ft->maxunitweight );
            fprintf ( detailed4, "</table>\n" );
            //ENDE FUNKTIONS

            //BEGINN LOADING
            if ( ft->loadcapacity ) {
               fprintf ( detailed5, "<table align=\"left\" id=\"H10\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\"> " );
               fprintf ( detailed5, "<tr><td align=\"center\" bgcolor=\"#20483f\">Loadable units</td></tr>" );
               fprintf ( detailed5, "<tr><td align=\"center\" >Not implemented jet</td></tr>" );
               fprintf ( detailed5, "</table>" );

               fprintf ( detailed5, "<table id=\"H10\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\">" );
               //         fprintf ( detailed5, "<tr><td align=\"center\" bgcolor=\"#20483f\">Maximum loading</td> <td>%d</td> </tr>", ft->maxsize );
               fprintf ( detailed5, "<tr><td align=\"center\" bgcolor=\"#20483f\">Max. unit weight</td><td>%d</td> </tr>", ft->maxunitweight );
               fprintf ( detailed5, "</table>" );
            } else {
               fprintf ( detailed5, "<table align}\"left\" id=\"H10\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\">" );
               fprintf ( detailed5, "<tr><td align=\"center\" bgcolor=\"#20483f\">Loadable units</td></tr>" );
               fprintf ( detailed5, "<tr><td align=\"center\" ><br>No transport unit!<br></td></tr>" );
               fprintf ( detailed5, "</table>" );
            }
            //ENDE LOADING

            //BEGINN DESCRIPTION
            fprintf ( detailed6, "<table align=\"left\" id=\"H10\" border=\"1\" bordercolordark=\"#333333\" bordercolorlight=\"#408050\" cellpadding=\"1\" cellspacing=\"1\">" );
            fprintf ( detailed6, "<tr><td align=\"center\" bgcolor=\"#20483f\">Unit description</td></tr>" );
            if ( !ft->infotext.empty() ) {
               string text = ft->infotext;
               if ( text.find ( "#color0#" ) != string::npos )
                  text.replace ( text.find  ("#color0#"), 8, " ");
               if ( text.find ( "#color1#" ) != string::npos )
                  text.replace ( text.find  ("#color1#"), 8, " ");
               if ( text.find ( "#color2#" ) != string::npos )
                  text.replace ( text.find  ("#color2#"), 8, " ");
               if ( text.find ( "#color3#" ) != string::npos )
                  text.replace ( text.find  ("#color3#"), 8, " ");
               if ( text.find ( "#color4#" ) != string::npos )
                  text.replace ( text.find  ("#color4#"), 8, " ");
               if ( text.find ( "#font0#" ) != string::npos )
                  text.replace ( text.find  ("#font0#"), 7, " ");
               if ( text.find ( "#font1#" ) != string::npos )
                  text.replace ( text.find  ("#font1#"), 7, " ");
               if ( text.find ( "#font2#" ) != string::npos )
                  text.replace ( text.find  ("#font2#"), 7, " ");
               if ( text.find ( "#crt#" ) != string::npos )
                  text.replace ( text.find  ("#crt#"), 5, " ");
               fprintf ( detailed6, "<tr><td><br>%s<br></td></tr>", text.c_str() );
            }
            //            fprintf ( detailed6, "<tr><td><br><br>%s</td></tr>", ft->infotext );
            else {
               fprintf ( detailed6, "<tr><td><br><br>No description available !<br><br></td></tr>" );
            }
            //Ende tabelle
            fprintf ( detailed6, "</table> \n" );
            //ENDE DESCRIPTION


            // NEU - noch anzupassen
            for ( int h = 0; h < 8; h++ ) {
               if ( ft->loadcapability & (1 << h))
                  printf("H�henstufe kann geladen werden" );

               if ( ft->loadcapabilityreq & (1 << h))
                  printf("H�henstufe mu� erreichbar sein, um geladen werden zu k�nnen" );

               if ( ft->loadcapability & (1 << h))
                  printf("H�henstufe darf nicht erreichbar sein" );
            }

            // ft->loadcapacity  ist maximale ladekapazit�t
            for ( int c = 0; c < cmovemalitypenum; c++ )
               if ( ft->vehicleCategoriesLoadable & (1 << c))
                  printf(" Kategorie %s kann geladen werden", cmovemalitypes[c] );

            for ( i = 0; i < ft->buildingsBuildable.size(); i++ ) {
               printf("es k�nnen die geb�ude mit ids von %d bis %d gebaut werden\n", ft->buildingsBuildable[i].from, ft->buildingsBuildable[i].to );
               for ( int b = 0; b < buildingtypenum; b++ ) {
                  pbuildingtype bld = getbuildingtype_forpos ( b );
                  if (     bld->id >= ft->buildingsBuildable[i].from
                           && bld->id <= ft->buildingsBuildable[i].to ) {
                     printf( "das geb�ude mit id %d und dem Namen %s kann gebaut werden\n", bld->id, bld->name.c_str() );
                  }
               }
            }

            for ( i = 0; i < ft->vehiclesBuildable.size(); i++ ) {
               printf("es k�nnen die einheiten mit ids von %d bis %d gebaut werden\n", ft->vehiclesBuildable[i].from, ft->vehiclesBuildable[i].to );
               for ( int b = 0; b < vehicletypenum; b++ ) {
                  pvehicletype veh = getvehicletype_forpos ( b );
                  if (     veh->id >= ft->vehiclesBuildable[i].from
                        && veh->id <= ft->vehiclesBuildable[i].to ) {
                     printf( "die einheit mit id %d und dem Namen %s kann gebaut werden\n", veh->id, veh->getName().c_str() );
                  }
               }
            }

            for ( i = 0; i < ft->objectsBuildable.size(); i++ ) {
               printf("es k�nnen die Objekte mit ids von %d bis %d gebaut werden\n", ft->objectsBuildable[i].from, ft->objectsBuildable[i].to );
               for ( int b = 0; b < objecttypenum; b++ ) {
                  pobjecttype obj = getobjecttype_forpos ( b );
                  if (     obj->id >= ft->objectsBuildable[i].from
                        && obj->id <= ft->objectsBuildable[i].to ) {
                     printf( "die einheit mit id %d und dem Namen %s kann gebaut werden\n", obj->id, obj->name.c_str() );
                  }
               }
            }

            // ABSCHLU� DER DOKUMENTE

            fprintf ( detailed1, "</body></html>\n");
            fprintf ( detailed2, "</body></html>\n");
            fprintf ( detailed3, "</body></html>\n");
            fprintf ( detailed4, "</body></html>\n");
            fprintf ( detailed5, "</body></html>\n");
            fprintf ( detailed6, "</body></html>\n");
            // Ende des Einheiten Dokuments

            fclose ( detailed );
            fclose ( detailed1 );
            fclose ( detailed2 );
            fclose ( detailed3 );
            fclose ( detailed4 );
            fclose ( detailed5 );
            fclose ( detailed6 );
            // closing the file

            printf(" done \n" );
            // we are writing this not to a file, but the screen

         }

      }

      // Dokument �bersicht Ende

      fprintf( overview , "</table></body></html>\n" );
      fprintf ( overview1, "</body></html>\n" );

      fclose ( overview );
      fclose ( overview1 );

   } /* endtry */
   catch ( tfileerror err ) {
      printf("\nfatal error accessing file %s \n", err.getFileName().c_str() );
      return 1;
   } /* endcatch */
   catch ( ASCexception ) {
      printf("\na fatal exception occured\n" );
      return 2;
   } /* endcatch */

   return 0;
};



