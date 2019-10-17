/**
 * @author Neo Xu (neo.xu1990@gmail.com)
 * @license The MIT License (MIT)
 * 
 * Copyright (c) 2019 Neo Xu
 * Port from rt-thread ulog to use it without rt-thread. License attached below.
*/
/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-04     armink       the first version
 */

//#include <rthw.h>
#include <ulog.h>

#ifdef ULOG_BACKEND_USING_CONSOLE

#if defined(ULOG_ASYNC_OUTPUT_BY_THREAD) && ULOG_ASYNC_OUTPUT_THREAD_STACK < 384
#error "The thread stack size must more than 384 when using async output by thread (ULOG_ASYNC_OUTPUT_BY_THREAD)"
#endif

static struct ulog_backend console;

#include "printf.h"
void ulog_console_backend_output(struct ulog_backend *backend, uint32_t level, const char *tag, bool is_raw,
        const char *log, size_t len)
{
  printf("%s", log);
}

int ulog_console_backend_init(void)
{
    console.output = ulog_console_backend_output;

    ulog_backend_register(&console, "console", true);

    return 0;
}

#endif /* ULOG_BACKEND_USING_CONSOLE */
