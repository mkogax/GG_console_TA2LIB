#include <ToppersASP-renesas_uno.h>

//#ifdef ARDUINO_UNOR4_MINIMA
#if defined(ARDUINO_UNOR4_MINIMA) || defined(ARDUINO_UNO4_WIFI)
#define Serial SerialUSB 
#endif /* ARDUINO_UNOWIFIR4 */



#include <GG.h>

/*
  GG for Arduino ** Simple console demo
  Arduino standard serial (115200bps / 8bit length / non-parity)
  Enter the help command to display the command list.
*/

void setup() {

  gg_start("** GG **\n>");    // Start processing GG for Arduino
  gg_con_RegistMemCMD();      // メモリ読み書きコマンド登録(md/ms/mf..)
  gg_tp_start(2, 3);          // 2番ピンと3番ピンにTP出力を割り振る(tpコマンド登録も)

  // 起動時の処理を見る場合(tp 10 30 コマンドの代わり)
  // GG_TP1 = 10;             // TP1出力=10   setup()/user_inirtn()
  // GG_TP2 = 30;             // TP2出力=30   loop()

  // タスク動作を見る場合(tp 100 200 コマンドの代わり)
  GG_TP1 = 100;               // TP1出力=100  task1
  GG_TP2 = 200;               // TP2出力=200  task2

  GG_TP_ON(10);     // setup()

  // commands.ino内のコマンドを登録
  mon_RegistCMD();            // TOPPERSモニタコマンドの登録
  task_RegistCMD();           // task制御コマンドの登録

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  GG_TP_OFF(10);    // StartToppersASP()では戻ってこないのでここでOFF

  // Start TOPPERS/ASP kernel
  StartToppersASP();

}

//
//  カーネルオブジェクトの初期化(カーネル起動後に呼び出される)
//

// ** タスク生成マクロ **
#define TA_CRETSK(tno, atr, tsk, pri, size)\
{\
  T_CTSK ctsk;\
  ER ercd;\
  ctsk.tskatr = atr;\
  ctsk.exinf = tno;   /* exinf=tno */\
  ctsk.task = tsk;\
  ctsk.itskpri = pri;\
  ctsk.stksz = size;\
  ctsk.stk = NULL;\
  ercd = cre_tsk(tno, &ctsk);\
  if (ercd == E_OK) {\
    gg_printf("cre_tsk(tno=%d) OK(%d)\n", tno, ercd);\
  } else {\
    gg_printf("cre_tsk(tno=%d) error!(%d)\n", tno, ercd);\
  }\
}

void user_inirtn(void) {

  GG_TP_ON(10);     // user_inirtn()

  TA_CRETSK(TASK1, TA_ACT, task1, 11, 1024);    // タスク優先度=11(優先度高い)
  TA_CRETSK(TASK2, TA_ACT, task2, 12, 1024);    // タスク優先度=12(優先度低い)

  GG_TP_OFF(10);

}

//
//  loop部
//
void loop() {

  GG_TP_ON(30);     // loop

  gg_con_Check();   // console processing (returns immediately if nothing is done)

  GG_TP_OFF(30);

}

//
//  task1/task2実行部
//
int task1_flag = 0;
int task2_flag = 0;
int dummy_flag = 0;

int dummy(long msec)        // ダミー負荷
{
  if (!dummy_flag) {              // ダミー負荷（時間経過待ちループ）
    long t = millis();            // 初期値
    while(msec>=(millis()-t)) ;   // 指定usec経過するまで待つ
  } else {                        // ダミー負荷（どうもdelay()の中で時間指定sleepしてる？）
    delay(msec);                  // delay()
  }
  return 0;
}

void task1(intptr_t exinf) {
  int count = 0;
  while (1) {
    GG_TP_ON(100);
    count++;
    dummy(task1_flag);
    GG_TP_OFF(100);
    tslp_tsk(10);   // 10msec待ち 
  }
}

void task2(intptr_t exinf) {
  int count = 0;
  while (1) {
    GG_TP_ON(200);
    count++;
    dummy(task2_flag);
    GG_TP_OFF(200);
    tslp_tsk(10);   // 10msec待ち 
  }
}


//
//  task制御コマンド
//
int C_task1(int argc, char **argv)   // task1制御
{
  if (argc>1) {
    task1_flag = gg_asc2int(argv[1]);
  }
  gg_printf("task1_flag = %d\n", task1_flag);
  return 0;
}

int C_task2(int argc, char **argv)   // task2制御
{
  if (argc>1) {
    task2_flag = gg_asc2int(argv[1]);
  }
  gg_printf("task2_flag = %d\n", task2_flag);
  return 0;
}

int C_dummy(int argc, char **argv)   // ダミー負荷制御(0=ループ処理,1=delay())
{
  if (argc>1) {
    dummy_flag = gg_asc2int(argv[1]);
  }
  gg_printf("dummt_flag = %d\n", dummy_flag);
  return 0;
}

int task_RegistCMD(void)            // task制御コマンドの登録
{
	GG_CON_CMDMRK("-- TASK --");			// 区切り行（helpでコマンド一覧のときに表示）
  GG_CON_CMDADD(C_task1,  "task1",  "[...]", "control task1" );
  GG_CON_CMDADD(C_task2,  "task2",  "[...]", "control task2" );
  GG_CON_CMDADD(C_dummy,  "dummy",  "[...]", "control dummy" );
  return 0;
}
