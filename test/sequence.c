/*    This program use to test the speed of showing  hanzi 
      (ETen specific Big5 coding)  
      should work fine under PC-DOS              
*/      
main()
{
  int b2_s[2],b2_end[2],code_start[4],code_end[4],out;
  int hanzi,j,hanzilsb;
  int count;
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
        count ++;
        printf("%c%c",(hanzi>>8),(hanzi&0xff));
        
        /* seem some program can't auto CR & LF, we do it instead :( */
        if(count==39) { printf("\n"); count = 0; }  
        }
     }
    }
    hanzi++;
    hanzilsb = hanzi & 0xff;
    }
}
		
