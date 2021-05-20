#ifndef __ALWAYSKNOW_LDS_H__
#define __ALWAYSKNOW_LDS_H__

#include "akfs.h"

/**
 * @brief 
 *   hook相关宏操作
 */
#define akfs_trace_declare(t)   \
    static typeof(t) __akfs_trace_##t; \
    akfs_trace_t \
    __attribute__((unused))                     \
    __attribute__((aligned(8)))                 \
    __attribute__((section(".hook.data")))             \
    __akfs_trace_var_##t = {    \
        .name = #t, \
        .function = __akfs_trace_##t, \
    };

#define akfs_trace_function(t) \
    __akfs_trace_##t

#define akfs_trace_get_original(t)    \
    __akfs_trace_var_##t.original

#define akfs_trace_get_variable(t) \
    __akfs_trace_var_##t

/**
 * @brief 
 *   channel段相关宏操作
 */
#define akfs_channel_declare(t)   \
    akfs_channel_t \
        __attribute__((unused))                     \
        __attribute__((aligned(1)))                 \
        __attribute__((section(".channel.data")))             \
        __akfs_channel_var_##t = {    \
            .name = #t, \
        };

#define akfs_channel_get_variable(t)    \
    __akfs_channel_var_##t

#endif
