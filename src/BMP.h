
/*=============================================================================
  bmplib, a simple library to create, modify, and write BMP image files.
  Derived from bandwidth developed by Zack T Smith <fbui@comcast.net>.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *============================================================================*/

#ifndef _BMP_H
#define _BMP_H

typedef struct {
	int width, height;
	unsigned long *pixels;
} BMP;

#define MINIFONT_HEIGHT (8)

extern BMP *BMP_new(int, int);
extern void BMP_delete(BMP *);
extern void BMP_clear(BMP *, unsigned long);
extern int BMP_write(BMP *, char *path);
extern void BMP_point(BMP *, int, int, unsigned long);
extern void BMP_line(BMP *, int x0, int y0, int x1, int y1, unsigned long);
extern void BMP_line_dashed(BMP *, int x0, int y0, int x1, int y1,
			    unsigned long);
extern void BMP_hline(BMP *, int x0, int x1, int y, unsigned long);
extern void BMP_vline(BMP *, int x, int y0, int y1, unsigned long);
extern void BMP_rect(BMP *, int x, int y, int w, int h, unsigned long);
extern void BMP_fillrect(BMP *, int x, int y, int w, int h, unsigned long);
extern unsigned long BMP_getpixel(BMP *, int, int);
extern int BMP_draw_mini_string(BMP *, char *, int x, int y, unsigned long);
extern int BMP_mini_string_width(char *);

#define RGB_BLACK (0)
#define RGB_GRAY (0xc0c0c0)
#define RGB_RED (0xff0000)
#define RGB_MAGENTA (0xff00ff)
#define RGB_GREEN (0xff00)
#define RGB_DARKGREEN (0x6400)
#define RGB_BLUE (0xff)
#define RGB_WHITE (0xffffff)
#define RGB_YELLOW (0xffff00)
#define RGB_CYAN (0xffff)
#define RGB_NAVYBLUE (0x80)
#define RGB_ORANGE (0xffa500)
#define RGB_DARKORANGE (0xff8c00)
#define RGB_CORAL (0xff7f50)
#define RGB_PURPLE (0xa020f0)
#define RGB_MAROON (0x800000)
#define RGB_SALMON (0xfa8072)
#define RGB_BRASS (0xc3a368)
#define RGB_LEMONYELLOW (0xfde910)
#define RGB_ROYALBLUE (0x4169e1)
#define RGB_DODGERBLUE (0x1e90ff)
#define RGB_TURQUOISE (0x40e0d0)
#define RGB_CADETBLUE (0x5f9ea0)
#define RGB_CHARTREUSE (0x7fff00)
#define RGB_DARKOLIVEGREEN (0x556b2f)
#define RGB_KHAKI (0xf0e68c)
#define RGB_DARKKHAKI (0xbdb76b)
#define RGB_GOLDENROD (0xdaa520)
#define RGB_VIOLET (0xee82ee)

#endif
