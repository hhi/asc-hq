/*
     This file is part of Advanced Strategic Command; http://www.asc-hq.de
     Copyright (C) 1994-2010  Martin Bickel  
 
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

#ifndef packageManagerH
#define packageManagerH

#include <set>
#include "versionidentifier.h"
#include "ascstring.h"
#include "basestreaminterface.h"
#include "itemrepository.h"
#include "gamemap.h"
#include "package.h"

class PackageManager {
   private:
      static void processContainer( const ContainerBase* container, std::set<ASCString>& archives );
   public:
       static void checkGame( GameMap* game );
       static void storeData( const GameMap* game );
};

class PackageData {
   public:
      typedef std::map<ASCString,Package*> Packages;
      Packages packages;   
      void read ( tnstream& stream );
      void write ( tnstream& stream ) const;
};



#endif
