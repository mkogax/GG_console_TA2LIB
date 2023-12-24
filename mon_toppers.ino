
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

#define kmmsz						_kernel_kmmsz
#define kmm							_kernel_kmm
extern const SIZE		kmmsz;    // kernel_cfg.c
extern MB_T *const		kmm;    // kernel_cfg.c

extern "C" {
#define kernel_malloc				_kernel_kernel_malloc
extern void *kernel_malloc(SIZE size);
}

/*
  --------------------------------------
  OS鑑賞用コマンド群
  --------------------------------------
*/

int C_inf(int argc, char **argv)   // TOPPERSの設定情報
{
  gg_printf("-- TOPPERS設定情報 --\n");
  gg_printf("[バージョン(TKERNEL_)]\n");
  gg_printf(" 0x%04X(MAKER) 0x%04X(PRID) 0x%04X(SPVER) 0x%04X(PRVER)\n",
        TKERNEL_MAKER,    /* カーネルのメーカーコード */
        TKERNEL_PRID,     /* カーネルの識別番号 */
        TKERNEL_SPVER,    /* カーネル仕様のバージョン番号 */
        TKERNEL_PRVER);   /* カーネルのバージョン番号 */
  gg_printf("[最大登録数(TNUM_)]\n");
  gg_printf(" %d(TSKID) %d(SEMID) %d(FLGSID) %d(DTQID) %d(PDQID) %d(MBXID) %d(MPFID) %d(CYCID) %d(ALMID) %d(ISRID)\n",
        TNUM_TSKID,         /* タスクの数 */
        TNUM_SEMID,         /* セマフォの数 */
        TNUM_FLGID,         /* イベントフラグ */
        TNUM_DTQID,         /* データキュー */
        TNUM_PDQID,         /* 優先度データキュー */
        TNUM_MBXID,         /* メールボックス */
        TNUM_MPFID,         /* 固定長メモリプール */
        TNUM_CYCID,         /* 周期ハンドラ */
        TNUM_ALMID,         /* アラームハンドラ */
        TNUM_ISRID);        /* 割込みサービスルーチン */
  return 0;
}

int C_sts(int argc, char **argv)    // TOPPERSの状態表示
{
  uint8_t *kmm_brk = (uint8_t *)kernel_malloc(0);   // 現在のシステムメモリ消費位置
  int i;
  gg_printf("-- TOPPERS状態 --\n");
  gg_printf("[システムメモリ]\n");
  gg_printf(" size=0x%08lX(kmmsz) top=0x%08lX(kmm) brk=0x%08lX(kmm_brk) 残り=0x%08lX(brk-kmm)\n",
        kmmsz,
        kmm,
        kmm_brk,
        kmm_brk-(uint8_t *)kmm);
  gg_printf("[タスク]\n");
  for (i=0; i<TNUM_TSKID; i++) {    // 未定義も含めて登録可能なタスク数を全部リストする
    int id = i+TMIN_TSKID;
    TCB *tcb = get_tcb(id);
    int ts = tcb->tstat;
    gg_printf("%3d TCB=0x%08lX ts=0x%02X", id, tcb, ts);
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
	GG_CON_CMDMRK("-- TOPPERS --");			// 区切り行（helpでコマンド一覧のときに表示）
  GG_CON_CMDADD(C_inf,    "inf",    "",       "設定情報" );
  GG_CON_CMDADD(C_sts,    "sts",    "",       "状態" );
  return 0;
}
