/*
 * SDLmm - a C++ wrapper for SDL and related libraries
 * Copyright � 2001 David Hedbor <david@hedbor.org>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef SDLMM_MISC_H
#define SDLMM_MISC_H

namespace SDLmm {
  //! Find the maximum of two values.
    template <class T>
    inline const T& Max(const T& t1, const T& t2)
    {
        if (t1 < t2)
            return t2;
        else
            return t1;
    }
    
  //! Find the minimum of two values.
    template <class T>
    inline const T& Min(const T& t1, const T& t2)
    {
        if (t1 < t2)
            return t1;
        else
            return t2;
    }
}

#endif // SDLMM_MISC_H
