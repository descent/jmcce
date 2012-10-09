
/***************************************************************************/
/*		JMCCE	Convert tit-format tui-format (tit2tui.c) 	   */
/*									   */
/*                 $Id: tit2tui.c,v 1.2 2002/05/15 13:44:30 kids Exp $ */
/***************************************************************************/

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "newimp.h"

#define _lcreat creat  // for DOS compatible
#define _lwrite write  // for DOS compatible
#define _lopen  open   // for DOS compatible
#define _lclose close  // for DOS compatible
#define _llseek lseek  // for DOS compatible
#define _lread  read   // for DOS compatible

//#define ANSI            //* Comment out for UNIX version     
#ifdef ANSI             //* ANSI compatible version          
#include <stdarg.h>
int average( int first, ... );
#else                   //* UNIX compatible version          
#include <varargs.h>
int average( va_list );
#endif

static int  s_dictionary =0;
static int  s_phrase =1;
	
struct HeaderTUI
{
        char    szId[4];
        char    szPrompt[20];
        char    szEngName[20];
        char    szVersion[5];
        char    szWin[3];
        char    szEnCode[4];
        char    szMisc[26];
        char    szAutoSelect;
        char    szReverse[61];
        char    szNotUsed[112];
                
};


//------------------------------------------------------------------------------------------
int s_TUI_Out( FILE* fpIn,int  fpOut, long  szIndex[][95],  int phrase );
int ParseHeader( FILE *fp,TUI_Header *tui );
void  HeaderTUI_Init(TUI_Header *tui );

void  HeaderTUI_Init(TUI_Header *tui )
{       
        tui->szId[0] ='T';
        tui->szId[1] ='U';
        tui->szId[2] ='I';
        tui->szId[3] ='1';
        tui->szEnCode[0] ='B';   
        tui->szEnCode[1] ='I';   
        tui->szEnCode[2] ='G';   
        tui->szEnCode[3] ='5';
}


void g_error(char*p1,  char *p2 )
{
        printf("%s %s\n", p1, p2 );

}


void g_message(char*p1)
{
        printf("%s\n", p1);
}

int compar(const void *a, const void *b)
{
	char *aa = (char *) a, *bb = (char *) b;
	int i;
	
	while (*aa != ' ') {
	
	  if (*aa > *bb) return 1;
	  if (*aa == *bb) { aa++; bb++; continue; }
	  return -1;
	
	}

}

void sort_tit(FILE *orig, FILE *sorted)
{
	char	buf[18000][400];
	char	*p = 0;
	int 	i = 0, j;
	
	while (feof(orig) == 0) {
	   fgets(buf[i], 400, orig);
	   
	   if (buf[i][0] == '#') continue;
	   
	   p = strstr(buf[i], "\\040");
           if (p != NULL) memset(p, ' ', 4);
          
	   if ( i < 18000) i++;
	   else {
	     printf("DICTIONARY too much!\n");
	     exit(1);
	   }
	   
	}
	
		
        printf("Sort %d lines\n", i);

	qsort(buf, i, 400, compar);
	for(j = 0; j < i+1; j++)
	  fputs(buf[j], sorted);	  

}
void tit2tui_module( char* pIn,  char*pOut )
{
	TUI_Header	headerTUI;

        int     tuiheader_size  = sizeof(  TUI_Header );
	long    szTUIPos[95][95];
	int     iOut;
	FILE    *fpIn, *fpInTemp;
	int     phrase;

        memset( (char*)&headerTUI,  0,  tuiheader_size );
        HeaderTUI_Init( &headerTUI );

        memset( (char*)&szTUIPos,  0, 95*95*4 );

        fpIn = fopen ( pIn, "r"); 
        if(fpIn==NULL){ fprintf(stderr,"open error:%s\n",  pIn); return; }
    
        fpInTemp = fopen ( "/tmp/tit2tui.temp", "w+"); 
        if(fpInTemp==NULL){ fprintf(stderr,"open error:/tmp/tit2tui.temp\n"); return; }

        iOut = _lcreat( pOut , 0 );
        if(iOut==-1){ fprintf(stderr,"open error:%s\n",pOut ); fclose(fpIn); return; }
        
        phrase  = ParseHeader( fpIn, &headerTUI );

        _lwrite( iOut, (char*)&headerTUI, tuiheader_size  );
        _lwrite( iOut, (char*)szTUIPos,    95*95*4 );

        sort_tit(fpIn, fpInTemp);
        rewind(fpInTemp);
        
 	headerTUI.szMaxKeyLenFullLimit =
 		s_TUI_Out(  fpInTemp, iOut, szTUIPos, phrase );
 		
        fclose(fpIn);
        fclose(fpInTemp);
        
	printf("MaxKeyLenFullLimit = %d\n",headerTUI.szMaxKeyLenFullLimit);
	
        _llseek( iOut, 0, SEEK_SET );
        _lwrite( iOut, (char*)&headerTUI, tuiheader_size  );
        _lwrite( iOut, (char*)szTUIPos,    95*95*4 );
        
        _lclose(iOut);
}




int main( int argc , char *argv[] )
{
        char szIn[40];
	char szOut[40];

        if (argc !=  3 )
        { 
            g_error("Usage: tit2tui","[tit file name] [tui file name]");
            return 0;
        }
        else
        {
             g_message("Converting...");
        }

        strcpy(szIn, argv[1] );
        strcpy(szOut, argv[2] );

        tit2tui_module( szIn,  szOut );

        g_message("Convert successed!");
        
        return 0;

}



int  ParseHeader( FILE *fp,  TUI_Header *tui )
{
        char     szBuf[500];
        char     szPrompt[50] ="";
        char     szSelect[50] ="";

        int      SELECT_ALWAYS = 1;
        int      SELECT_NEVER  = 2;
        int      KEY_SPACE        =     32;             
        int      phrase =  -1;  

        while ( fscanf(fp, "%s", szBuf) != EOF) 
        {       
                if(szBuf[0]=='#')
                {       
                        fgets(szBuf, 400, fp ); continue;
                }
                else if (strcmp (szBuf, "PROMPT:") == 0)
                {
		        int i;
			
                        fscanf(fp, "%s", szBuf);        
                        for(i=0; i<40; i++)
                        if( szBuf[i]=='\\' && szBuf[i+1]=='0' && szBuf[i+2]=='4' && szBuf[i+3]=='0'  )
                        {       szBuf[i] = KEY_SPACE;   
                                szBuf[i+1] = '\0';              
                                break;
                        }
                strcpy( szPrompt, szBuf );
                continue;
                }
                else if (strcmp (szBuf, "AUTOSELECT:") == 0)
                {
                        fscanf(fp, "%s", szBuf);        
			strcpy( szSelect, szBuf );
                        continue;
                }
                else if (strcmp (szBuf,"BEGINDICTIONARY") == 0)
                {       
                        phrase = s_dictionary;
                        break;
                }
                else if (strcmp (szBuf, "BEGINPHRASE") == 0  )
                {       
                        phrase = s_phrase;
                        break;
                }
        }
        
        strcpy( tui->szPrompt,  szPrompt );
	tui->szAutoSelect = SELECT_ALWAYS;
	
        if     (strcmp (szSelect, "NEVER" ) == 0)  tui->szAutoSelect = SELECT_NEVER;
        else if (strcmp (szSelect, "ALWAYS") == 0)  tui->szAutoSelect = SELECT_ALWAYS;
        else    fprintf(stderr, "unKnown AUTOSELECT:, use default ALWAYS.\n");

        if(phrase==-1) fprintf(stderr, "unKnown  BEGINDICTIONARY ?  BEGINPHRASE" );

        return  phrase;

}



int TUI_OutBlock( int fp, char* pSearch,  char* phraseOut)
{
        int n=0;
        unsigned char l = strlen( pSearch );
        unsigned char str;
	
        _lwrite( fp, (char*)&l, 1  );
        n++;

        if(l>2)   
        {       _lwrite( fp, (char*)(pSearch+2), (l-2) );
                n+=(l-2);
        }

        str = strlen(phraseOut);
        _lwrite( fp, (char *)&str,1);
        n++;

        _lwrite( fp, (char *)phraseOut,str);
        n+=str;
        return  n;

}



int s_TUI_Out( FILE* fpIn,  int  fpOut,  long szIndex[][95], int phrase )
{
        int     KEY_SPACE         =     32;             
        char    szTest[500];
        long    offset = 256 +95*95*4;
        char    szFirst[2]={'\0','\0'};
	int 	maxpress = 0;	
	int	count = 0, k;
	char 	*p;
	
        while ( fscanf(fpIn, "%s", szTest) != EOF) 
        {       
                if( szTest[0]=='#')
                {       
                        fgets( szTest, 400, fpIn );     continue;
                }
                else
                {
                        char    szSearch[22];
                        
                        if( (int)szTest[0] >=KEY_SPACE && (int)szTest[0] <=126 )
                        {
                                int  i;

                                
                                p = strstr(szTest, "\\040");
                                if (p != NULL) bzero(p, 4);
                                	
                                for( i=0; i<21; i++)
                                {
					if (szTest[i] == KEY_SPACE ||
					    szTest[i] == '\0') 
					{
					
						count = i;
						break;
					}
				}
                                strcpy(szSearch, szTest );
			       
                                if(strlen(szSearch)==1) 
                                  strcat( szSearch," "  );

                                if( szFirst[0] != szSearch[0] || szFirst[1] != szSearch[1]  ) 
                                {
				        int s0;
					int s1;
					char id;
					int n;
				
                                        szFirst[0] = szSearch[0];
                                        szFirst[1] = szSearch[1];

                                        s0 = szFirst[0] - KEY_SPACE;
                                        s1 = szFirst[1] - KEY_SPACE;

                                        szIndex[s0][s1] = (long)offset;

                                        id=0;
                                        n = _lwrite( fpOut, (char*)&id,1);
                                        offset+=n;
                                }
                                continue;
                        }
                        else
                        {      
                        	 
                        	//printf("count = %d\n",count);
                        	if (maxpress < count) maxpress = count;
                        	count = 0;
                                if( phrase==s_dictionary )
                                {
                                        int l = strlen(szTest) ;
                                        char szWord[3];
					int i;
										
                                        for(i=0; i<l ;i+=2)
                                        {
					        int n;
						
                                                szWord[0]=szTest[i]; 
                                                szWord[1]=szTest[i+1]; 
                                                szWord[2]='\0';
                                                n = TUI_OutBlock( fpOut, szSearch, szWord );
                                                offset+=n;
                                        }
                                }
                                else if( phrase==s_phrase )
                                {
                                        int n = TUI_OutBlock( fpOut, szSearch, szTest );
                                        offset+=n;
                                }

                                
                        }
                }

        }
	return maxpress;        
}
