/**
 * @author Neo Xu (neo.xu1990@gmail.com)
 * @license The MIT License (MIT)
 * 
 * Copyright (c) 2019 Neo Xu
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * @brief ush - micro-shell is a shell for embedded devices.
*/
#include "ush.h"

/**
 * The commands that defined by macro USH_REGISTER are stored in 'ushsection' section.
 * The commands that registered by function ush_cmd_append() is listed to list ush_list;
 * */
#ifdef __CC_ARM
extern int ushtable$$Base;	
extern int ushtable$$Limit;
#define _USH_TABLE_START      (ush_cmd_def *)&ushtable$$Base
#define _USH_TABLE_END        (ush_cmd_def *)&ushtable$$Limit
#elif defined(__GNUC__)
extern const uint32_t __start_ushsection;
extern const uint32_t __end_ushsection;
#define _USH_TABLE_START      (ush_cmd_def *)&__start_ushsection
#define _USH_TABLE_END        (ush_cmd_def *)&__end_ushsection
#endif

static ush_list_def *ush_list = {0};  /**< the ush command list added dynamically */ 

/**
 * @brief if the character is mark of end-of-line
*/
#define _ush_is_endofline(ch) ((ch == '\r') || (ch =='\n'))

/**
 * @brief if the character is a seperator
*/
#define _ush_is_seperator(ch) ((ch == ' ') || (ch =='\t'))

/**
 * @brief compare if the cmd is equal to the input string.
 * @return return 1 if the two string are same.
*/
static inline uint8_t _ush_is_str_same(const uint8_t *cmd, const uint8_t *str){
  while(*cmd && *str){
    if(*cmd != *str)
      return 0;
    cmd++; str++;
  }
  if((*cmd == 0) && (*str == 0))
    return 1; //the string has same length.
  return 0;
}

/**
 * @brief find a command from build-in table and dynamic list.
 * @param cmd: the command string.
 * @return return the function pointer if found, otherwise, return 0.
*/
static inline ush_func_def ush_find_cmd(uint8_t const *cmd){
  ush_cmd_def *pcmd;
  ush_list_def *plist;
  plist = ush_list; //find it in the list.
  while(plist){
    if(_ush_is_str_same(cmd, plist->item->cmd))
      return (ush_func_def)plist->item->func;
    plist = plist->next;
  }
  for(pcmd = _USH_TABLE_START;pcmd<_USH_TABLE_END;pcmd++){
    if(_ush_is_str_same(cmd, pcmd->cmd))
      return (ush_func_def)pcmd->func;
  }
  return 0;
}

/**
 * @brief Parse the input line buffer and split the input to argv.
 * @note: character '\r' or '\n' is end mark of valid input command. 
 *        character ' ' or '\t' is the sperator of parameters. 
 *        '\0' is regarded as a normall input.
 *        If pairs of character '\"' show up in parameter, the content between it them
 *        is regarded as one whole parameter, no matter what's inside of it. Example:
 *        [echo "hello world!""Nice to meet you" \n] is decoded to :
 *        param1: [echo]
 *        param2: [hello world!], note chareter \" is removed.
 *        param3: [Nice to meet you]
 *        All other characters are regarded as valid parameters.
 * @param pline: pointer to the line input buffer.
 * @param buff_len: line buffer size, the parser will stop if it reaches end of buffer.
 * @param pargc: pointer to store how many arguments are found.
 * @param argv: buffer used to store all pointers to parameters.
*/
static inline ush_error_def ush_parser(uint8_t *pline, uint32_t buff_len, uint32_t *pargc, uint8_t *argv[USH_MAX_ARG]){
  ush_error_def err = ush_error_ok;
  uint32_t argc;
	uint8_t flag_found_quotation = 0;
  uint8_t flag_escaping = 0;
  if(pline == 0) return ush_error_nullp;
  if(pargc == 0) return ush_error_nullp;
  if(argv == 0) return ush_error_nullp;
  if(buff_len == 0) return ush_error_nocmd;
  argc = 0;  //init it to zero.
  while(*pline){
    if(argc == USH_MAX_ARG){
      err = ush_error_maxarg;
      break;
    }
    //skip existing seperators
    while(*pline && buff_len){
      if(!_ush_is_seperator(*pline)) break;
      *pline = '\0';  //make is invalid.
      buff_len--;
      pline++;
    }
    //are we reaching to end of input?
    if(buff_len == 0)  break;
    if(*pline == '\"'){ //we are meeting a possible string.
      *pline = 0; /* Make it as 0 */
      uint8_t *pstr_start = ++pline;  //start of string.
      //do we have another valid '\""'? otherwise, the parameter is not completed.
      while(*pline && buff_len){
        if(flag_escaping)
          flag_escaping = 0;
        else if(*pline == '\"'){//great, we have found one pair of \"
            flag_found_quotation = 1;
            *pline = '\0';  //mark it as end of parameter.
            if(pline == pstr_start);//this is an empty string.
            else
              argv[argc++] = pstr_start;   
            pline++; buff_len--;	//go to next one before break.
            break;
          }
        else if(*pline == '\\'){ //next string should be escaped.
          flag_escaping = 1;
        }
				pline++; buff_len--;
      }
			if(flag_found_quotation == 0){
				err = ush_error_str;
        break;  //return and report founded arguements till now.
			}
      else{
        flag_found_quotation = 0;
        continue; /* We have found a string and pointer has been moved to end of string. */
      }
    }
    else
      argv[argc++] = pline;
    //skip this parameter
    while(*pline && buff_len){
      if(_ush_is_seperator(*pline)) break;
      if(_ush_is_endofline(*pline)) break;
      if(*pline == '\"') break; /* This is the case [hello"123 4"], so here " acts like a seperator */
      buff_len--;
      pline++;
    }
  }
  *pargc = argc;
  return err;
}
/**
 * @brief process a line buffer ended with \r or \n
*/
static inline ush_error_def ush_process_line(ush_def *ush){
  ush_func_def pfunc;
  int32_t ret;
  ush_error_def error = ush_error_ok;

  error = ush_parser(ush->linebuff, ush->w_pos, &ush->argc, ush->argv);
  if(error != ush_error_ok) return error;
  if(ush->argc < 1) return ush_error_nocmd;
  pfunc = ush_find_cmd(ush->argv[0]);
  if(pfunc){
    ret = pfunc(ush->argc, ush->argv);
    USH_Print("ret \t0x%08x\n", ret);
  }
  else{
    error = ush_error_nocmd;
    USH_Print("error command not found\n");
  }
  USH_Print(USH_ECHO);
  return error;
}

/**
 * @brief process the input data, execute the command if found.
 * @param pline: the buffer for input data.
 * @param len: length of the linebuffer.
 * @return return ush_error_ok if succeeded.
*/
ush_error_def ush_process_input(ush_def *ush, const uint8_t *pbuff, uint32_t len){
  ush_error_def err = ush_error_ok;
  uint8_t ch;
  if(ush == 0) return ush_error_nullp;
  if(pbuff == 0) return ush_error_nullp;
  while(len--){ /* copy all input to line buffer, and check if there is end-of-line */
    ch = *pbuff++;
    ush->linebuff[ush->w_pos] = ch;
    if(ush->state == ush_state_in_str){
      ush->w_pos ++;  //store it and move on.
      if(ch == '\\'){
        ush->state = ush_state_escaping;  //next input should be escaped, no matter what it is.
      }
      else if(ch == '\"'){  //end of string.
        ush->state = ush_state_normal;
      }
    }
    else if(ush->state == ush_state_escaping){
      /**
       *  No matter what charecter this one is, it's regarded as normal input.
       *  Specially for string \", this " is not regarded as end of string.
       */
      ush->w_pos++;//move on to next one.
      ush->state = ush_state_in_str;  //go back to string input mode.
    }
    else{//this is a normal input
      if(_ush_is_endofline(ch)){//input is completed
          ush->linebuff[ush->w_pos] = '\0';
          err = ush_process_line(ush);  
          //we don't check the return value, as we cannot do anything to it.
          ush->w_pos = 0; //rewind to start of line buffer.
          continue;
      }
      else if(ch == '\"'){  //the input is a string.
        ush->state = ush_state_in_str;
      }
      ush->w_pos++;  //this is just a normal input.
    }
    if(ush->w_pos == ush->buffsz){//the input is too long. we reset and return error.
      ush->w_pos = 0;
      ush->state = ush_state_normal;
    }
  }
  return err;
}

/**
 * @brief add a new command to list dynamically.
 * @param pitem: the list element
 * @return return ush_erro_ok.
*/
ush_error_def ush_cmdlist_append(ush_list_def *pitem){
  if(ush_list) //there is item in the list.
    pitem->next = ush_list;
  ush_list = pitem;
  return ush_error_ok;
}
/**
 * @brief init ush with provided buffer to store line input
 * @param pbuff: the line buffer.
 * @param len: length of the buffer.
 * @return return ush_erro_ok.
*/
ush_error_def ush_init(ush_def *ush, uint8_t *pbuff, uint32_t len){
  if(ush == 0) return ush_error_nullp;
  if(pbuff == 0) return ush_error_nullp;
  if(len == 0) return ush_error_buffsz;
  ush->argc = 0;
  ush->buffsz = len;
  ush->linebuff = pbuff;
  ush->state = ush_state_normal;
  ush->w_pos = 0;
  return ush_error_ok;
}

/**
 * @brief try to tanslate a string to number. double is not supported.
 * @param pstr: the input string.
 * @param len: length of the input string.
 * @param value: pointer to a 32bit address to store data.
 * @return return ush_error_ok if no erros in string.
*/
ush_error_def ush_str2num(const uint8_t *pstr, uint32_t len, ush_num_def* num_type, void *value){
  uint32_t num_base; /* base is either binary[0b] or oct[0] or hex[0x] or dec. */
  double fvalue;
  uint8_t point_pos;
  uint8_t flag_negative_sign = 0;
  uint8_t flag_point = 0;
  //firstly get over the seperator
  while(*pstr && len){
    if(_ush_is_seperator(*pstr)){
      pstr++;
      len --;
    }
    else
      break;
  }
  //detect sign
  if(len){
    if(*pstr == '-'){
      flag_negative_sign = 1;
      pstr ++;
      len --;
    }
  }
  else
    return ush_error_nullstr;

  //detect number base
  if(len){
    if(pstr[0] == '0'){// we need at least 2 characters for binary and hex based number.
      if(len>1){// started with 0 but has more number.
        pstr++;
        if((pstr[0] == 'b') || (pstr[0] == 'B')){//binary
          num_base = 2;
          pstr++; //goto number
        }
        else if((pstr[0] == 'x') || (pstr[0] == 'X')){//hex
          num_base = 16;
          pstr++; //goto number
        }
        else if(pstr[0] != '.') //except 0.123
          num_base = 8; //next character is number
        else
          num_base = 10;
      }
      else{
        *(uint32_t*)value = 0;
        *num_type = ush_num_uint32;
        return ush_error_ok;
      }
    }
    else
      num_base = 10;
  }
  else
    return ush_error_strillegal;  //null or illegal
  //decode string to number
  fvalue = 0;//init value to zero;
  point_pos = 0;
  while(*pstr && len){
    if(*pstr == '.'){
      if(flag_point){//already got a point
        return ush_error_strillegal;  //illegal string.
      }
      else
        flag_point = 1;
      pstr ++;
      len --;
      continue;
    }
    if(flag_point)
      point_pos++;  //got a input after point. eg: 9.1
    if(num_base == 2){
      fvalue *= 2;
      if(*pstr == '0')
        ;//pass
      else if(*pstr == '1')
        fvalue += 1;
      else
        return ush_error_strillegal;//illegal value
    }
    else if(num_base == 8){
      fvalue *= 8;
      if((*pstr >= '0') && (*pstr<='7'))
        fvalue += *pstr - '0';
      else
        return ush_error_strillegal;//illegal value
    }
    else if(num_base == 10){
      fvalue *= 10;
      if((*pstr >= '0') && (*pstr<='9'))
        fvalue += *pstr - '0';
      else
        return ush_error_strillegal;//illegal value
    }
    else{//base 16
      fvalue *= 16;
      if((*pstr >= '0') && (*pstr<='9'))
        fvalue += *pstr - '0';
      else if((*pstr >= 'a') && (*pstr<='f'))
        fvalue += *pstr - 'a' + 10;
      else if((*pstr >= 'A') && (*pstr<='F'))
        fvalue += *pstr - 'A' + 10;
      else
        return ush_error_strillegal;//illegal value
    }
    pstr ++;
    len --;
  }
  while(point_pos--)
    fvalue /= num_base;
  if(flag_point){
    if(flag_negative_sign)
      fvalue = -fvalue;
    *(float*)value = fvalue;
    *num_type = ush_num_float;
  }
  else{
    uint32_t uvalue = fvalue+0.5f;
    *num_type = ush_num_uint32;
    if(flag_negative_sign){
      *(int32_t*)value = -uvalue;
      *num_type = ush_num_int32;
    }
    else{
      *(uint32_t*)value = uvalue;
      *num_type = ush_num_int32;
    }
  }
  return ush_error_ok;
}

/**
 * @brief build-in function to say hello
*/
static int32_t ush_hello(uint32_t argc, uint8_t **argv){
  USH_Print("hello from ush \n");
  USH_Print("found argc:%d\n",argc);
  for(int i=0;i<argc;i++)
    USH_Print("argv[%d]=[%s]\n", i, argv[i]);
  return 0x12345678;
}
USH_REGISTER(ush_hello, hello, say hello and list the input args);

/**
 * @brief build-in function to print supported commands.
*/
static int32_t ush_help(uint32_t argc, uint8_t **argv){
  ush_cmd_def *pcmd;
  ush_list_def *plist = ush_list;
  USH_Print("ush version :%x.%x.%x. Usage:\n", (USH_VER_NUMBER>>8)&0xf,\
            (USH_VER_NUMBER>>4)&0xf,\
              USH_VER_NUMBER&0xf);
  for(pcmd = _USH_TABLE_START;pcmd<_USH_TABLE_END;pcmd++){
    USH_Print("%-8s --\t%s\n", pcmd->cmd, pcmd->desc);
  }
  if(ush_list)
  while(plist){
    USH_Print("%-8s --\t%s\n", plist->item->cmd, plist->item->desc);
    plist = plist->next;
  }
  return 0x87654321;
}
USH_REGISTER(ush_help, help, list the supported functions);
USH_REGISTER(ush_help, ?, list the supported functions);      /** example of registering one function with different command. */

/**
 * @brief convert the parameters to number and print it out.
*/
static int32_t ush_print_num(uint32_t argc, uint8_t **argv){
  ush_num_def num_type;
  uint32_t value;
  argc--;
  argv++;
  if(argc == 0){
    USH_Print("error need to input a number.\n");
  }
  while(argc--){
    uint32_t str_len = 0;
    for(int i=0;;i++){
      if((*argv)[i] == '\0')
        break;
      str_len ++;
    }
    if(ush_str2num(*argv, str_len, &num_type, &value) == ush_error_ok){
      USH_Print("value type is ");
      switch(num_type){
        case ush_num_uint32: USH_Print("uint32_t:%u\n",value); break;     
        case ush_num_int32 : USH_Print("int32_t:%d\n",(int32_t)value); break;    
        case ush_num_float : USH_Print("float:%f\n",*(float *)(&value)); break;
        default:USH_Print("unknown\n"); break;
      }
    }
    else
    {
      USH_Print("String[%s] is illegal\n", *argv);
    }
    argv ++;
  }
  return 0;
}
USH_REGISTER(ush_print_num, num, convert string to number);
//ush_error_def ush_str2num(const uint8_t *pstr, uint32_t len, ush_num_def* num_type, void *value){

/**
 * @brief example to add a new command to list dynamically.
*/
static int32_t ush_debug(uint32_t argc, uint8_t **argv){
  static const ush_cmd_def cmd_hello = {
      .func = ush_hello,
      .cmd = "h",
      .desc = "say hello"
  };
  static ush_list_def list_hello={
    .next = 0,
    .item = &cmd_hello,
  };  

  ush_cmdlist_append(&list_hello);
  return 0;
}
USH_REGISTER(ush_debug, debug, Enable the build-in commands);
