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

#ifndef _USH_H_
#define _USH_H_
#include "stdint.h"

#define USH_MAX_ARG 128
#define USH_ECHO "ush>>"

#include "printf.h"
#define USH_Print printf  /**< define the method to output message. */

#define USH_VER_NUMBER 0x010 //0.1.0

typedef enum{
  ush_error_ok = 0,         /**< all ok */
  ush_error_nullp = -1,     /**< null pointer */
  ush_error_nocmd = -2,     /**< no valid command found */
  ush_error_maxarg = -3,    /**< reached to maximum arguments allowed */
  ush_error_buffsz = -4,    /**< line buffer size is limted. */
  ush_error_str = -5,       /**< the string parameter is incompleted. */
  ush_error_nullstr = -6,   /**< input string is null. */
  ush_error_strillegal = -7,/**<illegal input string for number  */
}ush_error_def;

typedef int32_t (*ush_func_def)(int32_t argc, uint8_t ** argv);

typedef struct _ush_cmd{
  void *func;               /**< the function to deal with this command. */
  const uint8_t *cmd;          /**< the command string that calls this function. */
  const uint8_t *desc;         /**< description of this command. */
}ush_cmd_def;

typedef struct _ush_list{
  struct _ush_list *next;
  const ush_cmd_def *const item;
}ush_list_def;

#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define uniquename(func) CONCAT(func, __LINE__)

#define USH_REGISTER(func, name, desc) \
const ush_cmd_def uniquename(func)  __attribute__((section("ushtable"))) = \
{ \
	(void *)&func, \
  (uint8_t*)#name,	\
	(uint8_t*)#desc, \
}

typedef enum{
  ush_state_normal = 0,
  ush_state_in_str,   /**< ush is dealing with input string. */
  ush_state_escaping, /**< there is a '\' input in a string. */
}ush_state_def;

typedef enum{
  ush_num_uint32 = 0,  /**< number is unsigned */
  ush_num_int32,
  ush_num_float,
}ush_num_def;

typedef struct _ush{
  uint8_t *linebuff;  /**< input line buffer*/
  uint32_t buffsz;    /**< line buffer size. */
  uint32_t w_pos;     /**< current position in line buffer to write. */
  uint32_t argc;      /**< the arguements found in linebuffer till now.*/
  uint8_t  *argv[USH_MAX_ARG];  /**< pointers point to the arugments. */
  ush_state_def state;
}ush_def;

ush_error_def ush_init(ush_def *ush, uint8_t *pbuff, uint32_t len);
ush_error_def ush_cmdlist_append(ush_list_def *pitem);
ush_error_def ush_process_input(ush_def *ush, const uint8_t *pbuff, uint32_t len);
ush_error_def ush_str2num(const uint8_t *pstr, uint32_t len, ush_num_def* num_type, void *value);

#endif
