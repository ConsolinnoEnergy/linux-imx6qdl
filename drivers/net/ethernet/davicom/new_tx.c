

// ----------------------------------

//[../new_load/tx.c]
//tx.c
#if DM9051_CONF_TX
static void dm9051_tx_chk(struct net_device *dev, u8 *wrptr)
{
#if 0
    printk("dm9.tx_packets %lu ", dev->stats.tx_packets);
    printk("tx(%02x %02x %02x %02x %02x %02x ", wrptr[0], wrptr[1],wrptr[2],wrptr[3],wrptr[4],wrptr[5]);
    printk("%02x %02x   %02x %02x %02x %02x ", wrptr[6], wrptr[7],wrptr[8],wrptr[9],wrptr[10],wrptr[11]);
    printk("%02x %02x\n", wrptr[12],wrptr[13]);
#endif
}

static u16 check_cntStop(board_info_t *db)
{
#if 0	
	u16 cs;
	while (!spin_trylock(&db->statelock_tx1_rx1)) ; //if(!)
	cs = db->bt.prob_cntStopped;
	spin_unlock(&db->statelock_tx1_rx1); 
	return cs;
#endif	
	return (!skb_queue_empty(&db->txq));
}
#endif 	

//
//
// TX.C
//
//

// [ 577 - 643 ]

#if DM9051_CONF_TX 	
static void opening_wake_queue1(struct net_device *dev) //( u8 flag)
{
#if 0	
	board_info_t *db= netdev_priv(dev);
	
	while (!spin_trylock(&db->statelock_tx1_rx1)) ; //if(!)
	if (db->bt.prob_cntStopped)
	{
		db->bt.prob_cntStopped= 0;
		netif_wake_queue(dev);
	}
	spin_unlock(&db->statelock_tx1_rx1);
#endif

	board_info_t *db= netdev_priv(dev);
	if (db->bt.prob_cntStopped) {
		db->bt.prob_cntStopped= 0;
		netif_wake_queue(dev);
	}
}

static void toend_stop_queue1(struct net_device *dev, u16 stop_cnt)
{
#if 0	
	board_info_t *db= netdev_priv(dev);	
	while (!spin_trylock(&db->statelock_tx1_rx1)) ; //if(!)
	switch(stop_cnt)
	{
		case 1:
		db->bt.prob_cntStopped++;
		break;
		case NUM_QUEUE_TAIL:
		default:
		db->bt.prob_cntStopped= stop_cnt;
		break;
	}
	spin_unlock(&db->statelock_tx1_rx1); 
	
	if (stop_cnt<NUM_QUEUE_TAIL)
		return; // true;
	if (stop_cnt==NUM_QUEUE_TAIL)
	{
	  	netif_stop_queue(dev);
		return; // true;
	}
	
	//.wrong path, but anyhow call stop for it
	netif_stop_queue(dev);
	printk("[.wrong path]: WARN, anyhow call stop for it .. ");
	printk("(cntStop %d)\n", db->bt.prob_cntStopped);
	driver_dtxt_disp(db); // OPTIONAL CALLED
	return; // false;
#endif	

	board_info_t *db= netdev_priv(dev);	
	switch(stop_cnt) {
		case 1:
		  db->bt.prob_cntStopped++;
		  break;
		case NUM_QUEUE_TAIL:
		  db->bt.prob_cntStopped= stop_cnt;
		  break;
	}	
	if (stop_cnt==NUM_QUEUE_TAIL)
	  	netif_stop_queue(dev);
}
#endif

// -------------------------------------------------------------------------------------------

// [ 1706 - 1808 ]
		   
#if DM9051_CONF_TX
static int
dm9051_continue_xmit_inRX(board_info_t *db) //dm9051_continue_poll_xmit
{
		    struct net_device *dev = db->ndev;
		    int nTx= 0;

		    db->bt.local_cntTXREQ= 0;
		    db->bt.local_cntLOOP= 0;
			while(!skb_queue_empty(&db->txq)) // JJ-20140225, When '!empty'
			{
				  struct sk_buff *tx_skb;
				  int nWait = 0;
				  
				  tx_skb = skb_dequeue(&db->txq);
				  if (tx_skb != NULL) {
							       
						  if (db->nSCH_XMIT <= db->nSCH_XMIT_WAVE_PDL)
						  {
							char *p=  (char *) tx_skb->data;
							if (p[0] & 1)
							  printk("[dm9 inRX] tx Multi- %d, [%02x] [%02x] [%02x], len %d\n", db->nSCH_XMIT, p[0], p[1], p[2], tx_skb->len);
							else {
							  printk("[dm9 inRX] tx Uni- %d, [%02x] [%02x] [%02x], len %d\n", db->nSCH_XMIT, p[0], p[1], p[2], tx_skb->len);
							  
							  if (db->nSCH_XMIT == db->nSCH_XMIT_WAVE_PDL) {
								if (db->nSCH_XMIT_WAVE_PDL < NUM_TOTAL_ALL) {
								  db->nSCH_XMIT_WAVE_PDL += NUM_SCH_XMIT_WAVE;
								  printk("dm9 %s ----- ALLOW display xmit counter to %d -----\n", "_XmitInRx", db->nSCH_XMIT_WAVE_PDL);
								}
							  }
							}
						  }
#if DEF_SPIRW
					        while( (ior(db, DM9051_TCR) & TCR_TXREQ) && (++nWait < 20)) 
								;
#endif
					        if (nWait ==20)
								printk("[dm9] tx_step timeout B\n");
					        
					        //while( ior(db, DM9051_TCR) & TCR_TXREQ ) 
							//	; //driver_dtxt_step(db, 'B');
				    
					        if(db->bt.local_cntTXREQ==2)
					        {
					           //while( ior(db, DM9051_TCR) & TCR_TXREQ ) 
					            // ; //driver_dtxt_step(db, 'Z');
					           db->bt.local_cntTXREQ= 0;
					        }

						    nTx++;

#if DEF_SPIRW
							#if 0	
							if (1)
 								{
 									int i;
 									char *pb = (char *) tx_skb->data;
 									for (i=0; i<tx_skb->len; i++ )
 										*pb++ = i;
 								}
							#endif	
						    dm9051_outblk(db, tx_skb->data, tx_skb->len);
						    iow(db, DM9051_TXPLL, tx_skb->len);
						    iow(db, DM9051_TXPLH, tx_skb->len >> 8);
#ifdef JABBER_PACKET_SUPPORT						    
						    iow(db, DM9051_TCR, TCR_TXREQ | TCR_DIS_JABBER_TIMER);
#else
						    iow(db, DM9051_TCR, TCR_TXREQ);
#endif
#endif
						    dev->stats.tx_bytes += tx_skb->len;
						    dev->stats.tx_packets++;
						    /* done tx */
					        #if 1
						    dm9051_tx_chk(dev, tx_skb->data);
					        #endif
						    dev_kfree_skb(tx_skb);
				            db->bt.local_cntTXREQ++;
				            db->bt.local_cntLOOP++;
							#if 0
							  {   
					            u16 mdwr;
					            u16 txmr;
						        while (ior(db, DM9051_TCR) & TCR_TXREQ) 
									;
								
								mdwr= ior(db, 0x7a);
								mdwr |= (u16)ior(db, 0x7b) << 8;
								txmr= ior(db, 0x22);
								txmr |= (u16)ior(db, 0x23) << 8;
								printk("TX.e [txmr %03x mdwr %03x]\n", txmr, mdwr);
							}
							#endif 
				  } //if
			} //while

			#if 0 //checked ok!!
			if (db->nSCH_INT_NUm >= db->nSCH_INT_Num_Disp) { //( /*nTx>1 || */ !(db->nSCH_INT_NUm%50))
			  char *jmp_mark= " ";
			  u16 update_num_calc = ((db->nSCH_INT_NUm/100)*100) + 100;
			  db->nSCH_INT_Num_Disp += 100;
			  if (db->nSCH_INT_Num_Disp != update_num_calc) { //i.e. (db->nSCH_INT_Num_Disp < update_num_calc)
				  jmp_mark= "*";
				  db->nSCH_INT_Num_Disp = update_num_calc;
			  }
			}
			#endif
			
		    return nTx;
}
#endif

// [ 1809 - 1830 ]

static int dm9051_tx_irx(board_info_t *db)
{
	struct net_device *dev = db->ndev;
	int nTx = 0;
#if DM9051_CONF_TX

	if (check_cntStop(db)) 
	  //if (db->bt.prob_cntStopped)  
	  // This is more exactly right!!
	{
		  #if LOOP_XMIT
		    //mutex_lock(&db->addr_lock);
		    nTx= dm9051_continue_xmit_inRX(db); //=dm9051_continue_poll_xmit
		    opening_wake_queue1(dev); 
		    //mutex_unlock(&db->addr_lock);
		  #endif //LOOP_XMIT
	}
	
#endif //DM9051_CONF_TX
	return nTx;
}
