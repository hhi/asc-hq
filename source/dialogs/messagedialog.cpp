/*
     This file is part of Advanced Strategic Command; http://www.asc-hq.de
     Copyright (C) 1994-1999  Martin Bickel  and  Marc Schellenberger
 
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


#include "../global.h"

#include "messagedialog.h"

int extractHotkey( const ASCString& s )
{
   bool found = false;
   for ( int i = 0; i < s.length(); ++i ) {
      if ( found )
         return s[i];
      if ( s[i] == '~' )
         found = true;
   }
   return 0;
}


MessageDialog::MessageDialog(PG_Widget* parent, const PG_Rect& r, const std::string& windowtitle, const std::string& windowtext, const std::string& btn1text, const std::string& btn2text, PG_Label::TextAlign textalign, const std::string& style) :
      ASC_PG_Dialog(parent, r, windowtitle, MODAL, style), hotkey1(0), hotkey2(0)
{


   int buttonWidth = min( 120, r.Width() / 2 - 20 );
   PG_Rect btn1 = PG_Rect( r.Width() / 2 - buttonWidth - 10, r.Height() - 35, buttonWidth, 30 );

   my_btnok = new PG_Button(this, btn1, btn1text);
   my_btnok->SetID(1);
   my_btnok->sigClick.connect(slot(*this, &MessageDialog::handleButton));
   hotkey1 = extractHotkey( btn1text );

   PG_Rect btn2 = btn1;
   btn2.x = r.Width() / 2 + 10;

   my_btncancel = new PG_Button(this, btn2, btn2text);
   my_btncancel->SetID(2);
   my_btncancel->sigClick.connect(slot(*this, &MessageDialog::handleButton));
   hotkey2 = extractHotkey( btn2text );

   Init(windowtext, textalign, style);
}

MessageDialog::MessageDialog(PG_Widget* parent, const PG_Rect& r, const std::string& windowtitle, const std::string& windowtext, const std::string& btn1text, PG_Label::TextAlign textalign, const std::string& style) :
      ASC_PG_Dialog(parent, r, windowtitle, MODAL, style ), hotkey1(0), hotkey2(0), my_btncancel(NULL)
{

   int buttonWidth = min( 120, r.Width() - 20 );
   PG_Rect btn1 = PG_Rect( r.Width() / 2 - buttonWidth/2, r.Height() - 40, buttonWidth, 30 );

   my_btnok = new PG_Button(this, btn1, btn1text);
   my_btnok->SetID(1);
   my_btnok->sigClick.connect(slot(*this, &MessageDialog::handleButton));

   hotkey1 = extractHotkey( btn1text );
   Init(windowtext, textalign, style);
}

MessageDialog::MessageDialog(PG_Widget* parent, const PG_Rect& r, const std::string& windowtitle, const std::string& windowtext, PG_Label::TextAlign textalign, const std::string& style) :
      ASC_PG_Dialog(parent, r, windowtitle, MODAL, style ), hotkey1(0), hotkey2(0), my_btnok(NULL), my_btncancel(NULL)
{

   Init(windowtext, textalign, style);
}

bool MessageDialog::eventKeyDown (const SDL_KeyboardEvent *key)
{
   if (  key->keysym.sym == SDLK_ESCAPE ) {
      quitModalLoop(10);
      return true;
   }
   if (  key->keysym.sym == SDLK_RETURN || key->keysym.sym == SDLK_KP_ENTER ) {
      quitModalLoop(11);
      return true;
   }
   if (  key->keysym.sym == SDLK_SPACE ) {
      quitModalLoop(12);
      return true;
   }

   if (  key->keysym.unicode == hotkey1 && hotkey1 ) {
      quitModalLoop(1);
      return true;
   }

   if (  key->keysym.unicode == hotkey2 && hotkey2 ) {
      quitModalLoop(2);
      return true;
   }

   return false;
}


MessageDialog::~MessageDialog() {
   // delete my_btnok;
   // delete my_btncancel;
}

void MessageDialog::Init(const std::string& windowtext, int textalign, const std::string& style) {

   my_textbox = new PG_RichEdit(this, PG_Rect(10, 40, my_width-20, my_height-50 - 40));
   my_textbox->SendToBack();
   my_textbox->SetTransparency(255);
   my_textbox->SetText(windowtext);

   my_msgalign = textalign;

   LoadThemeStyle(style);
}

void MessageDialog::LoadThemeStyle(const std::string& widgettype) {
   PG_Window::LoadThemeStyle(widgettype);

   if ( my_btnok )
      my_btnok->LoadThemeStyle(widgettype, "Button1");

   if(my_btncancel) {
      my_btncancel->LoadThemeStyle(widgettype, "Button2");
   }
}

bool MessageDialog::handleButton(PG_Button* button)
{
   quitModalLoop( button ? button->GetID() : 0 );
   return true;
}






PG_Rect calcMessageBoxSize( const ASCString& message )
{
   int counter = 0;
   for ( int i = 0; i< message.length(); ++i)
      if ( message[i] == '\n' )
         counter++;

   return PG_Rect( -1, -1, 500, 150 + counter * 20 );
}



void errorMessageDialog( const ASCString& message )
{
   PG_Rect size = calcMessageBoxSize(message);
   MessageDialog msg( NULL, size, "Error", message, "OK", PG_Label::CENTER, "ErrorMessage" );
   msg.Show();
   msg.RunModal();
}

void warningMessageDialog( const ASCString& message )
{
   PG_Rect size = calcMessageBoxSize(message);
   MessageDialog msg( NULL, size, "Warning", message, "OK", PG_Label::CENTER, "WarningMessage" );
   msg.Show();
   msg.RunModal();
}

void infoMessageDialog( const ASCString& message )
{
   PG_Rect size = calcMessageBoxSize(message);
   MessageDialog msg( NULL, size, "Information", message, "OK" );
   msg.Show();
   msg.RunModal();
}


int  new_choice_dlg(const ASCString& title, const ASCString& leftButton, const ASCString& rightButton )
{
   PG_Rect size = calcMessageBoxSize(title);
   MessageDialog msg( NULL, size,"", "", leftButton, rightButton, PG_Label::CENTER, "Window" );
   msg.getTextBox()->SetFontSize( msg.getTextBox()->GetFontSize() + 3 );
   msg.getTextBox()->SetText(title);
      
   msg.Show();
   // PG_Widget::UpdateScreen();
   return msg.RunModal();
}

