#ifndef _JMCCE_C_ROUTINE_H_
#define _JMCCE_C_ROUTINE_H_

/* c_routine.h */

void vgalib_draw_ascii(int col, int y, unsigned char *bitmap, int color1);
void c_draw_hanzi (int col, int y, unsigned char *bitmap, int color);
void c_draw_ascii (int col, int y, unsigned char *bitmap, int color);
void c_scroll_up (int sy, int ey, int line, int bgcolor);
void c_scroll_down (int sy, int ey, int line, int bgcolor);
void c_clear_block (int col, int y, int width, int height, int bgcolor);
void c_clear_lines (int sy, int height, int bgcolor);
void c_bold_hanzi (unsigned char *buffer);
void c_bold_ascii (unsigned char *buffer);
void c_toggle_cursor (int col, int y);

#endif /* _JMCCE_C_ROUTINE_H_ */
