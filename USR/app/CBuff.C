#include "CBuff.h"

void CBuff_Init(CBuff_st *CBuff)
{
	CBuff->pWIndex = 0;//CBuff->Buff;
	CBuff->pRIndex = 0;//CBuff->Buff;
	CBuff->NumData = 0;
}

CBuff_Erro_t CBuff_Write(CBuff_st *CBuff,CBuff_t data)
{
	if(CBuff->NumData == CBuff_MAX_SIZE)
		return CBuff_EFULL;
	else
	{
		CBuff->Buff[CBuff->pWIndex] = data;
		//enter critical
		CBuff->NumData++;
		CBuff->pWIndex = (CBuff->pWIndex+1)%CBuff_MAX_SIZE;
		//exit critical
		return CBuff_EOK;
	}
}

CBuff_Erro_t CBuff_Read(CBuff_st *CBuff,CBuff_t *data)
{
	if(CBuff->NumData == 0)
		return CBuff_EEMPTY;
	else
	{
		*data = CBuff->Buff[CBuff->pRIndex];
		//enter critical
		CBuff->NumData --;
		CBuff->pRIndex = (CBuff->pRIndex+1)%CBuff_MAX_SIZE;
		//exit critical
		return CBuff_EOK;
	}
}
