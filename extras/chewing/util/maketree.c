/*
	This program reads in a dictionary with phone phrases(in uint16 form).
	Output a database file which indicates a phone phrase tree.
	each node represents a single phone.
	The output file was a random access file, a record was defined:
	{ 
		uint16 key; the phone data
		int32 phraseno; 
		int32 begin,end; the children of this node(-1,-1 indicate a leaf node)
	}
*/
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
/*
	defines
*/
#define MAX_PH_NODE			6400000
#define IN_FILE				"phoneid.dic"
#define OUTFILE				"fonetree.dat"
/* 
	typedefs 
*/
typedef unsigned short uint16;
typedef int	int32;

/* Forward declaration */
struct _tLISTNODE;

typedef struct _tNODE
{
	struct _tLISTNODE*	childList;
	uint16	key;
	int32	phraseno,nodeno;
} NODE;

typedef struct _tLISTNODE
{
	struct _tNODE *pNode;
	struct _tLISTNODE *next;
} LISTNODE;

/*
	global data
*/
NODE *root;
int32 ph_count;

NODE *queue[MAX_PH_NODE];
int head,tail;

int node_count;

void QueuePut(NODE *pN)
{
	queue[head++] = pN;
	if( head == MAX_PH_NODE ) {
		fprintf(stderr,"Queue size not enough!\n");
		exit(1);
	}
}

NODE* QueueGet(void)
{
	assert(head!=tail);
	return queue[tail++];
}

int QueueEmpty(void)
{
	return head==tail;
}

NODE *NewNode(uint16 key)
{
	NODE *pnew = (NODE*)malloc(sizeof(NODE));
	pnew->key = key;
	pnew->childList = NULL;
	pnew->phraseno = -1;
	pnew->nodeno = -1;
	return pnew;
}

void InitConstruct(void)
{
	/* root has special key value 0 */
	root = NewNode(0);
}

NODE* FindKey(NODE *pN,uint16 key)
{
	LISTNODE *p;

	for(p=pN->childList;p;p=p->next) {
		if( p->pNode->key == key)
			return p->pNode;
	}
	return NULL;
}

NODE* Insert(NODE *pN,uint16 key)
{
	LISTNODE *prev,*p;
	LISTNODE *pnew = (LISTNODE*)malloc(sizeof(LISTNODE));
	NODE *pnode = NewNode(key);
	
	pnew->pNode = pnode;
	pnew->next  = NULL;

	prev = pN->childList;
	if( !prev) {
		pN->childList = pnew;
	}
	else {
		for(p=prev->next;(p) && (p->pNode->key<key);prev=p,p=p->next)
			;
		prev->next = pnew;
		pnew->next = p;
	}
	return pnode;
}

void Construct(void)
{
	FILE *input = fopen(IN_FILE,"r");
	NODE *pointer,*tp;
	uint16 key;

	if(!input) {
		fprintf(stderr,"error opening \"phoneid.dic\"\n");
		exit(1);
	}
	InitConstruct();
	
	while(1) {	
		fscanf(input,"%hu",&key);
		if(feof(input))
			break;
		pointer = root;
		for( ;key!=0; fscanf(input,"%hu",&key)) {	/* for each phone in a phone phrase */
			if( (tp=FindKey(pointer,key)) ) {
				pointer = tp;
			}
			else {
				tp = Insert(pointer,key);
				pointer = tp;
			}
		}
		pointer->phraseno = ph_count++;
	}
}
/* Give the level-order travel number to each node */
void BFS1(void)
{
	NODE *pNode;
	LISTNODE *pList;

	QueuePut(root);
	while(!QueueEmpty()) {
		pNode = QueueGet();
		pNode->nodeno = node_count++;

		for(pList=pNode->childList;pList;pList=pList->next) {
			QueuePut(pList->pNode);
		}
	}
}

void BFS2(void)
{
	NODE *pNode;
	LISTNODE *pList;
	FILE *output = fopen(OUTFILE,"w");

	if(!output) {
		fprintf(stderr,"error opening file \"%s\" for output\n",OUTFILE);
		exit(1);
	}

	QueuePut(root);
	while(!QueueEmpty()) {
		pNode = QueueGet();
		
		fprintf(output,"%hu ",pNode->key);
		fprintf(output,"%d ",pNode->phraseno);

		/* compute the begin and end index */
		pList=pNode->childList;
		if( pList) {
			fprintf(output,"%d ",pList->pNode->nodeno);

			for(;pList->next;pList=pList->next) {
				QueuePut(pList->pNode);
			}
			QueuePut(pList->pNode);
			fprintf(output,"%d\n",pList->pNode->nodeno);
		}
		else
			fprintf(output,"-1 -1\n");

	}
	fclose(output);
}

int main(void)
{
	Construct();
	BFS1();		
	BFS2();

	return 0;
}
