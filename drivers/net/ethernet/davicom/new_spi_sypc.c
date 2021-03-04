// {1 ~ 3244}

// [1 - 60]

//[debug_as/board_info_1.h]
//[../new_load/sched.c]
//[../new_load/driver_ops.c]        
//[debug_as/driver_ops1.c]
//[../new_load/driver.c]
//[../new_load/spi.c]
//[../new_load/control_sub.c]
//[../new_load/tx.c]
//[../new_load/skb_rx_head.c]
//[../new_load/skb_rx_core.c]
							     
//[../new_load/rx.c]
//[../new_load/control_obj.c]
//[debug_as/eth_1.c]

//----------------------------------------------------------------------------------------

//[debug_as/board_info_1.h]
//
//
// SPI_DM9051.H
// SUB_DM9051.H
//
//
	       
//[APIs definitions] 
  //of //("driver.c") also ("spi.c"or"library.c")
  //of //("spi_user") also ("spi_dm9051.c")
#if DEF_SPIRW
typedef struct cb_info_t {
	/*[int (*xfer)(board_info_t *db unsigned len);]*/ //by.= /'dmaXFER'
        /*[int (*xfer)(board_info_t *db, u8 *txb, u8 *rxb, unsigned len);]*/ //by.= /'stdXFER'
         u8 (*iorb)(board_info_t *db, unsigned reg);
         void (*iowb)(board_info_t *db, unsigned reg, unsigned val);
         void (*inblk_defcpy)(board_info_t *db, u8 *buff, unsigned len, bool need_read); // 1st byte is the rx data.
         void (*inblk_noncpy)(board_info_t *db, u8 *buff, unsigned len); // reserve 1 byte in the head.
         int (*outblk)(board_info_t *db, u8 *buff, unsigned len);
	 /*
	 //struct spi_transfer Tfer;
	 //struct spi_message Tmsg; 
	 //struct spi_transfer *fer;
	 //struct spi_message *msg;
	 */
} cb_info;

static cb_info dm9;
#endif

  //of //("driver.c") also ("spi.c"or"library.c")
  //of //("spi_user") also ("spi_dm9051.c")
#if DEF_SPICORE
#define ior					dm9.iorb
#define iior					dm9.iorb
#define iow					dm9.iowb
#define iiow					dm9.iowb
#define dm9051_outblk				dm9.outblk
//#define dm9051_inblk_rxhead(d,b,l)		dm9.inblk_defcpy(d,b,l,true)
#define dm9051_inblk_noncpy(d,b,l)		dm9.inblk_noncpy(d,b,l)
//#define dm9051_inblk_dump(d,l)			dm9.inblk_defcpy(d,NULL,l,false)
#endif

// [1164 - 1206]

//[../new_load/spi.c]

//
// "spi.c"or"library", original spi_dm9051.c
//

//[Usage definitions]
//Usage
#define dmaXFER  dma_spi_xfer_buf
#define stdXFER  std_spi_xfer_buf

#ifndef RPI_CONF_SPI_DMA_YES
#define dm9051_space_alloc  std_space_request  //#define dm9051_space_request std_space_request
#define dm9051_space_free    std_space_free
#endif


#if DEF_SPIRW
#define dm9051_spirw_begin(d) dm9051_space_alloc(d)
#define dm9051_spirw_end(d)      dm9051_space_free(d)
#else
#define dm9051_spirw_begin(d)  // Never called, only called while define _DEF_SPIRW in above if-condition.
#define dm9051_spirw_end(d)  // Essentially called.
#endif


#if DMA3_P2_RSEL_1024F
//#define stdRX	std_read_rx_buf_1024
#else
 #ifdef QCOM_RX_DWORD_BOUNDARY
#define stdRX	std_read_rx_buf_ncpy_dword_boundary
 #else
#define stdRX	std_read_rx_buf_ncpy
#endif
#endif

#if DMA3_P2_TSEL_1024F
//#define stdTX 	std_write_tx_buf_1024
#else
 #ifdef QCOM_TX_DWORD_BOUNDARY
 #define stdTX 	std_write_tx_buf_dword_boundary
 #else
 #define stdTX 	std_write_tx_buf
 #endif
#endif


// [1220 - 1487]
// [1488 - 1555]

#if DEF_SPIRW
static int std_spi_xfer_buf(board_info_t *db, u8 *txb, u8 *rxb, unsigned len)
{
        int ret;
#if 1 //'DEF_SPICORE_IMPL1'
#ifdef QCOM_BURST_MODE
	db->spi_xfer2[0].tx_buf = txb;
	db->spi_xfer2[0].rx_buf = NULL;
	db->spi_xfer2[0].len = 1;
	//db->spi_xfer2[0].cs_change = 0;
	if (rxb==NULL)
	{
	db->spi_xfer2[1].tx_buf = txb+1;
	db->spi_xfer2[1].rx_buf = NULL;
	db->spi_xfer2[1].len = len;
	}
	else
	{
	db->spi_xfer2[1].tx_buf = txb+1;
    #ifdef QCOM_BURST_MODE
	db->spi_xfer2[1].rx_buf = rxb; //from [db->spi_xfer2[1].rx_buf = rxb+1];
    #endif
	db->spi_xfer2[1].len = len;
	}
	//db->spi_xfer2[1].cs_change = 0;
	ret = spi_sync(db->spidev, &db->spi_msg2);
#else
	db->fer->tx_buf = txb;
	db->fer->rx_buf = rxb;
	db->fer->len = len + 1;
	db->fer->cs_change = 0;
	ret = spi_sync(db->spidev, db->msg);
#endif
#endif
        if (ret < 0) {
		dbg_log("spi communication fail! ret=%d\n", ret);
        }
        return ret;
}
#endif

#if DEF_SPIRW
static int disp_std_spi_xfer_Reg(board_info_t *db, unsigned reg)
{
        int ret = 0;
        if (reg == DM9051_PIDL || reg == DM9051_PIDH ) {
                printk("dm905.MOSI.p.[%02x][..]\n",reg); 
        }
        if (reg == DM9051_PIDL || reg == DM9051_PIDH ) {
                printk("dm905.MISO.e.[..][%02x]\n", db->spi_sypc_buf[1]);  //'TxDatBuf'
        }
        return ret;
}
#endif

#if DEF_SPIRW
static u8 std_spi_read_reg(board_info_t *db, unsigned reg)
{
        u8 txb[2] = {0};
        u8 rxb[2] = {0};

        txb[0] = (DM_SPI_RD | reg);
        stdXFER(db, (u8 *)txb, rxb, 1); //cb.xfer_buf_cb(db, (u8 *)txb, rxb, 1); //std_spi_xfer_buf(db, (u8 *)txb, rxb, 1); //'dm9051_spi_xfer_buf'
#ifdef QCOM_BURST_MODE
  db->spi_sypc_buf[1] = rxb[0];
#else        
  db->spi_sypc_buf[1] = rxb[1]; //.std.read_reg //'TxDatBuf'
#endif  
  disp_std_spi_xfer_Reg(db, reg);
        //return rxb[1];
	return db->spi_sypc_buf[1];
}
static void std_spi_write_reg(board_info_t *db, unsigned reg, unsigned val)
{
        u8 txb[2] = {0};
        //if (!enable._dma) {
        //}
        txb[0] = (DM_SPI_WR | reg);
        txb[1] = val;
        stdXFER(db, (u8 *)txb, NULL, 1); //cb.xfer_buf_cb(db, (u8 *)txb, NULL, 1); //std_spi_xfer_buf(db, (u8 *)txb, NULL, 1); //'dm9051_spi_xfer_buf'
}

static void std_read_rx_buf(board_info_t *db, u8 *buff, unsigned len, bool need_read)
{
        //[this is for the (SPI_SYNC_TRANSFER_BUF_LEN - 1)_buf application.]
        unsigned one_pkg_len;
        unsigned remain_len = len, offset = 0;
        u8 txb[1];
        txb[0] = DM_SPI_RD | DM_SPI_MRCMD;
        do {
                // 1 byte for cmd
                if (remain_len <= (SPI_SYNC_TRANSFER_BUF_LEN - 1)) {
                        one_pkg_len = remain_len;
                        remain_len = 0;
                } else {
                        one_pkg_len = (SPI_SYNC_TRANSFER_BUF_LEN - 1);
                        remain_len -= (SPI_SYNC_TRANSFER_BUF_LEN - 1);
                }

                stdXFER(db, txb, db->spi_sypc_buf, one_pkg_len); //cb.xfer_buf_cb(db, txb, db->TxDatBuf, one_pkg_len); //std_spi_xfer_buf(db, txb, db->TxDatBuf, one_pkg_len); //'dm9051_spi_xfer_buf'
                if (need_read) {
			#ifdef QCOM_BURST_MODE
			memcpy(buff + offset, &db->spi_sypc_buf[0], one_pkg_len); 
			#else
                        memcpy(buff + offset, &db->spi_sypc_buf[1], one_pkg_len); //if (!enable._dma)//.read_rx_buf //'TxDatBuf'
			#endif
                        offset += one_pkg_len;
                }
        } while (remain_len > 0);
}
#endif

#if DEF_SPIRW
#if DEF_PRO 
  //&& DEF_SPIRW
  //&& DM_CONF_APPSRC
#if DMA3_P2_RSEL_1024F
/*static void std_read_rx_buf_1024(board_info_t *db, u8 *buff, unsigned len)
{
        //[this is for the 1024_buf application.(with copy operations)][It's better no-copy]
	u8 txb[1];
	int const pkt_count = (len + 1) / CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	int const remainder = (len + 1) % CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	//.if((len + 1)>CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES){	
		txb[0] = DM_SPI_RD | DM_SPI_MRCMD;
		if (pkt_count) {
			int blkLen;
			//(1)
			blkLen= CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count - 1; // minus 1, so real all is 1024 * n
			stdXFER(db, txb, db->spi_sypc_buf, blkLen);
			memcpy(&buff[1], &db->spi_sypc_buf[1], blkLen);
	        //.printk("dm9rx_EvenPar_OvLimit(%d ... \n", blkLen);
			//(1P)
			if (remainder) {
			  //.blkLen= remainder;
			  stdXFER(db, txb, db->spi_sypc_buf, remainder);
			  memcpy(buff + (CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count), &db->spi_sypc_buf[1], remainder);
		//.printk("dm9rx_EvenPar_OvRemainder(%d ... \n", blkLen);
			}
			return;
		}
		//(2)	
		if (remainder) {
			//stdXFER(db, txb, db->spi_sypc_buf, remainder-1);
			//memcpy(&buff[1], &db->spi_sypc_buf[1], remainder-1);
			//note: len= remainder-1
			stdXFER(db, txb, buff, len);
		}
		return;
	//.}
}*/
#else
#ifdef QCOM_RX_DWORD_BOUNDARY
static void std_read_rx_buf_ncpy_dword_boundary(board_info_t *db, u8 *buff, unsigned len)
{                     
				
        unsigned remain_len = len;
	unsigned offset = 0;
	
#define INTNL_4N1_CODE	1 
	
#ifdef INTERNAL_ONEBYTE_SPI_SYNC
#undef INTNL_4N1_CODE
#define INTNL_4N1_CODE	0 
#endif
	
#if INTNL_4N1_CODE			       
	unsigned pkg_len= len;
	if ((pkg_len+1) >= 4) {		
		u8 txbf[1];
		pkg_len = ((pkg_len + 1) >> 2) << 2; //[new for dword boundary]
		pkg_len--;
		//pkg_len = ((((pkg_len + 1) + 3) /4 )*4) - 4; //[new for dword boundary]
		//pkg_len  -= 1;
		
		//[do.here]
		txbf[0]= DM_SPI_RD | DM_SPI_MRCMD;
		stdXFER(db, txbf,& buff[offset], pkg_len);          
		
		remain_len -= pkg_len;
		offset += pkg_len;
	}
#endif	
	
	while(remain_len > 0) {                                        
		u8 txb[2] = {0};
		u8 rxb[2] = {0};
		txb[0] = DM_SPI_MRCMD; //(DM_SPI_RD | reg);
		stdXFER(db, (u8 *)txb, rxb, 1); 
	#ifdef QCOM_BURST_MODE
		buff[offset++] = rxb[0];
	#else	
		buff[++offset] = rxb[1];
	#endif
		remain_len--;
	}
}
#else   //QCOM_RX_DWORD_BOUNDARY
static void std_read_rx_buf_ncpy(board_info_t *db, u8 *buff, unsigned len)
{
        //[this is for the 0_buf application.][It's no-copy]
        u8 txb[1];
        txb[0] = DM_SPI_RD | DM_SPI_MRCMD;
        stdXFER(db, txb, buff, len);
}
#endif
#endif

#endif
#endif

#if DEF_SPIRW
#if DEF_PRO 
  //&& DEF_SPIRW
  //&& DM_CONF_APPSRC
  
#if DMA3_P2_TSEL_1024F
/*static int std_write_tx_buf_1024(board_info_t *db, u8 *buff, unsigned len)
{
	int blkLen;
	int const pkt_count = (len + 1)/ CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	int const remainder = (len + 1)% CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES;
	unsigned offset = 0;
	
	if((len + 1)>CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES){
		//(1)
		blkLen= CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES*pkt_count - 1;
		db->spi_sypc_buf[0] = DM_SPI_WR | DM_SPI_MWCMD;
		memcpy(&db->spi_sypc_buf[1], &buff[offset], blkLen);
                offset += blkLen;
		stdXFER(db, db->spi_sypc_buf, NULL, blkLen);
        //.printk("dm9tx_std_EvenPar_OvLimit(%d ... \n", blkLen);

		//(2)	
		blkLen= remainder;
		memcpy(&db->spi_sypc_buf[1], &buff[offset], blkLen);
                //offset += blkLen;
		stdXFER(db, db->spi_sypc_buf, NULL, blkLen);
        //.printk("dm9tx_std_EvenPar_OvRemainder(%d ... \n", blkLen);
	} else {
		db->spi_sypc_buf[0] = DM_SPI_WR | DM_SPI_MWCMD;
		memcpy(&db->spi_sypc_buf[1], buff, len);
		stdXFER(db, db->spi_sypc_buf, NULL, len);
	}
   return 0;
}*/
#else
#ifdef QCOM_TX_DWORD_BOUNDARY
static int std_write_tx_buf_dword_boundary(board_info_t *db, u8 *buff, unsigned len)
{
        unsigned remain_len = len;
        unsigned pkg_len, offset = 0;
	
//.	printk("[dm9][tx %d, dword-bound] %02x %02x %02x %02x %02x %02x", 
		//remain_len, buff[0], buff[1], buff[2], buff[3], buff[4], buff[5]);
	
        do {
                // 1 byte for cmd
                if (remain_len <= (SPI_SYNC_TRANSFER_BUF_LEN - 1)) {
			
                        pkg_len = remain_len;
			
                } else {
			
                        pkg_len = (SPI_SYNC_TRANSFER_BUF_LEN - 1);
			
                }
		
		if ((pkg_len+1) < 4) {
			pkg_len = 1;
		} else {

			pkg_len = ((pkg_len + 1) >> 2) << 2; //[new for dword boundary]
			pkg_len--;
			//pkg_len = ((((pkg_len + 1) + 3) /4 )*4) - 4; //[new for dword boundary]
			//pkg_len  -= 1;
		}
		
		remain_len -= pkg_len;

                db->spi_sypc_buf[0] = DM_SPI_WR | DM_SPI_MWCMD; //if (!enable._dma) { } //'TxDatBuf'
                memcpy(&db->spi_sypc_buf[1], buff + offset, pkg_len); //'TxDatBuf'
                
    //.if (!remain_len && (pkg_len!=len)){
    	//.switch (pkg_len){
    	//.	case 3:
    	//.		printk(" %02x %02x %02x", buff[offset], buff[offset+1], buff[offset+2]);
    	//.		break;
    	//.	case 1:
    	//.		printk(" %02x", buff[offset]);
    	//.		break;
    	//.}
    	//.printk(" [end.t.xfer %d]", pkg_len);
    //.} else {
    	//.switch (pkg_len){
    	//.	case 1:
    	//.		printk(" %02x", buff[offset]);
    	//.		break;
    	//.}
			//.printk(" [t.xfer %d]", pkg_len);
		//.}
                
                offset += pkg_len;
                stdXFER(db, db->spi_sypc_buf, NULL, pkg_len); //cb.xfer_buf_cb(db, db->TxDatBuf, NULL, pkg_len); //std_spi_xfer_buf(db, db->TxDatBuf, NULL, pkg_len); //'dm9051_spi_xfer_buf'
        } while (remain_len > 0);
	//.printk("\n");
        return 0;
}
#endif
#ifndef QCOM_TX_DWORD_BOUNDARY
static int std_write_tx_buf(board_info_t *db, u8 *buff, unsigned len)
{
        unsigned remain_len = len;
        unsigned pkg_len, offset = 0;
        do {
                // 1 byte for cmd
                if (remain_len <= (SPI_SYNC_TRANSFER_BUF_LEN - 1)) {
                        pkg_len = remain_len;
                        remain_len = 0;
                } else {
                        pkg_len = (SPI_SYNC_TRANSFER_BUF_LEN - 1);
                        remain_len -= (SPI_SYNC_TRANSFER_BUF_LEN - 1);
                }

                db->spi_sypc_buf[0] = DM_SPI_WR | DM_SPI_MWCMD; //if (!enable._dma) { } //'TxDatBuf'
                memcpy(&db->spi_sypc_buf[1], buff + offset, pkg_len); //'TxDatBuf'
                
                offset += pkg_len;
                stdXFER(db, db->spi_sypc_buf, NULL, pkg_len); //cb.xfer_buf_cb(db, db->TxDatBuf, NULL, pkg_len); //std_spi_xfer_buf(db, db->TxDatBuf, NULL, pkg_len); //'dm9051_spi_xfer_buf'

        } while (remain_len > 0);
        return 0;
}
#endif
#endif

#endif
#endif

void callback_setup(int dma_bff)
{
#ifdef RPI_CONF_SPI_DMA_YES 
        //enable_dma = dma_bff;
        //if (enable_dma) {
        //        dm9.iorb= dma_spi_read_reg;
        //        dm9.iowb= dma_spi_write_reg;
        //        dm9.inblk_defcpy= dma_read_rx_buf;  // 1st byte is the rx data.
        //        dm9.inblk_noncpy= dmaRX; // reserve 1 byte in the head. // dma_ with_ ncpy_
        //        dm9.outblk= dmaTX;
        //} else {
        //        dm9.iorb= std_spi_read_reg;
        //        dm9.iowb= std_spi_write_reg;
        //        dm9.inblk_defcpy= std_read_rx_buf;  // 1st byte is the rx data.
         //       dm9.inblk_noncpy= stdRX;
         //       dm9.outblk= stdTX;
        //}
#else
        #if DEF_SPIRW
        dm9.iorb= std_spi_read_reg;
        dm9.iowb= std_spi_write_reg;
        dm9.inblk_defcpy= std_read_rx_buf;  // 1st byte is the rx data.
        dm9.inblk_noncpy= stdRX;
        dm9.outblk= stdTX;
        #endif
#endif
}

static int  std_alloc(struct board_info *db)
{
        #ifdef RPI_CONF_SPI_DMA_YES
        printk("[ *dm9051 DRV ] spi mode[= std] using 'enable_dma' is 0\n");
        printk("[ *dm9051 DRV ] spi mode[= dma] But using kmalloc, TxDatBuf[] or std_alloc TxDatBuf\n"); //ADD.JJ
        #else
        //.printk("[ *dm9051 DRV ] spi mode[= std] using kmalloc, TxDatBuf[] or std_alloc TxDatBuf\n"); //ADD.JJ
        #endif

	#if DEF_SPIRW
        db->spi_sypc_buf = kmalloc(SPI_SYNC_TRANSFER_BUF_LEN, GFP_ATOMIC);
        
        if (!db->spi_sypc_buf)
                return -ENOMEM;
	#endif
		
        return 0; // no-Err
}

static void  std_free(struct board_info *db)
{
	     //.printk("[dm951_u-probe].s ------- Finsih using kfree, param (db->spi_sypc_buf) -------\n");  //ADD.JJ //'TxDatBuf'
		
		#if DEF_SPIRW
                kfree(db->spi_sypc_buf);
		#endif
}
    
static int std_space_request(struct board_info *db)
{
        callback_setup(0); // assign 0 to 'enable_dma'     
        /* Alloc non-DMA buffers */
        return std_alloc(db);
}
static void std_space_free(struct board_info *db)
{
        /* Free non-DMA buffers */
         std_free(db);
}

// --------------------------------------------------------------------------------------
// [ 91 - 110 ]

//
//
// board_info.c soure code.
//
//	
		 
#if DM_CONF_APPSRC
/*
 *  init (AppSrc)
 */
static void bcopen_rx_info_clear(struct rx_ctl_mach *pbc)
{
	pbc->rxbyte_counter= 
	pbc->rxbyte_counter0= 
	
	pbc->rxbyte_counter0_to_prt= 
#if 0	
	pbc->rx_brdcst_counter= 
	pbc->rx_multi_counter= 
#endif	
	pbc->rx_unicst_counter= 0;
	
	pbc->isbyte= 0xff; // Special Pattern
}
#endif

// ----------------------------------

// [ 74 - 90 ]

//
//
// spi_user.c
//
//     
		    
void Fifo_Reset_Disp_RunningEqu_Ending(board_info_t *db)
{
#if 0
	char *s;
	if (db->bC.rxbyte_counter0_to_prt >= 2)
	{
	  if ((db->_rwregs[0]!=db->_rwregs[1]) && (db->bC.isbyte & ISR_PRS))
		s= "(---Accumulat times---)Rare-case";
	  else if (db->_rwregs[0]!=db->_rwregs[1])
		s= "(---Accumulat times---)Diff";
	  else
		s= "(---Accumulat times---)Impossible";

	  printk("dm9-IOR wrRd %04x/%04x (RO %d.%d%c) ISR %02x rxb= %02x %s (%2d ++)\n",
	    db->_rwregs[0], db->_rwregs[1], db->calc>>8, db->calc&0xFF, '%', db->bC.isbyte, db->bC.dummy_pad, s,
	    db->bC.rxbyte_counter0_to_prt);
	}
#endif	
}

// [ 201 - 469 ]

#if DEF_OPE 
#endif

#if DEF_SPIRW
static void
reset_rst(board_info_t * db) {	
	iiow(db, DM9051_NCR, NCR_RST);
	//= 
	//__le16 txb[2]; 
	 // wbuff(DM_SPI_WR| DM9051_NCR | NCR_RST<<8, txb); //org: wrbuff(DM9051_NCR, NCR_RST, txb)
	 // xwrbyte(db, txb);
	mdelay(1);
}
static void
reset_bnd(board_info_t * db) {
	iiow(db, DM9051_MBNDRY, MBNDRY_BYTE);
	//= 
	//.__le16 txb[2]; 
	 // wbuff(DM_SPI_WR| 0x5e | 0x80<<8, txb); //org: wrbuff(DM9051_NCR, NCR_RST, txb)
	 // xwrbyte(db, txb);
	mdelay(1);
	//printk("iow[%02x %02x]\n", 0x5e, 0x80); //iow[x].[Set.MBNDRY.BYTEBNDRY]
}
#endif

static void
dm9051_reset(board_info_t * db)
{
	  mdelay(2); // delay 2 ms any need before NCR_RST (20170510)
	  #if DEF_SPIRW
	  reset_rst(db);
	  reset_bnd(db);
	  #endif      
	  #if DM_RX_HEAVY_MODE
	  db->rwregs1 = 0x0c00;
	  #endif
	  //[db->validlen_for_prebuf = 0;]
}       

#if DM_CONF_APPSRC
// ------------------------------------------------------------------------------
// state: 0 , fill '90 90 90 ...' e.g. dm9051_fi.fo_re.set(0, "fifo-clear0", db);
//		  1 , RST
//        2 , dump 'fifo-data...'
//		 11 , RST-Silent
// hstr:  String 'hstr' to print-out
//        NULL (no 'hstr' print)
// Tips: If (state==1 && hstr!=NULL)
//        Do increase the RST counter
// ------------------------------------------------------------------------------
static void dm9051_fifo_reset(u8 state, u8 *hstr, board_info_t *db)
{
	u8 pk;
	if (state==11)
	{
		if (hstr)
     		{
			#if DM_RX_HEAVY_MODE
			db->rx_rst_quan = 0;
			#endif
			++db->bC.DO_FIFO_RST_counter;
			Fifo_Reset_Disp_RunningEqu_Ending(db);
#ifdef ON_RELEASE
			rel_printk2("dm9-%s Len %03d RST_c %d\n", hstr, db->bC.RxLen, db->bC.DO_FIFO_RST_counter);
#else
			printk("dm9-%s Len %03d RST_c %d\n", hstr, db->bC.RxLen, db->bC.DO_FIFO_RST_counter); //printlog
#endif
		}
		dm9051_reset(db);	
		#if DEF_SPIRW
		iiow(db, DM9051_FCR, FCR_FLOW_ENABLE);	/* Flow Control */
		iiow(db, DM9051_PPCR, PPCR_SETTING); /* Fully Pause Packet Count */
#ifdef DM_CONF_POLLALL_INTFLAG
		#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE

			//#if defined DM_CONF_INTERRUPT_TEST_DTS_FALLING
			//	iiow(db, DM9051._INTCR, 0x01); //low active
			if (db->irq_type & (IIRQ_TYPE_LEVEL_HIGH | IIRQ_TYPE_EDGE_RISING) )
				iiow(db, DM9051_INTCR, 0x00); //high active(default)
				//iiow(db, DM9051._INTCR, 0x01);  //test
			//#elif defined DM_CONF_INTERRUPT_TEST_DTS_RISING
			//	iiow(db, DM9051._INTCR, 0x00); //high active(default)
			else
				iiow(db, DM9051_INTCR, 0x01); //low active
				//iiow(db, DM9051._INTCR, 0x00); //test
			//#endif
			
		#else
			#if (DRV_IRQF_TRIGGER == IRQF_TRIGGER_LOW)
			iiow(db, DM9051_INTCR, 0x01); //low active
			#else
			iiow(db, DM9051_INTCR, 0x00); //high active(default)
			#endif
		#endif
		//.iiow(db, DM9051_IMR, IMR_PAR | IMR_PTM | IMR_PRM); //...
#else
	     	//.iiow(db, DM9051_IMR, IMR_PAR);
#endif
		//..iiow(db, DM9051_IMR, IMR_PAR); //=int_reg_stop(db); 
	     	//iiow(db, DM9051_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
	     	//iiow(db, DM9051_RCR, db->rcr_all);
	     	#endif
		bcopen_rx_info_clear(&db->bC);
		DM9051_fifo_reset_flg = 1;
	     	return; 
     }
     if (state==1 || state==2 || state==3 || state==5)
     {
		if (hstr)
		{
			#if DM_RX_HEAVY_MODE
			db->rx_rst_quan = 0;
			#endif
			++db->bC.DO_FIFO_RST_counter;
			Fifo_Reset_Disp_RunningEqu_Ending(db);

			if (state == 1)
#ifdef ON_RELEASE
				rel_printk2("dm9-%s Len %03d RST_c %d\n", hstr, db->bC.RxLen, db->bC.DO_FIFO_RST_counter);
#else
				printk("dm9-%s Len %03d RST_c %d\n", hstr, db->bC.RxLen, db->bC.DO_FIFO_RST_counter); //"LenNotYeh", " %d", db->bC.DO_FIFO_RST_counter
#endif
			else if (state == 3)
			{
#ifdef ON_RELEASE
				rel_printk2("dm9-%s Len %03d RST_c %d (RXBErr %d)\n", hstr, db->bC.RxLen, 
						db->bC.DO_FIFO_RST_counter, db->bC.RXBErr_counter);
#else
				if (db->mac_process)          
					printk("dm9-%s Len %03d RST_c %d (RXBErr %d)\n", hstr, db->bC.RxLen, 
						db->bC.DO_FIFO_RST_counter, db->bC.RXBErr_counter);
				else
					printlog("dm9-%s Len %03d RST_c %d (RXBErr %d)\n", hstr, db->bC.RxLen, 
						db->bC.DO_FIFO_RST_counter, db->bC.RXBErr_counter);
#endif
			}
			else  if (state == 2) // STATE 2
			{
#ifdef ON_RELEASE
				rel_printk2("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter);
#else
				if (db->mac_process)          
					printk("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter);
				else
					printlog("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter); //"Len %03d ", db->bC.RxLen
#endif
			}
			else // STATE 5
			{
#ifdef ON_RELEASE
				rel_printk2("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter);
#else
				if (db->mac_process)          
					printk("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter);
				else
					printlog("dm9-%s (YES RST)(RXBErr %d)\n", hstr, db->bC.RXBErr_counter);
#endif
			}
		}
		dm9051_reset(db);
		#if DEF_SPIRW
		iiow(db, DM9051_FCR, FCR_FLOW_ENABLE);	/* Flow Control */
		if (hstr)
			iiow(db, DM9051_PPCR, PPCR_SETTING); /* Fully Pause Packet Count */
		else
		{
			pk= ior(db, DM9051_PPCR);
			iow(db, DM9051_PPCR, PPCR_SETTING); /* Fully Pause Packet Count */
		}
#ifdef DM_CONF_POLLALL_INTFLAG	
		#if DM_DM_CONF_RARE_PROJECTS_DTS_USAGE
			//#if defined DM_CONF_INTERRUPT_TEST_DTS_FALLING
			//	iiow(db, DM9051._INTCR, 0x01); //low active
			if (db->irq_type & (IIRQ_TYPE_LEVEL_HIGH | IIRQ_TYPE_EDGE_RISING) )
				iiow(db, DM9051_INTCR, 0x00); //high active(default)
				//iiow(db, DM9051_INTCR, 0x01); //test
			//#elif defined DM_CONF_INTERRUPT_TEST_DTS_RISING
			//	iiow(db, DM9051._INTCR, 0x00); //high active(default)
			else
				iiow(db, DM9051_INTCR, 0x01); //low active
				//iiow(db, DM9051_INTCR, 0x00); //test
			//#endif	
		#else
			#if (DRV_IRQF_TRIGGER == IRQF_TRIGGER_LOW)
			iiow(db, DM9051_INTCR, 0x01); //low active
			#else
			iiow(db, DM9051_INTCR, 0x00); //high active(default)
			#endif
		#endif
	     	//.iiow(db, DM9051_IMR, IMR_PAR | IMR_PTM | IMR_PRM);
#else
	     	//.iiow(db, DM9051_IMR, IMR_PAR);
#endif
			//..iiow(db, DM9051_IMR, IMR_PAR); //=int_reg_stop(db); 
	     	//iiow(db, DM9051_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
	     	//iiow(db, DM9051_RCR, db->rcr_all);
		#endif
		bcopen_rx_info_clear(&db->bC);
		DM9051_fifo_reset_flg = 1;
	     	return; 
	 }
     //if (state==2){ 
	    //printk("------- ---end--- -------\n");
	    //printk("Hang.System.JJ..\n");
	    //while (1);
	    
	    //("------- ---NO Do_reset( xx RxbErr ) --- -------\n");
     	//	if (hstr) printk("dm9-%s Len %03d (NO RST)(RXBErr %d)\n", hstr, db->bC.RxLen, db->bC.RXBErr_counter);
	    // 	bcopen_rx_info_clear(&db->bC);
     //}
     return; 
}  

/*
 *  disp
 */
static void dm9051_fifo_reset_statistic(board_info_t *db)
{
	if (!(db->bC.DO_FIFO_RST_counter%10)) {
		rel_printk1("dm9-Mac_OvFlwEr.Rxb&LargEr RST_c %d\n", db->bC.DO_FIFO_RST_counter);
		rel_printk1(" %d %d.%d %d\n", 
			db->bC.ERRO_counter, db->bC.OvrFlw_counter, db->bC.RXBErr_counter, db->bC.LARGErr_counter);
		if (db->bC.StatErr_counter)
			rel_printk1("dm9-RareFnd StatEr %d\n", db->bC.StatErr_counter);
	}
}

#if 0
// when reset: return 1
int dm9051_fifo_ERRO(int ana_test, u8 rxbyte, board_info_t *db)
{	
	char hstr[72];
	if (db->bC.rxbyte_counter==5 || /*db->bC.rxbyte_counter0==(NUMRXBYTECOUNTER-1)*/ db->bC.rxbyte_counter0==NUMRXBYTECOUNTER) {
	     
	    db->bC.RXBErr_counter++;
	    
	    #if 0 
	     //one_and_two_and_three all the same!
	     printk("RXBErr %d: %04x/%04x. rxb=%02X, rxb_cntr,cntr0 %d,%d \n", db->bC.RXBErr_counter, 
			db->_rwregs[0], db->_rwregs[1], rxbyte, db->bC.rxbyte_counter, db->bC.rxbyte_counter0);
	    #endif
	     
	     if (/*1*/ ana_test < 3 ) { /* tested-check-ok: if (!(db->bC.RXBErr_counter % 3)) */
	      sprintf(hstr, "dmfifo_reset( 03 RxbErr ) rxb=%02X .%04x/%04x", rxbyte, db->_rwregs[0], db->_rwregs[1]);
	      dm9051_fifo_reset(3, hstr, db);
	     
		  //u16 calc= _dm9051_rx_cap(db);
	      //printk("( RxbErr_cnt %d ) %d ++ \n", db->bC.RXBErr_counter, db->bC.rxbyte._counter0_to_prt);
	      //printk("rxb=%02X rxWrRd .%02x/%02x (RO %d.%d%c)\n", rxbyte, 
	      //  db->_rwregs[0], db->_rwregs[1], calc>>8, calc&0xFF, '%');
	      //if (!(db->bC.RXBErr_counter%5))
	      //{
	      //.driver_dtxt_disp(db);
	      //.driver_dloop_disp(db);
	      //}
	      //dm9051._fifo_reset(1, "dm9051._fifo_reset( RxbEr )", db);
	     
	      dm9051_fifo_reset_statistic(db);
	      return 1;
	     } 
	     else {                                                                                                                                                                                                                                                              
		if (db->mac_process)
		{
			sprintf(hstr, "Or. Do_reset( 02 RxbErr, macError-clear ) rxb=%02X .%04x/%04x", rxbyte, db->_rwregs[0], db->_rwregs[1]); //printk("macError {Just treat as a normal-unicast.}\n"); //, Get recover-clear
			dm9051_fifo_reset(5, hstr, db);
		}
		else
		{
			sprintf(hstr, "Or. Do_reset( 02 RxbErr ) rxb=%02X .%04x/%04x", rxbyte, db->_rwregs[0], db->_rwregs[1]);	
			dm9051_fifo_reset(2, hstr, db); //bcopen_rx_info_.clear(&db->bC); // as be done in 'dm9051._fifo_reset'
		}
		db->mac_process = 0;
	     }
	}
	return 0;
} 
#endif
#endif
