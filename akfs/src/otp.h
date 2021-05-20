#ifndef __ALWAYSKNOW_OTP_H__
#define __ALWAYSKNOW_OTP_H__

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include "ioctl.h"
#include "assertion.h"

/**
 * @brief otp ioctl指令
 */
#define AKFS_IOCTL_OTP _IO(AKFS_IOCTL_MAGIC ,20)

/**
 * @brief otp交互状态 
 */
enum AKFS_FSA_S{
    AKFS_FSA_WAIT    = 1,
    AKFS_FSA_REQ     = 2,
    AKFS_FSA_REPLY   = 4,
    AKFS_FSA_ACCESS  = 8,
};

/**
 * @brief 
 *  otp相关
 */
typedef struct akfs_otp_s{
    unsigned int status;
    unsigned int seed;
}akfs_otp_t;

int akfs_otp_authentiaction(akfs_otp_t *otp ,unsigned long args);

#endif
