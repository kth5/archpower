/*
 *  PS3 AV settings.
 *
 *  Copyright (C) 2006 Sony Computer Entertainment Inc.
 *  Copyright 2006 Sony Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  As a special exception, you may use this file as part of a free software
 *  library without restriction.  Specifically, if other files instantiate
 *  templates or use macros or inline functions from this file, or you compile
 *  this file and link it with other files to produce an executable, this
 *  file does not by itself cause the resulting executable to be covered by
 *  the GNU General Public License.  This exception does not however
 *  invalidate any other reasons why the executable file might be covered by
 *  the GNU General Public License.
 */

#if !defined(_B9AAF29B_E200_4804_82F1_A8EAC824F1F5_H)
#define _B9AAF29B_E200_4804_82F1_A8EAC824F1F5_H

#if defined(HAVE_ASM_PS3AV_H)
#include <asm/ps3av.h>
#endif

#if !defined(HAVE_DECL_PS3AV_MODE_MASK) || !HAVE_DECL_PS3AV_MODE_MASK
#define PS3AV_MODE_MASK      0x000F
#endif
#if !defined(HAVE_DECL_PS3AV_MODE_RGB) || !HAVE_DECL_PS3AV_MODE_RGB
#define PS3AV_MODE_RGB       0x0020
#endif
#if !defined(HAVE_DECL_PS3AV_MODE_FULL) || !HAVE_DECL_PS3AV_MODE_FULL
#define PS3AV_MODE_FULL      0x0080
#endif
#if !defined(HAVE_DECL_PS3AV_MODE_WHITE) || !HAVE_DECL_PS3AV_MODE_WHITE
#define PS3AV_MODE_WHITE     0x0200
#endif
#if !defined(HAVE_DECL_PS3AV_MODE_COLOR) || !HAVE_DECL_PS3AV_MODE_COLOR
#define PS3AV_MODE_COLOR     0x0400
#endif
#if !defined(HAVE_DECL_PS3AV_MODE_DITHER) || !HAVE_DECL_PS3AV_MODE_DITHER
#define PS3AV_MODE_DITHER    0x0800
#endif
#if !defined(HAVE_DECL_PS3AV_MODE_HDCP_OFF) || !HAVE_DECL_PS3AV_MODE_HDCP_OFF
#define PS3AV_MODE_HDCP_OFF  0x1000 /* Retail PS3 doesn't support this */
#endif

#endif
