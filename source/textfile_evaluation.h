/***************************************************************************
                          textfile_evaluation.h  -  description
                             -------------------
    begin                : Thu Oct 06 2002
    copyright            : (C) 2002 by Martin Bickel
    email                : bickel@asc-hq.org
 ***************************************************************************/

/*! \file textfile_evaluation.h
    \brief Functions to evaluate the parsed *.asctxt files
*/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef textfile_evaluationH
#define textfile_evaluationH

#include <vector>
#include "ascstring.h"
#include "errors.h"
#include "typen.h"
#include "textfileparser.h"


class PropertyContainer {
         bool reading;
         // ASCString filename;
      protected:
         int levelDepth;
         typedef list<ASCString> Level;
         Level level;

         TextPropertyGroup* textPropertyGroup;
      public:

         class Property {
            protected:
               ASCString name;
               ASCString lastName;
               PropertyContainer* propertyContainer;
               TextPropertyGroup::Entry* entry;
               virtual ASCString toString ( ) const = 0;
               void findEntry ();

               bool defaultValueAvail;
               virtual bool hasDefault() { return defaultValueAvail; };
               void writeProperty( );
               Property( bool defaultValueAvail_ ) : propertyContainer ( NULL ), entry ( NULL ), evaluated(false), defaultValueAvail (defaultValueAvail_) {};

             public:
               virtual ~Property() {};
               bool evaluated;
               virtual void evaluate ( ) = 0;
               void setName ( const ASCString& name_, const ASCString& lastName_ );
               const ASCString& getLastName() { return lastName; };
               void setPropertyContainer ( PropertyContainer* propertyContainer_ ) {  propertyContainer = propertyContainer_; };
          };
          friend class Property;



         typedef PointerList<Property*> Properties;
         Properties properties;

         virtual void openBracket( const ASCString& name );
         virtual void closeBracket();

         void addString ( const ASCString& name, ASCString& property );
         void addStringArray ( const ASCString& name, vector<ASCString>& property );
         void addInteger ( const ASCString& name, int& property );
         void addInteger ( const ASCString& name, int& property, int defaultValue );
         void addIntegerArray ( const ASCString& name, vector<int>& property );
         void addDFloatArray ( const ASCString& name, vector<double>& property );
         void addIntRangeArray ( const ASCString& name, vector<IntRange>& property, bool required = true );
         void addTagArray ( const ASCString& name, BitSet& property, int tagNum, const char** tags, bool inverted = false );
         void addTagInteger ( const ASCString& name, int& property, int tagNum, const char** tags, bool inverted = false );
         void addTagInteger ( const ASCString& name, int& property, int tagNum, const char** tags, int defaultValue, bool inverted = false );
         void addNamedInteger ( const ASCString& name, int& property, int tagNum, const char** tags );
         void addNamedInteger ( const ASCString& name, int& property, int tagNum, const char** tags, int defaultValue );
         void addImage ( const ASCString& name, void* &property, const ASCString& fileName );
         void addImageArray ( const ASCString& name, vector<void*> &property, const ASCString& fileName );
         void addBool  ( const ASCString& name, bool &property );
         void addBool  ( const ASCString& name, bool &property, bool defaultValue  );

         void run ( );
         bool isReading() { return reading; };
         void warning ( const ASCString& errmsg );
         void error ( const ASCString& errmsg );
         bool find ( const ASCString& name );
         virtual ASCString getFileName (  ) = 0;
         virtual ~PropertyContainer ( ) { };
      protected:
         PropertyContainer ( const ASCString& baseName, TextPropertyGroup* tpg, bool reading_ ) : reading( reading_ ), levelDepth ( 0 ), textPropertyGroup( tpg ) { };
         virtual ASCString getLocation() = 0;
      private:
         virtual void writeProperty ( Property& p, const ASCString& value ) = 0;
         void setup ( Property* p, const ASCString& name );

};

class PropertyReadingContainer : public PropertyContainer {
   protected:
         virtual ASCString getLocation() { return textPropertyGroup->location; };
   public:
         virtual ASCString getFileName (  ) { return textPropertyGroup->fileName; };
         PropertyReadingContainer ( const ASCString& baseName, TextPropertyGroup* tpg );
         ~PropertyReadingContainer (  );
         void writeProperty ( Property& p, const ASCString& value );
};

class PropertyWritingContainer : public PropertyContainer {
         tn_file_buf_stream stream;
   protected:
         virtual ASCString getLocation() { return stream.getLocation(); };
   public:
         virtual ASCString getFileName (  ) { return stream.getDeviceName(); };
         PropertyWritingContainer ( const ASCString& baseName, const ASCString& filename_ );
         ~PropertyWritingContainer();

         void writeProperty ( Property& p, const ASCString& value );
         virtual void openBracket( const ASCString& name );
         virtual void closeBracket();
};

extern void* getFieldMask();

#endif
