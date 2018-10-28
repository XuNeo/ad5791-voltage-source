#ifndef _CBUFF_H_
#define _CBUFF_H_

#define CBuff_t char //element type
#define CBuff_MAX_SIZE 64

typedef enum
{
	CBuff_EOK = 0,
	CBuff_EFULL = 1,
	CBuff_EEMPTY = 2
} CBuff_Erro_t;

typedef struct
{
	CBuff_t pWIndex;
	CBuff_t pRIndex;
	unsigned short NumData;
	CBuff_t Buff[CBuff_MAX_SIZE];
} CBuff_st;

void CBuff_Init(CBuff_st *CBuff);
CBuff_Erro_t CBuff_Write(CBuff_st *CBuff,CBuff_t data);
CBuff_Erro_t CBuff_Read(CBuff_st *CBuff,CBuff_t *data);

#endif
