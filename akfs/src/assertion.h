#ifndef __ALWAYSKNOW_MACRO_ASSERT_H__
#define __ALWAYSKNOW_MACRO_ASSERT_H__

/**
 * @brief  assert_error 
 *
 * @param condition
 * @param error
 *
 * @return 
 */
#define  assert_error(condition ,error) do{ \
    if(!(condition)){   \
        return (error); \
    }   \
}while(0)

/**
 * @brief  assert_ret 
 *
 * @param !(condition)
 *
 * @return 
 */
#define  assert_ret(condition)  \
    if(!(condition)){   \
        return; \
    }   

/**
 * @brief  assert_goto 
 *
 * @param condition
 * @param label
 * @param !(condition
 *
 * @return 
 */
#define  assert_goto(condition ,label ,ops) \
    if(!(condition)){   \
        ops;    \
        goto label; \
    }

#define  assert_break(condition ,ops) \
    if(!(condition)){   \
        ops;    \
        break; \
    }

#define  assert_continue(condition ,ops) \
    if(!(condition)){   \
        ops;    \
        continue; \
    }
/**
 * @brief  assert_void 
 *
 * @param condition
 * @param ops
 *
 * @return 
 */
#define  assert_void(condition ,ops)    do{ \
    if(!(condition)){   \
        ops;    \
    }   \
}while(0)

#endif
