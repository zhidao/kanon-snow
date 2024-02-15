// kanon snowdrops animation
// by Zhidao
//
// 2003.10.29. Created.
// 2024. 2.16. Last updated.

#include <unistd.h>
#include "kanon-snow.h"

#include "snow.s.xpm"
#include "snow.l.xpm"

#define VERSION "2.1"

enum{
  KS_HELP = 0,
  KS_MSG_PATH,
  KS_MSG_FILE,
  KS_VERBOSE,
  KS_INVALID,
};
zOption option[] = {
  { "h", "help",    NULL,             "このオプション一覧を表示", NULL, false },
  { "p", "path",    "<message path>", "メッセージファイルのあるディレクトリ", (char*)KS_DEFAULT_MSG_PATH, false },
  { "m", "msg",     "<message file>", "特定のメッセージファイル", (char*)KS_DEFAULT_MSG, false },
  { "v", "verbose", NULL,             "実行状況を逐一報告",       NULL, false },
  { NULL, NULL, NULL, NULL, NULL, false },
};

// canvas control

#define KS_TITLE "KANON雪降るアニメーション ver." VERSION
ksCanvasControl::ksCanvasControl()
{
  zListInit( &snow_list );
  zListInit( &msg_list );
  zxsLayerInit( &layer );
  zxsLayerCreatePixArray( &layer, 2 ); // 0:small 1:large snowdrop

  zxWindowCreate( &win, 0, 0, KS_WIDTH, KS_HEIGHT );
  zxWindowSetTitleMB( &win, KS_TITLE );
  zxWindowKeyEnable( &win );
  zxSpriteCreate( &win, &spr, 0, 0, KS_WIDTH, KS_HEIGHT );
  zxSpriteAddLayer( &spr, &layer );
  zxWindowDoubleBufferSetSprite( &win, &spr );
}

ksCanvasControl::~ksCanvasControl()
{
  zxsLayerDestroy( &layer );
  zListDestroy( ksSnowCell, &snow_list );
  zListDestroy( ksMessageCell, &msg_list );
  zxWindowClose( &win );
  zxExit();
}

void ksCanvasControl::read_background()
{
  zxPMCreate( &win, &layer.data.bg, KS_WIDTH, KS_HEIGHT );
  zxWindowSetBGColor( &win, zxGetColor( &win, (char*)"white" ) );
  zxPMClear( &win, &layer.data.bg );
  zxsLayerBGSetRegion( &layer, 0, 0, KS_WIDTH, KS_HEIGHT );
}

void ksCanvasControl::read_pattern()
{
  zxsLayerReadPixData( &win, &layer, KS_SNOW_S, (char**)snow_s );
  zxsLayerReadPixData( &win, &layer, KS_SNOW_L, (char**)snow_l );
}

void ksCanvasControl::read()
{
  read_background();
  read_pattern();
  read_msg();
}

void ksCanvasControl::open_window()
{
  zxWindowOpen( &win );
}

// canvas

void ksCanvasControl::update()
{
  zxsPattern *pat;
  ksSnowCell *sp;
  int i;

  for( i=0, pat=zListTail(&layer.data.plist), sp=zListTail(&snow_list);
       pat!=zListRoot(&layer.data.plist) && sp!=zListRoot(&snow_list);
       pat=zListCellNext(pat), sp=zListCellNext(sp), i++ ){
    zxsPatternSet( pat, 0, 0 );
    update_snow( pat, &sp->data );
  }
}

void ksCanvasControl::draw()
{
  zxSpriteUpdate( &spr );
  zxSpriteDrawAll( &win, &spr );
  draw_msg();
  zxSpriteAppearAll( &win, &spr );
  zxFlush();
}

// snow

void ksCanvasControl::snow_s_init()
{
  int x, y, dy;

  dy = KS_HEIGHT / KS_SNOW_S_NUM;
  for(int i=0; i<KS_SNOW_S_NUM; i++ ){
    x = zRandI( 0, KS_WIDTH );
    y =-zRandI( 0, dy ) - i * dy;
    ksSnowCell *snow = new ksSnowCell;
    if( !snow ) memalloc_error_abort();
    snow->data.init( short(x), short(y) );
    zxsLayerPatternListAddTail( &layer, KS_SNOW_S, 1, 1, x, y );
    snow->data.set_amp( KS_SNOW_AMP_S );
    snow->data.set_speed( zRandF( KS_SNOW_SPEED_S_MIN, KS_SNOW_SPEED_S_MAX ) );
    zListInsertTail( &snow_list, snow );
  }
}

void ksCanvasControl::snow_l_init()
{
  int x, y, dy;

  dy = KS_HEIGHT / KS_SNOW_L_NUM;
  for(int i=0; i<KS_SNOW_L_NUM; i++ ){
    x = zRandI( 0, KS_WIDTH );
    y =-zRandI( 0, dy ) - i * dy;
    ksSnowCell *snow = new ksSnowCell;
    if( !snow ) memalloc_error_abort();
    snow->data.init( short(x), short(y) );
    zxsLayerPatternListAddHead( &layer, KS_SNOW_L, 1, 1, x, y );
    snow->data.set_amp( KS_SNOW_AMP_L );
    snow->data.set_speed( zRandF( KS_SNOW_SPEED_L_MIN, KS_SNOW_SPEED_L_MAX ) );
    zListInsertHead( &snow_list, snow );
  }
}

void ksCanvasControl::snow_init(void)
{
  zRandInit();
  snow_s_init();
  snow_l_init();
}

void ksCanvasControl::update_snow(zxsPattern *pat, ksSnow *sp)
{
  if( ( sp->y += sp->speed ) > KS_HEIGHT ){
    sp->x = double( zRandI( 0, KS_WIDTH ) );
    sp->y = 0;
  }
  zxsPatternSetX( pat, short( sp->x+(sp->amp*sin(sp->phase)) ) );
  zxsPatternSetY( pat, short( sp->y ) );
  sp->phase += KS_SNOW_DPHASE;
}

// message

// skip Byte Order Marker
void ksCanvasControl::check_bom(ifstream &stream)
{
  char bom[3];

  stream.read( bom, 3 );
  if( ( 0xff & bom[0] ) != 0xef ||
      ( 0xff & bom[1] ) != 0xbb ||
      ( 0xff & bom[2] ) != 0xbf ) stream.seekg( 0 ); // rewind if BOM is not found.
}

void ksCanvasControl::read_msg()
{
  char buf[BUFSIZ];
  ifstream stream;

  stream.open( option[KS_MSG_FILE].arg );
  if( stream ){
    if( option[KS_VERBOSE].flag )
      cout << "メッセージファイル " << option[KS_MSG_FILE].arg << " を読み込みます." << endl;
  } else{
    sprintf( buf, "%s/%s", option[KS_MSG_PATH].arg, option[KS_MSG_FILE].arg );
    stream.open( buf );
    if( stream ){
      if( option[KS_VERBOSE].flag )
        cout << "メッセージファイル " << buf << " を読み込みます." << endl;
    } else{
      cerr << "メッセージファイル " << buf << " を開けません." << endl;
      exit( 1 );
    }
  }
  check_bom( stream );
  while( !stream.eof() ){
    stream.getline( buf, BUFSIZ );
    int len = strlen( buf );
    if( len == 0 ) continue;
    ksMessageCell *mp = new ksMessageCell;
    if( !mp ) memalloc_error_abort();
    mp->data.msg = new char [len+1];
    if( !mp->data.msg ) memalloc_error_abort();
    strcpy( mp->data.msg, buf );
    mp->data.x = 0.5*( KS_WIDTH - zxTextEscapement(mp->data.msg) );
    zListInsertHead( &msg_list, mp );
  }
  zListHead( &msg_list )->data.y = 0.5 * KS_HEIGHT;
  stream.close();
}

void ksCanvasControl::draw_msg_1(ksMessage *mp, double alpha)
{
  if( alpha > 1 ) return;
  alpha *= alpha;
  uword color = ulong( 0xffff * alpha );
  zxWindowSetColor( &win, zxGetColorFromRGB( &win, color, color, color ) );
  zxClipUnsetMask( &win );
  zxDrawStringMB( &win, (short)mp->x, (short)mp->y, mp->msg );
}

void ksCanvasControl::draw_msg_normal(ksMessageCell **mp)
{
  draw_msg_1( &(*mp)->data, fabs( (*mp)->data.y-0.5*KS_HEIGHT )/KS_MSG_DY );
  (*mp)->data.y += KS_MSG_SPEED;
  if( (*mp)->data.y > 0.5*KS_HEIGHT + KS_MSG_DY + KS_MSG_FADE_DY ){
    (*mp)->data.y = 0.5*KS_HEIGHT - KS_MSG_DY - KS_MSG_FADE_DY;
    (*mp) = zListCellNext(*mp);
  }
}

void ksCanvasControl::draw_msg_last(ksMessageCell **mp)
{
  static int count = 3 * KS_MSG_LAST_COUNT;
  double alpha;

  if( count > 2 * KS_MSG_LAST_COUNT )
    alpha = double( count - 2 * KS_MSG_LAST_COUNT )/KS_MSG_LAST_COUNT;
  else if( count > KS_MSG_LAST_COUNT )
    alpha = 0;
  else
    alpha = 1 - double( count )/KS_MSG_LAST_COUNT;
  draw_msg_1( &(*mp)->data, alpha );
  if( --count <= -KS_MSG_LAST_COUNT ){
    count = 3 * KS_MSG_LAST_COUNT;
    *mp = zListTail( &msg_list );
  }
}

void ksCanvasControl::draw_msg()
{
  static ksMessageCell *mp = zListTail( &msg_list );

  if( zListCellNext(mp) != zListRoot(&msg_list) )
    draw_msg_normal( &mp );
  else
    draw_msg_last( &mp );
}

void ksCanvasControl::loop()
{
  timespec req = { 0, KS_INTERVAL };

  while( 1 ){
    update();
    draw();
    while( zxDequeueEvent() != None )
      if( zxevent.type == KeyPress && zxKeySymbol() == XK_q ) return;
    nanosleep( &req, NULL );
  }
}

// usage
inline void usage()
{
  cout << "Usage: kanon-snow <options>" << endl;
  zOptionHelp( option );
  exit( 0 );
}

// initialization
ksCanvasControl *init(int argc, char *argv[])
{
  zStrList arglist;

  // parse options
  zOptionRead( option, argv+1, &arglist );
  if( option[KS_HELP].flag ) usage();
  if( option[KS_VERBOSE].flag )
    cout << KS_TITLE << endl;

  // initialization of graphics and locale
  if( option[KS_VERBOSE].flag )
    cout << "Xサーバ接続..." << endl;
  zxInit();
  if( option[KS_VERBOSE].flag )
    cout << "ロケール検索..." << endl;
  if( zxSetLocale() ) exit( 1 );
  if( option[KS_VERBOSE].flag )
    cout << "フォント作成..." << endl;
  zxFontSetCreate( KS_FONT );

  // creation of canvas control
  if( option[KS_VERBOSE].flag )
    cout << "キャンバスを作成します." << endl;
  ksCanvasControl *cc = new ksCanvasControl;
  if( !cc ) memalloc_error_abort();

  // initialization of message
  cc->read();
  return cc;
}

// main
int main(int argc, char *argv[])
{
  ksCanvasControl *cc = init( argc, argv );
  cc->snow_init();
  cc->open_window();
  cc->loop();
  return 0;
}
