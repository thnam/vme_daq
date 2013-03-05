/*********************************************************************

  Name:         v2718.h
  Created by:   Jimmy Ngai

  Contents:     V2718 VME-USB2.0 bridge include

  $Id: $
*********************************************************************/
#ifndef  V2718_INCLUDE_H
#define  V2718_INCLUDE_H

#include <stdio.h>
#include <string.h>
#include "mvmestd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  V2718_STATUS_RO          (DWORD) (0x0000)
#define  V2718_VME_CTRL_RW        (DWORD) (0x0001)
#define  V2718_FW_REV_RO          (DWORD) (0x0002)
#define  V2718_FW_DWNLD_RW        (DWORD) (0x0003)
#define  V2718_FL_ENA_RW          (DWORD) (0x0004)
#define  V2718_IRQ_STAT_RO        (DWORD) (0x0005)
#define  V2718_IN_REG_RW          (DWORD) (0x0008)
#define  V2718_OUT_REG_S_RW       (DWORD) (0x000A)
#define  V2718_IN_MUX_S_RW        (DWORD) (0x000B)
#define  V2718_OUT_MUX_S_RW       (DWORD) (0x000C)
#define  V2718_LED_POL_S_RW       (DWORD) (0x000D)
#define  V2718_OUT_REG_C_WO       (DWORD) (0x0010)
#define  V2718_IN_MUX_C_WO        (DWORD) (0x0011)
#define  V2718_OUT_MAX_C_WO       (DWORD) (0x0012)
#define  V2718_LED_POL_C_WO       (DWORD) (0x0013)
#define  V2718_PULSEA_0_RW        (DWORD) (0x0016)
#define  V2718_PULSEA_1_RW        (DWORD) (0x0017)
#define  V2718_PULSEB_0_RW        (DWORD) (0x0019)
#define  V2718_PULSEB_1_RW        (DWORD) (0x001A)
#define  V2718_SCALER0_RW         (DWORD) (0x001C)
#define  V2718_SCALER1_RO         (DWORD) (0x001D)
#define  V2718_DISP_ADL_RO        (DWORD) (0x0020)
#define  V2718_DISP_ADH_RO        (DWORD) (0x0021)
#define  V2718_DISP_DTL_RO        (DWORD) (0x0022)
#define  V2718_DISP_DTH_RO        (DWORD) (0x0023)
#define  V2718_DISP_PC1_RO        (DWORD) (0x0024)
#define  V2718_DISP_PC2_RO        (DWORD) (0x0025)
#define  V2718_LM_ADL_RW          (DWORD) (0x0028)
#define  V2718_LM_ADH_RW          (DWORD) (0x0029)
#define  V2718_LM_C_RW            (DWORD) (0x002C)

WORD v2718_Read16(MVME_INTERFACE *mvme, DWORD base, int offset);
void v2718_Write16(MVME_INTERFACE *mvme, DWORD base, int offset, WORD value);
DWORD v2718_Read32(MVME_INTERFACE *mvme, DWORD base, int offset);
void v2718_Write32(MVME_INTERFACE *mvme, DWORD base, int offset, DWORD value);

void v2718_MultiRead(MVME_INTERFACE *mvme, DWORD *addrs, DWORD *value, int ncycle, int *am, int *dmode);
void v2718_MultiWrite(MVME_INTERFACE *mvme, DWORD *addrs, DWORD *value, int ncycle, int *am, int *dmode);
void v2718_MultiRead16(MVME_INTERFACE *mvme, DWORD *addrs, WORD *value, int ncycle);
void v2718_MultiWrite16(MVME_INTERFACE *mvme, DWORD *addrs, WORD *value, int ncycle);
void v2718_MultiRead32(MVME_INTERFACE *mvme, DWORD *addrs, DWORD *value, int ncycle);
void v2718_MultiWrite32(MVME_INTERFACE *mvme, DWORD *addrs, DWORD *value, int ncycle);

void v2718_PulserConfSet(MVME_INTERFACE *mvme, WORD pulser, DWORD period, DWORD width, WORD pulseNo);
void v2718_PulserStart(MVME_INTERFACE *mvme, WORD pulser);
void v2718_PulserStop(MVME_INTERFACE *mvme, WORD pulser);

  enum v2718_PulserSelect {
    v2718_pulserA=0x0,
    v2718_pulserB=0x1,
  };

#ifdef __cplusplus
}
#endif

#endif // V2718_INCLUDE_H

/* emacs
 * Local Variables:
 * mode:C
 * mode:font-lock
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
