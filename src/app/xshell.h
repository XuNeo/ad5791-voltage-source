/************xShell*******************************************************
* used on stm32f103ze,compiler:arm_cc;not support IAR
*
* ver 0.1,2013-12-22,by ������ in xjtu,xi'an,shanxi,CHINA
**************************************************************************
* ver 0.2,2015-3-25,by ������ in xjtu,xi'an,shanxi,CHINA,
*    made it more easier to use
**************************************************************************
* how to use it:
* 1. set flag USE_XSHELL to 1,
*	   you can simple turn it off by set it to 0
* 2. add "--keep *.o(xShellTab)" to misc control in linker page(without "") 
* 3. complete function xShell_printf(const char*str,...),
*    if you dont want any output, just write a empty function
*    you can use this to simply it
*    #define xShell_print printf
*    this is much easier to realize the printf function
* 4. register your function use the macro xShell_FUN_REG(name, desc)
* 5. register your variable use the macro xShell_VAR_REG(name, desc)
*    the variable can be list to the terminal
* 6. when you received a character, execute 
*    xShell_InputChar(xShell_st *xShell,char c);
*    fistly, you should first define the struct (xShell),this is
*    the workspace of xshell, pass its pointer to this function, 
*    parameter 'c' is the charater you received
* 7. enjoy it.
*
****************************************************************************/
#ifndef _XSHELL_H_
#define _XSHELL_H_

/*user can modify this*/
#define USE_XSHELL 1
#define ENABLE_XSHELL_PRINTF 1
#define xShell_EN_ECHO 


/*user should not modify below this line*/
/****************************************/
#define MAX_TOKEN_NUM 10
#define MAX_PARAMETER 10
#define xSHELL_MAX_CHARS 128
#ifdef xShell_EN_ECHO 
#define xShell_PROMT "\nxShell>>"
#endif
typedef enum 
{
	xShell_EOK = 0,
	xShell_ESTR = 1,
	xShell_EFUNC,
	xShell_EPARA,
	xShell_ETOKEN_FULL,
	xShell_EUNKOWNSYMBOL,
	xShell_ENULLNODE,
}xShell_err_t;

typedef enum 
{
	xShell_TokenUnknown = 0,
	xShell_TokenFunc ,
	xShell_TokenPara,//function's parameter
	xShell_TokenVar,
	xShell_TokenStr,
	xShell_TokenConst,
}xShell_token_type;

typedef struct
{
	xShell_token_type token_type;
	unsigned char token_index;
	unsigned char token_end;
}xShell_token_st;

typedef struct
{
	const char *name;
	const char *description;
	void *addr;//the address of item;
	unsigned int nByteOfValue;//how much byte the value is;as for function ,this field must be zero
}xShell_Recorde_st;

typedef struct
{
	char  *pLine;	
	char line[xSHELL_MAX_CHARS];
	int line_position;
	xShell_token_st token[MAX_TOKEN_NUM];
	unsigned char token_count;
}xShell_st;

typedef unsigned long (*xShell_Func)();


#define xShell_FUN_REG(name, desc)					                                \
static const   char  xShell_fun_##name##_name[]  = #name;				            \
static const   char  xShell_fun_##name##_desc[]  = #desc;						    \
xShell_Recorde_st qsh_fun_##name##_record  __attribute__((section(".xShellTab"))) =  \
{							                                                    \
	xShell_fun_##name##_name,	                                                    \
	xShell_fun_##name##_desc,	                                                    \
	(void *)&name,		                                                        \
	0                                                                           \
}

#define xShell_VAR_REG(name, desc)					                   \
static const   char  xShell_var_##name##_name[] = #name;				           \
static const   char  xShell_var_##name##_desc[] = #desc;				           \
xShell_Recorde_st xShell_var_##name##_record  __attribute__((section(".xShellTab"))) = \
{							                                                   \
	xShell_var_##name##_name,	                                                   \
	xShell_var_##name##_desc,	                                                   \
	(void *)&name,		                                                       \
	sizeof(name)											       \
}

//use it in RTT 
void xShell_InputChar(xShell_st *xShell,char c);
#endif
