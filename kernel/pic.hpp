#pragma once

#define PIC0_ICW1  0x20
#define PIC0_OCW2  0x20
#define PIC0_IMR   0x21
#define PIC0_ICW2  0x21
#define PIC0_ICW3  0x21
#define PIC0_ICW4  0x21
#define PIC1_ICW1  0xa0
#define PIC1_OCW2  0xa0
#define PIC1_IMR   0xa1
#define PIC1_ICW2  0xa1
#define PIC1_ICW3  0xa1
#define PIC1_ICW4  0xa1

void IoWait() {
  IoOut8(0x80, 0);
}

void InitializePIC() {
  IoOut8(PIC0_IMR, 0xff);     /* 全ての割り込みを受け付けない */ 
  IoWait();
  IoOut8(PIC1_IMR, 0xff);     /* 全ての割り込みを受け付けない */
  IoWait();

  IoOut8(PIC0_ICW1, 0x11);    /* エッジトリガモード */
  IoWait();
  IoOut8(PIC0_ICW2, 0x20);    /* IRQ0-7は、INT20-27で受ける */
  IoWait();
  IoOut8(PIC0_ICW3, 0x04);    /* PIC1はIRQ2にて接続 */
  IoWait();
  IoOut8(PIC0_ICW4, 0x01);    /* ノンバッファモード */
  IoWait();

  IoOut8(PIC1_ICW1, 0x11);    /* エッジトリガモード */
  IoWait();
  IoOut8(PIC1_ICW2, 0x28);    /* IRQ8-15は、INT28-2fで受ける */
  IoWait();
  IoOut8(PIC1_ICW3, 0b010);   /* PIC1はIRQ2にて接続 */
  IoWait();
  IoOut8(PIC1_ICW4, 0x01);    /* ノンバッファモード */
  IoWait();
}

void EnablePS2interrupt() {
  IoOut8(PIC0_IMR, 0xf9);     /* 11111001 Allow Keyboard, PIC1 */
  IoWait();
  IoOut8(PIC1_IMR, 0xef);    /* 11101111 Allow Mouse */
  IoWait();

  /*
  while ((IoIn32(0x64) & 0x02) != 0);
  IoOut8(0x60, 0xd1);
  while ((IoIn32(0x64) & 0x02) != 0);
  IoOut8(0x60, 0xdf);
  while ((IoIn32(0x64) & 0x02) != 0);
  */
}