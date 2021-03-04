// Local compiler-option
/*  RX is basic essential */
/*  TX is made optional('DM9051_CONF_TX') */

#define DM9051_CONF_TX   				1

#if DM9051_CONF_TX
#define NUM_QUEUE_TAIL					0xFFFE   //(2) //(5)//(65534= 0xFFFE)MAX_QUEUE_TAIL  
#endif

#define NUM_SCH_XMIT_WAVE                               5
#define NUM_TRIPS_OF_WAVE                               5
#define NUM_TOTAL_ALL                                   (NUM_SCH_XMIT_WAVE * NUM_TRIPS_OF_WAVE) //is originally defined as 5*5

#define EN_DEBUG

#ifdef EN_DEBUG
#define dbg_log(format, args...)                                                                                        \
                do{                                                                                                                                         \
                                printk(KERN_ERR DRV_NAME": %s() _%d_: " format                       \
                                                                , __FUNCTION__                                                                          \
                                                                , __LINE__                                                                                      \
                                                                , ## args);                                                                                     \
                }while(0)
#else
#define dbg_log(format, args...)     ((void)0)
#endif //EN_DEBUG

