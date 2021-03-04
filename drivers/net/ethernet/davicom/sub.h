// "sub.h" [previous name : "sub_dm9051.h"]

// --- struct declaration ---
// ---  const definition  ---

#define WR_ISR_ENDOF_RXWORK_ONLY //JJ_20190813

#define IIRQ_TYPE_NONE			0
#define IIRQ_TYPE_EDGE_RISING	1
#define IIRQ_TYPE_EDGE_FALLING	2
#define IIRQ_TYPE_LEVEL_HIGH	4
#define IIRQ_TYPE_LEVEL_LOW		8

//[SPI_BUF_LEN]
#define SPI_SYNC_TRANSFER_BUF_LEN (4 + DM9051_PKT_MAX)
//[CUSTOM_BUF_LEN]
#define CMMB_SPI_INTERFACE_MAX_PKT_LENGTH_PER_TIMES (0x400)

#define DS_NUL							0
#define DS_POLL							1
#define DS_IDLE							2
#define CCS_NUL							0
//#define CCS_PROBE						1

#define	R_SCH_INIT		1
#define R_SCH_XMIT		2
#define R_SCH_INT		3
#define R_SCH_INFINI	4
#define R_SCH_INT_GLUE	5  // vs.R_SCH_.INT
//#define R_SCH_LINK	6
#define R_SCH_PHYPOLL	6  // extended

/* 3p6s.s */
asmlinkage __visible int printkr(const char *fmt, ...){
  return 0; 
}
EXPORT_SYMBOL(printkr);
/* 3p6s.e */
 
