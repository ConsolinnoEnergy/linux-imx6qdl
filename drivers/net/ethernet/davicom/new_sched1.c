
// [1208 - 1219]

#if DEF_PRO
void dm9051_spimsg_init(board_info_t *db)
{
        //spi_message_init(&dm9.Tmsg);
        //spi_message_add_tail(&dm9.Tfer,&dm9.Tmsg);
        #if DEF_SPICORE_IMPL1
        #ifdef QCOM_BURST_MODE
         memset(&db->spi_xfer2, 0, sizeof(struct spi_transfer)*2); //[Add.] 
         spi_message_init(&db->spi_msg2);
         spi_message_add_tail(&db->spi_xfer2[0], &db->spi_msg2);
         spi_message_add_tail(&db->spi_xfer2[1], &db->spi_msg2);
        #else
        memset(&db->Tfer, 0, sizeof(struct spi_transfer)); //[Add.] 
        spi_message_init(&db->Tmsg);
        spi_message_add_tail(&db->Tfer,&db->Tmsg);
        db->fer = &db->Tfer;
        db->msg = &db->Tmsg;
        #endif
        #endif
}
#endif

// [ 644 - 687 ]

//[../new_load/sched.c]
/* [Schedule Work to operate SPI rw] */

/*(or DM9051_Schedule.c)*/
/*static int dm9051_sch_cnt(u16 ChkD) // large equ +50 or less -3
{
	static u16 SavC= 0;
	
	if (SavC != ChkD) {
		
		u16 LessC= 0;
		if (SavC > 3)
			LessC= SavC - 3;
		
		if (ChkD < LessC) { //SavC
			SavC= ChkD;
			return 1; //less and reduce
		}
		
		if (ChkD >= (SavC+50)) {
			SavC= ChkD;
			return 1;
		}
	}
	return 0;
}*/
static int dm9051_sch_cnt1(u16 getVAL, u16 offset) // large equ +1 (as increasment)
{
	static u16 Sav1= 0;
	
	if (!offset) // an option that always no need printed, so return 0. 
		return 0;
	
	//offset default is as 1
	//if (Sav1 != getVAL) {
		if (getVAL >= (Sav1+offset)) {
			Sav1= getVAL;
			return 1;
		}
	//}
	return 0;
}

// [ 688 - 967 ]

//Testing...JJ5_DTS
#define GLUE_LICENSE_PHYPOLL		(3+2)
#define GLUE_LICENSE_INT			(3+1)
#define GLUE_LICENSE_LE_EXPIRE		(3-1)

static void 
dm9051_INTPschedule_isr(board_info_t *db, int sch_cause)
{
	//spin_lock(&db->statelock_tx_rx);//mutex_lock(&db->addr_lock);
	
	//.printk("R_SCH_XMIT %d (=%d) dm9051_start_xmit, Need skb = skb_dequeue(&db->txq) to get tx-data\n", R_SCH_XMIT, sch_cause);
	if (dm9051_sch_cnt1(db->nSCH_XMIT, 0)) //1500, 5500, 0
		printk("dm9-INFO TX %02d, sched R_SCH_XMIT %d (=%d) send skb_dequeue(txq).. \n", db->nSCH_XMIT, R_SCH_XMIT, sch_cause); //, db->rx_count
	
	db->sch_cause = sch_cause;
	
	#ifdef DM_CONF_POLLALL_INTFLAG
	if (sch_cause== R_SCH_INIT)
		return;
	if (sch_cause== R_SCH_INT_GLUE)
		return;
	//if (sch_cause== R_SCH._INFINI) 
	//	return;
	#endif
	
	switch (sch_cause)
	{
		case R_SCH_INIT:
			db->nSCH_INIT++; // if (m<250) m++; 
		//	schedule_delayed_work(&db->rx._work, 0); //dm9051_continue_poll
			break;
		case R_SCH_INFINI:
			db->nSCH_INFINI++;
		//	schedule_delayed_work(&db->rx._work, 0);  //dm9051_continue_poll
			break;
		//case R_SCH_LINK:
			//break;
		case R_SCH_PHYPOLL:
			break;
		case R_SCH_INT:
			db->nSCH_INT++;
		//	schedule_delayed_work(&db->rx._work, 0); //dm9051_continue_poll 
			break;
		case R_SCH_INT_GLUE:
			db->nSCH_INT_Glue++;
	#ifdef DM_CONF_POLLALL_INTFLAG		
			DM9051_int_token++; //.DM9051_int_token++;
	#endif	
			break;
		case R_SCH_XMIT:
	#ifdef DM_CONF_POLLALL_INTFLAG	
			DM9051_int_token++;
	#endif			
	        //db->_nSCH_XMIT++;
			//.printk("(%d)dm9051_start_xmit, Need skb = skb_dequeue(&db->txq) to get tx-data\n", db->_nSCH_XMIT);
			break;
	}
	
	#ifdef DM_CONF_TASKLET
	switch (sch_cause)
	{
		case R_SCH_INIT:
		case R_SCH_INFINI:
		//case R_SCH_LINK:
		case R_SCH_INT:
			tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0);
			break;
		case R_SCH_INT_GLUE:
			   tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0); 
			break;		
		case R_SCH_PHYPOLL:
#ifdef MORE_DM9051_MUTEX
			   tasklet_schedule(&db->phypoll_tl); //schedule_.delayed_work(&db->x, 0);
#else
			   tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0); 
#endif			   
			break;		
	#ifdef DM_CONF_POLLALL_INTFLAG		
		case R_SCH_XMIT:
#ifdef MORE_DM9051_MUTEX
			tasklet_schedule(&db->xmit_tl); //schedule_.delayed_work(&db->y, 0);
#else
			tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0);
#endif			   
			break;
	#endif			 	
	}
	#else //~DM_CONF_TASKLET
#if 1	
	//spin_lock(&db->statelock_tx_rx);//mutex_lock(&db->addr_lock);
	switch (sch_cause)
	{
  #ifdef DM_CONF_INTERRUPT
		#ifndef DM_CONF_THREAD_IRQ
		case R_SCH_INT:
			schedule_delayed_work(&db->rx_work, 0); 
			break;
		#endif
  #else
		case R_SCH_INIT:
		case R_SCH_INFINI: //'POLLING'
		case R_SCH_INT_GLUE:
		//case R_SCH_LINK:
			//schedule_delayed_work(&db->rx_work, 0); //dm9051_continue_poll
			//break;
			 schedule_delayed_work(&db->rx_work, 0); 
			break;		
  #endif
  
		case R_SCH_PHYPOLL:
#ifdef MORE_DM9051_MUTEX
			   schedule_delayed_work(&db->phypoll_work, 0);
#else
			   schedule_delayed_work(&db->rx_work, 0); 
#endif			   
			break;		
			
		#ifdef DM_CONF_INTERRUPT
		case R_SCH_XMIT:
		#ifdef MORE_DM9051_MUTEX
			schedule_delayed_work(&db->xmit_work, 0);
		#else
			schedule_delayed_work(&db->rx_work, 0); //dm9051_continue_poll
			   //[OR] schedule_delayed_work(&db->tx_work, 0); //(dm9051_tx_work) This which need tryLOck() or Mutex() design.
		#endif			   
		break;
		#endif			 	
		/* 0, Because @delay: number of jiffies to wait or 0 for immediate execution */
	}
	//spin_unlock(&db->statelock_tx_rx);//mutex_unlock(&db->addr_lock);
#endif	
	//spin_unlock(&db->statelock_tx_rx);//mutex_unlock(&db->addr_lock);
	#endif
}

 #ifdef DM_CONF_POLLALL_INTFLAG
 #elif DRV_POLL_1
 
#ifndef DM_EXTREME_CPU_MODE //20210204	
static void
dm9051_INTPschedule_weight(board_info_t *db, unsigned long delay)
{
	static int sd_weight = 0;
	
	#ifdef DM_CONF_TASKLET
	if (db->DERFER_rwregs[MD_ReadPTR] != db->DERFER_rwregs1[MD_ReadPTR]) {
		tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0); 
		return;
	}
	
	if (db->DERFER_rwregs1[RXM_WrtPTR] == db->DERFER_rwregs1[MD_ReadPTR]) {
		tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, delay); 
		return;
	}
	
	sd_weight++;
	if (!(sd_weight%3)) {
		if (sd_weight>=6000) /*(sd_weight>=5000) in disp no adj*/ 
			sd_weight = 0;
			
		if ( sd_weight == 0 && (db->DERFER_calc1>>8 )!= 0) // fewer disp
			printk("-[dm9 SaveCPU for: MDWA 0x%x (RO %d.%d%c)]-\n", db->DERFER_rwregs1[RXM_WrtPTR], db->DERFER_calc1>>8, db->DERFER_calc1&0xff, '%');
		
		tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, delay);  // slower ,
		return;
	}
	tasklet_schedule(&db->rx_tl); //schedule_.delayed_work(&db->r, 0); 
	#else //~DM_CONF_TASKLET
	
	if (db->DERFER_rwregs[MD_ReadPTR] != db->DERFER_rwregs1[MD_ReadPTR]) {
		schedule_delayed_work(&db->rx_work, 0); 
		return;
	}
		
	//good.all.readout
	if (db->DERFER_rwregs1[RXM_WrtPTR] == db->DERFER_rwregs1[MD_ReadPTR]) { //THis is also 'db->DERFER_calc1>>8 == 0'
		//mdwa = 0;
		schedule_delayed_work(&db->rx_work, delay); 
		return;
	}
			
	sd_weight++;
	if (!(sd_weight%3)) /* "slower" by more delay_work(delay) */
	/*if (!(sd_weight%5)) */
	/*if (!(sd_weight%6)) */ {
		
		//warn.(NoWrPkt)But_Read_CutCut (too slow read or rx-pointer.Err)
		/*if ((db->DERFER_calc1>>8) > 5) {
			sd_weight = 0;
			dm9051_fifo_reset(1, "dm9 (RxPoint.Err)", db);
			dm9051_fifo_reset_statistic(db);
			schedule_..delayed_work(&db->rx_work, 1);  //or 'delay'
			return;
		}*/
	
		//normal
		if (sd_weight>=6000) /*(sd_weight>=5000) in disp no adj*/ 
			sd_weight = 0;
			
		if ( sd_weight == 0 && (db->DERFER_calc1>>8 )!= 0) // fewer disp
			printk("-[dm9 SaveCPU for: MDWA 0x%x (RO %d.%d%c)]-\n", db->DERFER_rwregs1[RXM_WrtPTR], db->DERFER_calc1>>8, db->DERFER_calc1&0xff, '%');
		
		schedule_delayed_work(&db->rx_work, delay);  // slower ,
		return;
	}
	schedule_delayed_work(&db->rx_work, 0); 
	#endif
}
#endif //DM_EXTREME_CPU_MODE //20210204	
#endif

//
//
// SUB_DM9051.C
//
//

void dm_schedule_phy(board_info_t *db)
{  
	#ifdef DM_CONF_TASKLET
	tasklet_schedule(&db->phy_poll_tl);
	#else //~DM_CONF_TASKLET
  //schedule_delayed_work(&db->phy._poll, HZ * 2); to be 3 seconds instead
  //schedule_delayed_work(&db->phy._poll, HZ * 3);
	schedule_delayed_work(&db->phy_poll, HZ * 2);
	#endif
}     

void sched_work(board_info_t *db)
{
	#ifdef DM_CONF_TASKLET
	tasklet_schedule(&db->rxctrl_tl);
	#else //~DM_CONF_TASKLET
	#if 1
	/*[DM9051_Schedule.c]*/
	/* spin_lock/spin_unlock(&db->statelock); no need */
	schedule_work(&db->rxctrl_work);
	#endif
	#endif
}

/*[DM9051_Device_Ops.c]*/
void dm_sched_start_rx(board_info_t *db) // ==> OPEN_init_sched_delay_work
{
#if 1
	if (db->driver_state!=DS_POLL) {
	   db->driver_state= DS_POLL;
	   dm9051_INTPschedule_isr(db, R_SCH_INIT); //#ifndef DM_CONF_POLLALL_INTFLAG, #endif
	} 
#endif
}

netdev_tx_t dm_sched_tx_via_sched_rx(struct sk_buff *skb, struct net_device *dev)
{
#if 1
#if DM_CONF_APPSRC
	board_info_t *db = netdev_priv(dev);

	if (db->nSCH_XMIT <= db->nSCH_XMIT_WAVE_PDL)
	{
		//[Mostly reach 'NUM_TOTAL_ALL']
		//'NUM_TOTAL_ALL' = 'NUM_SCH_XMIT_WAVE'*'NUM_TRIPS_OF_WAVE' //is originally defined as 5*5
		if (db->nSCH_XMIT <= NUM_TOTAL_ALL) 
			db->nSCH_XMIT++; //finally,is 'NUM_TOTAL_ALL'+1, so also following print-out is ...
			
		if (db->nSCH_XMIT <= db->nSCH_XMIT_WAVE_PDL)
			printk("dm9 .ndo_start_xmit, %d times\n", db->nSCH_XMIT); //when is 'NUM_TOTAL_ALL'+1, so also this print-out is disabled.
		else // [if (db->nSCH_XMIT <= (NUM_TOTAL_ALL+1))]
			printk("dm9 .ndo_start_xmit, %d times.....\n", db->nSCH_XMIT); //when is 'NUM_TOTAL_ALL'+1, so also this print-out is the latest.
	}
		
	#if DM_CONF_APPSRC & DM9051_CONF_TX
	toend_stop_queue1(dev, 1 ); // for 'dm9051_tx'
	skb_queue_tail(&db->txq, skb); // JJ: a skb add to the tail of the list '&db->txq'
	#endif     
	#ifdef DM_CONF_POLLALL_INTFLAG
	dm9051_INTPschedule_isr(db, R_SCH_XMIT); // of 'dm9051_start_xmit', one_more_as_for_tx
	#endif
#endif
#endif
	return NETDEV_TX_OK;
}           

//[for 'rxctrl_work']
void dm_sched_multi(struct net_device *dev)
{
#if 1
#if DEF_PRO & DM_CONF_APPSRC
	board_info_t *db = netdev_priv(dev);
	sched_work(db);
#endif
#endif
}

// [ 1632 - 1682]
	   
static void
dm_hash_table_unlocked(struct net_device *dev)
{
	board_info_t *db = netdev_priv(dev);
#ifdef JABBER_PACKET_SUPPORT
	u8 rcr = RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN | RCR_DIS_WATCHDOG_TIMER;
#else	
	u8 rcr = RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN;
#endif	
#if DEF_SPIRW
	struct netdev_hw_addr *ha;
	int i, oft;
	u32 hash_val;
	u16 hash_table[4];
	for (i = 0, oft = DM9051_PAR; i < 6; i++, oft++)
		iiow(db, oft, dev->dev_addr[i]);

	/* Clear Hash Table */
	for (i = 0; i < 4; i++)
		hash_table[i] = 0x0;

	/* broadcast address */
	hash_table[3] = 0x8000;

	if (dev->flags & IFF_PROMISC)
		rcr |= RCR_PRMSC;

	if (dev->flags & IFF_ALLMULTI)
		rcr |= RCR_ALL;

	/* the multicast address in Hash Table : 64 bits */
	netdev_for_each_mc_addr(ha, dev) {
		hash_val = ether_crc_le(6, ha->addr) & 0x3f;
		hash_table[hash_val / 16] |= (u16) 1 << (hash_val % 16);
	}

	/* Write the hash table */
	for (i = 0, oft = DM9051_MAR; i < 4; i++) {
		iiow(db, oft++, hash_table[i]);
		iiow(db, oft++, hash_table[i] >> 8);
	}

	iow(db, DM9051_RCR, rcr);
#endif	
	db->rcr_all= rcr;
/*
//TEST
	db->rcr_all |= RCR_PRMSC | IFF_ALLMULTI;
	printk("Test db->rcr_all from %02x to %02x\n", rcr, db->rcr_all);
*/	
}

// [ 1683 - 1690]

static void 
dm_hash_table(board_info_t *db)
{
	struct net_device *dev = db->ndev; //board_info_t *db = netdev_priv(dev);
	mutex_lock(&db->addr_lock);
	dm_hash_table_unlocked(dev);	
    mutex_unlock(&db->addr_lock);
}

// [ 2798 - 2917]

static void 
rx_mutex_hash_table(board_info_t *db)
{
	if (db->Enter_hash)	
	{
		dm_hash_table(db);
		db->Enter_hash= 0;
	}
}    

// [ 175 - 200 ]
		
#if DEF_OPE | DM_CONF_APPSRC
/*
 *  INT 
 */
void int_reg_stop(board_info_t *db)
{
#if DEF_SPIRW
	iiow(db, DM9051_IMR, IMR_PAR); // Disable all interrupts 
	if (db->nSCH_INT && (db->nSCH_INT <= DM9_DBG_INT_ONOFF_COUNT))
		printk("[dm9IMR].[%02x].dis ------- nINT= %d\n",
			iior(db, DM9051_IMR), db->nSCH_INT);
#endif
}

void int_reg_start(board_info_t *db, char *headstr)
{		
#if DEF_SPIRW
	iiow(db, DM9051_IMR, db->imr_all); /*iow*/
	if (db->nSCH_INT && (db->nSCH_INT <= DM9_DBG_INT_ONOFF_COUNT))
		printk("%s.[%02x].ena ------- nINT= %d\n", headstr,
			iior(db, DM9051_IMR), db->nSCH_INT);// Re-enable by interrupt mask register
#endif
}
#endif

void IMR_DISABLE(board_info_t *db)
{
#ifdef DM_CONF_POLLALL_INTFLAG	
	if (!DM9051_int_en) { // Note.ok. 
		
	 //if (db->sch_cause!=R_SCH_INT) {
	//	printk("[Dbg condition: CASE-IS-IMPOSSIBLE] check (db->sch_cause!=R_SCH_INT) INTO rx-work~]\n");
	//	printk("[Dbg condition: CASE-IS-IMPOSSIBLE] list ([SCH_INIT,1][XMIT,2][INT,3][INFINI,4][GLUE,5][PHYPOLL,6]) db->sch_cause= %d\n", db->sch_cause);
	 //}
		
	 //if (db->sch_cause==R_SCH_INT) {
	 mutex_lock(&db->addr_lock);
	 int_reg_stop(db);
	 mutex_unlock(&db->addr_lock);  
	 //}
	}
#endif
}    

bool ISR_RE_STORE(board_info_t *db)
{
#ifdef DM_CONF_POLLALL_INTFLAG	
	static unsigned short ctrl_rduce = 0;
	if (!DM9051_int_en)  // Note that: Come-in with 'if (db->sch_cause==R_SCH_INT)' TRUE.
	{

		#if defined WR_ISR_ENDOF_RXWORK_ONLY //to-do-check-how-to...
		mutex_lock(&db->addr_lock);
		db->bC.isbyte= ior(db, DM9051_ISR); // Got ISR
		if (db->bC.isbyte & 0x7e) 
		{

			//if (db->bC.isbyte == 0x82) ; // [only 'PT']

			if (db->bC.isbyte & 0x03) //(db->bC.isbyte & 0x01)
				ctrl_rduce++; // [with 'PT' or 'PR']
			else // somethings, BUT without PT or PR
				printk("[isr_reg] ISR= 0x%02x (somethings, BUT without PT or PR) Warn-Rare: overflow suspected\n", db->bC.isbyte);
				
			iiow(db, DM9051_ISR, db->bC.isbyte); // Clear ISR status
		}
		else 
		{
			if (db->bC.isbyte & 0x01)
				iiow(db, DM9051_ISR, db->bC.isbyte); // Clear ISR status //printk("[int_reg].e WITH PR: Wr ISR= 0x%02x\n", db->bC.isbyte);
		}
		mutex_unlock(&db->addr_lock);  
		#endif

		return true;
	}
#endif
	return false;
}                                                

void IMR_ENABLE(board_info_t *db, int with_enable_irq)
{
	mutex_lock(&db->addr_lock);	
#ifdef DM_CONF_POLLALL_INTFLAG	
...................VFJNKJNKBR........... .....................................................................
	if (!DM9051_int_en) { // Note that: Come-in with 'if (db->sch_cause==R_SCH_INT)' TRUE.
	 
      if (with_enable_irq)  {
		  
		if ((db->nSCH_INT <= DM_CONF_SPI_DBG_INT_NUM))  // || (db->nSCH_INT == 24)
			printk("[%s][%d].enable_irq\n", "dm951_irq", db->nSCH_INT); //from-"dm9051_rx_work"
		int_en(db->ndev);
      }
		
	  int_reg_start(db, "[dm9IMR]"); // "dm9IMR_irx_work", rxp no chg, if ncr-rst then rxp 0xc00 
	}
	if (DM9051_fifo_reset_flg) {
#if DEF_SPIRW
	  iiow(db, DM9051_RCR, db->rcr_all); // if ncr-rst then rx enable
#endif
	  DM9051_fifo_reset_flg = 0;
	}
#else	
	//...................VFJNKJNKBR........... 
	if (DM9051_fifo_reset_flg) {
	  int_reg_start(db, "[dmIMR_poll_rx_work]"); // exactly ncr-rst then rxp to 0xc00
#if DEF_SPIRW
	  iiow(db, DM9051_RCR, db->rcr_all); // exactly ncr-rst then rx enable
#endif
	  DM9051_fifo_reset_flg = 0;
	}
#endif
    mutex_unlock(&db->addr_lock);
}

// [ 1557 - 1567 ]

//[../new_load/control_sub.c]
static int dm9051_sch_cnt_chang(u16 nEnter) // large equ +1 (as increasment)
{
	static u16 nSAVE= 0xffff;
	if (nEnter != nSAVE) {
		nSAVE= nEnter;
		return 1;
	}
	return 0;
}

// [ 1568 - 1630 ]

int rx_work_carrier(board_info_t *db)
{
	struct net_device *dev = db->ndev;
	unsigned nsr;
	int link;	
	static int try= 0;
	//ststic int ng_found = 0
	
	//if (1) {
	//[here!]
	//do {
	mutex_lock(&db->addr_lock);
#if DEF_SPIRW
		nsr= iior(db, DM9051_NSR);
#endif
		link= !!(nsr & 0x40); //& NSR_LINKST
			
		if (!link && try && !(try%250) && try<=750)
		  printk("[DM9051.carrier] nsr %02x, link= %d (try %d)\n", nsr, link, try);
		
		if (link) {
			if (db->linkA<3) 
			  db->linkA++;
		} else {
			if (db->linkA)
			  db->linkA--;
		}

		//db->linkBool= db->linkA ? 1 : 0;  //Rasp-save
		if (db->linkA) {
			db->linkBool= 1;
			try= 0; //ng_found= 0;
		} else {
			db->linkBool= 0;
			try++; //ng_found= 1;
		}

	if (db->linkBool) //(netif_carrier_ok(dev))
	{
		if (dm9051_sch_cnt_chang(db->nSCH_LINK))
		  printk("\n[DM9051.carrier] Link Status is: %d nsr %02x [SCHEDU cnt %d. try %d]\n", link, nsr, db->nSCH_LINK, try);
	}
	else
	{
		db->nSCH_LINK++;
		if (db->nSCH_LINK<3)
		  printk("[DM9051.carrier] Link Status is: %d\n", link); //"nsr %02x", nsr
	}
	
		
		if (netif_carrier_ok(dev) != db->linkBool) { //Add	
			if (db->linkBool)
			  netif_carrier_on(dev); //db->nSCH_LINK= 0;
			else
			  netif_carrier_off(dev);
			printk("[DM9051.phypoll] Link Status is: %d\n", link);
			//[+fifo reset] here! (tbd)
			if (db->linkBool) {
				int_reg_start(db, "[dmIMR_poll_Linkup]"); // IF exactly ncr-rst then rxp to 0xc00
				iiow(db, DM9051_RCR, db->rcr_all); // IF exactly ncr-rst then rx enable
				
				//again.IF-re-check
				read_mrrx(db, &db->mdra_regs[0]);
				printk("[DM9051.phypoll] dm9 such-as-reset-done when Linkup:\n");
				printk("[DM9051.phypoll] dm9 mdra:\n");
				printk("[DM9051.phypoll]  imr-Linkup %4x\n", db->mdra_reg_end);
				printk("[DM9051.phypoll]  rcr-Wr is %4x\n", db->rcr_all);
			}
		}
		
	mutex_unlock(&db->addr_lock);  
	return link;
	//} while ((++try < 8) && !db->linkBool);
	//}
}

// [ 2693 - 2764 ]

void rx_work_cyclekeep(board_info_t *db, int has_txrx) // link_sched_delay_work, INT_glue_sched_delay_work, and infini_sched_delay_work
{
	//struct net_device *dev = db->ndev;
    //if (!netif_carrier_ok(dev) && db->nSCH_LINK < 65500) 	//new-add
	//	dm9051_INTPschedule_isr(db, R_SCH_LINK);         	//new-add
  #ifdef DM_CONF_POLLALL_INTFLAG
	static u32 SSave_Num = 0;
	static u32 SSave_INT_B = 0;
	char *jmp_mark= "*";
  #endif
   
  #ifdef DM_CONF_POLLALL_INTFLAG
	if (DM9051_int_token) DM9051_int_token--;
	if (DM9051_int_token)
		dm9051_INTPschedule_isr(db, R_SCH_INT_GLUE); //again (opt-0)
		
	if (has_txrx)	
		dm9051_INTPschedule_isr(db, R_SCH_INFINI); //again (opt-0)
		
	if (db->nSCH_INT_NUm != db->nSCH_INT_B) {
		if ((SSave_Num != db->nSCH_INT_NUm) || (SSave_INT_B != db->nSCH_INT_B)) {
			#if 0
			
			//.Check ok.
			//.printk("[DM9_cyclekeep Check] INT.Num %5d(dis %5d), INT.Sch= %5d(en %d)%s\n",
			//.	db->nSCH_INT_NUm, db->nSCH_INT_NUm_A, db->nSCH_INT, db->nSCH_INT_B, jmp_mark);
				
			#endif	
			SSave_Num = db->nSCH_INT_NUm;
			SSave_INT_B= db->nSCH_INT_B;
			
			if (db->nSCH_INT_NUm > (db->nSCH_INT_B+10)) {
				jmp_mark = "**";
				db->nSCH_INT_NUm_A= db->nSCH_INT= db->nSCH_INT_B= db->nSCH_INT_NUm;
				printk("[DM9_cyclekeep ALL-SYNC-EQUAL] INT.Num %5d(dis %5d), INT.Sch= %5d(en %d)%s\n",
				  db->nSCH_INT_NUm, db->nSCH_INT_NUm_A, db->nSCH_INT, db->nSCH_INT_B, jmp_mark);
			}
		}
	}
		
  #elif DRV_POLL_1
  
	//dm9051_INTPschedule_isr(db, R_SCH_INFINI);
	//=
	// schedule_delayed_work(&db->rx_work, 0); //dm9051_rx_work

#ifdef DM_EXTREME_CPU_MODE //20210204	
  //(4.14.79-KT.POLL-2.2zcd.xTsklet.savecpu_5pt_JabberP.202002_nosave_20210204)
  //(lnx_dm9051_dts_Ver2.2zcd_R2_b2_savecpu5i2p_Tasklet5p_JabberP_pm_NEW2.0_extreme)
  schedule_delayed_work(&db->rx_work, 0);
#else //20210204	  
	#define DM_TIMER_EXPIRE1    1  //15
	#define DM_TIMER_EXPIRE2    0  //10
	#define DM_TIMER_EXPIRE3    0
	
	if (db->DERFER_rwregs[RXM_WrtPTR] == db->DERFER_rwregs1[RXM_WrtPTR])
		dm9051_INTPschedule_weight(db, DM_TIMER_EXPIRE1);
	else {
		//if ((db->DERFER_calc1>>8) < 50)
		//	schedule_delayed_work(&db->rx_work, DM_TIMER_EXPIRE2); // slow ,
		//else
	#ifdef DM_CONF_TASKLET
			tasklet_schedule(&db->rx_tl);
	#else //~DM_CONF_TASKLET
			schedule_delayed_work(&db->rx_work,  DM_TIMER_EXPIRE3); // faster ,
	#endif
	}
#endif //20210204	
	
  #endif
}

// [ 2918 - 2967 ]
// [ 2968 - 3011 ]

/* ----- This is to let it do rx (ca also process xmit) ----- */
static void dm9051_mutex_dm9051(board_info_t *db)
{
	//int link; link= 
	//printk("[dm9051.isr extend.s:\n");
	int	has_tx_rx= 0;
	static int dbg_first_in = 1;
	
	IMR_DISABLE(db);
	
	if (dbg_first_in) {
	  dbg_first_in = 0;
	  //printk("[dm9051_rx_work] ------- 03.s.first in. ------\n");
	  //rx_mutex_head(db);
	  //dm9051_rx_cap(db); // get db->_rwregs[0] & db->_rwregs[1]
	  //rx_mutex_tail(db);
	  //printk("[dm9051_rx_work] ------- 03.s. %x/%x. ------\n", db->_rwregs[0], db->_rwregs[1]);
	}

	rx_mutex_head(db);
	dm9051_disp_hdr_s_new(db);
	rx_mutex_tail(db);
	
	/* [dm9051_simple_mutex_dm9051].s.e */
	rx_work_carrier(db);
  #if 1	
	if (netif_carrier_ok(db->ndev)) {
  #endif
	do {
      rx_mutex_hash_table(db);
    
      //rx_tx_isr(db);=
      rx_mutex_head(db);
      has_tx_rx = rx_tx_isr0(db); // e.g. has_tx_rx = 0;
      rx_mutex_tail(db);
    
	} while(0);
  #if 1	  
	}
  #endif
  
	if (ISR_RE_STORE(db)) //if (IMR._ENABLE(db, 1))
		db->nSCH_INT_B++;

	rx_mutex_head(db);
    dm9051_disp_hdr_e_new(db);
	rx_mutex_tail(db);
    
	rx_work_cyclekeep(db, has_tx_rx); //[CYCLE-KEEP]
	
	if (DM9051_fifo_reset_flg) {
		IMR_ENABLE(db, 1);
				//again
				read_mrrx(db, &db->mdra_regs[0]);
				printk("dm9 reset-done then all-mutex's IMR-ENABLE:\n");
				printk("dm9 mdra:\n");
				printk(" imr-reset %4x\n", db->mdra_reg_end);
	} else
		IMR_ENABLE(db, 1);
}

/* ----- This is to let it do xmit ----- */
static void dm9051_simple_mutex_dm9051(board_info_t *db)
{
	rx_work_carrier(db);
  #if 1	
	if (netif_carrier_ok(db->ndev)) {
  #endif
	do {
      rx_mutex_hash_table(db);
    
      //rx_tx_isr(db);=
      rx_mutex_head(db);
      /*has_tx_rx= */ rx_tx_isr0(db); // has_tx_rx = NOUSED.
      rx_mutex_tail(db);
    
	} while(0);
  #if 1	  
	}
  #endif
  
  #ifdef DM_CONF_POLLALL_INTFLAG 
  //[ASR gpio only (high trigger) raising trigger].s
  #ifdef MORE_DM9051_INT_BACK_TO_STANDBY 
   //#ifdef DM_CONF_POLLALL_INTFLAG 
	if (DM9051_int_en )
	{
  //#endif
	     mutex_lock(&db->addr_lock);	
         db->bC.isbyte= ior(db, DM9051_ISR); // Got ISR
      
		if (db->bC.isbyte & 0x01)
		{
		  iiow(db, DM9051_ISR, db->bC.isbyte); //~bhdbd~~ // Clear ISR status
		  //;printk("--- dm9 check DM9051_INT_BACK_TO_STANDBY [%d]--- \n", db->nSCH_INT);
		}      
        mutex_unlock(&db->addr_lock);  
		
   //#ifdef DM_CONF_POLLALL_INTFLAG 
	}
  //#endif
 #endif	
  //[ASR gpio only (high trigger) raising trigger].e
 #endif
	//[added.]
	if (DM9051_fifo_reset_flg) {
		IMR_ENABLE(db, 1);
				//again
				read_mrrx(db, &db->mdra_regs[0]);
				printk("dm9 reset-done then simple-mutex's IMR-ENABLE:\n");
				printk("dm9 mdra:\n");
				printk(" imr-reset %4x\n", db->mdra_reg_end);
	} else
		IMR_ENABLE(db, 1);
}

// [3012 - 3244]

	#ifdef DM_CONF_TASKLET
	static void 
	dm_hash_table_task(unsigned long data)
	{
		board_info_t *db = (board_info_t *) data;
		db->Enter_hash= 1;
	}
	#else //~DM_CONF_TASKLET
	static void 
	dm_hash_table_work(struct work_struct *work)
	{
		board_info_t *db = container_of(work, board_info_t, rxctrl_work);
		db->Enter_hash= 1;
		//board_info_t *db = container_of(work, board_info_t, rxctrl_work);
		//dm_hash_table(db); //struct net_device *dev = db->ndev;
	}
	#endif
				
#ifdef DM_CONF_PHYPOLL

int db_phy = 0;
int nAll_run_gap = 0;

	#ifdef DM_CONF_TASKLET
	/*
	static void 
	dm_phy_poll_task(unsigned long data)
	{
		board_info_t *db = (board_info_t *) data;
		int a, b;
		
	#ifdef DM_CONF_POLLALL_INTFLAG 
	#if defined MORE_DM9051_MUTEX && defined  MORE_DM9051_MUTEX_EXT
	mutex_lock(&db->spi_lock);
	if (!DM9051_int_en) {
		mutex_unlock(&db->spi_lock);
		goto sched_phy;
	}
	mutex_unlock(&db->spi_lock);
	#else
	if (!DM9051_int_en)
		goto sched_phy;
	#endif
	#else		
	//if (!DM9051_int_en_OF_poll) goto sched_phy;
	#endif	

	//debug.NOT.in_rx_work.s!
	a = (int) db->nSCH_INT_NUm;
	b = (int) db->nSCH_INT_B;
	if (a != (b + nAll_run_gap)) { 
		nAll_run_gap = a - b; // record the diff.
	}
	db_phy++; 
	//debug.NOT.in_rx_work.e!
	
	dm9051_INTPschedule_isr(db, R_SCH_PHYPOLL);  //extended-add
	
	#ifdef DM_CONF_POLLALL_INTFLAG 
sched_phy:
	#else
//sched_phy:
	#endif
	if (netif_running(db->ndev))
	  dm_schedule_phy(db);
	}*/
	
	#else //~DM_CONF_TASKLET
	
static void 
dm_phy_poll(struct work_struct *w)
{ 
//#ifdef DM_CONF_PHYPOLL
	struct delayed_work *dw = to_delayed_work(w);
	board_info_t *db = container_of(dw, board_info_t, phy_poll);
	int a, b;
	
	//if.in.rx_work.procedure.s!
#ifdef DM_CONF_POLLALL_INTFLAG 
  #if defined MORE_DM9051_MUTEX && defined  MORE_DM9051_MUTEX_EXT
	mutex_lock(&db->spi_lock);
	if (!DM9051_int_en) {
		mutex_unlock(&db->spi_lock);
		goto sched_phy;
	}
	mutex_unlock(&db->spi_lock);
  #else
	if (!DM9051_int_en)
		goto sched_phy;
  #endif
#else		
	//if (!DM9051_int_en_OF_poll) goto sched_phy;
#endif	
	//if.in.rx_work.procedure.e!
	
	//debug.NOT.in_rx_work.s!
	a = (int) db->nSCH_INT_NUm;
	b = (int) db->nSCH_INT_B;
	if (a != (b + nAll_run_gap)) { 
		nAll_run_gap = a - b; // record the diff.
		//.printk("dm_phypol %d[run-gap %d][PHY-SCHED-rx-work-OUT_OF-INT].CHK. INT.Num %5d(dis %5d), INT.Sch= %5d(en %d).\n",
		//.	db_phy, nAll_run_gap, db->nSCH_INT_NUm, db->nSCH_INT_NUm_A, db->nSCH_INT, db->nSCH_INT_B);
	}
	db_phy++; 
	//debug.NOT.in_rx_work.e!
	
	//dm_netdevice_carrier(db);
	dm9051_INTPschedule_isr(db, R_SCH_PHYPOLL);  //extended-add
	
#ifdef DM_CONF_POLLALL_INTFLAG 
sched_phy:
#else
//sched_phy:
#endif
	if (netif_running(db->ndev))
	  dm_schedule_phy(db);
//#endif
}
  #endif
#endif //DM_CONF_PHYPOLL

	#ifdef DM_CONF_TASKLET
	/*static void dm9051_rx_task(unsigned long data) {
		board_info_t *db = (board_info_t *) data;
		#ifdef MORE_DM9051_MUTEX
		mutex_lock(&db->spi_lock);
		#endif

		dm9051_mutex_dm9051(db);
		
		#ifdef MORE_DM9051_MUTEX
		mutex_unlock(&db->spi_lock);
		#endif
	}*/
	#else //~DM_CONF_TASKLET
	
	#ifdef DM_CONF_THREAD_IRQ
	static void dm9051_rx_work_proc(board_info_t *db) {
		#ifdef MORE_DM9051_MUTEX
		mutex_lock(&db->spi_lock);
		#endif

		dm9051_mutex_dm9051(db);
		
		#ifdef MORE_DM9051_MUTEX
		mutex_unlock(&db->spi_lock);
		#endif
	}
	#else
	 static void dm9051_rx_work(struct work_struct *work) { //TODO. (over-night ? result)
		struct delayed_work *dw = to_delayed_work(work);
		board_info_t *db = container_of(dw, board_info_t, rx_work);
		
		#ifdef MORE_DM9051_MUTEX
		mutex_lock(&db->spi_lock);
		#endif

		dm9051_mutex_dm9051(db);
		
		#ifdef MORE_DM9051_MUTEX
		mutex_unlock(&db->spi_lock);
		#endif
	  }
	#endif
  #endif //DM_CONF_TASKLET

#ifdef MORE_DM9051_MUTEX
	#ifdef DM_CONF_TASKLET
	static void dm9051_phypoll_tasklet(unsigned long data){
		board_info_t *db = (board_info_t *) data;
		mutex_lock(&db->spi_lock);
		dm9051_simple_mutex_dm9051(db); //dm9051_mutex_dm9051(db);
		mutex_unlock(&db->spi_lock);
	}
	static void dm9051_xmit_tasklet(unsigned long data){
		//[or by spin_lock_irq(&db->hwlock)/spin_unlock_irq(&db->hwlock)]
		board_info_t *db = (board_info_t *) data;
		mutex_lock(&db->spi_lock);
		dm9051_simple_mutex_dm9051(db); //dm9051_mutex_dm9051(db);
		mutex_unlock(&db->spi_lock);
	}
	#else //~DM_CONF_TASKLET
static void dm9051_phypoll_work(struct work_struct *work) {
	struct delayed_work *dw = to_delayed_work(work);
	board_info_t *db = container_of(dw, board_info_t, phypoll_work);
	mutex_lock(&db->spi_lock);
	dm9051_simple_mutex_dm9051(db); //dm9051_mutex_dm9051(db);
	mutex_unlock(&db->spi_lock);
}
static void dm9051_xmit_work(struct work_struct *work) {
	struct delayed_work *dw = to_delayed_work(work);
	board_info_t *db = container_of(dw, board_info_t, xmit_work);
	mutex_lock(&db->spi_lock);
	dm9051_simple_mutex_dm9051(db); //dm9051_mutex_dm9051(db);
	mutex_unlock(&db->spi_lock);
}
		#endif
#endif //MORE_DM9051_MUTEX

void define_delay_work(board_info_t * db)
{
	#ifdef DM_CONF_TASKLET
	/*
	tasklet_init(&db->rxctrl_tl, dm_hash_table_task,(unsigned long) db);
	#ifdef DM_CONF_PHYPOLL	
	tasklet_init(&db->phy_poll_tl, dm_phy_poll_task,(unsigned long) db);
	#endif
	tasklet_init(&db->rx_tl, dm9051_rx_task, (unsigned long) db);

	#ifdef MORE_DM9051_MUTEX
	tasklet_init(&db->phypoll_tl, dm9051_phypoll_tasklet, (unsigned long) db);
	tasklet_init(&db->xmit_tl, dm9051_xmit_tasklet, (unsigned long) db);
	#endif
	*/
	#else //~DM_CONF_TASKLET
	
	INIT_WORK(&db->rxctrl_work, dm_hash_table_work);
	#ifdef DM_CONF_PHYPOLL	
	INIT_DELAYED_WORK(&db->phy_poll, dm_phy_poll);
	#endif
    
    #ifndef DM_CONF_THREAD_IRQ
    INIT_DELAYED_WORK(&db->rx_work, dm9051_rx_work); //(dm9051_continue_poll); // old. 'dm9051_INTP_isr()' by "INIT_WORK"
    #endif
    
	#ifdef MORE_DM9051_MUTEX
	INIT_DELAYED_WORK(&db->phypoll_work, dm9051_phypoll_work);
	INIT_DELAYED_WORK(&db->xmit_work, dm9051_xmit_work);
	#endif
  #endif
}

// [ 968 - 1001 ]

//[../new_load/driver_ops.c]    
		       
/*[when DM9051 stop]*/
void sched_delay_work_cancel(board_info_t * db)
{
	#ifdef DM_CONF_TASKLET
	#ifdef DM_CONF_PHYPOLL
	tasklet_kill(&db->phy_poll_tl);
	#endif
	tasklet_kill(&db->rxctrl_tl);
	tasklet_kill(&db->rx_tl);
	#ifdef MORE_DM9051_MUTEX
	tasklet_kill(&db->phypoll_tl);
	tasklet_kill(&db->xmit_tl);
	#endif
	#else //~DM_CONF_TASKLET
	
	#ifdef DM_CONF_PHYPOLL
	cancel_delayed_work_sync(&db->phy_poll);
	#endif
	
//.flush_work(&db->rxctrl_work); /* stop any outstanding work */
  #ifndef DM_CONF_THREAD_IRQ
	cancel_delayed_work_sync(&db->rx_work); //flush_work(&db->rx_work);
	#endif
	
#ifdef MORE_DM9051_MUTEX
	cancel_delayed_work_sync(&db->phypoll_work);
	cancel_delayed_work_sync(&db->xmit_work);
#endif
  #endif
}

// [ 1002 - 1017 ]

/* ops */         

/* event: play a schedule starter in condition */
static netdev_tx_t 
DM9051_START_XMIT(struct sk_buff *skb, struct net_device *dev) //void sta_xmit_sched_delay_work(board_info_t * db)
{                 
	return dm_sched_tx_via_sched_rx(skb, dev);
}

/* play with a schedule starter */
static void 
dm9051_set_multicast_list_schedule(struct net_device *dev)
{
	dm_sched_multi(dev);
}

int
dm9051_set_mac_address(struct net_device *dev, void *p)
{
	char *s = p;
	printk("dm9051_set_mac_address %x %x %x  %x %x %x\n", s[0],s[1],s[2],s[3],s[4],s[5]);
	return eth_mac_addr(dev, p);
}
