
/****************************************************************************/
/*            JMCCE  - hbffont.c                                            */
/*                                                                          */
/*                    $Id: hbffont.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $   */
/****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <asm/bitops.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "jmcce.h"
#include "c_routine.h"
#include "hbffont.h"

extern int gEncode;

struct HBFrange
{
  unsigned int l;
  unsigned int h;
  long offset;
  char bmfname[50];
  int address_table[256];
  int fd;
  off_t st_size;
  unsigned char *font_mem;
}
 *seg;

static int nseg, total_range;
unsigned char *hanzi_buffer_normal;
unsigned char *hanzi_buffer_underline;

unsigned char ascii_font[256][18];
unsigned char *ascii_buffer_normal;
unsigned char *ascii_buffer_underline;

inline void
font_done (void)
{
  hbfClose ();
}

inline void
font_init (void)
{
  open_ascii_font (DEFAULT_ASCFONT);

  if (gEncode == BIG5) {
    open_hanzi_font (DEFAULT_HZFONT_BIG5);
  }
  if (gEncode == GB) {
    open_hanzi_font (DEFAULT_HZFONT_GB);
  }
}

int
open_ascii_font (unsigned char *ascii_font_name)
{
  int fd, nread, i, j;
  unsigned char tmpfont[256][16];

  fd = open (ascii_font_name, O_RDONLY);
  if (fd == -1) {
    fprintf (stderr, "fatal error: cannot open ascii font file %s\n",
	     ascii_font_name);
    exit (-1);
  }

  nread = read (fd, &tmpfont, sizeof (tmpfont));
  close (fd);
  if (nread != sizeof (tmpfont)) {
    fprintf (stderr, "fatal error: error reading ascii font file %s\n",
	     ascii_font_name);
    return 0;
  }

  for (i = 0; i < 256; i++) {
    for (j = 0; j < 16; j++)
      ascii_font[i][j] = tmpfont[i][j];
    ascii_font[i][16] = ascii_font[i][17] = 0;
  }

  /* setup the normal & underline buffer */

  ascii_buffer_normal = (unsigned char *) calloc (18, sizeof (unsigned char));
  ascii_buffer_underline =
    (unsigned char *) calloc (18, sizeof (unsigned char));

  ascii_buffer_normal[16] = ascii_buffer_normal[17] = 0;
  ascii_buffer_underline[16] = 0xff;
  ascii_buffer_underline[17] = 0;

  return 1;
}

int
open_hanzi_font (unsigned char *fn)
{
  /*  initialize Underline & normal font support (16+2)*2=36 */

  hanzi_buffer_normal = (unsigned char *) calloc (36, sizeof (unsigned char));
  hanzi_buffer_underline =
    (unsigned char *) calloc (36, sizeof (unsigned char));

  hanzi_buffer_normal[32] = hanzi_buffer_normal[33] = 0;
  hanzi_buffer_normal[34] = hanzi_buffer_normal[35] = 0;
  hanzi_buffer_underline[32] = hanzi_buffer_underline[33] = 0xff;
  hanzi_buffer_underline[34] = hanzi_buffer_underline[35] = 0;

  return hbfOpen (fn);
}

int
hbfOpen (char *hbffile)
{
  FILE *HBFfp;
  int i, j, m, mx, my, nb2;
  char cst[255];
  char path[255];

  /* only hbfOpen need to know  Byte 2 information */
  struct Bytes
  {
    unsigned int l;
    unsigned int h;
    unsigned int a;
  }
   *b2;

  /* use for making address look up table */
  int table1, table2, count, count1;

  struct stat attrib;

  /* generate font path */

  strcpy (path, hbffile);
  j = strlen (hbffile);
  if (path[j - 1] == '/')
    j -= 2;
  for (; (j > 0) && (path[j] != '/'); j--);
  path[++j] = '\0';
  i = strlen (path);
  if ((HBFfp = fopen (hbffile, "r")) == NULL) {
    fprintf (stderr, "Can't open HBF file -> %s\n", hbffile);
    return 0;
  }

  /*  check if it is a 16x16 font */

  while (fgets (cst, 119, HBFfp) != NULL)
    if (strncmp (cst, "HBF_BITMAP_BOUNDING_BOX", 23) == 0)
      break;
  sscanf (cst, "%*s%d%d", &mx, &my);

  if ((mx != 16) || (my != 16)) {
    fprintf (stderr, "Not a 16x16 hanzi font specfied\n");
    return 0;
  }

  /*  get byte 2 ranges */

  while (fgets (cst, 119, HBFfp) != NULL)
    if (strncmp (cst, "HBF_START_BYTE_2_RANGES", 23) == 0)
      break;

  sscanf (cst, "%*s%d", &nb2);
  b2 = (struct Bytes *) calloc (nb2, sizeof (struct Bytes));
  if (b2 == NULL) {
    fprintf (stderr, "Not enough memory\n");
    return 0;
  }

  total_range = 0;
  for (j = 0; j < nb2; j++) {
    while (fgets (cst, 119, HBFfp) != NULL)
      if (strncmp (cst, "HBF_BYTE_2_RANGE", 16) == 0)
	break;
    sscanf (cst, "%*s%i-%i", &(b2[j].l), &(b2[j].h));
    b2[j].a = b2[j].h - b2[j].l + 1;
    total_range += b2[j].a;
  }

  /*  get code ranges information */

  while (fgets (cst, 119, HBFfp) != NULL)
    if (strncmp (cst, "HBF_START_CODE_RANGES", 21) == 0)
      break;
  sscanf (cst, "%*s%d", &nseg);
  seg = (struct HBFrange *) calloc (nseg, sizeof (struct HBFrange));
  if (seg == NULL) {
    fprintf (stderr, "Not enough memory\n");
    return 0;
  }

  for (j = 0; j < nseg; j++) {
    while (fgets (cst, 119, HBFfp) != NULL)
      if (strncmp (cst, "HBF_CODE_RANGE", 14) == 0)
	break;

    sscanf (cst, "%*s%i-%i%s%li", &(seg[j].l), &(seg[j].h),
	    seg[j].bmfname, &(seg[j].offset));

    /*  setting up address look up table */

    count1 = 0;
    count = seg[j].l & 0xff;
    for (table1 = 0; table1 < 256; table1++, count = (count + 1) % 256) {
      seg[j].address_table[count] = -1;
      for (table2 = 0; table2 < nb2; table2++) {
	if ((count >= b2[table2].l) && (count <= b2[table2].h))
	  seg[j].address_table[count] = count1;
      }
      if (seg[j].address_table[count] != -1)
	count1++;
    }

    /*  open font file  now              */

    seg[j].fd = 0;
    if (j > 0) {
      for (m = 0; m < j; m++)
	if (strcmp (seg[m].bmfname, seg[j].bmfname) == 0) {
	  seg[j].fd = seg[m].fd;
	  seg[j].font_mem = seg[m].font_mem;
	}
    }
    if (!seg[j].fd) {
      strcpy (cst, path);
      strcat (cst, seg[j].bmfname);
      seg[j].fd = open (cst, O_RDONLY);
      if (!seg[j].fd) {
	fprintf (stderr, "Can't open bitmap font file -> %s\n", path);
	return 0;
      }
      stat (cst, &attrib);
      seg[j].st_size = attrib.st_size;
      seg[j].font_mem = (unsigned char *) mmap ((caddr_t) 0, attrib.st_size,
						PROT_READ,
						MAP_FILE | MAP_SHARED,
						seg[j].fd, (off_t) 0);
    }
  }

  for (i = 0; i < nseg; i++) {
    close (seg[i].fd);
    seg[i].font_mem += seg[i].offset;
  }

  free (b2);
  fclose (HBFfp);
  return 1;
}

void
hbfClose ()
{
  int j;
  for (j = 0; j < nseg; j++)
    munmap (seg[j].font_mem - seg[j].offset, seg[j].st_size);
  free (seg);
}

unsigned char *
ascGetBitmap (unsigned char c, unsigned char mode)
{
  /*
     if (c == 32)  //Is Space, then return normal, no underline...
     {
     memcpy(ascii_buffer_normal,&ascii_font[c][0],16);
     return ascii_buffer_normal_head;
     }
   */
  switch (mode) {
  case BOLD:
    memcpy (ascii_buffer_normal, &ascii_font[c][0], 16);
    c_bold_ascii (ascii_buffer_normal);
    return ascii_buffer_normal;

  case UNDERLINE:
    memcpy (ascii_buffer_underline, &ascii_font[c][0], 16);
    return ascii_buffer_underline;

  case BOLD_UNDERLINE:
    memcpy (ascii_buffer_underline, &ascii_font[c][0], 16);
    c_bold_ascii (ascii_buffer_underline);
    return ascii_buffer_underline;
  }
  return ascii_buffer_normal;
}

/* if the ch is a valid code in this HBF then return its bitmap,
   otherwise return NULL */
unsigned char *
hbfGetBitmap (unsigned int ch, unsigned char mode, int p_font_bytes)
{				/* add parameter fontbytes by Taiwan Linux Co LTD */
  /*unsigned char *hbfGetBitmap(unsigned int ch,unsigned char mode) */
  int i;
  int lsb, offset;
  unsigned char *addr;

  lsb = ch & 0xff;
#if 0
  if (ch < 0xA1A1 || lsb < 0xA1)
    return NULL;
#endif
  for (i = 0; i < nseg; i++)
    if (ch >= seg[i].l && ch <= seg[i].h) {
      offset = seg[i].address_table[lsb];
      if (offset == -1)
	return NULL;
      ch -= seg[i].l;

      if (p_font_bytes == 30) {
	addr = ((ch >> 8) * total_range + offset) * 30 + seg[i].font_mem;
      } else {
	addr = ((ch >> 8) * total_range + offset) * 32 + seg[i].font_mem;
      }

      switch (mode) {
      case STANDARD:

	if (p_font_bytes == 30) {
	  memcpy (hanzi_buffer_normal, addr, 30);
	} else {
	  memcpy (hanzi_buffer_normal, addr, 32);
	}
	return hanzi_buffer_normal;

      case UNDERLINE:
	memcpy (hanzi_buffer_underline, addr, 32);
	return hanzi_buffer_underline;

      case BOLD:
	memcpy (hanzi_buffer_normal, addr, 32);
	c_bold_hanzi (hanzi_buffer_normal);
	return hanzi_buffer_normal;

      case BOLD_UNDERLINE:
	memcpy (hanzi_buffer_underline, addr, 32);
	c_bold_hanzi (hanzi_buffer_underline);
	return hanzi_buffer_underline;
      }
      return NULL;
    }
  return NULL;
}
