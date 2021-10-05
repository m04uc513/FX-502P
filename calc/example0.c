/*
 * examle0.c
 */

#include "program.h"


/* 
 * プログラム
 * 
 * KeyCode	Comment
 * ------------ -------------------------------------
 * P0		P0 でプログラムを起動させる
 * HLT 1	プログラムの一旦停止し入力を促す
 * Min 0	レジスタ0に値を入力
 * 1		1から開始
 * LBL 0	ループ用ラベル
 * *		乗算
 * MR 0		レジスタ0の値を呼び出し
 * INV DSZ 	レジスタ0の値から1を引き、0になるまで
 * GOTO 0	LBL0にジャンプする
 * =		n! の計算結果が表示される
 * RET		終了。 
 */

unsigned char code[] = {
  0xD0,
  0xFD, 0x01,
  0xC6, 0x00, 0x00,
  0x01,
  0xF0, 0x00,
  0xE1,
  0xC7, 0x00, 0x00,
  0xFF, 0x04,
  0xF1, 0x00,
  0xE5,
  0xDF
};

int routine[10];
int label_raw[10][10];
int *label[10];

char *message[] = {
  "階乗計算",
  "基数を入力してください。"
};

void
Example0_setup(struct program *prg)
{
  int i;

  for (i = 0; i < 10; i++) {
    label[i] = &label_raw[i][0];
  }
    
  prg->code    = code;
  prg->length  = sizeof(code)/sizeof(unsigned char);
  prg->routine = routine;
  prg->label   = label;
  prg->message = message;
}
