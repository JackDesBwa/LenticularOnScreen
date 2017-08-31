//    Copyright 2013,2014 Mercy Yamada.
//    for personal use only. not for commercial use.
//
//    This file is part of "Neil and Eliza".
//
//    "Neil and Eliza" is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    "Neil and Eliza" is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with "Neil and Eliza".  If not, see <http://www.gnu.org/licenses/>.
//

// References about Gupta-Sproull antialiasing algorithm
// http://d.hatena.ne.jp/nodamushi/20101223/1293136222
// http://d.hatena.ne.jp/ousttrue/20090224/1235499257

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double pitch = 7;
int view_num = 8;
int mirror = 0;
int print = 0;

int WX, WY; // size
int XT, YT; // inclination

double Lvalue = 1.0; // luminosity

int mapPos;
int mapW;

int dx, dy;
double xx0, xx1, yy0, yy1;

int XT0, YT0;

unsigned char **mapP;

static int table0[] = {0xc7, 0xc6, 0xc2, 0xbc, 0xb3, 0xa9, 0x9c, 0x8e,
                       0x7f, 0x70, 0x62, 0x54, 0x46, 0x3a, 0x2f, 0x25,
                       0x1c, 0x15, 0x0e, 0x09, 0x05, 0x03, 0x01, 0x00};
static int table[24];

double p1, p2, p3, p4;
inline double distance(double x, double y) {
  return abs(p1 * x - p2 * y - p3) / p4;
}

inline int getAlph(double distance) {
  if (distance >= 1)
    return 0;
  return table[(int)(distance * 23)];
}

int col;

#define XY2PT(x, y, widex) (((y) * (widex)) + (x))

inline void aapset_vertical(double xx, double yy) {
  int x = int(xx);
  int y = int(yy);
  if ((x < 1) || (x >= (mapW - 1)) || (y < 1) || (y >= (WY - 1)))
    goto skipV;
  xx = double(x);
  yy = double(y);
  col = getAlph(distance(xx, yy));
  mapP[mapPos][XY2PT(x, y, mapW)] = (unsigned char)col;
  col = getAlph(distance(xx + 1, yy));
  mapP[mapPos][XY2PT(x + 1, y, mapW)] = (unsigned char)col;
  col = getAlph(distance(xx - 1, yy));
  mapP[mapPos][XY2PT(x - 1, y, mapW)] = (unsigned char)col;
skipV:;
}

inline void aapset_horizontal(double xx, double yy) {
  int x = int(xx);
  int y = int(yy);
  if ((x < 1) || (x >= (mapW - 1)) || (y < 1) || (y >= (WY - 1)))
    goto skipH;
  xx = double(x);
  yy = double(y);
  col = getAlph(distance(xx, yy));
  mapP[mapPos][XY2PT(x, y, mapW)] = (unsigned char)col;
  col = getAlph(distance(xx, yy + 1));
  mapP[mapPos][XY2PT(x, y + 1, mapW)] = (unsigned char)col;
  col = getAlph(distance(xx, yy - 1));
  mapP[mapPos][XY2PT(x, y - 1, mapW)] = (unsigned char)col;
skipH:;
}

inline void GuptaSproull(double SX, double SY) {

  xx0 = SX;
  yy0 = SY;
  xx1 = dx + xx0;
  yy1 = dy + yy0;

  p1 = (yy1 - yy0);
  p2 = (xx1 - xx0);
  p3 = yy1 * xx0 + xx1 * yy0;
  p4 = hypot((xx1 - xx0), (yy1 - yy0));

  double xl, yl;
  bool straight; // Whether it is aligned horizontally or vertically
  bool vertical; // Whether or not to proceed more in the vertical direction

  xl = abs(dx);
  yl = abs(dy);
  straight = yl == 0 || xl == 0;
  vertical = straight ? xl == 0 : yl > xl;

  straight = yl == 0 || xl == 0;
  vertical = straight ? xl == 0 : yl > xl;

  // This time, we do not need.
  if (straight) {
    if (vertical) {
      for (double y = yy0; y != yy1; y++)
        aapset_vertical(SX, y);
    } else {
      for (double x = xx0; x < WX; x++)
        aapset_horizontal(x, SY);
    }
    return;
  }

  if (yl < xl)
    vertical = !vertical;
  if (mirror)
    vertical = !vertical;

  // Is this OK?
  if (vertical) {
    double dt = xl / yl;
    double x, yy;
    if (mirror) {
      x = xx1;
      yy = yy1;
    } else {
      x = xx0;
      yy = yy0;
    }

    for (double y = yy; y < WY; y++) {

      aapset_vertical(x, y);

      if (mirror) {
        x += dt;
      } else {
        x -= dt;
      }
    }
  } else {
    double dt = yl / xl;
    double y, xx;
    if (mirror) {
      y = yy0;
      xx = xx0;
    } else {
      y = yy1;
      xx = xx1;
    }

    for (double x = xx; x < mapW; x++) {

      aapset_horizontal(x, y);

      if (mirror) {
        y += dt;
      } else {
        y -= dt;
      }
    }
  }
}

double init_table(double pitch, int view_num, int mirror) {
  double SizeOfLine = (pitch / view_num) * Lvalue;
  for (int i = 0; i < 24; i++)
    table[i] = int(double(table0[i]) * SizeOfLine);

  double ml;
  if (YT != 0)
    ml = (double(WY) / double(YT));
  else
    ml = (double(WX) / double(XT0));
  dx = double(XT0) * ml;
  dy = double(YT) * ml;

  if (!mirror)
    dx = -dx;

  return dx;
}

#define C2R(c) ((int)(((c)&0xff0000) >> 16))
#define C2G(c) ((int)(((c)&0xff00) >> 8))
#define C2B(c) ((int)((c)&0xff))
#define RGB2C(r, g, b) ((r)*0x10000 + (g)*0x100 + (b))

void test_MAP_ch(unsigned char *d, int map) {
  int c;
  for (int y = 0; y < WY; y++) {
    for (int x = 0; x < WX; x++) {
      c = mapP[map][XY2PT(x, y, mapW)];
      d[XY2PT(x * 3, y, WX * 3)] = c;
      d[XY2PT(x * 3 + 1, y, WX * 3)] = c;
      d[XY2PT(x * 3 + 2, y, WX * 3)] = c;
    }
  }
}
void test_MAP(unsigned int *d, int map) {
  int c;
  for (int y = 0; y < WY; y++) {
    for (int x = 0; x < WX; x++) {
      c = mapP[map][XY2PT(x, y, mapW)];
      d[XY2PT(x, y, WX)] = RGB2C(c, c, c);
    }
  }
}

void test_MAP_full(unsigned int *d, int map) {
  int c;
  for (int y = 0; y < WY; y++) {
    for (int x = 0; x < mapW; x++) {
      c = mapP[map][XY2PT(x, y, mapW)];
      d[XY2PT(x, y, mapW)] = RGB2C(c, c, c);
    }
  }
}

void mergePt_int(unsigned int *s, unsigned int *d, int map, int x, int y) {
  int rs, gs, bs;
  int rd, gd, bd;
  int rm, gm, bm;
  int r, g, b;

  int W;
  int col;
  W = mapW / 3;

  col = s[XY2PT(x, y, W)];
  rs = C2R(col);
  gs = C2G(col);
  bs = C2B(col);

  rm = (int)mapP[map][XY2PT(x * 3, y, mapW)];
  gm = (int)mapP[map][XY2PT(x * 3 + 1, y, mapW)];
  bm = (int)mapP[map][XY2PT(x * 3 + 2, y, mapW)];

  r = (rs * rm) / 255;
  g = (gs * gm) / 255;
  b = (bs * bm) / 255;

  col = d[XY2PT(x, y, W)];
  rd = C2R(col);
  gd = C2G(col);
  bd = C2B(col);

  rd += r;
  gd += g;
  bd += b;

  if (rd > 255)
    rd = 255;
  if (gd > 255)
    gd = 255;
  if (bd > 255)
    bd = 255;

  d[XY2PT(x, y, W)] = RGB2C(rd, gd, bd);
}

void mergePt(unsigned char *s, unsigned char *d, int map, int x, int y) {
  int rs, gs, bs;
  int rd, gd, bd;
  int rm, gm, bm;
  int r, g, b;

  int W;
  W = mapW / 3;

  rs = s[XY2PT(x * 3, y, mapW)];
  gs = s[XY2PT(x * 3 + 1, y, mapW)];
  bs = s[XY2PT(x * 3 + 2, y, mapW)];

  rm = (int)mapP[map][XY2PT(x * 3 + 2, y, mapW)];
  gm = (int)mapP[map][XY2PT(x * 3 + 1, y, mapW)];
  bm = (int)mapP[map][XY2PT(x * 3, y, mapW)];

  r = (rs * rm) / 255;
  g = (gs * gm) / 255;
  b = (bs * bm) / 255;

  rd = d[XY2PT(x * 3, y, mapW)];
  gd = d[XY2PT(x * 3 + 1, y, mapW)];
  bd = d[XY2PT(x * 3 + 2, y, mapW)];

  rd += r;
  gd += g;
  bd += b;

  if (rd > 255)
    rd = 255;
  if (gd > 255)
    gd = 255;
  if (bd > 255)
    bd = 255;

  d[XY2PT(x * 3, y, mapW)] = rd;
  d[XY2PT(x * 3 + 1, y, mapW)] = gd;
  d[XY2PT(x * 3 + 2, y, mapW)] = bd;
}

void mergePt00(unsigned char *s, unsigned char *d, int map, int x, int y) {
  int rs, gs, bs;
  int rd, gd, bd;
  int rm, gm, bm;
  int r, g, b;

  int W;
  W = mapW / 3;

  rs = s[XY2PT(x * 3, y, mapW)];
  gs = s[XY2PT(x * 3 + 1, y, mapW)];
  bs = s[XY2PT(x * 3 + 2, y, mapW)];

  rm = (int)mapP[map][XY2PT(x * 3, y, mapW)];
  gm = (int)mapP[map][XY2PT(x * 3 + 1, y, mapW)];
  bm = (int)mapP[map][XY2PT(x * 3 + 2, y, mapW)];

  r = (rs * rm) / 255;
  g = (gs * gm) / 255;
  b = (bs * bm) / 255;

  rd = d[XY2PT(x * 3, y, mapW)];
  gd = d[XY2PT(x * 3 + 1, y, mapW)];
  bd = d[XY2PT(x * 3 + 2, y, mapW)];

  rd += r;
  gd += g;
  bd += b;

  if (rd > 255)
    rd = 255;
  if (gd > 255)
    gd = 255;
  if (bd > 255)
    bd = 255;

  d[XY2PT(x * 3, y, mapW)] = rd;
  d[XY2PT(x * 3 + 1, y, mapW)] = gd;
  d[XY2PT(x * 3 + 2, y, mapW)] = bd;
}

void mergePt_prn(unsigned int *s, unsigned int *d, int map, int x, int y) {
  int rs, gs, bs;
  int rd, gd, bd;
  int rm, gm, bm;
  int r, g, b;

  int W;
  int col;
  W = mapW;

  col = s[XY2PT(x, y, W)];
  rs = C2R(col);
  gs = C2G(col);
  bs = C2B(col);

  rm = (int)mapP[map][XY2PT(x * 3, y, mapW)];
  gm = (int)mapP[map][XY2PT(x * 3 + 1, y, mapW)];
  bm = (int)mapP[map][XY2PT(x * 3 + 2, y, mapW)];

  r = (rs * rm) / 255;
  g = (gs * gm) / 255;
  b = (bs * bm) / 255;

  col = d[XY2PT(x, y, W)];
  rd = C2R(col);
  gd = C2G(col);
  bd = C2B(col);

  rd += r;
  gd += g;
  bd += b;

  if (rd > 255)
    rd = 255;
  if (gd > 255)
    gd = 255;
  if (bd > 255)
    bd = 255;

  d[XY2PT(x, y, W)] = RGB2C(rd, gd, bd);
}

int alloc_MAP()
// Normal：0
// One step error：1
// Two-step error：2
{

  if (print)
    mapW = WX;
  else {
    mapW = WX * 3;
    XT0 = XT * 3;
  }

  int sx = mapW;
  int sy = WY;

  if ((mapP = (unsigned char **)malloc((view_num) * sizeof(unsigned char *))) ==
      NULL) {
    return 1;
  }
  for (int i = 0; i < view_num; i++) {
    if ((mapP[i] = (unsigned char *)malloc(((sx) * (sy)) *
                                           sizeof(unsigned char))) == NULL) {
      return 2;
    }
  }

  for (int i = 0; i < view_num; i++)
    for (int p = 0; p < (sx * sy); p++)
      mapP[i][p] = 0;

  return 0;
}

void free_MAP() {
  for (int i = 0; i < view_num; i++)
    free(mapP[i]);
}

void make_MAP() {
  double dx = init_table(pitch, view_num, mirror);

  double dt = pitch / double(view_num);

  int ct = 0;
  double pt = 0;

  double ep;

  if (mirror) {
    pt = -dx;
    ep = mapW;
  } else {
    pt = 0;
    ep = mapW - dx;
  }

  while (pt < ep) {
    mapPos = ct;

    GuptaSproull(pt, 0);

    pt += dt;
    ct++;
    if (ct == view_num)
      ct = 0;
  }
}
