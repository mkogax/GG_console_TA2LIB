
#include "kernel_cfg.h"


/*
  --------------------------------------
  OS鑑賞用コマンド群のために
  仕方なくToppersASP-renesas_unoからコピーしてきた部分
  --------------------------------------
*/

//#include "../asp_wo_cfg/kernel/kernel_impl.h"
//  できれば上記のように[mydoc]/Arduino/libraries/ToppersASP-renesas_uno を参照したい
//  しかしArdinoでコンパイル時にエラーが発生するため
//  ここに利用する範囲だけコピーして展開している      2023.12.03 M.Kogan

typedef struct queue {
	struct queue *p_next;		/* 次エントリへのポインタ */
	struct queue *p_prev;		/* 前エントリへのポインタ */
} QUEUE;
#define TINIB             void              // ダミー
#define WINFO             void              // ダミー
#define BIT_FIELD_UINT		unsigned int
#define BIT_FIELD_BOOL		BIT_FIELD_UINT
#define	TBIT_TCB_PRIORITY		8		/* priorityフィールドのビット幅 */
typedef struct task_context_block {
	void	*sp;		/* スタックポインタ */
	FP		pc;			/* プログラムカウンタ */
} TSKCTXB;

typedef struct task_control_block {
	QUEUE			task_queue;		/* タスクキュー */
	const TINIB		*p_tinib;		/* 初期化ブロックへのポインタ */

#ifdef UINT8_MAX
	uint8_t			tstat;			/* タスク状態（内部表現）*/
#else /* UINT8_MAX */
	BIT_FIELD_UINT	tstat : 8;		/* タスク状態（内部表現）*/
#endif /* UINT8_MAX */
#if defined(UINT8_MAX) && (TBIT_TCB_PRIORITY == 8)
	uint8_t			priority;		/* 現在の優先度（内部表現）*/
#else /* defined(UINT8_MAX) && (TBIT_TCB_PRIORITY == 8) */
	BIT_FIELD_UINT	priority : TBIT_TCB_PRIORITY;
									/* 現在の優先度（内部表現）*/
#endif /* defined(UINT8_MAX) && (TBIT_TCB_PRIORITY == 8) */
	BIT_FIELD_BOOL	actque : 1;		/* 起動要求キューイング */
	BIT_FIELD_BOOL	wupque : 1;		/* 起床要求キューイング */
	BIT_FIELD_BOOL	enatex : 1;		/* タスク例外処理許可状態 */

	TEXPTN			texptn;			/* 保留例外要因 */
	WINFO			*p_winfo;		/* 待ち情報ブロックへのポインタ */
	TSKCTXB			tskctxb;		/* タスクコンテキストブロック */
} TCB;

#define tcb_table					_kernel_tcb_table

extern TCB	tcb_table[];
#define TMIN_TSKID		1		/* タスクIDの最小値 */
#define INDEX_TSK(tskid)	((uint_t)((tskid) - TMIN_TSKID))
#define get_tcb(tskid)		(&(tcb_table[INDEX_TSK(tskid)]))
#define get_tcb_self(tskid)	((tskid) == TSK_SELF ? p_runtsk : get_tcb(tskid))


/*
  --------------------------------------
  OS鑑賞用コマンド群
  --------------------------------------
*/

int C_os(int argc, char **argv)   // TOPPERSの状態表示
{
  

  return 0;
}

int C_tcb(int argc, char **argv)   // TCBの状態表示
{
  int i;
  for (i=0; i<TNUM_TSKID; i++) {    // 未定義も含めて登録可能なタスク数を全部リストする
    int id = i+TMIN_TSKID;
    TCB *tcb = get_tcb(id);
    int ts = tcb->tstat;
    gg_printf("[%2d] TCB=0x%08lX ts=0x%02X", id, tcb, ts);
    gg_printf("(%c%c%c ", (ts&1)?'R':'-', (ts&2)?'W':'-', (ts&4)?'S':'-');
    if (ts&6) {
      switch(ts>>3) {
      case 0:  gg_printf("WAIT_DLY "); break;
      case 1:  gg_printf("WAIT_SLP "); break;
      case 2:  gg_printf("WAIT_RDTQ"); break;
      case 3:  gg_printf("WAIT_RPDQ"); break;
      case 4:  gg_printf("WAIT_SEM "); break;
      case 5:  gg_printf("WAIT_FLG "); break;
      case 6:  gg_printf("WAIT_SDTQ"); break;
      case 7:  gg_printf("WAIT_SPDQ"); break;
      case 8:  gg_printf("WAIT_MBX "); break;
      case 9:  gg_printf("WAIT_MPF "); break;
      default: gg_printf("WAIT_????"); break;
       }
    } else {
      gg_printf("         ");
    }
    gg_printf(")");
    gg_printf(" pri=%2d act=%d wup=%d etx=%d\n", tcb->priority, tcb->actque, tcb->wupque, tcb->enatex);
  }   

  return 0;
}


int mon_RegistCMD(void)     // TOPPERSモニタコマンドの登録
{
  GG_CON_CMDADD(C_os,     "os",     "[...]", "explore the current status of TOPPERS" );
  GG_CON_CMDADD(C_tcb,    "tcb",    "[...]", "explore the TCB" );
  return 0;
}
