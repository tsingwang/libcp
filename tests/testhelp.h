/* This is a really minimal testing framework for C.
 *
 * Example:
 *
 * test_cond("Check if 1 == 1", 1==1)
 * test_cond("Check if 5 > 10", 5 > 10)
 * test_report()
 *
 * ----------------------------------------------------------------------------
 *
 * Copyright (c) 2010-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __TESTHELP_H
#define __TESTHELP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int __failed_tests = 0;
static int __test_num = 0;
static int __failed_test_count = 0;
static const char *__failed_test_names[256];
static int __test_color_enabled = -1;

static inline int __test_use_color(void) {
    if (__test_color_enabled == -1) {
        __test_color_enabled = isatty(STDOUT_FILENO) ? 1 : 0;
    }
    return __test_color_enabled;
}

static inline const char *__test_label_pass(void) {
    return __test_use_color() ? "\033[32mPASS\033[0m" : "PASS";
}

static inline const char *__test_label_fail(void) {
    return __test_use_color() ? "\033[31mFAIL\033[0m" : "FAIL";
}

static inline void __test_record(const char *descr, int ok) {
    __test_num++;
    printf("[%02d] %-32s %s\n", __test_num, descr, ok ? __test_label_pass() : __test_label_fail());
    if (!ok) {
        __failed_tests++;
        if (__failed_test_count < (int)(sizeof(__failed_test_names) / sizeof(__failed_test_names[0]))) {
            __failed_test_names[__failed_test_count++] = descr;
        }
    }
}

static inline void __test_finish(void) {
    if (__failed_tests == 0) {
        printf("\n%s %d/%d tests\n", __test_label_pass(), __test_num, __test_num);
        return;
    }

    printf("\n%s %d/%d tests\n", __test_label_fail(), __test_num - __failed_tests, __test_num);
    printf("Failed tests:\n");
    for (int i = 0; i < __failed_test_count; i++) {
        printf("- %s\n", __failed_test_names[i]);
    }
    exit(1);
}

#define test_cond(descr,_c) do { \
    __test_record((descr), (_c)); \
} while(0)

#define test_report() do { \
    __test_finish(); \
} while(0)

#endif
