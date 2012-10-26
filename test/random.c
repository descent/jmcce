/*    This program use to test the speed of showing  hanzi 
      (ETen specific Big5 coding)  
      should work fine under PC-DOS              
*/      
#include <stdlib.h>
main()
{
  int b2_s[2],b2_end[2],code_start[4],code_end[4],out;
  int hanzi,j,hanzilsb;
  int count,column;
  float count1;
  int hanzi_code[13868];
  b2_s[0] = 0x40;b2_s[1] = 0xa1;           /* Big5 byte2 ranges 2  */
  b2_end[0] = 0x7e;b2_end[1] = 0xfe;         
  code_start[0]=0xa140;code_end[0]=0xa3bf; /* spcfont.24  408 chars*/
  code_start[1]=0xa440;code_end[1]=0xc67e; /* stdfont.24  5401 chars*/  
  code_start[2]=0xc6a1;code_end[2]=0xc8d3; /* spcfsupp.24  365 chars*/
  code_start[3]=0xc940;code_end[3]=0xf9fe; /* stdfont.24  7693 chars*/
  
  out = 1;
  count = 0;
  hanzi = code_start[0];
  hanzilsb = hanzi & 0xff;
  while((hanzi!=code_end[3]+1))
    {
    if(hanzilsb<b2_s[0]);
    else if(hanzilsb>b2_end[1]);
    else if((hanzilsb>b2_end[0])&&(hanzilsb<b2_s[1]));
    else 
    {
    for(j=0;j<4;j++)
     {
     if((hanzi>=code_start[j])&&(hanzi<=code_end[j]))
        {
        hanzi_code[count] = hanzi;
        count++;
        }
     }
    }
    hanzi++;
    hanzilsb = hanzi & 0xff;
    }
    column = 0;
  for(j=0;j<13867;j++)
     {
again:  count1 = (random()/0x25990);
     count = (int) count1;
     if(count>13867) goto again; 
     printf("%c%c",(hanzi_code[count]>>8),(hanzi_code[count]&0xff)); 
     column++;
     if(column==39) {printf("\n"); column = 0;}
     }  
}
		
