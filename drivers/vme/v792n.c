/*********************************************************************

  Name:         v792n.c
  Created by:   Jimmy Ngai

  Contents:     V792N 16ch. QDC

  Based on v792.c by Pierre-Andre Amaudruz
                
  $Id: $
*********************************************************************/
#include <stdio.h>
#include <string.h>
#include <signal.h>
#if defined(OS_LINUX)
#include <unistd.h>
#endif
#include "v792n.h"

WORD v792n_Read16(MVME_INTERFACE *mvme, DWORD base, int offset)
{
  int cmode;
  WORD data;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  data = mvme_read_value(mvme, base+offset);
  mvme_set_dmode(mvme, cmode);
  return data;
}

void v792n_Write16(MVME_INTERFACE *mvme, DWORD base, int offset, WORD value)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+offset, value);
  mvme_set_dmode(mvme, cmode);
}

DWORD v792n_Read32(MVME_INTERFACE *mvme, DWORD base, int offset)
{
  int cmode;
  DWORD data;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D32);
  data = mvme_read_value(mvme, base+offset);
  mvme_set_dmode(mvme, cmode);
  return data;
}

void v792n_Write32(MVME_INTERFACE *mvme, DWORD base, int offset, DWORD value)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D32);
  mvme_write_value(mvme, base+offset, value);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
int v792n_DataReady(MVME_INTERFACE *mvme, DWORD base)
{
  int data_ready, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  data_ready = mvme_read_value(mvme, base+V792N_CSR1_RO) & 0x1;
  mvme_set_dmode(mvme, cmode);
  return data_ready;
}

/*****************************************************************/
int v792n_isEvtReady(MVME_INTERFACE *mvme, DWORD base)
{
  int csr;
  csr = v792n_CSR1Read(mvme, base);
  return (csr & 0x100);
}

/*****************************************************************/
int v792n_isBusy(MVME_INTERFACE *mvme, DWORD base)
{
  int status, busy, timeout, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  timeout = 1000;
  do {
    status = mvme_read_value(mvme, base+V792N_CSR1_RO);
    busy = status & 0x4;
    timeout--;
  } while (busy || timeout);
  mvme_set_dmode(mvme, cmode);
  return (busy != 0 ? 1 : 0);
}

/*****************************************************************/
/*
Read single event, return event length (number of entries)
*/
int v792n_EventRead(MVME_INTERFACE *mvme, DWORD base, DWORD *pdest, int *nentry)
{
#define USE_BLT_READ_2

#ifdef USE_SINGLE_READ
  DWORD hdata;
  int   cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D32);

  *nentry = 0;
  if (v792n_DataReady(mvme, base)) {
    do {
      hdata = mvme_read_value(mvme, base);
    } while (!(hdata & 0x02000000)); // skip up to the header

    pdest[*nentry] = hdata;
    *nentry += 1;
    do {
      pdest[*nentry] = mvme_read_value(mvme, base);
      *nentry += 1;
    } while (!(pdest[*nentry-1] & 0x04000000)); // copy until the trailer

    nentry--;
  }

  mvme_set_dmode(mvme, cmode);
#endif // USE_SINGLE_READ

#ifdef USE_BLT_READ_1
  DWORD hdata, data[V792N_MAX_CHANNELS+2];
  int   cam, cmode, cblt, cnt, i;

  mvme_get_am(mvme, &cam);
  mvme_get_dmode(mvme, &cmode);
  mvme_get_blt(mvme, &cblt);
  mvme_set_dmode(mvme, MVME_DMODE_D32);

  *nentry = 0;
  if (v792n_DataReady(mvme, base)) {
    do {
      hdata = mvme_read_value(mvme, base);
    } while (!(hdata & 0x02000000)); // skip up to the header

    mvme_set_am(mvme, MVME_AM_A32_SB);
    mvme_set_blt(mvme, MVME_BLT_BLT32);
    cnt = (hdata >> 8) & 0x3F;

    mvme_read(mvme, data, base, (cnt+1)*4);
    pdest[0] = hdata;
    for (i=1;i<=cnt+1;i++)
      pdest[i] = data[i-1];

    *nentry = cnt + 2;
  }

  mvme_set_am(mvme, cam);
  mvme_set_dmode(mvme, cmode);
  mvme_set_blt(mvme, cblt);
#endif // USE_BLT_READ_1

#ifdef USE_BLT_READ_2
  int   cam, cmode, cblt, cnt;

  mvme_get_am(mvme, &cam);
  mvme_get_dmode(mvme, &cmode);
  mvme_get_blt(mvme, &cblt);
  mvme_set_dmode(mvme, MVME_DMODE_D32);

  *nentry = 0;
//  if (v792n_DataReady(mvme, base)) {
    mvme_set_am(mvme, MVME_AM_A32_SB);
    mvme_set_blt(mvme, MVME_BLT_BLT32);
    mvme_read(mvme, pdest, base, (V792N_MAX_CHANNELS+2)*4);
    cnt = (pdest[0] >> 8) & 0x3F;
    *nentry = cnt + 2;
//  }

  mvme_set_am(mvme, cam);
  mvme_set_dmode(mvme, cmode);
  mvme_set_blt(mvme, cblt);
#endif //USE_BLT_READ_2

  return *nentry;
}

/*****************************************************************/
/*
Read nentry of data from the data buffer
*/
int v792n_DataRead(MVME_INTERFACE *mvme, DWORD base, DWORD *pdest, int nentry)
{
  int  cmode, status;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D32);
//  nentry = 128;
  if (v792n_DataReady(mvme, base)) {
    status = mvme_read(mvme, pdest, base, nentry*4);
  }
  mvme_set_dmode(mvme, cmode);
  return status;
}

/*****************************************************************/
void v792n_DataClear(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_BIT_SET2_RW, 0x4);
  mvme_write_value(mvme, base+V792N_BIT_CLEAR2_WO, 0x4);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_EvtCntRead(MVME_INTERFACE *mvme, DWORD base, DWORD *evtcnt)
{
  int cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  *evtcnt  = mvme_read_value(mvme, base+V792N_EVT_CNT_L_RO);
  *evtcnt += (mvme_read_value(mvme, base+V792N_EVT_CNT_H_RO) << 16);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_EvtCntReset(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_EVT_CNT_RST_WO, 1);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_IntSet(MVME_INTERFACE *mvme, DWORD base, int level, int vector)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_INT_VECTOR_WO, (vector & 0xFF));
  mvme_write_value(mvme, base+V792N_INT_LEVEL_WO, (level & 0x7));
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_IntEnable(MVME_INTERFACE *mvme, DWORD base, int level)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_EVTRIG_REG_RW, (level & 0x1F));
  /* Use the trigger buffer for int enable/disable
  mvme_write_value(mvme, base+V792N_INT_LEVEL_WO, (level & 0x7));
  */
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_IntDisable(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_EVTRIG_REG_RW, 0);
  /* Use the trigger buffer for int enable/disable
     Setting a level 0 reboot the VMIC !
  mvme_write_value(mvme, base+V792N_INT_LEVEL_WO, 0);
  */
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_EvtTriggerSet(MVME_INTERFACE *mvme, DWORD base, int count)
{
  int cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_EVTRIG_REG_RW, (count & 0x1F));
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_SingleShotReset(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_SINGLE_RST_WO, 1);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_SoftReset(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_BIT_SET1_RW, V792N_SOFT_RESET);
  mvme_write_value(mvme, base+V792N_BIT_CLEAR1_WO, V792N_SOFT_RESET);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
/**
 * cause a software trigger
 */
void v792n_Trigger(MVME_INTERFACE *mvme, DWORD base) {
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_SWCOMM_WO, 0);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
/**
 * Read Thresholds
 */
int v792n_ThresholdRead(MVME_INTERFACE *mvme, DWORD base, WORD *threshold)
{
  int k, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);

  for (k=0; k<V792N_MAX_CHANNELS ; k++) {
    threshold[k] = mvme_read_value(mvme, base+V792N_THRES_BASE+2*k) & 0x1FF;
  }
  mvme_set_dmode(mvme, cmode);
  return V792N_MAX_CHANNELS;
}

/*****************************************************************/
/**
 * Write Thresholds and read them back
 */
int v792n_ThresholdWrite(MVME_INTERFACE *mvme, DWORD base, WORD *threshold)
{
  int k, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  for (k=0; k<V792N_MAX_CHANNELS ; k++) {
    mvme_write_value(mvme, base+V792N_THRES_BASE+2*k, threshold[k] & 0x1FF);
  }

  for (k=0; k<V792N_MAX_CHANNELS ; k++) {
    threshold[k] = mvme_read_value(mvme, base+V792N_THRES_BASE+2*k) & 0x1FF;
  }

  mvme_set_dmode(mvme, cmode);
  return V792N_MAX_CHANNELS;
}

/*****************************************************************/
int v792n_CSR1Read(MVME_INTERFACE *mvme, DWORD base)
{
  int status, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  status = mvme_read_value(mvme, base+V792N_CSR1_RO);
  mvme_set_dmode(mvme, cmode);
  return status;
}

/*****************************************************************/
int v792n_CSR2Read(MVME_INTERFACE *mvme, DWORD base)
{
  int status, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  status = mvme_read_value(mvme, base+V792N_CSR2_RO);
  mvme_set_dmode(mvme, cmode);
  return status;
}

/*****************************************************************/
int v792n_BitSet2Read(MVME_INTERFACE *mvme, DWORD base)
{
  int status, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  status = mvme_read_value(mvme, base+V792N_BIT_SET2_RW);
  mvme_set_dmode(mvme, cmode);
  return status;
}

/*****************************************************************/
void v792n_BitSet2Set(MVME_INTERFACE *mvme, DWORD base, WORD pat)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_BIT_SET2_RW, pat);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_BitSet2Clear(MVME_INTERFACE *mvme, DWORD base, WORD pat)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_BIT_CLEAR2_WO, pat);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
/**
 * read Control Register 1 (0x1010,16 bit)
 */
WORD v792n_ControlRegister1Read(MVME_INTERFACE *mvme, DWORD base) {
  int cmode;
  WORD pat;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  pat = mvme_read_value(mvme, base+V792N_CR1_RW);
  mvme_set_dmode(mvme, cmode);
  return pat;
}

/*****************************************************************/
/**
 * write Control Register 1 (0x1010,16 bit)
 */
void v792n_ControlRegister1Write(MVME_INTERFACE *mvme, DWORD base, WORD pat) {
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_CR1_RW, pat);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_OnlineSet(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_BIT_CLEAR2_WO, 0x2);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_OfflineSet(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_BIT_SET2_RW, 0x2);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_BlkEndEnable(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  WORD pat;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  pat = mvme_read_value(mvme, base+V792N_CR1_RW);
  mvme_write_value(mvme, base+V792N_CR1_RW, (pat|0x04)&0x74);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_OverRangeEnable(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_BIT_CLEAR2_WO, 0x08);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_OverRangeDisable(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_BIT_SET2_RW, 0x08);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_LowThEnable(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_BIT_CLEAR2_WO, 0x10);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_LowThDisable(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_BIT_SET2_RW, 0x10);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
void v792n_EmptyEnable(MVME_INTERFACE *mvme, DWORD base)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+V792N_BIT_SET2_RW, 0x1000);
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
/**
Sets all the necessary paramters for a given configuration.
The configuration is provided by the mode argument.
Add your own configuration in the case statement. Let me know
your setting if you want to include it in the distribution.
@param *mvme VME structure
@param  base Module base address
@param mode  Configuration mode number
@param *nentry number of entries requested and returned.
@return MVME_SUCCESS
*/
int  v792n_Setup(MVME_INTERFACE *mvme, DWORD base, int mode)
{
  int  cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);

  printf("V792N at VME A32 0x%08x:\n", base);
  switch (mode) {
  case 0x1:
    printf("Default setting after power up (mode:%d)\n", mode);
    printf("\n");
    break;
  case 0x2:
    printf("Modified setting (mode:%d)\n", mode);
    printf("Empty Enable, Over Range Disable, Low Th Disable\n");
    printf("\n");
    v792n_EmptyEnable(mvme, base);
    v792n_OverRangeDisable(mvme, base);
    v792n_LowThDisable(mvme, base);
    break;
  case 0x3:
    printf("Modified setting (mode:%d)\n", mode);
    printf("Empty Enable, Over Range Disable, Low Th Disable, Block End Enable\n");
    printf("\n");
    v792n_EmptyEnable(mvme, base);
    v792n_OverRangeDisable(mvme, base);
    v792n_LowThDisable(mvme, base);
    v792n_BlkEndEnable(mvme, base);
    break;
  case 0x4:
    printf("Modified setting (mode:%d)\n", mode);
    printf("Low Th Disable, Block End Enable\n");
    printf("\n");
    v792n_LowThDisable(mvme, base);
    v792n_BlkEndEnable(mvme, base);
    break;
  default:
    printf("Unknown setup mode\n");
    printf("\n");
    mvme_set_dmode(mvme, cmode);
    return -1;
  }
  mvme_set_dmode(mvme, cmode);
  return 0;
}

/*****************************************************************/
void  v792n_Status(MVME_INTERFACE *mvme, DWORD base)
{
  int status, cmode, i;
  WORD threshold[V792N_MAX_CHANNELS];
  DWORD evtcnt;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  printf("V792N at VME A32 0x%08x:\n", base);
  status = mvme_read_value(mvme, base+V792N_FIRM_REV);
  printf("Firmware revision: 0x%x\n", status);
  if (status == 0xFFFF) {
    printf("Module not present!\n");
    return;
  }
  status = v792n_CSR1Read(mvme, base);
  printf("CSR1: 0x%x\n", status);
  printf("DataReady    :%s\t", status & 0x1 ? "Y" : "N");
  printf(" - Global Dready:%s\t", status & 0x2 ? "Y" : "N");
  printf(" - Busy         :%s\n", status & 0x4 ? "Y" : "N");
  printf("Global Busy  :%s\t", status & 0x8 ? "Y" : "N");
  printf(" - Amnesia      :%s\t", status & 0x10 ? "Y" : "N");
  printf(" - Purge        :%s\n", status & 0x20 ? "Y" : "N");
  printf("Term ON      :%s\t", status & 0x40 ? "Y" : "N");
  printf(" - TermOFF      :%s\t", status & 0x80 ? "Y" : "N");
  printf(" - Event Ready  :%s\n", status & 0x100 ? "Y" : "N");
  status = v792n_CSR2Read(mvme, base);
  printf("CSR2: 0x%x\n", status);
  printf("Buffer Empty :%s\t", status & 0x2 ? "Y" : "N");
  printf(" - Buffer Full  :%s\n", status & 0x4 ? "Y" : "N");
  int dtype = (status & 0xF0) >> 4;
  printf("Daughter card type (CSEL/DSEL)  :%d%d%d%d (0x%x) ",
         status & 0x80 ? 1 : 0, 
         status & 0x40 ? 1 : 0,
         status & 0x20 ? 1 : 0,
         status & 0x10 ? 1 : 0,
         dtype);
  switch (dtype) {
  default:
    printf("\n");
    break;
  case 2:
    printf("V792 32ch QDC\n");
    break;
  }
  status = v792n_BitSet2Read(mvme, base);
  printf("BitSet2: 0x%x\n", status);
  printf("Test Mem     :%s\t", status & 0x1 ? "Y" : "N");
  printf(" - Offline      :%s\t", status & 0x2 ? "Y" : "N");
  printf(" - Clear Data   :%s\n", status & 0x4  ? "Y" : "N");
  printf("Over Range En:%s\t", status & 0x8  ? "Y" : "N");
  printf(" - Low Thres En :%s\t", status & 0x10 ? "Y" : "N");
  printf(" - Auto Incr    :%s\n", status & 0x800 ? "Y" : "N");
  printf("Empty Enable :%s\t", status & 0x1000 ? "Y" : "N");
  printf(" - Slide sub En :%s\t", status & 0x2000 ? "Y" : "N");
  printf(" - All Triggers :%s\n", status & 0x4000 ? "Y" : "N");
  v792n_EvtCntRead(mvme, base, &evtcnt);
  printf("Event counter: %d\n", evtcnt);
  printf("Iped value: %d\n", v792n_Read16(mvme, base, V792N_IPED_RW));

  v792n_ThresholdRead(mvme, base, threshold);
  for (i=0;i<V792N_MAX_CHANNELS;i+=2) {
    printf("Threshold[%2i] = 0x%4.4x\t   -  ", i, threshold[i]);
    printf("Threshold[%2i] = 0x%4.4x\n", i+1, threshold[i+1]);
  }
  mvme_set_dmode(mvme, cmode);
}

/*****************************************************************/
int v792n_isPresent(MVME_INTERFACE *mvme, DWORD base)
{
  int status, cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  status = mvme_read_value(mvme, base+V792N_FIRM_REV);
  mvme_set_dmode(mvme, cmode);
  if (status == 0xFFFF)
    return 0;
  else
    return 1;
}

/*****************************************************************/
/**
 * decoded printout of readout entry
 * Not to be trusted for data decoding but acceptable for display
 * purpose as its implementation is strongly compiler dependent and
 * not flawless.
 * @param v
 */
void v792n_printEntry(const v792n_Data* v) {
  switch (v->data.type) {
  case v792n_typeMeasurement:
    printf("Data=0x%08x Measurement ch=%3d v=%6d over=%1d under=%1d\n",
	   (int)v->raw,v->data.channel,v->data.adc,v->data.ov,v->data.un);
    break;
  case v792n_typeHeader:
    printf("Data=0x%08x Header geo=%2x crate=%2x cnt=%2d\n",
    	   (int)v->raw,v->header.geo,v->header.crate,v->header.cnt);
    break;
  case v792n_typeFooter:
    printf("Data=0x%08x Footer geo=%2x evtCnt=%7d\n",
    	   (int)v->raw,v->footer.geo,v->footer.evtCnt);
    break;
  case v792n_typeFiller:
    printf("Data=0x%08x Filler\n",(int)v->raw);
    break;
  default:
    printf("Data=0x%08x Unknown %04x\n",(int)v->raw,v->data.type);
    break;
  }
}

/* emacs
 * Local Variables:
 * mode:C
 * mode:font-lock
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
