// To be complete for customize purpose

#ifdef QCOM_CONF_BOARD_YES
#define QCOM_RX_DWORD_BOUNDARY
#define QCOM_TX_DWORD_BOUNDARY
#define QCOM_BURST_MODE
#endif
#ifndef QCOM_CONF_BOARD_YES
#define QCOM_RX_DWORD_BOUNDARY //[TEMP] //#define QCOM_RX_DWORD_BOUNDARY   //RX NOT simulate 
#define QCOM_TX_DWORD_BOUNDARY
#define QCOM_BURST_MODE
#endif

 /* ----------------------------- */
 /* {Easy Engineer selection}     */
 /* ----------------------------- */
//.#define MACRO_ASR_QUECTEL
       
 /* ----------------------------- */
 /* {Easy customization config}   */
 /* ----------------------------- */
#ifdef MACRO_ASR_QUECTEL

 ,  DSV, ,V , ,EERETERRTN.....

	#undef DTS_CONF_YES
	#define DTS_CONF_YES

	#undef DM_CONF_INTERRUPT
	//#define DM_CONF_INTERRUPT

	#undef DM_CONF_1024_BUF_CASE_YES
	//#define DM_CONF_1024_BUF_CASE_YES

	#undef MTK_CONF_XY6762TEMP
	//#define MTK_CONF_XY6762TEMP

	#undef DM_CONF_SPI_TEST_BLKIN_SPLIT
	//#undef DM_CONF_SPI_TEST_BLKLEN

#endif

// To be complete for everyone who is to make experiment (put at below section area)
#ifndef MACRO_ASR_QUECTEL
#if 1

	//
	//  Davicom-RPI-test
	//
	#undef MTK_CONF_XY6762TEMP
	
//#undef DM_CONF_MAX_SPEED_HZ
//#define DM_CONF_MAX_SPEED_HZ	15600000

//#define DM_CONF_ANY_BUF_CASE_SKB_RX_CORE
#define ANY_BUF_NUM	768 //[64 (MacErr early, With Request time out), 128][512, 640, 768, ...]

#endif
#endif
