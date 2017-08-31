//
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

//
// http://minus9d.hatenablog.com/entry/2014/03/21/003350
//

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <math.h>
#include <stdio.h>

#pragma comment(lib, "opencv_core2410.lib")
#pragma comment(lib, "opencv_highgui2410.lib")

extern double pitch;
extern int view_num;
extern int mirror;
extern int print;

extern int WX, WY;
extern int XT, YT;

extern double Lvalue;

void mergePt(unsigned char *s, unsigned char *d, int map, int x, int y);

int alloc_MAP();
void free_MAP();
void make_MAP();
cv::Mat temp;

cv::Mat view1;
cv::Mat view2;
cv::Mat disp;
cv::Mat calib;

char execpath[1024];

#ifdef _WIN32

#elif __linux__

#elif __APPLE__
#include <mach-o/dyld.h> //_NSGetExecutablePath
#endif

int getexecpath() {
  int ret = 0;

#ifdef _WIN32

#elif __linux__

#elif __APPLE__
  uint32_t size = sizeof(execpath);
  if (_NSGetExecutablePath(execpath, &size) == 0) {
    printf("executable path is %s\n", execpath);
    ret = 0;
  } else {
    printf("buffer too small; need size %u\n", size);
    ret = 1;
  }

  size_t len = strlen(execpath);
  while (1) {
    if (execpath[len] == '/')
      break;
    len--;
  }
  execpath[len + 1] = '\0';
  printf("path is %s\n", execpath);
#endif

  return ret;
}

int adjust_2v(unsigned char *v1, unsigned char *v2, unsigned char *d) {

  view_num = ((int(floor(pitch + 1 + 0.5))) / 2 + 1) * 2;

  // alloc map memory
  int mem_alloc = 0;

  int r = alloc_MAP();
  switch (r) {
  case 2:
    printf("\n---error:malloc:1---\n");
    goto err;
    break;
  case 1:
    printf("\n---out of memory---\n");
    goto err;
    break;
  }

  mem_alloc = 1;
  make_MAP();
  for (int i = 0; i < WX * WY * 3; i++)
    d[i] = 0;

  for (int i = 0; i < view_num; i++) {

    if (i < view_num / 2) {
      for (int y = 0; y < WY; y++) {
        for (int x = 0; x < WX; x++) {
          mergePt(v1, d, i, x, y);
        }
      }
    } else {
      for (int y = 0; y < WY; y++) {
        for (int x = 0; x < WX; x++) {
          mergePt(v2, d, i, x, y);
        }
      }
    }
  }
err:
  if (mem_alloc)
    free_MAP();

  return r;
}

bool dragging = false;

int clickX;
int dtX;
int nowX;

int centerX;
int geta;
char msg[64];

#define STATE_BEGIN 0
#define STATE_SLANT 1
#define STATE_PITCH 2
#define STATE_QUIT 3
#define STATE_DISP 4
#define STATE_RDISP 5

int state = STATE_SLANT;
int state0;

int button_w;
int button_h;
int sbx1, sbx2, sby1, sby2;
int pbx1, pbx2, pby1, pby2;
int qbx2, qbx1, qby1, qby2;

int dbx1, dbx2, dby1, dby2;
int rbx1, rbx2, rby1, rby2;

int dx1, dx2, dy0, dy1, dy2, dy3, dy4, dy5;

int workx1, workx2, worky1, worky2;

bool area_slant = false;
bool area_pitch = false;
bool area_quit = false;
bool area_work = false;
bool area_digit[5] = {false, false, false, false, false};
bool area_disp = false;
bool area_rdisp = false;

bool pitch_in = false;

#define COL_BUTTON cv::Scalar(200, 200, 200)
#define COL_BUTTON_HI cv::Scalar(240, 240, 240)
#define COL_BLACK cv::Scalar(0, 0, 0)
#define COL_RED cv::Scalar(0, 0, 200)
#define COL_YELLOW cv::Scalar(0, 255, 255)
#define COL_GRAY cv::Scalar(100, 100, 100)
#define COL_DARKGRAY cv::Scalar(50, 50, 50)

#define COL_VIEW_1 cv::Scalar(0, 255, 255) // yellow
#define COL_VIEW_2 cv::Scalar(255, 0, 255) // cyan

double ptmp = 0;

void calc_pitch() {
  double t = pitch;
  if (area_digit[0])
    pitch = ptmp + dtX;
  if (area_digit[1])
    pitch = ptmp + double(dtX) / 10;
  if (area_digit[2])
    pitch = ptmp + double(dtX) / 100;
  if (area_digit[3])
    pitch = ptmp + double(dtX) / 1000;
  if (area_digit[4])
    pitch = ptmp + double(dtX) / 10000;

  if (pitch < 2.0)
    pitch = t;
}

void my_mouse_callback(int event, int x, int y, int flags, void *param) {

#define SLANT_AREA (x >= sbx1) && (x <= sbx2) && (y >= sby1) && (y <= sby2)
#define PITCH_AREA (x >= pbx1) && (x <= pbx2) && (y >= pby1) && (y <= pby2)
#define QUIT_AREA (x >= qbx1) && (x <= qbx2) && (y >= qby1) && (y <= qby2)
#define DISP_AREA (x >= dbx1) && (x <= dbx2) && (y >= dby1) && (y <= dby2)
#define RDISP_AREA (x >= rbx1) && (x <= rbx2) && (y >= rby1) && (y <= rby2)
#define WORK_AREA                                                              \
  (x >= workx1) && (x <= workx2) && (y >= worky1) && (y <= worky2)

#define DIGI_1_AREA (x >= dx1) && (x <= dx2) && (y >= dy0) && (y < dy1)
#define DIGI_2_AREA (x >= dx1) && (x <= dx2) && (y >= dy1) && (y < dy2)
#define DIGI_3_AREA (x >= dx1) && (x <= dx2) && (y >= dy2) && (y < dy3)
#define DIGI_4_AREA (x >= dx1) && (x <= dx2) && (y >= dy3) && (y < dy4)
#define DIGI_5_AREA (x >= dx1) && (x <= dx2) && (y >= dy4) && (y < dy5)

  switch (event) {
  case cv::EVENT_MOUSEMOVE:
    if (SLANT_AREA)
      area_slant = true;
    else
      area_slant = false;
    if (PITCH_AREA)
      area_pitch = true;
    else
      area_pitch = false;
    if (DISP_AREA)
      area_disp = true;
    else
      area_disp = false;
    if (RDISP_AREA)
      area_rdisp = true;
    else
      area_rdisp = false;
    if (QUIT_AREA)
      area_quit = true;
    else
      area_quit = false;
    if (!dragging) {
      (DIGI_1_AREA) ? area_digit[0] = true : area_digit[0] = false;
      (DIGI_2_AREA) ? area_digit[1] = true : area_digit[1] = false;
      (DIGI_3_AREA) ? area_digit[2] = true : area_digit[2] = false;
      (DIGI_4_AREA) ? area_digit[3] = true : area_digit[3] = false;
      (DIGI_5_AREA) ? area_digit[4] = true : area_digit[4] = false;
    }

    if (WORK_AREA) {
      area_work = true;

      if (dragging) {
        dtX = x - clickX;
        if (state == STATE_PITCH)
          dtX /= 10;
      }
    } else
      area_work = false;
    break;

  case cv::EVENT_LBUTTONDOWN:
    if (area_slant)
      state = STATE_SLANT;
    if (area_pitch) {
      state = STATE_PITCH;
      pitch_in = true;
    }
    if (area_disp) {
      state0 = state;
      state = STATE_DISP;
    }
    if (area_rdisp) {
      state0 = state;
      state = STATE_RDISP;
    }

    if (area_quit)
      state = STATE_QUIT;

    if (area_work) {
      dragging = true;
      clickX = x;
      if (state == STATE_PITCH)
        ptmp = pitch;
    }
    break;

  case cv::EVENT_LBUTTONUP:
    if (area_work) {
      dragging = false;
      if (state == STATE_SLANT)
        nowX += dtX;
      dtX = 0;
    }
    break;
  }
}

void draw_button(const char *const s, int x1, int x2, int y1, int y2, cv::Mat m,
                 bool li, int hi, double size) {
  cv::Scalar col;
  if (hi)
    col = COL_BUTTON_HI;
  else
    col = COL_BUTTON;
  cv::rectangle(m, cv::Point(x1, y1), cv::Point(x2, y2), col, -1, 4);
  cv::rectangle(m, cv::Point(x1, y1), cv::Point(x2, y2), COL_BLACK, 1, 4);
  sprintf(msg, "%s", s);
  if (li)
    col = COL_YELLOW;
  else
    col = COL_RED;
  cv::putText(m, msg, cv::Point(x1 + button_w / 4, y1 + (button_h / 5) * 3),
              cv::FONT_HERSHEY_SIMPLEX, size, col, 2, CV_AA);
}

void display() {
  char fn[255];
  char fnbase[] = "pic/blender_00000_";
  char ext[] = ".jpg";

  int view_num0 = view_num;
  int WX0 = WX, WY0 = WY;

  cv::Mat disp;
  cv::Mat src;
  int ct = 0;

  sprintf(fn, "%s%s%d%s", execpath, fnbase, ct, ext);
  disp = cv::imread(fn);
  if (disp.data == NULL) {
    sprintf(ext, ".png");
    sprintf(fn, "%s%s%d%s", execpath, fnbase, ct, ext);
    printf("%s\n", fn);
    disp = cv::imread(fn);
  }
  if (disp.data != NULL) {
    WX = disp.cols;
    WY = disp.rows;

    while (1) {
      sprintf(fn, "%s%s%d%s", execpath, fnbase, ct, ext);
      src = cv::imread(fn);
      if (src.data == NULL)
        break;
      ct++;
    }
    view_num = ct;

    // alloc map memory
    int mem_alloc = 0;

    int r = alloc_MAP();
    switch (r) {
    case 2:
      printf("\n---error:malloc:1---\n");
      goto display_err;
      break;
    case 1:
      printf("\n---out of memory---\n");
      goto display_err;
      break;
    }

    mem_alloc = 1;
    make_MAP();

    for (int i = 0; i < WX * WY * 3; i++)
      disp.data[i] = 0;

    int mapP;
    for (int i = 0; i < view_num; i++) {
      sprintf(fn, "%s%s%d%s", execpath, fnbase, i, ext);
      src = cv::imread(fn);

      if (state == STATE_RDISP)
        mapP = view_num - i - 1;
      else
        mapP = i;
      for (int y = 0; y < WY; y++) {
        for (int x = 0; x < WX; x++) {
          mergePt(src.data, disp.data, mapP, x, y);
        }
      }
    }
    cv::imshow("DISP", disp);
  display_err:
    if (mem_alloc)
      free_MAP();
  }
  view_num = view_num0;
  WX = WX0, WY = WY0;
  state = state0;
}

int main(int argc, const char *argv[]) {
  const char *keys = {"{w width  |640 |width}"
                      "{h height |512 |height }"
                      "{p pitch  |8.5 |pitch  }"};
  cv::CommandLineParser parser(argc, argv, keys);

  WX = parser.get<int>("w");
  WY = parser.get<int>("h");
  pitch = parser.get<double>("p");

  printf("[ Neil and Eliza autostereoscopic-image calibrator ]\n");

  cv::Mat line_work(cv::Size(WX * 3, WY), CV_8UC1, COL_DARKGRAY);
  cv::Mat line_temp;

  getexecpath();

  cv::Scalar col;

  int mlt;
  mlt = ((WX * 100) << 1) / 1000;
  mlt = (mlt >> 1) + (mlt & 1);

  geta = (100 * mlt) / 100;

  centerX = line_work.cols / 2;
  button_w = (149 * mlt) / 100;
  button_h = geta - 1;
  sbx1 = 0;
  sbx2 = sbx1 + button_w;
  sby1 = 0;
  sby2 = sby1 + button_h;
  pbx1 = sbx2 + 1;
  pbx2 = pbx1 + button_w;
  pby1 = 0;
  pby2 = pby1 + button_h;
  dbx1 = pbx2 + 1;
  dbx2 = dbx1 + button_w;
  dby1 = 0;
  dby2 = dby1 + button_h;
  rbx1 = dbx2 + 1;
  rbx2 = rbx1 + button_w;
  rby1 = 0;
  rby2 = dby1 + button_h;

  qbx2 = WX - 1;
  qbx1 = qbx2 - button_w;
  qby1 = 0;
  qby2 = qby1 + button_h;

  workx1 = 0;
  workx2 = WX - 1;
  worky1 = geta;
  worky2 = WY - geta - 1;

  double fnsize = (1.0 * mlt) / 100;

  std::string name = "Neil and Eliza calibration";

  cv::Mat image(cv::Size(WX, WY), CV_8UC3, COL_GRAY);
  temp = image.clone();
  view1 = image.clone();
  view2 = image.clone();
  calib = image.clone();

  cv::namedWindow(name, CV_WINDOW_AUTOSIZE);

  cv::setMouseCallback(name, my_mouse_callback, (void *)&image);

  dx1 = 0;
  dx2 = WX - 1;
  dy0 = geta;
  dy1 = geta + ((WY - geta * 2) / 5);
  dy2 = geta + ((WY - geta * 2) * 2) / 5;
  dy3 = geta + ((WY - geta * 2) * 3) / 5;
  dy4 = geta + ((WY - geta * 2) * 4) / 5;
  dy5 = geta + ((WY - geta * 2) * 5) / 5;

//
// http://tanjoin.hatenablog.com/entry/20121114/1352891706
//
#ifdef _WIN32
#define CV_WAITKEY_CURSORKEY_TOP 2490368
#define CV_WAITKEY_CURSORKEY_BOTTOM 2621440
#define CV_WAITKEY_CURSORKEY_RIGHT 2555904
#define CV_WAITKEY_CURSORKEY_LEFT 2424832

#elif __linux__
#define CV_WAITKEY_CURSORKEY_TOP 82
#define CV_WAITKEY_CURSORKEY_BOTTOM 84
#define CV_WAITKEY_CURSORKEY_RIGHT 83
#define CV_WAITKEY_CURSORKEY_LEFT 81

#elif __APPLE__
#define CV_WAITKEY_CURSORKEY_TOP 63232
#define CV_WAITKEY_CURSORKEY_BOTTOM 63233
#define CV_WAITKEY_CURSORKEY_RIGHT 63235
#define CV_WAITKEY_CURSORKEY_LEFT 63234
#endif

#define CV_WAITKEY_ENTER 13
#define CV_WAITKEY_ESC 27
#define CV_WAITKEY_SPACE 32
#define CV_WAITKEY_TAB 9

  while (1) {
    image.copyTo(temp);

    int c = cv::waitKey(15);
    bool cursor = false;
    switch (c) {
    case CV_WAITKEY_ESC:
      state = STATE_QUIT;
      break;
    case CV_WAITKEY_CURSORKEY_LEFT:
    case 'a':
    case 'A':
      if ((state == STATE_SLANT) && (!dragging))
        nowX--;
      if ((state == STATE_PITCH) && (!dragging)) {
        dtX--;
        cursor = true;
      }
      break;
    case CV_WAITKEY_CURSORKEY_RIGHT:
    case 'd':
    case 'D':
      if ((state == STATE_SLANT) && (!dragging))
        nowX++;
      if ((state == STATE_PITCH) && (!dragging)) {
        dtX++;
        calc_pitch();
        cursor = true;
      }
      break;
    }
    if (cursor) {
      ptmp = pitch;
      calc_pitch();
      pitch_in = true;
      dtX = 0;
    }

    if (state == STATE_QUIT)
      break;

    if ((nowX + dtX) < 0)
      mirror = 1;
    else
      mirror = 0;

#define XY2PT(x, y, widex) (((y) * (widex)) + (x))
    if (state == STATE_SLANT) {
      line_work.copyTo(line_temp);
      cv::line(line_temp, cv::Point(centerX + nowX + dtX, geta),
               cv::Point(centerX - nowX - dtX, WY - geta), cv::Scalar(255), 1,
               CV_AA);
      for (int y = geta; y < (WY - geta) + 1; y++) {
        for (int x = 0; x < WX; x++) {
          temp.data[XY2PT(x * 3 + 2, y, WX * 3)] =
              line_temp.data[XY2PT(x * 3, y, WX * 3)];
          temp.data[XY2PT(x * 3 + 1, y, WX * 3)] =
              line_temp.data[XY2PT(x * 3 + 1, y, WX * 3)];
          temp.data[XY2PT(x * 3, y, WX * 3)] =
              line_temp.data[XY2PT(x * 3 + 2, y, WX * 3)];
        }
      }
    }

    if (state == STATE_PITCH) {
      if ((dragging) || (pitch_in)) {
        pitch_in = false;
        if (!cursor)
          calc_pitch();
        view1.setTo(COL_VIEW_1);
        view2.setTo(COL_VIEW_2);

        switch (adjust_2v(view1.data, view2.data, calib.data)) {
        case 2:
          printf("\n---error:malloc:1---\n");
          goto end;
          break;
        case 1:
          printf("\n---out of memory---\n");
          goto end;
          break;
        }
      }
      calib.copyTo(temp);
    }

    //
    draw_button("slant", sbx1, sbx2, sby1, sby2, temp, area_slant,
                state == STATE_SLANT, fnsize);
    draw_button("pitch", pbx1, pbx2, pby1, pby2, temp, area_pitch,
                state == STATE_PITCH, fnsize);
    draw_button("disp >", dbx1, dbx2, dby1, dby2, temp, area_disp,
                state == STATE_DISP, fnsize);
    draw_button("disp <", rbx1, rbx2, rby1, rby2, temp, area_rdisp,
                state == STATE_RDISP, fnsize);
    draw_button("quit", qbx1, qbx2, qby1, qby2, temp, area_quit,
                state == STATE_QUIT, fnsize);

    if (state == STATE_SLANT) {
      XT = abs((centerX - nowX - dtX) - (centerX + nowX + dtX));
      YT = ((WY - geta) - geta) * 3;
    }

    if ((state == STATE_DISP) || (state == STATE_RDISP))
      display();

    sprintf(msg, "XT:%5d", XT);
    cv::putText(temp, msg, cv::Point((WX * 5) / 100, WY - (geta * 3) / 5),
                cv::FONT_HERSHEY_SIMPLEX, fnsize, COL_RED, 2, CV_AA);

    sprintf(msg, "YT:%5d", YT);
    cv::putText(temp, msg, cv::Point((WX * 5) / 100, WY - (geta * 2) / 10),
                cv::FONT_HERSHEY_SIMPLEX, fnsize, COL_RED, 2, CV_AA);

    sprintf(msg, "pitch:");
    cv::putText(temp, msg, cv::Point(WX / 2, WY - geta / 2),
                cv::FONT_HERSHEY_SIMPLEX, fnsize, COL_RED, 2, CV_AA);
    sprintf(msg, "%8.4f", pitch);
    char s[2];
    for (int i = 0; i < 8; i++) {
      s[0] = msg[i];
      s[1] = '\0';
      col = COL_RED;
      if (state == STATE_PITCH) {
        if (area_digit[0] && (i == 2))
          col = COL_YELLOW;
        if (area_digit[1] && (i == 4))
          col = COL_YELLOW;
        if (area_digit[2] && (i == 5))
          col = COL_YELLOW;
        if (area_digit[3] && (i == 6))
          col = COL_YELLOW;
        if (area_digit[4] && (i == 7))
          col = COL_YELLOW;
      }

      cv::putText(temp, s,
                  cv::Point(WX / 2 + geta + i * (geta / 5), WY - geta / 2),
                  cv::FONT_HERSHEY_SIMPLEX, fnsize, col, 2, CV_AA);
    }

    if (mirror) {
      fprintf(stderr, "param: -x %5d -y %5d -pitch %8.4f -mirror\r", XT, YT,
              pitch);
      fflush(stderr);
    } else {
      fprintf(stderr, "param: -x %5d -y %5d -pitch %8.4f        \r", XT, YT,
              pitch);
      fflush(stderr);
    }
    cv::imshow(name, temp);

    if (cursor) {
      while (cv::waitKey(15) != -1)
        ;
    }
  }
end:
  return 0;
}
