/**
* @brief akfs_otp_authentiaction 
* @return 
*   -EEXIST 表示该cmd不是otp相关操作
*       调用方          被调用方
*1.     发起otp请求     检查当前状态是否为REQ状态
*                       如果是REQ状态，则生成随机数返回
*                       状态设置为REPLY状态
*2.     收到随机数
*       使用随机数调用otp接口，计算出校验值
*       计算出应答值
*3.     发起otp应答请求
*       携带随机数和校验值
*4.                     校验应答请求
*                       校验成功状态转为ACCESS状态
*                       校验失败状态转为REQ状态    
*/

#include "otp.h"

static int akfs_otp_seed(void);
static void akfs_otp_calc(akfs_otp_t *g);
static int akfs_otp_req(akfs_otp_t *g ,akfs_otp_t *u ,void __user*);
static int akfs_otp_reply(akfs_otp_t *g ,akfs_otp_t *u ,void __user*);

/**
 * @brief akfs_otp_authentiaction 
 *   otp认证接口
 */
int akfs_otp_authentiaction(akfs_otp_t *__uotp ,unsigned long args)
{
    int ret = 0;
    akfs_otp_t user_otp = {0};

    //验证通过之后不需要再次验证
    assert_error(__uotp->status != AKFS_FSA_ACCESS ,-EACCES);

    assert_error(!copy_from_user(&user_otp ,(void __user *)args ,sizeof(akfs_otp_t)) ,-EACCES);

    switch(__uotp->status)
    {
        case AKFS_FSA_REQ:
            {
                ret = akfs_otp_req(__uotp ,&user_otp ,(void __user*)args);
                break;
            }
        case AKFS_FSA_REPLY:
            {
                ret = akfs_otp_reply(__uotp ,&user_otp,(void __user *)args);
                break;
            }
        default:
            {
                return -EACCES;
            }
    }

    return ret;
}

/**
 * @brief akfs_otp_seed 
 *  otp seed生成接口
 *  可根据需求调整
 */
static int akfs_otp_seed(void)
{
    return jiffies;
}

/**
 * @brief akfs_otp_calc 
 *   otp计算接口
 *  可根据需求调整
 *  需要和用户层保持一致
 */
static void akfs_otp_calc(akfs_otp_t *g)
{
    g->seed++;
}

/**
 * @brief akfs_otp_req 
 *   处理otp req状态
 */
static int akfs_otp_req(akfs_otp_t *g ,akfs_otp_t *u ,void *__user args)
{
    assert_error(u->status == AKFS_FSA_REQ ,-EACCES);

    //生成随机数 设置下一阶段状态
    g->seed = akfs_otp_seed();    
    g->status = AKFS_FSA_REPLY;

    //返回给用户层
    //copy_to_user失败则重新REQ
    assert_void(!copy_to_user(args ,g ,sizeof(akfs_otp_t)) ,
            g->status = AKFS_FSA_REQ; return -EACCES;);

    //内核进行reply阶段的计算
    akfs_otp_calc(g);

    return 0;
}

/**
 * @brief akfs_otp_req 
 *   处理otp reply状态
 */
static int akfs_otp_reply(akfs_otp_t *g ,akfs_otp_t *u ,void *__user args)
{
    //校验 状态和计算的值一致
    assert_error((g->status == u->status) && (g->seed == u->seed) ,-EACCES);

    g->status = AKFS_FSA_ACCESS;

    return 0;
}
