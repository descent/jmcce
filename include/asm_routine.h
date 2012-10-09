#ifndef _JMCCE_ASM_ROUTINE_H_
#define _JMCCE_ASM_ROUTINE_H_

#define GRA_I   0x3CE		/* Graphics Controller Index */
#define GRA_D   0x3CF		/* Graphics Controller Data Register */

void asm_draw_hanzi (int col, int y, unsigned char *bitmap, int color);
void asm_draw_ascii (int col, int y, unsigned char *bitmap, int color);
void asm_scroll_up (int sy, int ey, int line, int bgcolor);
void asm_scroll_down (int sy, int ey, int line, int bgcolor);
void asm_clear_block (int col, int y, int width, int height, int bgcolor);
void asm_clear_lines (int sy, int height, int bgcolor);
void asm_bold_hanzi (unsigned char *buffer);
void asm_bold_ascii (unsigned char *buffer);
void asm_toggle_cursor (int col, int y);

#endif /* _JMCCE_ASM_ROUTINE_H_ */
