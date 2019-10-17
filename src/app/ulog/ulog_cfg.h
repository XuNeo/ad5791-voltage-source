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

#define ULOG_OUTPUT_TIME
#define ULOG_TIME_USING_TIMESTAMP
//#define ULOG_USING_ASYNC_OUTPUT  this mode is removed. This port targets for no-os usage. Thus it makes no sense to port async mode.
//#define ULOG_USE_USH //use micro shell: USH
//ULOG_OUTPUT_TIME
//RT_USING_SOFT_RTC

#define ULOG_USING_FILTER
#define ULOG_OUTPUT_LEVEL
#define ULOG_USING_ISR_LOG
#define ULOG_OUTPUT_TAG
#define ULOG_USING_COLOR
#define ULOG_OUTPUT_FLOAT
#define ULOG_BACKEND_USING_CONSOLE

#define RT_NAME_MAX					(8)

#define ULOG_GET_SYSTICK()  0 //rt_tick_get()
#endif
