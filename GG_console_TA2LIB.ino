#include <ToppersASP-renesas_uno.h>

#ifdef ARDUINO_UNOR4_MINIMA
#define Serial SerialUSB 
#endif /* ARDUINO_UNOWIFIR4 */



#include <GG.h>

/*
  GG for Arduino ** Simple console demo
  Arduino standard serial (115200bps / 8bit length / non-parity)
  Enter the help command to display the command list.
*/

void setup() {
  // Serial.begin(9600);
  // while (!Serial)
  //   ;

  gg_start("** GG **\n>");    // Start processing GG for Arduino
  gg_con_RegistMemCMD();      // register memory command

  // commands.ino内のコマンドを登録
  GG_CON_CMDADD(C_os,     "os",     "[...]", "explore the current status of TOPPERS" );
  GG_CON_CMDADD(C_tcb,    "tcb",    "[...]", "explore the TCB" );
  GG_CON_CMDADD(C_task1,  "task1",  "[...]", "control task1" );
  GG_CON_CMDADD(C_task2,  "task2",  "[...]", "control task2" );
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Start TOPPERS/ASP kernel
  StartToppersASP();

}

int task1_flag = 0;
int task2_flag = 0;

void task1(intptr_t exinf) {
  int count = 0;
  while (1) {
    count++;
    if (task1_flag) {
      Serial.print("task1 is running. ");
      Serial.println(count);
    }
    delay(1000);  // wait for a second
  }
}

void task2(intptr_t exinf) {
  int count = 0;
  while (1) {
    count++;
    if (task2_flag) {
      Serial.print("task2 is running. ");
      Serial.println(count);
    }
    delay(1000);  // wait for a second
  }
}

void user_inirtn(void) {
  T_CTSK ctsk;
  ER ercd;

  ctsk.tskatr = TA_ACT;
  ctsk.exinf = 1;
  ctsk.task = task1;
  ctsk.itskpri = 10;
  ctsk.stksz = 1024;
  ctsk.stk = NULL;
  ercd = cre_tsk(TASK1, &ctsk);
  if (ercd != E_OK) {
    Serial.print("cre_tsk(TASK1) error! ercd = ");
    Serial.println(ercd);
  }

  ctsk.task = task2;
  ercd = cre_tsk(TASK2, &ctsk);
  if (ercd != E_OK) {
    Serial.print("cre_tsk(TASK2) error! ercd = ");
    Serial.println(ercd);
  }
}

void loop() {
  gg_con_Check();  // console processing (returns immediately if nothing is done)
}
