
#if DM_RX_HEAVY_MODE
#define	dm9051_disp_hdr_s_new(b) DM9051_DISP_HDR_S(b) 
#define	dm9051_disp_hdr_e_new(b) DM9051_DISP_HDR_E(b) 
#define	rx_mutex_head(b)	RX_MUTEX_HEAD(b)
#define	rx_mutex_tail(b)	RX_MUTEX_TAIL(b)
#else
#define	dm9051_disp_hdr_s_new(b)
#define	dm9051_disp_hdr_e_new(b)
#define	rx_mutex_head(b)
#define	rx_mutex_tail(b)
#endif

// [ 2443 - 2446 ]

//rx.c

#if DM_RX_HEAVY_MODE
//static void dm9051_disp_hdr_s(board_info_t *db)
static void DM9051_DISP_HDR_S(board_info_t *db)
{
	u16 calc= dm9051_rx_cap(db);
	db->DERFER_rwregs[RXM_WrtPTR]= db->rwregs[0];
	db->DERFER_rwregs[MD_ReadPTR]= db->rwregs[1];
	db->DERFER_calc= calc;
}
//static void dm9051_disp_hdr_e(board_info_t *db, int rxlen)
//static void dm9051_disp_hdr_e_new(board_info_t *db)
static void DM9051_DISP_HDR_E(board_info_t *db)
{
	u16 calc= dm9051_rx_cap(db);
	db->DERFER_rwregs1[RXM_WrtPTR]= db->rwregs[0];
	db->DERFER_rwregs1[MD_ReadPTR]= db->rwregs[1];
	db->DERFER_calc1= calc;
} 
		 
void RX_MUTEX_HEAD(board_info_t *db)
{
  #ifdef DM_CONF_POLLALL_INTFLAG	
	  mutex_lock(&db->addr_lock);
	  //.iiow(db, DM9051._IMR, IMR._PAR); // Disable all interrupts 
  #elif DRV_POLL_1
	  mutex_lock(&db->addr_lock);
  #endif
}           
void RX_MUTEX_TAIL(board_info_t *db)
{
  #ifdef DM_CONF_POLLALL_INTFLAG
	//.iiow(db, DM9051._IMR, db->imr._all); // Re-enable interrupt mask 
    mutex_unlock(&db->addr_lock);
  #elif DRV_POLL_1
    mutex_unlock(&db->addr_lock);
  #endif
}  
#endif

#if DM_RX_HEAVY_MODE == 0

static int dm9051_isr_ext2(board_info_t *db);

#ifdef DM_CONF_POLLALL_INTFLAG
   .....fdhgtrhrthytjtuyyu.. 
   //int rx_tx_isr0 (board_info_t *db) [NO SUPPORT INTERRUPT].....
#endif

int rx_tx_isr0(board_info_t *db)
{
	int nTX, nRx;
	int nAllSum= 0;
	
	do {
	  nTX= dm9051_tx_irx(db);
	  nRx= dm9051_isr_ext2(db); //dm9051_continue_poll_rx(db);
	  nAllSum += nTX;
	  nAllSum += nRx;
	} while (nTX || nRx);	
	
	return nAllSum;
}
#endif


#if DM_RX_HEAVY_MODE == 0

struct dm9051_rxhdr0 { //old
	u8	RxPktReady;
	u8	RxStatus;
	__le16	RxLen;
} __packed;

struct spi_rxhdr { //new
	u8	padb;
	u8	spiwb;
	struct dm9051_rxhdr {
	  u8	RxPktReady;
	  u8	RxStatus;
	  __le16	RxLen;
	} rxhdr;
} __packed;

#define	RXHDR_SIZE	sizeof(struct dm9051_rxhdr)

// [ xxxxxxxxxxxx ]
// [ scan and skb ]
// [ xxxxxxxxxxxx ]

// [ 1905 - 2016 ]
// [ 2018 - 2085 ]
							     
 #define DM_TYPE_ARPH	0x08
 #define DM_TYPE_ARPL	0x06
 #define DM_TYPE_IPH	0x08
 #define DM_TYPE_IPL	0x00
static bool SB_skbing_packet_chk_data(board_info_t *db, u8 *phd, u8 *rdptr)
{
	struct net_device *dev = db->ndev;
	u8 flg_disp = 0;
	u16 len;

	if ((phd[1] & 0x40) && (!(rdptr[0] &1)))
	{
		flg_disp = 1;
		//printk("\n[@dm9.multiErr start-with rxb= 0x%0x]\n", prxhdr->RxPktReady);
		dev->stats.rx_length_errors= 3;
		dev->stats.rx_crc_errors=              6;
		dev->stats.rx_fifo_errors=                 9;
		dev->stats.rx_missed_errors=                 12;
		dev->stats.rx_over_errors++;
		printk("\n[@dm9.multiErr (rxhdr %02x %02x %02x %02x)] mac %02x %02x %02x, %lu\n", phd[0], phd[1], phd[2], phd[3], rdptr[0], rdptr[1], rdptr[2],
			dev->stats.rx_over_errors); //dev->stats
		#if 0
		printk("dm9.dmfifo_reset( 10 multiErr ) mac %02x %02x %02x rxhdr %02x %02x %02x %02x {multi before rst RST_c= %d}\n", 
				rdptr[0], rdptr[1], rdptr[2],
				phd[0], phd[1], phd[2], phd[3],
				db->bC.DO_FIFO_RST_counter);
		#endif
	}
	else if (rdptr[0]!=dev->dev_addr[0] || rdptr[1]!=dev->dev_addr[1] || rdptr[2]!=dev->dev_addr[2])
	{                                  
		if ((rdptr[4]==DM_TYPE_ARPH && rdptr[5]==DM_TYPE_ARPL) && (rdptr[12]!=DM_TYPE_ARPH || rdptr[13]!=DM_TYPE_ARPL)) // special in data-skip
			; // error=fail //;;[current has rdptr[12]/rdptr[13]]
		if ((rdptr[4]==DM_TYPE_IPH && rdptr[5]==DM_TYPE_IPL) && (rdptr[12]!=DM_TYPE_IPH || rdptr[13]!=DM_TYPE_IPL))  // special in data-skip
			; // error=fail //;;[current has rdptr[12]/rdptr[13]]
		else if (rdptr[0]&1) //'skb->data[0]'
			return true;
			
		#if 1
		flg_disp = 1;
		//if (db->mac_process) { //"[ERRO.found.s]"
			//char hstr[72];
			//sprintf(hstr, "dmfifo_reset( 11 macErr ) mac %02x %02x %02x rxhdr %02x %02x %02x %02x", 
			//	rdptr[0], rdptr[1], rdptr[2],
			//	phd[0], phd[1], phd[2], phd[3]);
			//db->mac_process = 0;
			//db->bC.ERRO_counter++;
			//dm9051_fifo_reset(11, hstr, db);
			//dm9051_fifo_reset_statistic(db);
			//return false;
		//} else {
			//"[ERRO.found.s treat as no-error]"
			//printk("\n[@dm9.macErr start-with rxb= 0x%0x]\n", prxhdr->RxPktReady);
			dev->stats.rx_frame_errors++;
			printk("\n[@dm9.frame err (hdr %02x %02x %02x %02x)] [%02x %02x %02x %02x %02x %02x] %lu\n", 
				phd[0], phd[1], phd[2], phd[3], rdptr[0], rdptr[1], rdptr[2], rdptr[3], rdptr[4], rdptr[5],
				dev->stats.rx_frame_errors);
				
			//[01 00 9e 00] unicast and len is less 256 ;"Custom report 20210204/'lan_aging_error3.log'"
			len = (phd[3] << 8 ) + phd[2];
			if (phd[0]==0x01 && phd[1]==0x00 && phd[3]==0x00){
				printk("[@dm9.[warn] unknow uni-cast frame (hdr %02x %02x %02x %02x)] %02x %02x %02x %02x %02x %02x\n", 
					phd[0], phd[1], phd[2], phd[3], rdptr[0], rdptr[1], rdptr[2], rdptr[3], rdptr[4], rdptr[5]);
				//; fail //return true;
			} else if (phd[0]==0x01 && phd[1]==0x00 && phd[3]!=0x00)
				printk("[@dm9.[warn] unknow uni-cast long frame (hdr %02x %02x %02x %02x)] len %d\n", 
					phd[0], phd[1], phd[2], phd[3], len);
				
			#if 0
			printk("dm9.dmfifo_reset( 11 macErr ) mac %02x %02x %02x rxhdr %02x %02x %02x %02x {before rst RST_c= %d}\n", 
				rdptr[0], rdptr[1], rdptr[2],
				phd[0], phd[1], phd[2], phd[3],
				db->bC.DO_FIFO_RST_counter); //(quick)
			#endif
			//"(This got return true!!)"    
			//db->mac_process = 1 - db->mac_process;
		//}
		#endif    
	}
	//else
	//{
	//	if (db->mac_process) printk("macError-clear {rx-unicast %02x %02x %02x rxhdr %02x %02x %02x %02x}\n", 
	//		rdptr[0], rdptr[1], rdptr[2],
	//		phd[0], phd[1], phd[2], phd[3]);
	//	db->mac_process = 0;
	//}             
	
	if (flg_disp)
	{
		//packet ...    
		#if MSG_DBG
		u16 calc;
		calc= dm9051_rx_cap(db);   
		printk("[dm9] %02x/%02x (scan_enter)\n", db->rwregs_enter, db->rwregs1_enter); //[trick.extra]
		printk("[dm9] %02x/(wrp scan.end)\n", db->rwregs_scanend ); //[trick.extra]
		printk("[dm9] %02x/%02x (scan.end RO %d.%d%c)\n", db->rwregs[0], db->rwregs[1], calc>>8, calc&0xFF, '%');
		#endif
		//  printk("[dm9] ior-mac %02X %02X %02X %02X %02X %02X\n",
		//	    ior(db, 0x10), ior(db, 0x11), 
		//	    ior(db, 0x12), ior(db, 0x13), 
		//	    ior(db, 0x14), ior(db, 0x15));
		//  printk("[dm9] ior-RCR 0x%02X\n", ior(db, DM9051_RCR));
		#if 0
		printnb_packet(rdptr, prxhdr->RxLen-4);         
		printnb_packet(&rdptr[prxhdr->RxLen-4], 4);
		#endif
		return false;
	}
	//if (db->flg_rxhold_evt)
	//	printk("[dm9].encounter NN-cast, dm9051__chk_data, ...End...\n");
	
	db->bC.rx_unicst_counter++;
	db->nSCH_UniRX++;
	return true;
}

struct sk_buff *SB_skbing_trans(struct net_device *dev, char * buffp, int RxLen, int SKB_size)
{
	board_info_t *db = netdev_priv(dev);     
	struct sk_buff *skb;
	u8 *rdptr;
	u8 rxbyte = db->bC.dummy_pad;
	static int asr_rsrv_test = 0;
#ifdef ASL_RSRV_RX_HDR
	if ((skb = dev_alloc_skb(SKB_size)) == NULL)  {
			printk("dm9051 [!ALLOC skb size %d fail]\n", SKB_size);  //.......................db m ds   ................
			return NULL;
	}
#else
	if ((skb = dev_alloc_skb(SKB_size - ASL_RSRV_RX_HDR_LEN)) == NULL)  {
			printk("dm9051 [!ALLOC skb size %d fail]\n", SKB_size - ASL_RSRV_RX_HDR_LEN);
			return NULL;
	}
#endif
	if (asr_rsrv_test==0) {
		//asr_rsrv_test = 1;
		printk("[dm9].peek ------------ RxLen 4 RSRV.s= %d %d %d---------\n", RxLen,
			 4, ASL_RSRV_RX_HDR_LEN);
		printk("[dm9].peek ------------ skb_len.s= %d -------------------\n", skb->len);
	}   
	skb_reserve(skb, 2);
	
	if (asr_rsrv_test==0) {
		printk("[dm9].peek ------------ skb_put.i(x), x= %d -------------------\n", RxLen - 4);
		printk("[dm9].peek ------------ skb_len.s(put)= %d -------------------\n", skb->len);
	}
	
	/* A pointer to the first byte of the packet */
	rdptr = (u8 *) skb_put(skb,  RxLen - 4);  // [save 4] SKB_size - 4 - ASL_RSRV_RX_HDR_LEN - 4
	memcpy(rdptr, buffp, RxLen - 4); // [save 4] &sbufp[p]
	
	if (asr_rsrv_test==0) {
		printk("[dm9].peek ------------ skb_len.e(put)= %d -------------------\n", skb->len);
	}                   
	// ?? if (!dm9051_chk_data(db, rdptr, RxLen))
	//	return nRx; ? rwregs1; ?          
	dev->stats.rx_bytes += RxLen;
	skb->protocol = eth_type_trans(skb, dev);  // [JJ found: skb->len -= 14]
		 
	if (asr_rsrv_test==0) {
		asr_rsrv_test = 1;
		printk("[dm9].peek --- due to eth_type_trans(skb, dev), skb->len -= 14 ---\n");
		printk("[dm9].peek ------------ skb_len.e= %d -------------------\n", skb->len);
		printk("[dm9].peek ------------ skb_alloc.is= %d -------------------\n", RxLen + 4 + ASL_RSRV_RX_HDR_LEN);
	}
	
	if (dev->features & NETIF_F_RXCSUM) {
		if ((((rxbyte & 0x1c) << 3) & rxbyte) == 0)
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb_checksum_none_assert(skb);
	}
	
	rel_printk6("[DM9].netif_rx: skb->len %d\n", skb->len);
	printnb_packet6(skb->data, 32); 
	
	if (in_interrupt())
		netif_rx(skb);
	else
		netif_rx_ni(skb);

	dev->stats.rx_packets++;  
	return skb;
}

// [ xxxxxxxxxxxx ]
// [ scan and skb ]
// [ xxxxxxxxxxxx ]
#if 0
struct sk_buff *trans(struct net_device *dev, char * buffp, int RxLen, int SKB_size)
{
	board_info_t *db = netdev_priv(dev);     
	struct sk_buff *skb;
	u8 *rdptr;
	u8 rxbyte = db->bC.dummy_pad;
	
	//
	if ((skb = dev_alloc_skb(RxLen + 4 + ASL_RSRV_RX_HDR_LEN)) == NULL)  { // SKB_size= RxLen + 4
			printk("dm9051 [!ALLOC skb size %d fail]\n", RxLen + 4 + ASL_RSRV_RX_HDR_LEN);
			return 0;
	}
	skb_reserve(skb, 2);
	/* A pointer to the first byte of the packet */
	rdptr = (u8 *) skb_put(skb,  RxLen - 4);  // [save 4] SKB_size - 4 - ASL_RSRV_RX_HDR_LEN - 4
	//dm9051_inblk_noncpy(db, rdptr-1, RxLen); /*rdptr-1*/
	
	
	memcpy(rdptr, &db->sbuf[1], RxLen - 4); // [save 4] &sbufp[p]
	
	#if 1
	skb->protocol = eth_type_trans(skb, dev);  // [JJ found: skb->len -= 14]
	if (dev->features & NETIF_F_RXCSUM) {
		if ((((rxbyte & 0x1c) << 3) & rxbyte) == 0)
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb_checksum_none_assert(skb);
	}
	#endif
	
	if (in_interrupt())
		netif_rx(skb);
	else
		netif_rx_ni(skb);
	
	db->ndev->stats.rx_bytes += RxLen;
	db->ndev->stats.rx_packets++; 
	return skb;
}
#endif
															    
void list_all_scan(board_info_t *db, int nR, int nRR)
{                                                                                                                                        
	int i;
	//int p = 1;
	for (i=0; i< nRR; i++){
		if (i<nR)
		  printk(" (scan[%d]) %4x, sizeof(rxhdr) + XX XX, bnd-len 0x%x (Y)\n", i, db->mdra_regs[i], db->len_rxhdr_pkt[i]);
		  //printk(" (scan[%d]) %4x, sizeof(rxhdr) + %02x %02x, bnd-len 0x%x (Y)\n", i, db->mdra_regs[i], db->len_rxhdr_pkt[i]);
		else
		  printk(" (scan[%d]) %4x, sizeof(rxhdr) + XX XX, bnd-len 0x%x (Err)\n", i, db->mdra_regs[i], db->len_rxhdr_pkt[i]);
		  //printk(" (scan[%d]) %4x, sizeof(rxhdr) + %02x %02x, bnd-len 0x%x (Err)\n", i, db->mdra_regs[i], db->len_rxhdr_pkt[i]);
	}
	//printk("  scan_end %4x\n", db->mdra_reg_end);
	printk("  scan_end %4x\n", db->mdra_reg_end);
}
void list_all_headlist(char *headstr, int nR)
{
	int i;
	//.printk("dm9 %s scanx report: %dY(1Err)\n", headstr, nR); // "/%d", nRR //...b.bttrttr.....
	printnb("dm9 %s scanx report:", headstr);
	for (i=0; i< nR; i++)
		printnb(" Y");
	printnb(" Err");
	printnb("\n");
}
int dump_all_headlist(int nR, int nRR)
{
	int s,i;
	s = 0;
	if (nRR > 5) s= nRR - 5;
	printnb("dm9 [dump pkt] skbx report:");
	for (i=s; i< nRR; i++){
		if (i<nR)
			printnb(" Y");
		else
			printnb(" Err");
	}
	printnb("\n");
	return s;
}
void dump_all_skb(board_info_t *db, int s, int nR, int nRR)
{
	char *sbufp = db->sbuf;
	int i, p = 1;
	//struct dm9051_rxhdr rxhdr;   
	//int RxLen;
	//s = 0; //if (nRR > 5) s= nRR - 5;
	printnb_init(1);// here, not essential, since done in probe
	
	//calc p
	for (i=0; i< s; i++){
		printk("   skb[%d]  %4x, sizeof(rxhdr) + %02x %02x, bnd-len 0x%x (NOT_dump)\n", i, db->mdra_regs[i], sbufp[p+2], sbufp[p+3], db->len_rxhdr_pkt[i]);
		p += db->len_rxhdr_pkt[i];
	}
	
	for (i=s; i< nRR; i++){
		//[detect-check]
		if (i>nR) printk(" {Warn too more}");
		//[normal]
		//printk("   skb[%d]  %4x, sizeof(rxhdr) + %02x %02x, bnd-len 0x%x (Y)\n", i, db->mdra_regs[i], sbufp[p+2], sbufp[p+3], db->len_rxhdr_pkt[i]);
		//printk("   skb[%d]  %4x, sizeof(rxhdr) + %02x %02x, bnd-len 0x%x (Err)\n", i, db->mdra_regs[i], sbufp[p+2], sbufp[p+3], db->len_rxhdr_pkt[i]);
		printk("   skb[%d]  %4x, sizeof(rxhdr) + %02x %02x\n", i, db->mdra_regs[i], sbufp[p+2], sbufp[p+3]);
		if (i<nR) {
		  printk("   bnd-len 0x%x (Y)\n", db->len_rxhdr_pkt[i]);
		  printk("   inblk len 4 + 0x%x(= %d)\n", db->len_rxhdr_pkt[i]-RXHDR_SIZE, db->len_rxhdr_pkt[i]-RXHDR_SIZE);
		} else {
		  printk("   bnd-len 0x%x (Err)\n", db->len_rxhdr_pkt[i]);
		  printk("   inblk len 4 + 0x%x(= %d)\n", db->RdForLen, db->RdForLen);
		}
		  
		//RxLen = db->len_rxhdr_pkt[i] ; //instead.
		
		//memcpy((char *)&rxhdr, &sbufp[p], RXHDR_SIZE);
		//RxLen = rxhdr.RxLen;
		printnb_rx_fifo(&sbufp[p], RXHDR_SIZE,
			&sbufp[p+RXHDR_SIZE], db->len_rxhdr_pkt[i] - RXHDR_SIZE);
		//p += sizeof(rxhdr);
		p += db->len_rxhdr_pkt[i];
	}
}

static void read_mrrx(board_info_t *db, u16 *ptrmrr)
{
#if DEF_SPIRW	
	*ptrmrr= ior(db, DM9051_MRRL);
	*ptrmrr |= (u16)ior(db, DM9051_MRRH) << 8; 
#endif	
	db->mdra_reg_end = *ptrmrr; // save every-time
}

#ifndef QCOM_BURST_MODE
static void dbg_inblk(board_info_t *db, char *prevPtr, int rdforlen)
{
	char bf1 = *prevPtr; //sbuff[p-1];
	dm9051_inblk_noncpy(db, prevPtr, rdforlen); /*rdptr-1= &sbuff[p-1]= */
	*prevPtr = bf1;
}
#endif

static int nRRMAX = 0;
static int dm9051_scanx(board_info_t *db, int nMax)
{
	int nRR;
	int nR = 0, scanRR = 0; //[note: 'nR' is totally equal to 'scanRR']
	char *sbuff = db->sbuf;
	int p = 1;
	int pns = 1; //next start
	struct dm9051_rxhdr *prxhdr; //struct spi_rxhdr spirxhdr;
	
	int RdForLen, RxLen;
	u8 rxbyte;
	#ifndef QCOM_BURST_MODE
	char bf1;
	#endif
	
	db->bC.isbyte= ior(db, DM9051_ISR);	// Got ISR	
	db->bC.isr_clear = 0;

	read_mrrx(db, &db->mdra_regs[nR]);
	while(scanRR < nMax) {
		rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */
		rxbyte= ior(db, DM_SPI_MRCMDX);	/* Dummy read */
		if (rxbyte != DM9051_PKT_RDY){
			if (!db->bC.isr_clear)
				iow(db, DM9051_ISR, 0xff);
			break; //exhaust-empty
		}
		db->bC.dummy_pad = rxbyte;
	if ((p+RXHDR_SIZE) > (SCAN_LEN_HALF+1)){
	  printk("dm9 p+RXHDR_SIZE over-range\n");
	  break; //protect
	}
	  
		//dm9051_inblk_noncpy(db, &spirxhdr.spiwb, RXHDR_SIZE);
		#ifdef QCOM_BURST_MODE
		dm9051_inblk_noncpy(db, &sbuff[p], RXHDR_SIZE);
		#else
		bf1 = sbuff[p-1];
		dm9051_inblk_noncpy(db, &sbuff[p-1], RXHDR_SIZE);
		sbuff[p-1] = bf1;
		#endif
		iow(db, DM9051_ISR, 0xff);
		db->bC.isr_clear = 1;
		
		prxhdr = (struct dm9051_rxhdr*) &sbuff[p];
		RxLen = prxhdr->RxLen;
		p += RXHDR_SIZE;  
	if ((p+RxLen) > (SCAN_LEN_HALF+1)) {
	  printk("dm9 warn reach, p+RxLen over SCAN_LEN_HALF+1 (%d > %d)\n", p+RxLen, SCAN_LEN_HALF+1);
	  //break; //protect
	  //[return 0]
	  printk("dm9-pkt-Wrong RxLen and range (RxLen %x= %d range %d)\n", RxLen, RxLen, DM9051_PKT_MAX);
	  //[inblk the substead rx-len]
	  RdForLen = SCAN_LEN_HALF - p; //read-for-len, instead
	  printk("dm9-read-for-len, instead %x = %d\n", RdForLen, RdForLen);
	  goto inblk_dump;
	}
	if (RxLen > DM9051_PKT_MAX){
	  int s; //added
	  printk("dm9-pkt-Wrong RxLen over-range (%x= %d > %x= %d)\n", RxLen, RxLen, DM9051_PKT_MAX, DM9051_PKT_MAX);
	  //break;
	  RdForLen = DM9051_PKT_MAX; //read-for-len, instead
	  printk("dm9-read-for-len, instead %x = %d\n", RdForLen, RdForLen);
inblk_dump:

	  printnb_rx_fifo(&sbuff[p-RXHDR_SIZE], RXHDR_SIZE, //for-dbg-check
			&sbuff[p-RXHDR_SIZE], RXHDR_SIZE);
			
	  //[inblk this and dump-list]
	  db->RdForLen = RdForLen;
	  #ifdef QCOM_BURST_MODE
	  dm9051_inblk_noncpy(db, &sbuff[p], RdForLen);
	  #else
	  dbg_inblk(db, &sbuff[p-1], RdForLen); //=rdptr-1 =&sbuff[p-1]
	  #endif
	  
	  printnb_rx_fifo(&sbuff[p], 24, //for-dbg-check
			&sbuff[p], 24);
			
	  db->len_rxhdr_pkt[nR] = RXHDR_SIZE + RxLen;
	  
	  //nR++;
	  nRR = scanRR + 1;
	  read_mrrx(db, &db->mdra_regs[nRR]);
	  
	  //[dump-here]
				do {         
					  char hstr[72];
					  sprintf(hstr, "dmfifo_reset( 11 RxLenErr ) mac %02x %02x %02x %02x rxhdr %02x %02x %02x %02x (quick)", 
						sbuff[p], sbuff[p+1], sbuff[p+2], sbuff[p+3],
						sbuff[p-4], sbuff[p-3], sbuff[p-2], sbuff[p-1]);
					db->bC.ERRO_counter++;
					dm9051_fifo_reset(11, hstr, db);
					dm9051_fifo_reset_statistic(db);
				} while (0);
				
				/* debug */
				list_all_headlist("[RxLenerr]", nR); //printk("dm9 [RxLenerr] .....
				printk("dm9 mdra:\n");
				list_all_scan(db, nR, nRR/*nRR*/);
				
				s = dump_all_headlist(nR, nRR/*nRR*/);
				
				printk("dm9 mdra:\n");
				dump_all_skb(db, s, nR, nRR/*nRR*/);
				
				read_mrrx(db, &db->mdra_regs[0]);
				printk("dm9 reset-done:\n");
				printk(" RxLenErr&MacOvrSft_Er %d, RST_c %d\n", db->bC.ERRO_counter, db->bC.DO_FIFO_RST_counter);
				printk("dm9 mdra:\n");
				printk(" ncr-reset %4x\n", db->mdra_reg_end);
	  //[return 0]
	  db->sbuf[1] = 0; //for skip dm9051_skbx()
	  nRRMAX = 0; //re-again
	  //[p += RxLen;] //Such as for normal next for normal inblk, But everything thing NCR resrt.
	  return 0;
	}
	  
		#ifdef QCOM_BURST_MODE
		  dm9051_inblk_noncpy(db, &sbuff[p], RxLen); /*rdptr*/
		#else
		  bf1 = sbuff[p-1];
		  dm9051_inblk_noncpy(db, &sbuff[p-1], RxLen); /*rdptr-1*/
		  sbuff[p-1] = bf1;
		 #endif
			iow(db, DM9051_ISR, 0xff);
			db->bC.isr_clear = 1;
		  
		  p += RxLen;
		  db->len_rxhdr_pkt[nR] = RXHDR_SIZE + RxLen;

		  nR++;
		  scanRR = nR;
		  read_mrrx(db, &db->mdra_regs[nR]);
		  
		  pns = p;
	}
	sbuff[pns] = 0; // in case, to not 0x01.
	return scanRR;
}
static void dm9051_dbg_scanx(int nRR)
{
	if (nRR && (nRRMAX < nRR)) {
		nRRMAX = nRR;
		printk("------------------ dm9 scanx nRRMAX %d -----------\n", nRRMAX);
		if (nRRMAX == NUM_SCANX)
		  printk("--------------------------------------------------\n");
	}
}
static int dm9051_skbx(board_info_t *db, int nRR)
{
	char *sbufp = db->sbuf;
	int p = 1;
	int s,nR = 0;
	int skbRR = 0, RxLen;
	u8 *rdptr;
	struct sk_buff *skb;
	struct dm9051_rxhdr rxhdr;   
	
	while(sbufp[p] == DM9051_PKT_RDY)
	{                                                    
		db->bC.dummy_pad = sbufp[p];
		memcpy((char *)&rxhdr, &sbufp[p], RXHDR_SIZE);
		RxLen = rxhdr.RxLen;
		         
		p += sizeof(rxhdr);          
		if (!SB_skbing_packet_chk_data(db, &sbufp[p-sizeof(rxhdr)], &sbufp[p])) { //(u8 *) //(u8 *) 
				db->RdForLen = (u16) RxLen;
				do {         
					  char hstr[72]; //dump_all_skb
					  sprintf(hstr, "dmfifo_reset( 11 macErr ) mac %02x %02x %02x %02x rxhdr %02x %02x %02x %02x (quick)", 
						sbufp[p], sbufp[p+1], sbufp[p+2], sbufp[p+3],
						sbufp[p-4], sbufp[p-3], sbufp[p-2], sbufp[p-1]);
					db->bC.ERRO_counter++;
					dm9051_fifo_reset(11, hstr, db);
					dm9051_fifo_reset_statistic(db);
				} while (0);
				
				/* debug */
				list_all_headlist("[mac err]", nR); //printk("dm9 [mac err] .....
				printk("dm9 mdra:\n");
				list_all_scan(db, nR, nRR);
				
				//dump	
				s = dump_all_headlist(nR, nRR);
				//dump	
				printk("dm9 mdra:\n");
				dump_all_skb(db, s, nR, nRR);
				//printk("dm9 dump summary: %dY(+%d NG_PKT)\n", nR, nRR - nR);
				
				//new
				read_mrrx(db, &db->mdra_regs[0]);
				printk("dm9 reset-done:\n");
				printk(" MacOvrSft_Er %d, RST_c %d\n", db->bC.ERRO_counter, db->bC.DO_FIFO_RST_counter);
				printk("dm9 mdra:\n");
				printk(" ncr-reset %4x\n", db->mdra_reg_end);
				nRRMAX = 0; //re-again
				return skbRR; //nR;
		}
		
#ifdef ASL_RSRV_RX_HDR		
		if ((skb = dev_alloc_skb(RxLen + 4 + ASL_RSRV_RX_HDR_LEN)) == NULL)  
#else
		if ((skb = dev_alloc_skb(RxLen + 4)) == NULL) // SKB_size= RxLen + 4
#endif
		{
				printk("dm9051 [!ALLOC skb size %d fail]\n", RxLen + 4);
				return skbRR; //nR;
		}
		skb_reserve(skb, 2);
		rdptr = (u8 *) skb_put(skb,  RxLen - 4);  // [save 4] SKB_size - 4 - ASL_RSRV_RX_HDR_LEN - 4
		memcpy(rdptr, &sbufp[p], RxLen - 4); // [save 4] &sbufp[p]
		#if 1
		skb->protocol = eth_type_trans(skb, db->ndev);  // [JJ found: skb->len -= 14]
		if (db->ndev->features & NETIF_F_RXCSUM) {
			//if ((((rxbyte & 0x1c) << 3) & rx, int nRRbyte) == 0)
			//	skb->ip_summed = CHECKSUM_UNNECESSARY;
			//else
				skb_checksum_none_assert(skb);
		}
		#endif
		if (in_interrupt())
			netif_rx(skb);
		else
			netif_rx_ni(skb);
			
		db->ndev->stats.rx_bytes += RxLen;
		db->ndev->stats.rx_packets++; 
		
		p += RxLen ;
		nR++;
		skbRR = nR;
	}
	return skbRR;
}
//[dm9051_isr_ext2(db)= dm9051_ScanSkbX(db, nMax)]
static int dm9051_isr_ext2(board_info_t *db)
{
	int nR = dm9051_scanx(db, NUM_SCANX);
	dm9051_dbg_scanx(nR);
	return dm9051_skbx(db, nR);
}
#endif
