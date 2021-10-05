/*
 * fx502p.c
 */

//#define DEBUG

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "fx502p.h"

#include "display.h"

extern long calcExpress(const char *string);

/*
 * local facilities
 *
 *  int  nbyte(unsigned char *bcd, int off);
 *  void print_context(void);
 *  void print_program(struct program *prg);
 *
 */

/*
 * fx502p structure
 */

#define STS_NORMAL	0x00000000
#define STS_NUMERIC	0x00000001

struct fx502p_context {
  struct program *prg;	// program
  
  int  pc;		// program counter
  int *sp;		// stack pointer
  int  stack[10];

  int  psec;		// program section
  unsigned int status;	// numeric input or not

  struct display x;
  struct display y;
  struct display m0[11];
  struct display m1[11];

  char arithmetic[4096];
};

struct fx502p_opcode {
  char *mne;	// ニーモニック
  int   len;	// バイト数（-1=未定）
  int (*func)(struct fx502p_context *);
};


/*
 * OP code table
 */

static struct fx502p_opcode plane[2][256];

int
nbyte(unsigned char *bcd, int off)
{
  int code = bcd[off];
  int len  = plane[0][code].len;
  if (len > 0) return(len);
  code = bcd[off+1];
  len  = plane[1][code].len;
  return(len);
}

char mnestr[256];

char *
mnemonic(unsigned char *bcd, int off)
{
  char buf[16];
  int n = nbyte(bcd, off);
  int code = bcd[off];
  bzero(mnestr, 256);
  strcat(mnestr, plane[0][code].mne);
  if (n == 1) return(mnestr);

  if (code != 0xFF) {
    snprintf(buf, 16, " %02X", bcd[off+1]);
    strcat(mnestr, buf);
    if (n == 2) return(mnestr);
    snprintf(buf, 16, " %02X", bcd[off+2]);
    strcat(mnestr, buf);
    return(mnestr);
  } else {
    strcat(mnestr, " ");
    code = bcd[off+1];
    strcat(mnestr, plane[1][code].mne);
    if (n == 2) return(mnestr);
    snprintf(buf, 16, " %02X", bcd[off+2]);
    strcat(mnestr, buf);
    return(mnestr);
  }
}

void
planeset(int plan,
	 unsigned char opcode,
	 char *mnemonic,
	 int length,
	 int (*func)(struct fx502p_context *))
{
  struct fx502p_opcode *p = &plane[plan][opcode];
  p->mne = mnemonic;
  p->len = length;
  p->func = func;
}

// 0x00-0x09
int
op_numeric(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
#ifdef DEBUG
  printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
#endif
  // アスキーに変換してｘレジスタに追加
  if (ctx->status == STS_NORMAL) dis_reset(&ctx->x);
  dis_add(&ctx->x, prg->code[ctx->pc]+'0');
#ifdef DEBUG
  printf("       X: \"%s\"\n", dis_str(&ctx->x));
#endif
  ctx->arithmetic[strlen(ctx->arithmetic)] = prg->code[ctx->pc]+'0';
  ctx->status = STS_NUMERIC;
  return(0);
}

// 0xC6
int
op_memory_in(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
  struct display *m;
#ifdef DEBUG
  printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
#endif
  // 第１オペラントはセレクタ
  // 第２オペラントはレジスタ番号（Ｆは１０番に割り当て）
  if (prg->code[ctx->pc+1] == 1) {
    m = &ctx->m1[prg->code[ctx->pc+2]];
  } else {
    m = &ctx->m0[prg->code[ctx->pc+2]];
  }
  *m = ctx->x;
#ifdef DEBUG  
  printf("     M%d%d: \"%s\"\n",
	 prg->code[ctx->pc+1],
	 prg->code[ctx->pc+2],
	 dis_str(m));
#endif
  ctx->status = STS_NORMAL;
  return(0);
}

// 0xC7
int
op_memory_recall(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
  struct display *m;
#ifdef DEBUG  
  printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
#endif
  // 第１オペラントはセレクタ
  // 第２オペラントはレジスタ番号（Ｆは１０番に割り当て）
  if (prg->code[ctx->pc+1] == 1) {
    m = &ctx->m1[prg->code[ctx->pc+2]];
  } else {
    m = &ctx->m0[prg->code[ctx->pc+2]];
  }
  ctx->x = *m;
#ifdef DEBUG  
  printf("       X: \"%s\"\n", dis_str(&ctx->x));
#endif
  char s[16];
  bzero(s, 16);
  snprintf(s, 16, "%d", dis_integer(&ctx->x));
  strcat(ctx->arithmetic, s);

  ctx->status = STS_NORMAL;
  return(0);
}

// 0xD0-0xD9
int
op_program_section(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
  int psec = prg->code[ctx->pc];
  if (psec == 0xFF) {
    psec = prg->code[ctx->pc+1];
  }
  ctx->psec = psec - 0xD0;
#ifdef DEBUG  
  printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
#else
  printf("P%d: %s\n", ctx->psec, prg->message[0]);
#endif
  ctx->status = STS_NORMAL;
  return(0);
}

// 0xDF
int
op_return(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
#ifdef DEBUG  	 
  printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
#endif
  if (ctx->sp == &ctx->stack[9]) {
#ifdef DEBUG  	 
    printf("     : program end\n");
#endif
    printf("%s\n", dis_str(&ctx->x));
    return(-1);
  }

  ctx->status = STS_NORMAL;
  printf("     : you need implement return op for GSB.\n");
  exit(0);
}

// 0xE1-0xE4
int
op_arithmetic(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
#ifdef DEBUG
  printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
#endif
  switch (prg->code[ctx->pc]) {
  case 0xE1: strcat(ctx->arithmetic, "*"); break;
  case 0xE2: strcat(ctx->arithmetic, "/"); break;
  case 0xE3: strcat(ctx->arithmetic, "+"); break;
  case 0xE4: strcat(ctx->arithmetic, "-"); break;
  default: break;
  }
#ifdef DEBUG
  printf("     ---> %s\n", ctx->arithmetic);
#endif
  dis_reset(&ctx->x);
  ctx->status = STS_NUMERIC;
  return(0);
}

// 0xE5 -- equal
int
op_equal(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
  long val;
#ifdef DEBUG
  printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
  printf("     ---> %s\n", ctx->arithmetic);
#endif
  val = calcExpress(ctx->arithmetic);  //const char *string);
  dis_set_long(&ctx->x, val);
#ifdef DEBUG
  printf("        : \"%s\"\n", dis_str(&ctx->x));
#endif
  bzero(ctx->arithmetic, 4096);
  ctx->status = STS_NORMAL;
  return(0);
}

// 0xF0
int
op_label(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
#ifdef DEBUG
  printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
#endif
  ctx->status = STS_NORMAL;
  return(0);
}

// 0xF1
int
op_goto(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
  int lbl = prg->code[ctx->pc+1];
#ifdef DEBUG
  printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
#endif
  ctx->pc = prg->label[ctx->psec][lbl];
#ifdef DEBUG
  printf("      jump to LBL %d, offset %d\n", lbl, ctx->pc);
#endif
  ctx->status = STS_NORMAL;
  return(1);
}

char hltbuf[16];

// 0xFD
int
op_temporary_halt(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
  int no = prg->code[ctx->pc+1];	// オペランドはメッセージID
#ifdef DEBUG
  printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
  printf("       *: %s\n", prg->message[no]);
#else
  printf("%s\n", prg->message[no]);
#endif
  fgets(hltbuf, 16, stdin);
  dis_reset(&ctx->x);
  dis_set_string(&ctx->x, hltbuf);
#ifdef DEBUG
  printf("      IN: \"%s\"\n", dis_str(&ctx->x));
#endif
  ctx->status = STS_NORMAL;
  return(0);
}

// 0xFF&01 -- Increment and Skip on Zero
// The content of the M00 register is incremented and,
// if the result is zero, the next command is skipped;
// otherwise the next command is executed.		
// int op_increment_skip_zero(struct fx502p_context *ctx)


// 0xFF&04 -- Decrement and Skip on Zero
// The content of the M00 register is decremented and,
// if the result is zero, the next command is skipped;
// otherwise the next command is executed.		
int
op_decrement_skip_zero(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
  int r, n;
#ifdef DEBUG
  printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
#endif
  r = dis_integer(&ctx->m0[0]);
  dis_set_integer(&ctx->m0[0], --r);
#ifdef DEBUG
  printf("     M00: %d\n", r);
#endif
  if (r == 0) {
    ctx->pc += 2;
#ifdef DEBUG
    printf("     skip next command\n");
#endif
  }
  ctx->status = STS_NORMAL;
  return(0);
}


void
fx502p_optbl_setup(void)
{
  int n, i, j;
  
  planeset(0, 0x00, "0",    1, op_numeric);
  planeset(0, 0x01, "1",    1, op_numeric);
  planeset(0, 0x02, "2",    1, op_numeric);
  planeset(0, 0x03, "3",    1, op_numeric);
  planeset(0, 0x04, "4",    1, op_numeric);
  planeset(0, 0x05, "5",    1, op_numeric);
  planeset(0, 0x06, "6",    1, op_numeric);
  planeset(0, 0x07, "7",    1, op_numeric);
  planeset(0, 0x08, "8",    1, op_numeric);
  planeset(0, 0x09, "9",    1, op_numeric);

  planeset(0, 0xC6, "Min",  3, op_memory_in);

  planeset(0, 0xC7, "MR",   3, op_memory_recall);
  
  planeset(0, 0xD0, "P0",   1, op_program_section);
  planeset(0, 0xD1, "P1",   1, op_program_section);
  planeset(0, 0xD2, "P2",   1, op_program_section);
  planeset(0, 0xD3, "P3",   1, op_program_section);
  planeset(0, 0xD4, "P4",   1, op_program_section);
  planeset(0, 0xD5, "P5",   1, op_program_section);
  planeset(0, 0xD6, "P6",   1, op_program_section);
  planeset(0, 0xD7, "P7",   1, op_program_section);
  planeset(0, 0xD8, "P8",   1, op_program_section);
  planeset(0, 0xD9, "P9",   1, op_program_section);
  planeset(0, 0xDD, "MODE", 2, NULL);
  planeset(0, 0xDF, "RET",  1, op_return);
  
  planeset(0, 0xE0, "AC",   1, NULL);
  planeset(0, 0xE1, "*",    1, op_arithmetic);
  planeset(0, 0xE2, "/",    1, op_arithmetic);
  planeset(0, 0xE3, "+",    1, op_arithmetic);
  planeset(0, 0xE4, "-",    1, op_arithmetic);
  planeset(0, 0xE5, "=",    1, op_equal);
  planeset(0, 0xED, ".",    1, NULL);

  planeset(0, 0xF0, "LBL",  2, op_label);
  planeset(0, 0xF1, "GOTO", 2, op_goto);
  planeset(0, 0xF2, "GSB",  2, NULL);
  planeset(0, 0xFD, "HLT",  2, op_temporary_halt);
  planeset(0, 0xFF, "INV", -1, NULL);
  
  planeset(1, 0x01, "ISZ",  2, NULL);
  planeset(1, 0x02, "x=0",  2, NULL);
  planeset(1, 0x03, "x=F",  2, NULL);
  planeset(1, 0x04, "DSZ",  2, op_decrement_skip_zero);
  planeset(1, 0x05, "x>=0", 2, NULL);
  planeset(1, 0x06, "x>=F", 2, NULL);
  
  planeset(1, 0xD0, "P0",   2, op_program_section);
  planeset(1, 0xD1, "P1",   2, op_program_section);
  planeset(1, 0xD2, "P2",   2, op_program_section);
  planeset(1, 0xD3, "P3",   2, op_program_section);
  planeset(1, 0xD4, "P4",   2, op_program_section);
  planeset(1, 0xD5, "P5",   2, op_program_section);
  planeset(1, 0xD6, "P6",   2, op_program_section);
  planeset(1, 0xD7, "P7",   2, op_program_section);
  planeset(1, 0xD8, "P8",   2, op_program_section);
  planeset(1, 0xD9, "P9",   2, op_program_section);
  planeset(1, 0xDD, "NOP",  2, NULL);
  planeset(1, 0xDF, "RET",  2, NULL);

  planeset(1, 0xED, "RAN#", 2, NULL);

  planeset(1, 0xFD, "PAUSE",2, NULL);
}

/*
 * context
 */

static struct fx502p_context context;

void
fx502p_context_setup(void)
{
  struct fx502p_context *ctx = &context;
  int i;

  ctx->pc = 0;
  ctx->sp = &ctx->stack[9];
  ctx->psec = -1;
  ctx->status = STS_NORMAL;
  
  dis_reset(&ctx->x);
  dis_reset(&ctx->y);
  for (i = 0; i < 11; i++) {
    dis_reset(&ctx->m0[i]);
    dis_reset(&ctx->m1[i]);
  }
  
  bzero(ctx->arithmetic, 4096);
}

void
print_context(void)
{
  struct fx502p_context *ctx = &context;
  int i;

  printf("# context\n");
  printf("  pc: %d\n", ctx->pc);
  printf("  sp: %p\n", ctx->sp);
  printf(" stack:\n");
  for (i = 0; i < 10; i++) {
    printf("  %p: %d\n", &ctx->stack[i], ctx->stack[i]);
  }
  printf("  x: \"%s\"\n",  dis_str(&ctx->x));
  printf("  y: \"%s\"\n",  dis_str(&ctx->y));
  printf(" memory:\n");
  for (i = 0; i < 10; i++) {
    printf("  m0%d: \"%s\"\n", i, dis_str(&ctx->m0[i]));
  }  
  for (i = 0; i < 10; i++) {
    printf("  m1%d: \"%s\"\n", i, dis_str(&ctx->m1[i]));
  }  
}

/*
 * interface
 */
void
fx502p_setup(void)
{
  fx502p_optbl_setup();
  fx502p_context_setup();
}

void
print_program(struct fx502p_context *ctx)
{
  struct program *prg = ctx->prg;
  int n, i;
  
  printf("# Pn section offset\n");
  for (n = 0; n < 10; n++) {
    printf("P%1d:\t%d\n", n, prg->routine[n]);
  }
  printf("\n");
  
  printf("# Label offset\n");
  for (n = 0; n < 10; n++) {
    printf("  P%1d\n", n);
    for (i = 0; i < 10; i++) {
      printf("    LBL %d:\t%d\n", i, prg->label[n][i]);
    }
  }
  printf("\n");
}

int
fx502p_program_setup(struct program *prg)
{
  struct fx502p_context *ctx = &context;
  int len = prg->length;
  int pc;
  int n, i;
  int P = -1;
  int L = -1;

  ctx->prg = prg;

  // Pn セクションとラベルの初期化
  for (n = 0; n < 10; n++) {
    prg->routine[n] = -1;
    for (i = 0; i < 10; i++) {
      prg->label[n][i] = -1;
    }
  }

  // Pn セクションとラベルのフェッチ
  for (pc = 0, n = 0; pc < len; pc += n) {
    n = nbyte(prg->code, pc);
    //printf("%3d:", pc);
    for (i = 0; i < n; i++) {
      //printf(" %02X", prg->code[pc+i]);
    }
    if (prg->code[pc] >= 0xD0 &&
	prg->code[pc] <= 0xD9) {
      P = prg->code[pc]-0xD0;
      //printf("\t\tP%d\t%d", P, pc);
      prg->routine[P] = pc;
    }
    if (prg->code[pc] == 0xF0) {
      L =  prg->code[pc+1];
      //printf("\tLBL %d\t%d", L, pc);
      prg->label[P][L] = pc;
    }
    //printf("\n");
  }

  //print_program(ctx);
  
  return(0);
}



int
fx502p_exec(void)
{
  struct fx502p_context *ctx = &context;
  struct program *prg = ctx->prg;
  struct fx502p_opcode *p;
  int code, n, ret;

  ctx->pc = 0;
  ctx->sp = &ctx->stack[9];

  for (ctx->pc = 0, n = 0; ctx->pc < prg->length;) {
    n = nbyte(prg->code, ctx->pc);

    code = prg->code[ctx->pc];
    p = &plane[0][code];
    if (code == 0xFF) {
      code = prg->code[ctx->pc+1];
      p = &plane[1][code];
    }

    if (p->func == NULL) {
      printf("%3d: %s\n", ctx->pc, mnemonic(prg->code, ctx->pc));
      fprintf(stderr, "# not defined function\n");
      exit(1);
    } else {
      ret = (*p->func)(ctx);
    }

    if (ret < 0) break;
    if (!ret) ctx->pc += n;
  }

  return(0);
}

