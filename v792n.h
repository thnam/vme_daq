/*********************************************************************

  Name:         v792n.h
  Created by:   Jimmy Ngai

  Contents:     V792N 16ch. QDC include

  Based on v792.h by Pierre-Andre Amaudruz

  $Id: $
*********************************************************************/
#ifndef  V792N_INCLUDE_H
#define  V792N_INCLUDE_H

#include <stdio.h>
#include <string.h>
#include "mvmestd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  V792N_MAX_CHANNELS       (DWORD) 16
#define  V792N_REG_BASE           (DWORD) (0x1000)
#define  V792N_FIRM_REV           (DWORD) (0x1000)
#define  V792N_GEO_ADDR_RW        (DWORD) (0x1002)
#define  V792N_MCST_CBLT_RW       (DWORD) (0x1004)
#define  V792N_BIT_SET1_RW        (DWORD) (0x1006)
#define  V792N_BIT_CLEAR1_WO      (DWORD) (0x1008)
#define  V792N_SOFT_RESET         (DWORD) (0x1<<7)
#define  V792N_INT_LEVEL_WO       (DWORD) (0x100A)
#define  V792N_INT_VECTOR_WO      (DWORD) (0x100C)
#define  V792N_CSR1_RO            (DWORD) (0x100E)
#define  V792N_CR1_RW             (DWORD) (0x1010)
#define  V792N_ADER_H_RW          (DWORD) (0x1012)
#define  V792N_ADER_L_RW          (DWORD) (0x1014)
#define  V792N_SINGLE_RST_WO      (DWORD) (0x1016)
#define  V792N_MCST_CBLT_CTRL_RW  (DWORD) (0x101A)
#define  V792N_EVTRIG_REG_RW      (DWORD) (0x1020)
#define  V792N_CSR2_RO            (DWORD) (0x1022)
#define  V792N_EVT_CNT_L_RO       (DWORD) (0x1024)
#define  V792N_EVT_CNT_H_RO       (DWORD) (0x1026)
#define  V792N_INCR_EVT_WO        (DWORD) (0x1028)
#define  V792N_INCR_OFFSET_WO     (DWORD) (0x102A)
#define  V792N_LD_TEST_RW         (DWORD) (0x102C)
#define  V792N_DELAY_CLEAR_RW     (DWORD) (0x102E)
#define  V792N_FCLR_WIN_RW        (DWORD) (0x102E)
#define  V792N_BIT_SET2_RW        (DWORD) (0x1032)
#define  V792N_BIT_CLEAR2_WO      (DWORD) (0x1034)
#define  V792N_W_MEM_TEST_WO      (DWORD) (0x1036)
#define  V792N_MEM_TEST_WORD_H_WO (DWORD) (0x1038)
#define  V792N_MEM_TEST_WORD_L_WO (DWORD) (0x103A)
#define  V792N_CRATE_SEL_RW       (DWORD) (0x103C)
#define  V792N_TEST_EVENT_WO      (DWORD) (0x103E)
#define  V792N_EVT_CNT_RST_WO     (DWORD) (0x1040)
#define  V792N_IPED_RW            (DWORD) (0x1060)
#define  V792N_R_MEM_TEST_WO      (DWORD) (0x1064)
#define  V792N_SWCOMM_WO          (DWORD) (0x1068)
#define  V792N_SLIDECONST_RW      (DWORD) (0x106A)
#define  V792N_AAD_RO             (DWORD) (0x1070)
#define  V792N_BAD_RO             (DWORD) (0x1072)
#define  V792N_THRES_BASE         (DWORD) (0x1080)

WORD v792n_Read16(MVME_INTERFACE *mvme, DWORD base, int offset);
void v792n_Write16(MVME_INTERFACE *mvme, DWORD base, int offset, WORD value);
DWORD v792n_Read32(MVME_INTERFACE *mvme, DWORD base, int offset);
void v792n_Write32(MVME_INTERFACE *mvme, DWORD base, int offset, DWORD value);

int  v792n_DataReady(MVME_INTERFACE *mvme, DWORD base);
int  v792n_isEvtReady(MVME_INTERFACE *mvme, DWORD base);
int  v792n_isBusy(MVME_INTERFACE *mvme, DWORD base);
int  v792n_EventRead(MVME_INTERFACE *mvme, DWORD base, DWORD *pdest, int *nentry);
int  v792n_DataRead(MVME_INTERFACE *mvme, DWORD base, DWORD *pdest, int nentry);
void v792n_DataClear(MVME_INTERFACE *mvme, DWORD base);
void v792n_EvtCntRead(MVME_INTERFACE *mvme, DWORD base, DWORD *evtcnt);
void v792n_EvtCntReset(MVME_INTERFACE *mvme, DWORD base);
void v792n_IntSet(MVME_INTERFACE *mvme, DWORD base, int level, int vector);
void v792n_IntEnable(MVME_INTERFACE *mvme, DWORD base, int level);
void v792n_IntDisable(MVME_INTERFACE *mvme, DWORD base);
void v792n_EvtTriggerSet(MVME_INTERFACE *mvme, DWORD base, int count);
void v792n_SingleShotReset(MVME_INTERFACE *mvme, DWORD base);
void v792n_SoftReset(MVME_INTERFACE *mvme, DWORD base);
void v792n_Trigger(MVME_INTERFACE *mvme, DWORD base);
int  v792n_ThresholdRead(MVME_INTERFACE *mvme, DWORD base, WORD *threshold);
int  v792n_ThresholdWrite(MVME_INTERFACE *mvme, DWORD base, WORD *threshold);
int  v792n_CSR1Read(MVME_INTERFACE *mvme, DWORD base);
int  v792n_CSR2Read(MVME_INTERFACE *mvme, DWORD base);
int  v792n_BitSet2Read(MVME_INTERFACE *mvme, DWORD base);
void v792n_BitSet2Set(MVME_INTERFACE *mvme, DWORD base, WORD pat);
void v792n_BitSet2Clear(MVME_INTERFACE *mvme, DWORD base, WORD pat);
WORD v792n_ControlRegister1Read(MVME_INTERFACE *mvme, DWORD base);
void v792n_ControlRegister1Write(MVME_INTERFACE *mvme, DWORD base, WORD pat);
void v792n_OnlineSet(MVME_INTERFACE *mvme, DWORD base);
void v792n_OfflineSet(MVME_INTERFACE *mvme, DWORD base);
void v792n_BlkEndEnable(MVME_INTERFACE *mvme, DWORD base);
void v792n_OverRangeEnable(MVME_INTERFACE *mvme, DWORD base);
void v792n_OverRangeDisable(MVME_INTERFACE *mvme, DWORD base);
void v792n_LowThEnable(MVME_INTERFACE *mvme, DWORD base);
void v792n_LowThDisable(MVME_INTERFACE *mvme, DWORD base);
void v792n_EmptyEnable(MVME_INTERFACE *mvme, DWORD base);
void v792n_CrateSet(MVME_INTERFACE *mvme, DWORD base, DWORD *evtcnt);
void v792n_DelayClearSet(MVME_INTERFACE *mvme, DWORD base, int delay);
int  v792n_Setup(MVME_INTERFACE *mvme, DWORD base, int mode);
void v792n_Status(MVME_INTERFACE *mvme, DWORD base);
int  v792n_isPresent(MVME_INTERFACE *mvme, DWORD base);

  enum v792n_DataType {
    v792n_typeMeasurement=0,
    v792n_typeHeader     =2,
    v792n_typeFooter     =4,
    v792n_typeFiller     =6
  };

  typedef union {
    DWORD raw;
    struct v792n_Entry {
      unsigned adc:12; // bit0 here
      unsigned ov:1;
      unsigned un:1;
      unsigned _pad_1:3;
      unsigned channel:4;
      unsigned _pad_2:3;
      unsigned type:3;
      unsigned geo:5;
    } data ;
    struct v792n_Header {
      unsigned _pad_1:8; // bit0 here
      unsigned cnt:6;
      unsigned _pad_2:2;
      unsigned crate:8;
      unsigned type:3;
      unsigned geo:5;
    } header;
    struct v792n_Footer {
      unsigned evtCnt:24; // bit0 here
      unsigned type:3;
      unsigned geo:5;
    } footer;
  } v792n_Data;

  typedef union {
    DWORD raw;
    struct {
      unsigned DataReady:1; // bit0 here
      unsigned GlobalDataReady:1;
      unsigned Busy:1;
      unsigned GlobalBusy:1;
      unsigned Amnesia:1;
      unsigned Purge:1;
      unsigned TermOn:1;
      unsigned TermOff:1;
      unsigned EventReady:1; //bit 8 here
    };
  } v792n_StatusRegister1;
  typedef union {
    DWORD raw;
    struct {
      unsigned _pad_1:1; // bit0 here
      unsigned BufferEmpty:1;
      unsigned BufferFull:1;
      unsigned _pad_2:1;
      unsigned PB:4;
      //unsigned DSEL0:1;
      //unsigned DSEL1:1;
      //unsigned CSEL0:1;
      //unsigned CSEL1:1;
    };
  } v792n_StatusRegister2;
  typedef union {
    DWORD raw;
    struct {
      unsigned _pad_1:2;
      unsigned BlkEnd:1;
      unsigned _pad_2:1;
      unsigned ProgReset:1;
      unsigned BErr:1;
      unsigned Align64:1;
    };
  } v792n_ControlRegister1;
  typedef union {
    DWORD raw;
    struct {
      unsigned MemTest:1;
      unsigned OffLine:1;
      unsigned ClearData:1;
      unsigned OverRange:1;
      unsigned LowThresh:1;
      unsigned _pad_1:1;//bit5
      unsigned TestAcq:1;
      unsigned SLDEnable:1;
      unsigned StepTH:1;
      unsigned _pad_2:2;//bits 9-10
      unsigned AutoIncr:1;
      unsigned EmptyProg:1;
      unsigned SlideSubEnable:1;
      unsigned AllTrg:1;
    };
  } v792n_BitSet2Register;

  void v792n_printEntry(const v792n_Data* v);

#ifdef __cplusplus
}
#endif

#endif // V792N_INCLUDE_H

/* emacs
 * Local Variables:
 * mode:C
 * mode:font-lock
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
