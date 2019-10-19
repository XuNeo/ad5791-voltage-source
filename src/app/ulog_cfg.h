/**
 * @author Neo Xu (neo.xu1990@gmail.com)
 * @license The MIT License (MIT)
 * 
 * Copyright (c) 2019 Neo Xu
 * Port from rt-thread ulog to use it without rt-thread. License attached below.
*/

#ifndef _ULOG_CFG_H_
#define _ULOG_CFG_H_

#define RT_USING_ULOG

#define ULOG_OUTPUT_LVL LOG_LVL_DBG

#define ULOG_USING_USH //use micro shell: USH
#define ULOG_USING_FILTER
#define ULOG_USING_COLOR
#define ULOG_USING_PRINTF //use custom printf module instead of stdio.h

#define ULOG_OUTPUT_LEVEL
#define ULOG_OUTPUT_TAG
#define ULOG_OUTPUT_TIME

#define ULOG_BACKEND_USING_CONSOLE

#define RT_NAME_MAX					(8)

//#define ULOG_USING_ISR_LOG Not supported
//#define ULOG_USING_SYSLOG not supported
//#define ULOG_USING_ASYNC_OUTPUT  this mode is removed. This port targets for no-os usage. Thus it makes no sense to port async mode.
#endif
