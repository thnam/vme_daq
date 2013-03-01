/********************************************************************

  Name:         v1718.c
  Created by:   Jimmy Ngai

  Contents:     Midas VME standard (MVMESTD) layer for CAEN V1718
                VME-USB2.0 Bridge using CAENVMElib Linux library

  $Id: $

\********************************************************************/

#ifdef __linux__
#ifndef OS_LINUX
#define OS_LINUX
#endif
#endif

#ifdef OS_LINUX

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CAENVMElib.h"

#endif // OS_LINUX

#include "v1718.h"

/*------------------------------------------------------------------*/

/********************************************************************\

  MIDAS VME standard (MVMESTD) functions

\********************************************************************/

int mvme_open(MVME_INTERFACE **vme, int idx)
{
   *vme = (MVME_INTERFACE *) malloc(sizeof(MVME_INTERFACE));
   if (*vme == NULL)
      return MVME_NO_MEM;

   memset(*vme, 0, sizeof(MVME_INTERFACE));

   /* open VME */
   if (CAENVME_Init(cvV1718, 0, idx, &(*vme)->handle) != cvSuccess)
      return MVME_NO_INTERFACE;

   /* default values */
   (*vme)->am        = MVME_AM_DEFAULT;
   (*vme)->dmode     = MVME_DMODE_D32;
   (*vme)->blt_mode  = MVME_BLT_NONE;
   (*vme)->table     = NULL; // not used

   return MVME_SUCCESS;
}

/*------------------------------------------------------------------*/

int mvme_close(MVME_INTERFACE *vme)
{
   CAENVME_End(vme->handle);

   free(vme);

   return MVME_SUCCESS;
}

/*------------------------------------------------------------------*/

int mvme_sysreset(MVME_INTERFACE *vme)
{
   CAENVME_SystemReset(vme->handle);

   return MVME_SUCCESS;
}

/*------------------------------------------------------------------*/

int mvme_write(MVME_INTERFACE *vme, mvme_addr_t vme_addr, void *src, mvme_size_t n_bytes)
{
   mvme_size_t i;
   int status=0, n;
   int hvme;
   hvme = vme->handle;

   n = 0;

   /* D8 */
   if (vme->dmode == MVME_DMODE_D8) {
      for (i=0 ; i<n_bytes ; i++)
         status = CAENVME_WriteCycle(hvme, vme_addr, src+i, vme->am, cvD8);
      n = n_bytes;
   /* D16 */
   } else if (vme->dmode == MVME_DMODE_D16) {
      /* normal I/O */
      if (vme->blt_mode == MVME_BLT_NONE) {
         for (i=0 ; i<(n_bytes>>1) ; i++)
            status = CAENVME_WriteCycle(hvme, vme_addr, src+(i<<1), vme->am, cvD16);
         n = n_bytes;
      /* FIFO BLT */
      } else if ((vme->blt_mode == MVME_BLT_BLT32FIFO) || (vme->blt_mode == MVME_BLT_MBLT64FIFO))
         status = CAENVME_FIFOBLTWriteCycle(hvme, vme_addr, src, n_bytes, vme->am, cvD16, &n);
      /* BLT */
      else
         status = CAENVME_BLTWriteCycle(hvme, vme_addr, src, n_bytes, vme->am, cvD16, &n);
   /* D32 */
   } else if (vme->dmode == MVME_DMODE_D32) {
      /* normal I/O */
      if (vme->blt_mode == MVME_BLT_NONE) {
         for (i=0 ; i<(n_bytes>>2) ; i++)
            status = CAENVME_WriteCycle(hvme, vme_addr, src+(i<<2), vme->am, cvD32);
         n = n_bytes;
      /* FIFO BLT */
      } else if (vme->blt_mode == MVME_BLT_BLT32FIFO)
         status = CAENVME_FIFOBLTWriteCycle(hvme, vme_addr, src, n_bytes, vme->am, cvD32, &n);
      /* BLT */
      else
         status = CAENVME_BLTWriteCycle(hvme, vme_addr, src, n_bytes, vme->am, cvD32, &n);
   /* D64 */
   } else if (vme->dmode == MVME_DMODE_D64) {
      /* FIFO MBLT */
      if (vme->blt_mode == MVME_BLT_MBLT64FIFO) 
         status = CAENVME_FIFOMBLTWriteCycle(hvme, vme_addr, src, n_bytes, vme->am, &n);
      /* MBLT */
      else
         status = CAENVME_MBLTWriteCycle(hvme, vme_addr, src, n_bytes, vme->am, &n);
   }

   if (status != cvSuccess)
      n = 0;

   return n;
}

/*------------------------------------------------------------------*/

int mvme_write_value(MVME_INTERFACE *vme, mvme_addr_t vme_addr, unsigned int value)
{
   int status=0, n;
   int hvme;
   hvme = vme->handle;

   if (vme->dmode == MVME_DMODE_D8)
      n = 1;
   else if (vme->dmode == MVME_DMODE_D16)
      n = 2;
   else
      n = 4;

   /* D8 */
   if (vme->dmode == MVME_DMODE_D8)
      status = CAENVME_WriteCycle(hvme, vme_addr, &value, vme->am, cvD8);
   /* D16 */
   else if (vme->dmode == MVME_DMODE_D16)
      status = CAENVME_WriteCycle(hvme, vme_addr, &value, vme->am, cvD16);
   /* D32 */
   else if (vme->dmode == MVME_DMODE_D32)
      status = CAENVME_WriteCycle(hvme, vme_addr, &value, vme->am, cvD32);

   if (status != cvSuccess)
      n = 0;

   return n;
}

/*------------------------------------------------------------------*/

int mvme_read(MVME_INTERFACE *vme, void *dst, mvme_addr_t vme_addr, mvme_size_t n_bytes)
{
   mvme_size_t i;
   int status=0, n;
   int hvme;
   hvme = vme->handle;

   n = 0;

   /* D8 */
   if ((vme->dmode == MVME_DMODE_D8) || (vme->blt_mode == MVME_BLT_NONE)) {
      for (i=0 ; i<n_bytes ; i++)
         status = CAENVME_ReadCycle(hvme, vme_addr, dst+i, vme->am, cvD8);
      n = n_bytes;
   /* D16 */
   } else if (vme->dmode == MVME_DMODE_D16) {
      /* normal I/O */
      if (vme->blt_mode == MVME_BLT_NONE) {
         for (i=0 ; i<(n_bytes>>1) ; i++)
            status = CAENVME_ReadCycle(hvme, vme_addr, dst+(i<<1), vme->am, cvD16);
         n = n_bytes;
      /* FIFO BLT */
      } else if ((vme->blt_mode == MVME_BLT_BLT32FIFO) || (vme->blt_mode == MVME_BLT_MBLT64FIFO))
         status = CAENVME_FIFOBLTReadCycle(hvme, vme_addr, dst, n_bytes, vme->am, cvD16, &n);
      /* BLT */
      else
         status = CAENVME_BLTReadCycle(hvme, vme_addr, dst, n_bytes, vme->am, cvD16, &n);
   /* D32 */
   } else if (vme->dmode == MVME_DMODE_D32) {
      /* normal I/O */
      if (vme->blt_mode == MVME_BLT_NONE) {
         for (i=0 ; i<(n_bytes>>2) ; i++)
            status = CAENVME_ReadCycle(hvme, vme_addr, dst+(i<<2), vme->am, cvD32);
         n = n_bytes;
      /* FIFO BLT */
      } else if (vme->blt_mode == MVME_BLT_BLT32FIFO)
         status = CAENVME_FIFOBLTReadCycle(hvme, vme_addr, dst, n_bytes, vme->am, cvD32, &n);
      /* BLT */
      else
         status = CAENVME_BLTReadCycle(hvme, vme_addr, dst, n_bytes, vme->am, cvD32, &n);
   /* D64 */
   } else if (vme->dmode == MVME_DMODE_D64) {
      /* FIFO MBLT */
      if (vme->blt_mode == MVME_BLT_MBLT64FIFO)
         status = CAENVME_FIFOMBLTReadCycle(hvme, vme_addr, dst, n_bytes, vme->am, &n);
      /* MBLT */
      else
         status = CAENVME_MBLTReadCycle(hvme, vme_addr, dst, n_bytes, vme->am, &n);
   }

   if ((status != cvSuccess) && (status != cvBusError))
      n = 0;

   return n;
}

/*------------------------------------------------------------------*/

unsigned int mvme_read_value(MVME_INTERFACE *vme, mvme_addr_t vme_addr)
{
   unsigned int data;
   int status=0;
   int hvme;
   hvme = vme->handle;

   data = 0;

   /* D8 */
   if (vme->dmode == MVME_DMODE_D8)
      status = CAENVME_ReadCycle(hvme, vme_addr, &data, vme->am, cvD8);
   /* D16 */
   else if (vme->dmode == MVME_DMODE_D16)
      status = CAENVME_ReadCycle(hvme, vme_addr, &data, vme->am, cvD16);
   /* D32 */
   else if (vme->dmode == MVME_DMODE_D32)
      status = CAENVME_ReadCycle(hvme, vme_addr, &data, vme->am, cvD32);

   return data;
}

/*------------------------------------------------------------------*/

int mvme_set_am(MVME_INTERFACE *vme, int am)
{
   vme->am = am;
   return MVME_SUCCESS;
}

/*------------------------------------------------------------------*/

int mvme_get_am(MVME_INTERFACE *vme, int *am)
{
   *am = vme->am;
   return MVME_SUCCESS;
}

/*------------------------------------------------------------------*/

int mvme_set_dmode(MVME_INTERFACE *vme, int dmode)
{
   vme->dmode = dmode;
   return MVME_SUCCESS;
}

/*------------------------------------------------------------------*/

int mvme_get_dmode(MVME_INTERFACE *vme, int *dmode)
{
   *dmode = vme->dmode;
   return MVME_SUCCESS;
}

/*------------------------------------------------------------------*/

int mvme_set_blt(MVME_INTERFACE *vme, int mode)
{
   vme->blt_mode = mode;
   return MVME_SUCCESS;
}

/*------------------------------------------------------------------*/

int mvme_get_blt(MVME_INTERFACE *vme, int *mode)
{
   *mode = vme->blt_mode;
   return MVME_SUCCESS;
}

/*------------------------------------------------------------------*/

/********************************************************************\

  Board specific functions

\********************************************************************/

WORD v1718_Read16(MVME_INTERFACE *mvme, DWORD base, int offset)
{
  int cmode;
  WORD data;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  data = mvme_read_value(mvme, base+offset);
  mvme_set_dmode(mvme, cmode);
  return data;
}

void v1718_Write16(MVME_INTERFACE *mvme, DWORD base, int offset, WORD value)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+offset, value);
  mvme_set_dmode(mvme, cmode);
}

DWORD v1718_Read32(MVME_INTERFACE *mvme, DWORD base, int offset)
{
  int cmode;
  DWORD data;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D32);
  data = mvme_read_value(mvme, base+offset);
  mvme_set_dmode(mvme, cmode);
  return data;
}

void v1718_Write32(MVME_INTERFACE *mvme, DWORD base, int offset, DWORD value)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D32);
  mvme_write_value(mvme, base+offset, value);
  mvme_set_dmode(mvme, cmode);
}

/*------------------------------------------------------------------*/

void v1718_MultiRead(MVME_INTERFACE *mvme, DWORD *addrs, DWORD *value, int ncycle, int *am, int *dmode)
{
   mvme_size_t i;
   CVAddressModifier *cvAMs;
   CVDataWidth *cvDWs;
   CVErrorCodes *cvECs;
   int hvme;
   hvme = mvme->handle;

   cvAMs = (CVAddressModifier *) malloc(ncycle*sizeof(CVAddressModifier));
   cvDWs = (CVDataWidth *) malloc(ncycle*sizeof(CVDataWidth));
   cvECs = (CVErrorCodes *) malloc(ncycle*sizeof(CVErrorCodes));

   for (i = 0; i < ncycle; i++) {
      cvAMs[i] = am[i];

      if (dmode[i] == MVME_DMODE_D8)
         cvDWs[i] = cvD8;
      else if (dmode[i] == MVME_DMODE_D16)
         cvDWs[i] = cvD16;
      else if (dmode[i] == MVME_DMODE_D32)
         cvDWs[i] = cvD32;
      else
         cvDWs[i] = cvD32;

      cvECs[i] = 0;
   }

   CAENVME_MultiRead(hvme, addrs, value, ncycle, cvAMs, cvDWs, cvECs);

   free(cvAMs);
   free(cvDWs);
   free(cvECs);
}

void v1718_MultiWrite(MVME_INTERFACE *mvme, DWORD *addrs, DWORD *value, int ncycle, int *am, int *dmode)
{
   mvme_size_t i;
   CVAddressModifier *cvAMs;
   CVDataWidth *cvDWs;
   CVErrorCodes *cvECs;
   int hvme;
   hvme = mvme->handle;

   cvAMs = (CVAddressModifier *) malloc(ncycle*sizeof(CVAddressModifier));
   cvDWs = (CVDataWidth *) malloc(ncycle*sizeof(CVDataWidth));
   cvECs = (CVErrorCodes *) malloc(ncycle*sizeof(CVErrorCodes));

   for (i = 0; i < ncycle; i++) {
      cvAMs[i] = am[i];

      if (dmode[i] == MVME_DMODE_D8)
         cvDWs[i] = cvD8;
      else if (dmode[i] == MVME_DMODE_D16)
         cvDWs[i] = cvD16;
      else if (dmode[i] == MVME_DMODE_D32)
         cvDWs[i] = cvD32;
      else
         cvDWs[i] = cvD32;

      cvECs[i] = 0;
   }

   CAENVME_MultiWrite(hvme, addrs, value, ncycle, cvAMs, cvDWs, cvECs);

   free(cvAMs);
   free(cvDWs);
   free(cvECs);
}

void v1718_MultiRead16(MVME_INTERFACE *mvme, DWORD *addrs, WORD *value, int ncycle)
{
   mvme_size_t i;
   DWORD *buffer;
   CVAddressModifier *cvAMs;
   CVDataWidth *cvDWs;
   CVErrorCodes *cvECs;
   int hvme;
   hvme = mvme->handle;

   buffer = (DWORD *) malloc(ncycle*sizeof(DWORD));
   cvAMs = (CVAddressModifier *) malloc(ncycle*sizeof(CVAddressModifier));
   cvDWs = (CVDataWidth *) malloc(ncycle*sizeof(CVDataWidth));
   cvECs = (CVErrorCodes *) malloc(ncycle*sizeof(CVErrorCodes));

   for (i = 0; i < ncycle; i++) {
      buffer[i] = 0;
      cvAMs[i] = mvme->am;
      cvDWs[i] = cvD16;
      cvECs[i] = 0;
   }

   CAENVME_MultiRead(hvme, addrs, buffer, ncycle, cvAMs, cvDWs, cvECs);

   for (i = 0; i < ncycle; i++)
      value[i] = buffer[i];

   free(buffer);
   free(cvAMs);
   free(cvDWs);
   free(cvECs);
}

void v1718_MultiWrite16(MVME_INTERFACE *mvme, DWORD *addrs, WORD *value, int ncycle)
{
   mvme_size_t i;
   DWORD *buffer;
   CVAddressModifier *cvAMs;
   CVDataWidth *cvDWs;
   CVErrorCodes *cvECs;
   int hvme;
   hvme = mvme->handle;

   buffer = (DWORD *) malloc(ncycle*sizeof(DWORD));
   cvAMs = (CVAddressModifier *) malloc(ncycle*sizeof(CVAddressModifier));
   cvDWs = (CVDataWidth *) malloc(ncycle*sizeof(CVDataWidth));
   cvECs = (CVErrorCodes *) malloc(ncycle*sizeof(CVErrorCodes));

   for (i = 0; i < ncycle; i++) {
      buffer[i] = value[i];
      cvAMs[i] = mvme->am;
      cvDWs[i] = cvD16;
      cvECs[i] = 0;
   }

   CAENVME_MultiWrite(hvme, addrs, buffer, ncycle, cvAMs, cvDWs, cvECs);

   free(buffer);
   free(cvAMs);
   free(cvDWs);
   free(cvECs);
}

void v1718_MultiRead32(MVME_INTERFACE *mvme, DWORD *addrs, DWORD *value, int ncycle)
{
   mvme_size_t i;
   CVAddressModifier *cvAMs;
   CVDataWidth *cvDWs;
   CVErrorCodes *cvECs;
   int hvme;
   hvme = mvme->handle;

   cvAMs = (CVAddressModifier *) malloc(ncycle*sizeof(CVAddressModifier));
   cvDWs = (CVDataWidth *) malloc(ncycle*sizeof(CVDataWidth));
   cvECs = (CVErrorCodes *) malloc(ncycle*sizeof(CVErrorCodes));

   for (i = 0; i < ncycle; i++) {
      cvAMs[i] = mvme->am;
      cvDWs[i] = cvD32;
      cvECs[i] = 0;
   }

   CAENVME_MultiRead(hvme, addrs, value, ncycle, cvAMs, cvDWs, cvECs);

   free(cvAMs);
   free(cvDWs);
   free(cvECs);
}

void v1718_MultiWrite32(MVME_INTERFACE *mvme, DWORD *addrs, DWORD *value, int ncycle)
{
   mvme_size_t i;
   CVAddressModifier *cvAMs;
   CVDataWidth *cvDWs;
   CVErrorCodes *cvECs;
   int hvme;
   hvme = mvme->handle;

   cvAMs = (CVAddressModifier *) malloc(ncycle*sizeof(CVAddressModifier));
   cvDWs = (CVDataWidth *) malloc(ncycle*sizeof(CVDataWidth));
   cvECs = (CVErrorCodes *) malloc(ncycle*sizeof(CVErrorCodes));

   for (i = 0; i < ncycle; i++) {
      cvAMs[i] = mvme->am;
      cvDWs[i] = cvD32;
      cvECs[i] = 0;
   }

   CAENVME_MultiWrite(hvme, addrs, value, ncycle, cvAMs, cvDWs, cvECs);

   free(cvAMs);
   free(cvDWs);
   free(cvECs);
}

/*------------------------------------------------------------------*/

/*****************************************************************/
/**
Simple configuration of the pulsers.
@param *mvme   VME structure
@param pulser  0=PulserA, 1=PulserB
@param period  period in ns
@param width   pulse width in ns
@param pulseNo number of pulses, 0=infinite
*/
void v1718_PulserConfSet(MVME_INTERFACE *mvme, WORD pulser, DWORD period, DWORD width, WORD pulseNo)
{
   CVPulserSelect pulSel = 0;
   CVOutputSelect outSel = 0;
   CVTimeUnits unit = 0;
   int hvme = mvme->handle;

   switch (pulser) {
   case v1718_pulserA:
      pulSel = cvPulserA;
      outSel = cvOutput0;
      break;
   case v1718_pulserB:
      pulSel = cvPulserB;
      outSel = cvOutput2;
      break;
   }
   if (period < 25*256) {
      period /= 25;
      width /= 25;
      unit = cvUnit25ns;
   } else if (period < 1600*256) {
      period /= 1600;
      width /= 1600;
      unit = cvUnit1600ns;
   } else if (period < 410000*256) {
      period /= 410000;
      width /= 410000;
      unit = cvUnit410us;
   } else {
      period /= 104000000;
      width /= 104000000;
      unit = cvUnit104ms;
   }
   if (width == 0)
      width = 1;
   else if (width > 255)
      width = 255;
   CAENVME_SetOutputConf(hvme, outSel, cvDirect, cvActiveHigh, cvMiscSignals);
   CAENVME_SetOutputConf(hvme, outSel+1, cvDirect, cvActiveHigh, cvMiscSignals);
   CAENVME_SetPulserConf(hvme, pulSel, period, width, unit, pulseNo, cvManualSW, cvManualSW);
}

/*------------------------------------------------------------------*/

void v1718_PulserStart(MVME_INTERFACE *mvme, WORD pulser)
{
   CVPulserSelect pulSel = 0;
   int hvme = mvme->handle;

   switch (pulser) {
   case v1718_pulserA:
      pulSel = cvPulserA;
      break;
   case v1718_pulserB:
      pulSel = cvPulserB;
      break;
   }
   CAENVME_StartPulser(hvme, pulSel);
}

/*------------------------------------------------------------------*/

void v1718_PulserStop(MVME_INTERFACE *mvme, WORD pulser)
{
   CVPulserSelect pulSel = 0;
   int hvme = mvme->handle;

   switch (pulser) {
   case v1718_pulserA:
      pulSel = cvPulserA;
      break;
   case v1718_pulserB:
      pulSel = cvPulserB;
      break;
   }
   CAENVME_StopPulser(hvme, pulSel);
}
