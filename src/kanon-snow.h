// KANON snowdrops animation
// by Zhidao
//
// 2003.10.29. Created.
// 2020. 9. 8. Last updated.

#ifndef __KANON_SNOW_H
#define __KANON_SNOW_H

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <zeda/zeda.h>
#include <zx11/zxfont.h>
#include <zx11/zxinput.h>
#include <zx11/zxsprite.h>
#include <math.h>

using namespace std;

#define KS_WIDTH  640
#define KS_HEIGHT 480

#define KS_FONT "-*-*-*-*-*--17-*-*-*-*-*-*"

#define KS_SNOW_AMP_S         8
#define KS_SNOW_AMP_L        12
#define KS_SNOW_SPEED_S_MIN   1
#define KS_SNOW_SPEED_S_MAX   2
#define KS_SNOW_SPEED_L_MIN   3
#define KS_SNOW_SPEED_L_MAX   6
#define KS_SNOW_DPHASE        0.05

#ifndef KS_DEFAULT_MSG_PATH
#define KS_DEFAULT_MSG_PATH "~/.kanon-snow"
#endif

#ifndef KS_DEFAULT_MSG
#define KS_DEFAULT_MSG "message"
#endif

#define KS_MSG_DY            50

#define KS_MSG_FADE_DY       10
#define KS_MSG_SPEED          1
#define KS_MSG_LAST_COUNT   100

#define KS_SNOW_S             0
#define KS_SNOW_L             1
#define KS_SNOW_S_NUM        16
#define KS_SNOW_L_NUM         8

#define KS_INTERVAL    30000000

// memory allocation error
inline void memalloc_error_abort()
{
  cerr << "メモリが足りません." << endl;
  exit( 1 );
}

// snow movement property
struct ksSnow{
  double x, y;
  double phase, amp, speed;

  ksSnow(){}

  void init(short ox, short oy){
    x = double(ox);
    y = double(oy);
    set_phase( zRandF( -M_PI, M_PI ) );
  }
  void set_phase(double p){ phase = p; }
  void set_amp(double a){ amp = a; }
  void set_speed(double spd){ speed = spd; }
};
zListClass( ksSnowList, ksSnowCell, ksSnow );

// message property

struct ksMessage{
  char *msg;
  double x, y;

  ksMessage(){
    msg = NULL;
    x = 0.5*KS_WIDTH;
    y = 0.5*KS_HEIGHT - KS_MSG_DY - KS_MSG_FADE_DY;
  }
};
zListClass( ksMessageList, ksMessageCell, ksMessage );

// canvas control
class ksCanvasControl{
  zxWindow win;
  zxSprite spr;
  zxsLayer layer;
  ksSnowList snow_list;
  ksMessageList msg_list;

  void read_background();
  void read_pattern();
  void snow_s_init();
  void snow_l_init();
  void update_snow(zxsPattern *pat, ksSnow *sp);

  void draw_msg_1(ksMessage *mp, double alpha);
  void draw_msg_normal(ksMessageCell **mp);
  void draw_msg_last(ksMessageCell **mp);

  void read_msg();
  void draw_msg();

  void update();
  void draw();
 public:
  ksCanvasControl();
  ~ksCanvasControl();

  void read();
  void open_window();
  void snow_init();
  void loop();
};

#endif // __KANON_SNOW_H
