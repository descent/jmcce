
/****************************************************************************/
/*            JMCCE  - asm_routine.S                                        */
/*                                                                          */
/*            	$Id: asm_routine.S,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $     */
/****************************************************************************/




#define GRA_I   0x3CE   /* Graphics Controller Index */
#define GRA_D   0x3CF   /* Graphics Controller Data Register */

.code32
.text	
.globl asm_draw_hanzi
.globl asm_draw_ascii
.globl asm_scroll_up
.globl asm_scroll_down
.globl asm_clear_block
.globl asm_clear_lines
.globl asm_bold_hanzi
.globl asm_bold_ascii
.globl asm_toggle_cursor
.globl asm_set_split
.comm graph_mem,4	
.comm active_console,4

/****************************************************************************
 *    void asm_draw_ascii(int col,int y,unsigned char *bitmap,int color);   *
 *         col:      0-79     column value                                  * 
 *         y:        0-479    y coordinate                                  *
 *         bitmap:   18 bytes unsigned char buffer                          *
 *         color:    0-3 bit: ForeGround 8-11 bit: BackGround               *
 ****************************************************************************/

COL    =  8
Y      =  12
BITMAP =  16
COLOR  =  20

.align 4
asm_draw_ascii:
#if 0	
	# calculate the base address
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edi
	pushl	%esi
        pushl   %edx
	pushl	%ecx
        pushl   %ebx
      
	movl	Y(%ebp),%edi
	leal	0(%edi,%edi,4),%edi
	sall	$4,%edi
	addl	COL(%ebp),%edi
	addl	graph_mem,%edi
	movl	BITMAP(%ebp),%esi
		
	# setup video graphic registers
	mov	$GRA_I,%dx
	movw	$0x0a05,%ax
	outw	%ax,%dx
	movw	$7,%ax
	outw	%ax,%dx
	mov	$0x03,%ax
	outw	%ax,%dx
	movb	$8,%al
	outb	%al,%dx

	inc	%dx
        movl    $18,%ecx
	mov	COLOR(%ebp),%ebx

ascii_loop:
        movb	(%esi),%al
	outb	%al,%dx
	andb	%bl,(%edi)
	not	%al
	outb	%al,%dx
	andb	%bh,(%edi)	
	addl	$80,%edi
	incl	%esi
        loop ascii_loop

	mov	$GRA_I,%dx
	mov	$3,%ax
	outw	%ax,%dx
	mov	$0x0805,%ax
	outw	%ax,%dx

        popl    %ebx
	popl	%ecx
        popl    %edx
	popl	%esi
	popl	%edi
	movl	%ebp,%esp
	popl	%ebp
#endif
	ret

/****************************************************************************
 *    void asm_draw_hanzi(int col,int y,unsigned char *bitmap,int color);   *
 *         col:      0-79     column value                                  *
 *         y:        0-479    y coordinate                                  *
 *         bitmap:   36 bytes unsigned char buffer                          *
 *         color:    0-3 bit: ForeGround 8-11 bit: BackGround               *
 ****************************************************************************/

COL    =  8
Y      =  12
BITMAP =  16
COLOR  =  20

.align 4
asm_draw_hanzi:
#if 0	
	# calculate the base address
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%edi
	pushl	%esi
        pushl   %ebx
        pushl   %ecx
	pushl	%edx

	movl	Y(%ebp),%edi
	leal	0(%edi,%edi,4),%edi
	sall	$4,%edi
	addl	COL(%ebp),%edi
	addl	graph_mem,%edi
	movl	BITMAP(%ebp),%esi
		
	# setup video graphic registers
	movw	$GRA_I,%dx
	movw	$0x0a05,%ax
	outw	%ax,%dx
	movw	$7,%ax
	outw	%ax,%dx
	movw	$03,%ax
	outw	%ax,%dx
	movb	$8,%al
	outb	%al,%dx

        movl    $18,%ecx
	incw	%dx
	mov	COLOR(%ebp),%ebx
hanzi_loop:
	movb	(%esi),%al
	outb	%al,%dx
	andb	%bl,(%edi)
	not	%al
	outb	%al,%dx
	andb	%bh,(%edi)

	incl	%esi
	incl	%edi
	movb	(%esi),%al
	outb	%al,%dx
	andb	%bl,(%edi)
	not	%al
	outb	%al,%dx
	andb	%bh,(%edi)
	addl	$79,%edi
	incl	%esi
        loop    hanzi_loop

	movw	$GRA_I,%dx
	movw	$3,%ax
	outw	%ax,%dx
	movw    $0x0805,%ax
	outw	%ax,%dx
   
        popl    %edx
	popl	%ecx
        popl    %ebx
	popl	%esi
	popl	%edi
	movl	%ebp,%esp
	popl	%ebp
#endif
	ret

/****************************************************************************
 *         void asm_scroll_up(int sy,int ey,int line,int bgcolor);          *
 *             sy:       0-479    start y coordinate                        *
 *             ey:       0-479    end y coordinate                          *
 *                       (scroll area include sy & ey)                      *
 *             line:     scroll up lines                                    *
 *             bgcolor:  0-3 bit: BackGround Color                          *
 ****************************************************************************/

SY      = 8
EY      = 12
LINE    = 16
BGCOLOR = 20

.align 4
asm_scroll_up:	
#if 1
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	pushl	%edi
	movw	%ds,%ax
	movw    %ax,%es

	# calculate the source base
	# Y * 80
	movl	SY(%ebp),%edi		# edi = SY
	movl	EY(%ebp),%ecx		# ecx = EY-SY ie height
	subl	%edi,%ecx
	leal	0(%edi,%edi,4),%edi      
	sall	$4,%edi			# edi = SY*80
	# graph_mem[Y*80]
	addl	graph_mem,%edi         # edi = graph_mem[SY*80]
	movl	LINE(%ebp),%esi         # esi = LINE 
	subl	%esi,%ecx               # ecx = EY-SY-LINE+1
					# i.e. the height of area which
					# must be scroll
	incl	%ecx
	leal	0(%esi,%esi,4),%esi	
	sall	$4,%esi			
	addl	%edi,%esi		# esi = graph_mem[(SY+LINE)*80]
					# i.e. the first position must be copy
	# set to write mode 1
	movw	$GRA_I,%dx
	movw	$0x0105,%ax
	outw	%ax,%dx
	leal	0(%ecx,%ecx,4),%ecx	# in 8bit mode 80 bytes must be copy
	sall	$4,%ecx		

	rep 				
	movsb	                        # Here cant use movsw/movsl
                                        # it can only use 8 bit mode

	# blank the below line
	# use mode 
	movw	$0x0a05,%ax
	outw	%ax,%dx
	movw	$0xff08,%ax
	outw	%ax,%dx
	movw	$7,%ax
	outw	%ax,%dx
	movw	$3,%ax
	outw	%ax,%dx
	movw	BGCOLOR(%ebp),%ax
	movl	LINE(%ebp),%ecx
	leal	0(%ecx,%ecx,4),%ecx
	sall	$4,%ecx

	rep 
	stosb                            # here can use 32 bit mode ?
	mov	$0x0805,%ax

	outw	%ax,%dx
   	pop	%edi
	pop	%esi
	movl	%ebp,%esp
	pop	%ebp
#endif
	ret

/****************************************************************************
 *       void asm_scroll_down(int sy,int ey,int line,int bgcolor);          *
 *             sy:       0-479    start y coordinate                        *
 *             ey:       0-479    end y coordinate                          *
 *                       (scroll area include sy & ey)                      *
 *             line:     scroll down lines                                  *
 *             bgcolor:  0-3 bit: BackGround Color                          *
 ****************************************************************************/

SY      = 8
EY      = 12
LINE    = 16
BGCOLOR = 20

.align 4
asm_scroll_down:	
#if 1
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	pushl	%edi
	mov 	%ds,%ax
	mov	%ax,%es

	# calculate the source base
	# Y * 80
        movl    EY(%ebp),%edi
        incl    %edi                    # edi = EY+1

        movl    LINE(%ebp),%esi
        negl    %esi
        addl    %edi,%esi               # esi = EY+1-LINE

	movl	SY(%ebp),%ecx		# ecx = EY
        negl    %ecx
	addl	%esi,%ecx               # ecx = EY+1-LINE-SY
                                        # i.e. the height of area which
                                        # must be scroll
	leal	0(%edi,%edi,4),%edi      
	sall	$4,%edi			# edi = (EY+1)*80-1
	addl	graph_mem,%edi    
        decl    %edi
       
	leal	0(%esi,%esi,4),%esi	
	sall	$4,%esi			
	addl	graph_mem,%esi		# esi = (EY+LINE-1)*80-1 
        decl    %esi                    # i.e. the first position must be copy

        leal    0(%ecx,%ecx,4),%ecx     # in 8bit mode 80 bytes must be copy
        sall    $4,%ecx                 # ecs * 80

	# set to write mode 1
	mov	$GRA_I,%dx
	movw	$0x0105,%ax
	outw	%ax,%dx

        std
	rep 				
	movsb				

	# blank the above line
	# use mode 
	movw	$0x0a05,%ax
	outw	%ax,%dx
	movw	$0xff08,%ax
	outw	%ax,%dx
	movw	$7,%ax
	outw	%ax,%dx
	movw	$3,%ax
	outw	%ax,%dx
	movw	BGCOLOR(%ebp),%ax
	movl	LINE(%ebp),%ecx
 
	leal	0(%ecx,%ecx,4),%ecx
	sall	$4,%ecx

	rep 
	stosb               #16 bit is OK, 32bit(stosl) is not OK!
        cld
	mov	$0x0805,%ax
	outw	%ax,%dx

   	pop	%edi
	pop	%esi
	movl	%ebp,%esp
	pop	%ebp
#endif
	ret

/****************************************************************************
 *  void asm_clear_block(int col,int y,int width,int height,int bgcolor);   *
 *         col:      0-79  left column value                                *
 *         y:        9-479 upper y coordinate                               *
 *         width:    1-80  clear block width(in columns)                    *
 *         height:   1-480 clear block height(in scanlines)                 *
 *         bgcolor:  0-3 bit: BackGround Color                              *
 ****************************************************************************/

COL     = 8        
Y       = 12      
WIDTH   = 16  
HEIGHT  = 20
BGCOLOR = 24

.align 4
asm_clear_block:
#if 0
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %esi
        pushl   %edi
        pushl   %ebx
        pushl   %ecx
        pushl   %edx

        mov     %ds,%ax
        mov     %ax,%es

        movl    Y(%ebp),%edi
        leal    0(%edi,%edi,4),%edi 
        sall    $4,%edi                      # edi = Y * 80 + COL
        addl    graph_mem,%edi
        addl    COL(%ebp),%edi
 
        movl    $80,%esi
        subl    WIDTH(%ebp),%esi             # esi=80-Width 
        movl    HEIGHT(%ebp),%ebx

        # set up register 
        movw    $GRA_I,%dx
        movw    $0x0a05,%ax
        outw    %ax,%dx
        movw    $0xff08,%ax
        outw    %ax,%dx
        movw    $7,%ax
        outw    %ax,%dx
        movw    $3,%ax
        outw    %ax,%dx
        movw    BGCOLOR(%ebp),%ax
 fill:
        movl    WIDTH(%ebp),%ecx
        rep
        stosb   
        addl    %esi,%edi

        decl    %ebx
        jnz     fill

        movw    $0x0805,%ax
        outw    %ax,%dx
       
        popl    %edx
        popl    %ecx
        popl    %ebx
        popl    %edi
        popl    %esi
        movl    %ebp,%esp
        popl    %ebp
#endif
        ret

/****************************************************************************
 *           void asm_clear_lines(int sy,int height,int bgcolor);           *
 *              sy:       0-479 upper y coordinate                          *
 *              height:   1-480 clear block height(in scanlines)            *
 *              bgcolor:  0-3 bit: BackGround Color                         * 
 ****************************************************************************/

SY      = 8
HEIGHT  = 12
BGCOLOR = 16

.align 4
asm_clear_lines:
#if 0
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %edi
        pushl   %ecx

        movw    %ds,%ax
        movw    %ax,%es

        movl    SY(%ebp),%edi
        leal    0(%edi,%edi,4),%edi
        sall    $4,%edi                      # edi = SY * 80 
        addl    graph_mem,%edi
 
        movl    HEIGHT(%ebp),%ecx
        leal    0(%ecx,%ecx,4),%ecx          # ecx= height * 80
        sall    $4,%ecx

        # set mode
        movw    $GRA_I,%dx
        movw    $0x0a05,%ax
        outw    %ax,%dx
        movw    $0xff08,%ax
        outw    %ax,%dx
        movw    $7,%ax
        outw    %ax,%dx
        movw    $3,%ax
        outw    %ax,%dx
      
        movw    BGCOLOR(%ebp),%ax
        rep
        stosb

        movw    $0x0805,%ax
        outw    %ax,%dx

        popl    %ecx
        popl    %edi
        movl    %ebp,%esp
        popl    %ebp
#endif
        ret


/***************************************************************************
 *               void asm_bold_hanzi(unsigned char *buffer);               *
 *                  buffer:  32 bytes hanzi bitmap buffer                  *
 ***************************************************************************/

BUFFER = 8

.align 4
asm_bold_hanzi:
#if 0
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %eax
        pushl   %ebx
        pushl   %ecx
        pushl   %edx

        movl    BUFFER(%ebp),%ebx
        movl    $16,%ecx
 HANZI_next:
        movw    (%ebx),%ax
        movw    %ax,%dx

        sarw    $1,%ax
        andw    $0x7f7f,%ax
        orw     %ax,%dx
    /* the hanzi effect is acceptable, 16*16 is large enough for or */

        movw    %dx,(%ebx)
        addl    $2,%ebx
        loop HANZI_next

        popl    %edx
        popl    %ecx
        popl    %ebx
        popl    %eax
        movl    %ebp,%esp
        popl    %ebp
#endif
        ret


/***************************************************************************
 *               void asm_bold_ascii(unsigned char *buffer);               *
 *                  buffer:  16 bytes hanzi bitmap buffer                  *
 ***************************************************************************/

BUFFER = 8

.align 4
asm_bold_ascii:
#if 0
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %eax
        pushl   %ebx
        pushl   %ecx
        pushl   %edx

        movl    BUFFER(%ebp),%ebx
        movl    $16,%ecx
 ASCII_next:
        movb    (%ebx),%al
        movb    %al,%dl
  
        salb    $1,%al
        andb    $0xC0,%al
        orb     %al,%dl
   /* the effect is not good, because 8x16 is too narrow for 'or' */

        movb    %dl,(%ebx)
        incl    %ebx
        loop ASCII_next

        popl    %edx
        popl    %ecx
        popl    %ebx
        popl    %eax
        movl    %ebp,%esp
        popl    %ebp
#endif
        ret

/***************************************************************************
 *               void asm_toggle_cursor(int col,int y);                    *
 *                       col:      0-79  column value                      *
 *                         y:      0-479 y coordinate                      * 
 ***************************************************************************/

COL = 8
Y   = 12

.align 4
asm_toggle_cursor:
#if 0
        pushl   %ebp
        movl    %esp,%ebp
        pushl   %ecx
        pushl   %ebx

        movl    Y(%ebp),%ecx
        leal    0(%ecx,%ecx,4),%ecx
        sall    $4,%ecx
        addl    COL(%ebp),%ecx
        addl    graph_mem,%ecx            #edi = 80*Y+COL

        movw    $GRA_I,%dx
        movb    $3,%al
        outb    %al,%dx
        incw    %dx
        inb     %dx,%al
        movb    %al,%bh                   # old Reg 3 put in bh
        movb    $0x18,%al
        outb    %al,%dx                   # set Reg 3 to 0x18

        decw    %dx
        movb    $8,%al
        outb    %al,%dx
        incw    %dx
        inb     %dx,%al
        movb    %al,%bl                   # old Reg 8 put in bl
        movb    $0xff,%al
        outb    %al,%dx                   # set Reg 8 to 0xff

        movb    (%ecx),%al
        movb    %al,(%ecx)                # write after read 
        addl    $80,%ecx                  # cursor width 8, height 2
        movb    (%ecx),%al
        movb    %al,(%ecx)    

        decw    %dx
        movb    $3,%al
        outb    %al,%dx
        movb    %bh,%al
        incw    %dx
        outb    %al,%dx                   # Restore the old Reg 3

        decw    %dx
        movb    $8,%al
        outb    %al,%dx
        movb    %bl,%al
        incw    %dx
        outb    %al,%dx                   # Restore the old Reg 8

        popl    %ebx
        popl    %ecx
        movl    %ebp,%esp
        popl    %ebp
#endif
        ret

.align 4
asm_set_split:
#if 0
	mov	$0,%cx		# get the offset
	mov	%ch,%ah		# put high part into ah
	mov	$0x0c,%al	# this is the start at register index
	mov	$0x3d4,%dx	# the CRTC command channel
	out	%ax,%dx		# set the high byte
	inc	%al		# point to start at register two
	mov	%cl,%ah		# get the low byte
	out	%ax,%dx		# set the low byte.

	mov	$0x3da,%dx	#status
lp1:	
	in	%dx,%al
	and	$8,%al
	jz	lp1		#wait for retrace
	mov	$432,%bx	# set our page height here
	shl	$1,%ebx		# double it for the pixel height
	mov	$0x3d4,%dx 	# The CRTC command channel
	mov	$0x18,%al	# point to Line Compare
	mov	%bl,%ah		# get the lower 8 (0-7) bits
	out	%ax,%dx		# send them to the register
	mov	$0x11,%al	# vert retrace
	out	%al,%dx		# set it
	inc	%dx		# data channel
	in	%dx,%al		# get protect bit
	and	$0x7f,%al	# shut it off!
	out	%al,%dx		# unprotect.
	dec	%dx		# back to command
	mov	$07,%al		# point at the Overflow register
	out	%al,%dx		# select it
	inc	%dx		# move to data channel again
	in	%dx,%al		# read current settings
	mov	%bh,%ah		# get high bits of split again
	and	$1,%ah		# mask off bit 8
	shl	$4,%ah		# shift it to bit 4
	and	$0x0ef,%al	# clear any existing bit 4
	or	%ah,%al		# or them together to set
	out	%al,%dx		# and send new value
	dec	%dx		# move back to command channel
	mov	$0x09,%al	# point to Max Scan Line
	out	%al,%dx		# select it
	inc	%dx		# move to data channel
	in	%dx,%al		# get current settings
	mov	%bh,%ah		# get the high bits of our split
	and	$2,%ah		# mask off bit 9
	shl	$5,%ah		# shift it into position (bit 6)
	and	$0x0bf,%al	# clear any bit 6 of the current settings
	or	%ah,%al		# or them together to set 6 if we need
	out	%al,%dx		# send them to the register
	dec	%dx
	mov	$0x11,%al	# vert retrace
	out	%al,%dx
	inc	%dx
	in	%dx,%al		# get protect bit
	or	$0x80,%al	# turn it on
	out	%al,%dx		# protect
#endif
	ret
