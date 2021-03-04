/*
 * drivers/net/dm9051.c
 * Copyright 2017 Davicom Semiconductor,Inc.
 *
 * 	This program is free software; you can redistribute it and/or
 * 	modify it under the terms of the GNU General Public License
 * 	as published by the Free Software Foundation; either version 2
 * 	of the License, or (at your option) any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * (C) Copyright 2017-2021 DAVICOM Semiconductor,Inc. All Rights Reserved.
 *  V2.0 - 20190829, Support DTS usage, Support Interrupt.
 *  V2.1 - 20190910, Support Non-DTS usage and Interrupt. 
 *  (Still not tesed for Polling. MTK cpu support is to be added.) 
 *  V2.2 - 20190912, Add spi_setup, add POLLING support
 *  (Test ... save file test ...and test ...)
 *  (Test ... save file test ...and test2 ...)
 *  V2.2xc - 20190924, Support ASR 1024 buf limitation.
 *         - With the tested log which is result-good with few RXBErr and fewer macErr.  
 *  V2.2yc - 20190929, Support ASR 0 buf limitation.
 *         - Verify with Module Mode, NON-DTS, INT OK.
 *         - Verify with Module Mode, NON-DTS, Polling OK.
 *         - Verify with Module Mode, DTS, INT OK.
 *         - Verify with Module Mode, DTS, Polling OK.
 *         - Verify DM_CONF_MAX_SPEED_HZ	individually as 15600000 and 31200000 OK.
 *  V2.2zc - 20191015, Test.Good
 *  V2.2zcd.beta- 20191023, Add Supp Conf SPI dma yes
 *  V2.2zcd - 20191024, test Supp Conf SPI dma yes, OK
 *  V2.2zcd_R1 - 20191105, make 5 major macros in 'conf_ver.h'
 *  V2.2zcd_R2b - 20191108, put spi fer/msg in 'board_info.h'
 *  V2.2zcd_R2b2_savecpu2 - 20191122, save cpu in polling mode
 *  V2.2zcd_R2b2_savecpu3i - 20191126, Correcting interrupt mode ('int_get_attribute' & 'int_get_begin')
 *  V2.2zcd_R2b2_savecpu3i2 - 20191126, Str[]
 *  V2.2zcd_R2b2_savecpu5i - 20191128, Interrupt & sched
 *  V2.2zcd_R2b2_savecpu5i2p_xTsklet_thread - 20191211, Tasklet Not Suitable, But request_threaded_irq()
 *  V2.2zcd_R2b2_savecpu5i2p_xTsklet1 - 20191213, ASL_RSRV_RX_HDR_LEN                                                              
 *  V2.2zcd_R2b2_savecpu5i2p_xTsklet1p - 20191215 code-format
 *  V2.2zcd_R2b2_savecpu5i2p_xTsklet2p - 20191218 RXB explore
 *  V2.2zcd_R2b2_savecpu5i2p_xTsklet3p - 20191218 MTK MT6762       
 *  V2.2zcd_R2b2_savecpu5i3p_xTsklet3p - 20191218 RX CHECK & mtu   
 *  V2.2zcd_R2b2_savecpu5i3p_xTsklet5p - 20191220 Quick process 1516_issue, Release mode (ON_RELEASE) support,  
 *                                       Create "conf_rev.h" which is for customization purpose~
 *  V2.2zcd_tsklet_savecpu_5pt - 20191227 Make the SCAN_LEN (also SCAN_LEN_HALF) as 98 KB (conf_ver.h)
 *                                       To improve the RX procedure, Also well analysis everything in RX.
 *  V2.2zcd_tsklet_savecpu_5pt_Jabber - 20200121 Make Jabber support
 *  V2.2zcd_tsklet_savecpu_5pt_JabberP - 20200206 Add rx errors, rx frames statistics
 *  V2.2zcd_tsklet_savecpu_5pt_JabberP_PM - 20200331 PM is for suspend/resume function
 *  v2.2zcd_lnx_dm9051_dts_Ver2.2zcd_R2_b2_savecpu5i2p_Tasklet5p_JabberP_pm_NEW2.0_extreme - 20210204 Add 
 *                                       the DM_EXTREME_CPU_MODE definition for if poor performance
 *  v2.2zcd_light_rx - 20210222, new_tx.c for tx, new_rx.c for light rx
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/cache.h>
#include <linux/crc32.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <asm/delay.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/spi/spi.h>
extern int spi_register_board_info(struct spi_board_info const *info, unsigned n); //had been. #define CONFIG_SPI, used in local static-make for dm9051.o
#include "dm9051.h"
#include "conf_ver.h"

#include "def_kt.h" 
#include "def_board.h"
#include "def_generation.h"
#include "def_const_opt.h"

#define DM_RX_HEAVY_MODE	1  // default: set 1 to be NON-LIGHT_RX

#ifdef DM_LIGHT_RX  // light rx
#undef DM_RX_HEAVY_MODE
#define DM_RX_HEAVY_MODE	0  // light rx: set 0
#endif

#include "board_info.h"


u8 DM9051_fifo_reset_flg = 0;
#include "sub.h" //"sub_dm9051.h"   
#include "board_dev0.c"
#include "ethtool_ops.c"
#include "ethtool_ops1.c"
#define RXM_WrtPTR	0			// Write Read RX FIFO Buffer Writer Pointer
#define MD_ReadPTR	1			// Write Read RX FIFO Buffer Read Pointer


#if DM_RX_HEAVY_MODE
	#include "unload.cc"
	#include "unload1.cc"       

#else
	#include "new_printnb.c"
	//(#include "new_load.c")=
	#include "new_spi_sypc.c"
	#include "new_tx.c"
	#include "new_rx.c" //#include "one_rx_esp32_jj2.c" //#include "one_rx_single_jj0.c" //#include "one_rx.c"
	#include "new_sched1.c"
	//(#include "unload1.c").ok
	//(#include "new_load1.c").ok
	//#include "unload1.c"=
	#include "new_load1.c"        
	
#endif

#if DMA3_P4_KT
/*3p*/
static int dm9051_drv_suspend(struct spi_device *spi, pm_message_t state)
{
    board_info_t *db = dev_get_drvdata(&spi->dev);
    struct net_device *ndev = db->ndev;
	if (ndev) {
//.	db->in_suspend = 1;
		if (!netif_running(ndev)) 
			return 0;

		netif_device_detach(ndev);
        dm9051_stop(ndev);
	}
	return 0;
}

static int dm9051_drv_resume(struct spi_device *spi)
{
    board_info_t *db = dev_get_drvdata(&spi->dev);
    struct net_device *ndev = db->ndev;
	if (ndev) {
		if (netif_running(ndev)) {
            dm9051_open(ndev);
			netif_device_attach(ndev);
		}

//.	db->in_suspend = 0;
	}
	return 0;
}
#endif

#ifdef CONFIG_PM_SLEEP
//[User must config KConfig to PM_SLEEP for the power-down function!!]
#if DMA3_P4N_KT
static int dm9051_drv_suspend(struct device *dev) //(struct spi_device *spi)  //...
{
    board_info_t *db = dev_get_drvdata(dev); //(&spi->dev);
    struct net_device *ndev = db->ndev;
	if (ndev) {
		if (!netif_running(ndev)) 
			return 0;
		netif_device_detach(ndev);
		dm9051_stop(ndev);
	}
	return 0;
}
static int dm9051_drv_resume(struct device *dev) //(struct spi_device *spi)   //...
{
    board_info_t *db = dev_get_drvdata(dev); //(&spi->dev);
    struct net_device *ndev = db->ndev;
	if (ndev) {
		if (netif_running(ndev)) {
			dm9051_open(ndev);
			netif_device_attach(ndev);
		}
	}
	return 0;
}
#endif
#endif

#if DMA3_P4N_KT
static SIMPLE_DEV_PM_OPS (dm9051_drv_pm_ops, dm9051_drv_suspend, dm9051_drv_resume);
#endif

static struct of_device_id dm9051_match_table[] = { //"davicom,dm9051" (dts_yes_driver_table.h)
	{	.compatible = "davicom,dm9051",	},
	{}
};

struct spi_device_id dm9051_spi_id_table = {"dm9051", 0}; //DRVNAME_9051 (dts_yes_driver_table.h)

static struct spi_driver dm9051_driver = {
	.driver	= {
		.name  = DRVNAME_9051, //"dm9051"
		.owner = THIS_MODULE,
		#if DMA3_P4N_KT
		.pm	= &dm9051_drv_pm_ops,
		#endif
		.of_match_table = dm9051_match_table,
		.bus = &spi_bus_type,
	},
	.probe   = dm9051_probe,
	.remove  = dm9051_drv_remove,
	.id_table = &dm9051_spi_id_table,
	#if DMA3_P4_KT
	/*3p*/ .suspend = dm9051_drv_suspend,
	/*3p*/ .resume = dm9051_drv_resume,
	#endif
	#if DMA3_P4N_KT
	//.suspend = dm9051_drv_suspend,
	//.resume = dm9051_drv_resume,
	#endif
};

// 1----------------------------------------------------------------------------------------------------------

#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
   /* this no need register board information ! */
#else
#if DMA3_P6_DRVSTATIC
 /* Joseph 20151030 */
 extern int spi_register_board_info(struct spi_board_info const *info, unsigned n);
#else
 /* Joseph: find/delete/new */
 static unsigned verbose = 3;
 module_param(verbose, uint, 0);
 MODULE_PARM_DESC(verbose,
 "0 silent, >0 show gpios, >1 show devices, >2 show devices before (default=3)");

 static struct spi_device *spi_device;

 static void dm9051_device_spi_delete(struct spi_master *master, unsigned cs)
 {
	struct device *dev;
	char str[32];

	snprintf(str, sizeof(str), "%s.%u", dev_name(&master->dev), cs);

	dev = bus_find_device_by_name(&spi_bus_type, NULL, str);
	if (dev) {
		if (verbose)
			pr_info(DRVNAME_9051": Deleting %s\n", str);
		device_del(dev);
	}
 }
 static int dm9051_spi_register_board_info(struct spi_board_info *spi, unsigned n)
 {
	/* Joseph_20151030: 'n' is always 1, ARRAY_SIZE(table) is 1 table-item in this design  */
	struct spi_master *master;

	master = spi_busnum_to_master(spi->bus_num);
	if (!master) {
		pr_err(DRVNAME_9051 ":  spi_busnum_to_master(%d) returned NULL\n",
								spi->bus_num);
		return -EINVAL;
	}
	/* make sure it's available */
	dm9051_device_spi_delete(master, spi->chip_select);
	spi_device = spi_new_device(master, spi);
	put_device(&master->dev);
	if (!spi_device) {
		pr_err(DRVNAME_9051 ":    spi_new_device() returned NULL\n");
		return -EPERM;
	}
	return 0;
 }

 static void dm9051_spi_unregister_board(void)
 {
	//----------------------
	//[#ifdef MODULE #endif]
	//----------------------
	if (spi_device) {
		device_del(&spi_device->dev);
		kfree(spi_device);
	}
 }
#endif
#endif   

void conf_spi_board(void)
{
/* ------------------------------------------------------------------------------------------- */
/* #if DM_DM_CONF_INSTEAD_OF_DTS_AND_BE_DRVMODULE                                              */
/*   dm9051_spi_register_board_info(dm9051_spi_board_devs, ARRAY_SIZE(dm9051_spi_board_devs)); */
/* #else                                                                                       */
/* #endif                                                                                      */
/* ------------------------------------------------------------------------------------------- */

  #if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
   /* this no need register board information ! */
  #else
    #if DMA3_P6_DRVSTATIC
      spi_register_board_info(dm9051_spi_board_devs, ARRAY_SIZE(dm9051_spi_board_devs));
    #else
      dm9051_spi_register_board_info(dm9051_spi_board_devs, ARRAY_SIZE(dm9051_spi_board_devs));
    #endif
  #endif
}

void unconf_spi_board(void)
{
/* ---------------------------------------------- */
/* #if DM_DM_CONF_INSTEAD_OF_DTS_AND_BE_DRVMODULE */
/*     dm9051_spi_unregister_board();             */
/* #else                                          */
/* #endif                                         */
/* ---------------------------------------------- */

  #if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
   /* this no need register board information ! */
  #else
    #if DMA3_P6_DRVSTATIC
    #else
      dm9051_spi_unregister_board();
    #endif
  #endif
}

static int __init  
dm9051_init(void)
{
    printk("\n");
  //printk("[dm951_insmod].s -------- 00.s --------\n");
    printk("[dm951_s_insmod].s ");
    printk("  --");
    printk("  --");
    printk("  --");
    printk("--00.e--");
#if 0
    conf_spi_print(-);
    //;;NO 'db' here! / ;;NO 'spi' here!
	//;unsigned chipid= dm9051_chipid(db); 
	//;if (chipid==(DM9051_ID>>16))
	//;	printk("Read [DM9051_PID] = %04x OK\n", chipid);
#endif    
    printk("  --");
    printk("  --");
    printk("  --\n");
    printk("%s, DRV %s\n", CARDNAME_9051, MSTR_DTS_VERSION);
    printk("%s, DRV %s\n", CARDNAME_9051, MSTR_MOD_VERSION);
    printk("%s, DRV %s\n", CARDNAME_9051, MSTR_INT_VERSION);
    printk("%s, SPI %s\n", CARDNAME_9051, RD_MODEL_VERSION);
    printk("%s, SPI %s\n", CARDNAME_9051, WR_MODEL_VERSION);
    
    printk("%s Driver loaded, V%s (%s)\n", CARDNAME_9051, 
	DRV_VERSION, "LOOP_XMIT"); //str_drv_xmit_type="LOOP_XMIT"
	
#ifdef MTK_CONF_YES
    printk("%s, SPI %s\n", CARDNAME_9051, MSTR_MTKDMA_VERSION);
#endif
    printk("%s, SPI %s\n", CARDNAME_9051, MSTR_DMA_SYNC_VERSION);

#if 0
	dm9051_hw_reset();
#endif
	conf_spi_board();
	printk("[dm951_s_insmod].e\n");
	return spi_register_driver(&dm9051_driver);
}

static void dm9051_cleanup(void)
{
    printk("dm9051_exit.s\n");
	unconf_spi_board();
    printk("[dm951_e_rmmod].s ------- 00.s -------\n");
	spi_unregister_driver(&dm9051_driver);
    printk("[dm951_e_rmmod].e ------- 00.e -------\n");
}

module_init(dm9051_init);
module_exit(dm9051_cleanup);

MODULE_AUTHOR("Joseph CHANG <joseph_chang@davicom.com.tw>");
MODULE_DESCRIPTION("Davicom DM9051 network driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:dm9051");
