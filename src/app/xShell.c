#include "string.h"
#include "xShell.H"
#include "xprintf.h"

#if ENABLE_XSHELL_PRINTF
/*compelte function xShell_printf(const char*str,...)*/
#include "stdio.h"
#define xShell_printf xPrintf
#else
void xShell_printf(const char*str,...){};
#endif
	
#if USE_XSHELL

extern int xShellTab$$Base;	
extern int xShellTab$$Limit;

static unsigned long xShell_Parameter[MAX_PARAMETER];

#define _xShell_Record_Base  (xShell_Recorde_st *)&xShellTab$$Base
#define _xShell_Record_Limit (xShell_Recorde_st *)&xShellTab$$Limit

static xShell_Recorde_st* xShell_SearchFunc(char*pFuncName)
{
	xShell_Recorde_st *pRecord = _xShell_Record_Base;
	
	for(;pRecord<_xShell_Record_Limit;pRecord++)
	{
		if(pRecord->nByteOfValue!=0)//function ,this field must be zero
		{
			continue;
		}
		else
		{
			if(strcmp(pFuncName,pRecord->name)==0)
				return pRecord;
		}
	}
	
	return (xShell_Recorde_st*)0;
}

static xShell_Recorde_st* xShell_SearchVar(char*pVarName)
{
	xShell_Recorde_st *pRecord = (xShell_Recorde_st *)_xShell_Record_Base;
	
	for(;pRecord<_xShell_Record_Limit;pRecord++)
	{
		if(pRecord->nByteOfValue ==0)//variable ,this field must not be zero
		{
			continue;
		}
		else
		{
			if(strcmp(pVarName,pRecord->name)==0)
				return pRecord;
		}
	}
	
	return (xShell_Recorde_st*)0;
}

static xShell_Recorde_st*xShell_Search(char*pVarName)
{
	xShell_Recorde_st *pRecord = (xShell_Recorde_st *)_xShell_Record_Base;
	
	for(;pRecord<_xShell_Record_Limit;pRecord++)
	{
		if(strcmp(pVarName,pRecord->name)==0)
			return pRecord;
	}
	
	return (xShell_Recorde_st*)0;	
}

static unsigned long xShell_CallFunc(xShell_Recorde_st*pRecord,unsigned int parameters)
{
	xShell_Func f = NULL;
	unsigned long r=0;
	
	if(pRecord == NULL)
		return (unsigned long)-1l;
	
	f = (xShell_Func)(pRecord->addr);
	switch (parameters)
	{
	case 0:
		r = f(0);
		break;

	case 1:
		r = f(xShell_Parameter[0]);
		break;

	case 2:
		r = f(xShell_Parameter[0], xShell_Parameter[1]);
		break;

	case 3:
		r = f(xShell_Parameter[0], xShell_Parameter[1], xShell_Parameter[2]);
		break;

	case 4:
		r = f(xShell_Parameter[0], xShell_Parameter[1], xShell_Parameter[2], xShell_Parameter[3]);
		break;

	case 5:
		r = f(xShell_Parameter[0], xShell_Parameter[1], xShell_Parameter[2], xShell_Parameter[3],
				xShell_Parameter[4]);
		break;

	case 6:
		r = f(xShell_Parameter[0], xShell_Parameter[1], xShell_Parameter[2], xShell_Parameter[3],
				xShell_Parameter[4], xShell_Parameter[5]);
		break;

	case 7:
		r = f(xShell_Parameter[0], xShell_Parameter[1], xShell_Parameter[2], xShell_Parameter[3],
				xShell_Parameter[4], xShell_Parameter[5], xShell_Parameter[6]);
		break;

	case 8:
		r = f(xShell_Parameter[0], xShell_Parameter[1], xShell_Parameter[2], xShell_Parameter[3],
				xShell_Parameter[4], xShell_Parameter[5], xShell_Parameter[6], xShell_Parameter[7]);
		break;

	case 9:
		r = f(xShell_Parameter[0], xShell_Parameter[1], xShell_Parameter[2], xShell_Parameter[3],
				xShell_Parameter[4], xShell_Parameter[5], xShell_Parameter[6], xShell_Parameter[7],
				xShell_Parameter[8]);
		break;

	case 10:
		r = f(xShell_Parameter[0], xShell_Parameter[1], xShell_Parameter[2], xShell_Parameter[3],
				xShell_Parameter[4], xShell_Parameter[5], xShell_Parameter[6], xShell_Parameter[7],
				xShell_Parameter[8], xShell_Parameter[9]);
		break;
	}
	return r;
}
static unsigned long xShell_CallVar(xShell_Recorde_st*pRecord)
{
	unsigned long temp = 0;
	char *pData = NULL;
	int i ;
	
	if(pRecord == NULL)
		return (unsigned long)-1;
		
	pData = (char*)pRecord->addr;
	i = pRecord->nByteOfValue;
	switch(i)
	{
		case 1: temp = (unsigned long)(*pData);break;
		case 2: temp = (unsigned long)(*((unsigned short*)pData));break;
		case 4: 
		default:temp = *((unsigned int*)pData);break;
	}
	return temp;
}

static unsigned int xShell_Pow(unsigned int m,unsigned int n)
{
	unsigned int Result=1;	 
	while(n--)
		Result*=m;    
	return Result;
}

static unsigned int xShell_Str2Num(char*Str,unsigned long *Res)
{
	unsigned int Temp;
	unsigned int Num=0;			  
	unsigned int HexDec=10;
	char *p;
	p=Str;
	*Res=0;
	while(1)
	{//convert BIG chars to small chars "ABCDEF"//16??
		if((*p>='A'&&*p<='F')||(*p=='X'))
			*p=*p+0x20;
		else if(*p=='\0')break;
		p++;
	}
	p=Str;
	while(1)
	{
		if((*p<='9'&&*p>='0')||(*p<='f'&&*p>='a')||(*p=='x'&&Num==1))
		{//
			if(*p>='a')HexDec=16;	
			Num++;					
		}else if(*p=='\0')break;	
		else return 1;				
		p++; 
	} 
	p=Str;			    
	if(HexDec==16)		
	{
		if(Num<3)return 2;		
		if(*p=='0' && (*(p+1)=='x'))//
		{
			p+=2;	
			Num-=2;
		}else return 3;
	}else if(Num==0)return 4;
	while(1)
	{
		if(Num)Num--;
		if(*p<='9'&&*p>='0')Temp=*p-'0';	
		else Temp=*p-'a'+10;				 
		*Res+=Temp*xShell_Pow(HexDec,Num);		   
		p++;
		if(*p=='\0')break;
	}
	return 0;
}

static char *xShell_delete_space(char * pStr)
{
	//delete space at begin of pStr and end of pStr
	char *p = pStr;
	char *pStart = NULL;
	
	while(*p == ' ')p++;
	pStart = p;
	
	p = pStr;
	p += strlen(pStr)-1;
	while(*p == ' ') *p-- = '\0';
	
	return pStart;
}

static char xShell_IslegalChar(char c)
{
	if(c>=32&&c<=126)
		return 1;
	else
		return 0;	
}
static char xShell_IslegalID(char c)
{
	if((c>='a'&&c<='z') || (c>='A'&&c<='Z') || c=='_')
		return 1;
	else
		return 0;	
}

static xShell_err_t xShell_ParameterToNum(char *pStr,unsigned long *out)
{
	xShell_Recorde_st* pRecord = NULL;
	int len;

	if(*pStr == 0)
		return xShell_EPARA;

	if(xShell_IslegalID(*pStr))
	{//it is must be a varible
		pRecord = xShell_SearchVar(pStr);
		if( pRecord == NULL)
			return xShell_EPARA;
		else
		{
			*out = xShell_CallVar(pRecord);
		}
	}
	else if(*pStr == '\"')
	{
		len = strlen(pStr)-1;
		if(pStr[len] != '\"' || len==0)
			return xShell_ESTR;
		pStr[len] = '\0';
		*out = (unsigned long)(pStr+1);
	}
	else
	{
		if(xShell_Str2Num(pStr,out)!=0)
			return xShell_EPARA;
	}
	return xShell_EOK;
}

static xShell_err_t xShell_token(xShell_st *xShell)
{
//	xShell_err_t err_code = xShell_EOK;
	char *pIstr;
	unsigned int Index,tempIndex;
	unsigned int len;
//1. delete all ' 'before any string
	pIstr = xShell_delete_space(xShell->line);
	xShell->pLine = pIstr;
	Index = 0;
	if(*pIstr == '\0')
		return xShell_EOK;
//2. the first character should be "a-z","A-z",'_'
	if(!(
		(pIstr[Index]>='a'&&pIstr[Index]<='z') \
		|| (pIstr[Index]>='A'&&pIstr[Index]<='Z') \
		|| pIstr[Index]=='_')
		)
		return xShell_ESTR;
//3.seperate all token
	Index = 0;
	tempIndex = 0;
	xShell->token_count = 0;
	while(1)
	{
		if(pIstr[Index] == '\0')
		{
			//it is not a funtion or string or const,it must be a varible
			if(xShell->token_count == 0)
			{
				xShell->token[xShell->token_count].token_type = xShell_TokenVar;
				xShell->token[xShell->token_count].token_index = 0;
				xShell->token[xShell->token_count].token_end = Index-1;
				xShell->token_count++;
			}
			break;
		}
		if(xShell_IslegalChar(pIstr[Index]) == 0)
			return xShell_ESTR;
		if(pIstr[Index] == '(')
		{
			if(xShell->token_count != 1)//there must "be" "one" function name before,
			{
				xShell->token[xShell->token_count].token_type = xShell_TokenFunc;
				xShell->token[xShell->token_count].token_index = 0;
				xShell->token[xShell->token_count].token_end = Index-1;
				xShell->token_count++;
				tempIndex = Index+1;
				//check if it is really a function call
				//check ')'
				len = strlen(pIstr);
				if(pIstr[len-1] != ')')
					return xShell_EFUNC;//function not complte
				//check if parameter is enough
				//xShell->call_type = xShell_CallFunction;
			}
			else
				return xShell_EFUNC;
		}
		if(pIstr[Index] == ')')
		{
			//')' only can be showed up in funciton call
			//the first token must be TokenFunc
			if(xShell->token[0].token_type != xShell_TokenFunc)
				return xShell_ESTR;
			if(tempIndex!=Index)//there maybe some parameter,regard ' 'as parameter too
			{
				xShell->token[xShell->token_count].token_type = xShell_TokenPara;
				xShell->token[xShell->token_count].token_index = tempIndex;
				xShell->token[xShell->token_count].token_end = Index-1;
				xShell->token_count++;
			}
			//else "()",function has no parameter
			else
			{
				if(pIstr[tempIndex-1]!='(')
					return xShell_EPARA;
			}
		}
		if(pIstr[Index] == ',')
		{
			//',' only can be showed up in funciton call
			//the first token must be TokenFunc
			if(xShell->token[0].token_type != xShell_TokenFunc)
				return xShell_ESTR;
			if(xShell->token_count > MAX_TOKEN_NUM-1)
				return xShell_ETOKEN_FULL;
			if(tempIndex!=Index)//there maybe some parameter,regard ' 'as parameter too
			{
				xShell->token[xShell->token_count].token_type = xShell_TokenPara;
				xShell->token[xShell->token_count].token_index = tempIndex;
				xShell->token[xShell->token_count].token_end = Index-1;
				xShell->token_count++;
				tempIndex = Index+1;
			}
			else
				return xShell_EPARA;
		}
		Index++;	
	}
	return xShell_EOK;	
}

static xShell_err_t xShell_paser(xShell_st* xShell)
{
	xShell_err_t err_code;
	int i;
	char *pToken = NULL;
	xShell_Recorde_st *pRecord=NULL;
#ifdef xShell_EN_ECHO
	unsigned long oValue;
	xShell_printf("\n");
#endif	
	err_code = xShell_token(xShell);
	if(err_code!=xShell_EOK)
		return err_code;
	if(*xShell->pLine == '\0')
		return xShell_EOK;//input is only "   "
//delete all space not needed
	for(i=0;i<xShell->token_count;i++)
	{
		xShell->pLine[xShell->token[i].token_end+1] = '\0';//cut the string
		pToken = xShell->pLine+xShell->token[i].token_index;
		xShell->token[i].token_index = xShell_delete_space(pToken)\
											-xShell->pLine;
		pToken = xShell->pLine+xShell->token[i].token_index;
		xShell->token[i].token_end = xShell->token[i].token_index + strlen(pToken) - 1;
		
		if(xShell->token[i].token_index<xShell->token[i].token_end)
			xShell->token[i].token_end = xShell->token[i].token_index;
	}	
//check function or variable or equition,equation has not completed
	if(xShell->token[0].token_type == 	xShell_TokenFunc)
	{
		pToken = xShell->pLine+xShell->token[0].token_index;
		pRecord = xShell_SearchFunc(pToken);
		if(pRecord == 0)
			return  xShell_ENULLNODE;
		for(i=1;i<xShell->token_count;i++)
		{
			if(xShell_ParameterToNum(xShell->pLine+xShell->token[i].token_index,xShell_Parameter+i-1) != xShell_EOK)
				return xShell_EPARA;
		}
		//call function
#ifdef xShell_EN_ECHO
		oValue = xShell_CallFunc(pRecord,xShell->token_count-1);
		xShell_printf("\nr:%8ld,a:0x%08x",oValue,(unsigned int)pRecord->addr);
#else
		xShell_CallFunc(pRecord,xShell->token_count-1);
#endif		
	}
	else if(xShell->token[0].token_type == 	xShell_TokenVar)
	{
		pToken = xShell->pLine+xShell->token[0].token_index;
		pRecord = xShell_Search(pToken);
		if(pRecord == NULL)
			return xShell_EUNKOWNSYMBOL; 
#ifdef xShell_EN_ECHO
		oValue = xShell_CallVar(pRecord)	;
		xShell_printf("\nr:%8ld,a:0x%08x",oValue,(unsigned int)pRecord->addr);
#else
		xShell_CallVar(pRecord)	;
#endif
	}	
	return xShell_EOK;
}
static void xShell_PrintErro(xShell_err_t err_code)
{
	switch(err_code)
	{
		case xShell_ESTR:
			xShell_printf("String Erro");
			break;
		case xShell_EFUNC:
			xShell_printf("Function Erro");
			break;
		case xShell_EPARA:
			xShell_printf("Parameter erro");
			break;
		case xShell_ETOKEN_FULL:
			xShell_printf("Token Full");
			break;
		case xShell_ENULLNODE:
			xShell_printf("Null Node");
			break;
		case xShell_EUNKOWNSYMBOL:
			xShell_printf("Unkown Symbol");
			break;
		default:;
	}
}

void xShell_InputChar(xShell_st *xShell,char c)
{
	xShell_err_t err_code = xShell_EOK;
	/* handle CR key */
	if (c == '\r')
	{
		c = '\r';
	}
	/* handle tab key */
	else if (c == '\t')
	{
		/* auto complete */
	//		finsh_auto_complete(&shell->line[0]);
		/* re-calculate position */
	//		shell->line_position = strlen(shell->line);
//			continue;
		return;
	}
	/* handle backspace key */
	else if (c == 0x7f || c == 0x08)
	{
		if (xShell->line_position != 0)
		{
			xShell_printf("%c %c", c, c);
		}
		if (xShell->line_position <= 0) xShell->line_position = 0;
		else xShell->line_position --;
		xShell->line[xShell->line_position] = 0;
		return;
	}

	/* handle end of line, break */
	if (c == '\r' || c == '\n')
	{
		if (xShell->line_position != 0) 
		{
			xShell->line[xShell->line_position] = '\0';
			err_code = xShell_paser(xShell); 
			xShell_PrintErro(err_code);
			
			memset(xShell, 0, sizeof(xShell_st));
		}
#ifdef xShell_EN_ECHO
		xShell_printf(xShell_PROMT);
#endif
		return;
	}
	/* normal character */
	xShell->line[xShell->line_position] = c; c = 0;
#ifdef xShell_EN_ECHO
	xShell_printf("%c", xShell->line[xShell->line_position]);
#endif
	xShell->line_position ++;
	
	return;
}

void list(void)
{
	char flag = 0;
	xShell_Recorde_st *pRecord = _xShell_Record_Base;
	
	for(;pRecord<_xShell_Record_Limit;pRecord++)
	{
		if(pRecord->nByteOfValue!=0)//function ,this field must be zero
		{
			continue;
		}
		else
		{
			if(flag == 0)
			{
				flag = 1;
				xShell_printf("functions:\n");
			}
			xShell_printf(pRecord->name);
			xShell_printf("()\t--");
			xShell_printf(pRecord->description);
			xShell_printf("\n");
		}
	}
	
	flag = 0;
	pRecord = _xShell_Record_Base;
	for(;pRecord<_xShell_Record_Limit;pRecord++)
	{
		if(pRecord->nByteOfValue!=0)//function ,this field must be zero
		{
			if(flag == 0)
			{
				flag = 1;
				xShell_printf("variables:\n");
			}
			xShell_printf(pRecord->name);
			xShell_printf("\t--");
			xShell_printf(pRecord->description);
			xShell_printf("\n");
		}
		else
		{
			continue;
		}
	}
}
xShell_FUN_REG(list,list the functions and variables);
void hello(void)
{
	#ifdef ENABLE_XSHELL_PRINTF
	xShell_printf("\nHello from xshell\n");
	xShell_printf("version 0.2 2015.3.25@XJTU,Xi'an,Shannxi\n");
	xShell_printf("type \"list()\"to list all functions\n");
	#endif
}
xShell_FUN_REG(hello,say hello);
#else
void xShell_InputChar(xShell_st *xShell,char c)
{
	
}
#endif
