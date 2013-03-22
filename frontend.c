/********************************************************************\

  Name:         frontend.c
  Created by:   Jimmy Ngai
  Date:         May 9, 2010

	Modified by:  Tran Hoai Nam 
	Date: 2013-03-22

  Contents:     Experiment specific readout code (user part) of
                Midas frontend.
		Supported VME modules:
		CAEN V1718 VME-USB Bridge
		CAEN v792 32 CH QDC

\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "midas.h"
#include "mcstd.h"
#include "mvmestd.h"
//#include "experim.h"
#include "vme/v792.h"
#include "vme/v1718.h"

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
char *frontend_name = "Frontend";
/* The frontend file name, don't change it */
char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = FALSE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 3000;

/* maximum event size produced by this frontend */
INT max_event_size = 10000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 10 * 10000;

/* number of channels */
#define N_ADC      16

/* VME hardware */
MVME_INTERFACE *myvme;

/* VME base address */
/*DWORD V1718_BASE  = 0x12000000;*/
DWORD V792_BASE  = 0x54320000;

/*-- Function declarations -----------------------------------------*/

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();
INT computer_busy(int busy);

INT read_trigger_event(char *pevent, INT off);

/*-- Equipment list ------------------------------------------------*/

#undef USE_INT

EQUIPMENT equipment[] = {

   {"Trigger",               /* equipment name */
    {1, 0,                   /* event ID, trigger mask */
     "SYSTEM",               /* event buffer */
#ifdef USE_INT
     EQ_INTERRUPT,           /* equipment type */
#else
     EQ_POLLED,              /* equipment type */
#endif
		 // no idea about functionality of the LAM_SOURCE
     //LAM_SOURCE(CRATE, LAM_STATION(SLOT_ADC)), [> event source <]
     LAM_SOURCE(0, 0xFFFFFF), /* event source crate 0, all stations */
     "MIDAS",                /* format */
     TRUE,                   /* enabled */
     RO_RUNNING |            /* read only when running */
     RO_ODB,                 /* and update ODB */
     500,                    /* poll for 500ms */
     0,                      /* stop run after this event limit */
     0,                      /* number of sub events */
     0,                      /* don't log history */
     "", "", "",},
    read_trigger_event,      /* readout routine */
    },

   {""}
};

#ifdef __cplusplus
}
#endif

/********************************************************************\
              Callback routines for system transitions

  These routines are called whenever a system transition like start/
  stop of a run occurs. The routines are called on the following
  occations:

  frontend_init:  When the frontend program is started. This routine
                  should initialize the hardware.

  frontend_exit:  When the frontend program is shut down. Can be used
                  to releas any locked resources like memory, commu-
                  nications ports etc.

  begin_of_run:   When a new run is started. Clear scalers, open
                  rungates, etc.

  end_of_run:     Called on a request to stop a run. Can send
                  end-of-run event and close run gates.

  pause_run:      When a run is paused. Should disable trigger events.

  resume_run:     When a run is resumed. Should enable trigger events.

\********************************************************************/

INT init_vme_modules()
{
   /* default settings */

   v792_SoftReset(myvme, V792_BASE);
   v792_Setup(myvme, V792_BASE, 2);
//   v792_Status(myvme, V792_BASE);

   return SUCCESS;
}

/*-- Frontend Init -------------------------------------------------*/

INT frontend_init()
{
   INT status;

   /* open VME interface */
   status = mvme_open(&myvme, 0);

   /* set am to A32 non-privileged Data */
   mvme_set_am(myvme, MVME_AM_A32_ND);

	 /*CAENVME_SetOutputConf(BHandle,cvOutput0,cvDirect,cvActiveHigh,cvManualSW);*/
	 v792_BusErrorEnable(myvme,V792_BASE);
	 v1718_PulserConfSet(myvme,v1718_pulserA,1000,200,0);
	 computer_busy(1);
   /* initialize all VME modules */
   init_vme_modules();

   v792_OfflineSet(myvme, V792_BASE);
   v792_DataClear(myvme, V792_BASE);

   /* print message and return FE_ERR_HW if frontend should not be started */
   if (status != MVME_SUCCESS) {
      cm_msg(MERROR, "frontend_init", "VME interface could not be opened.");
      return FE_ERR_HW;
   }

   return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
   /* close VME interface */
   mvme_close(myvme);

   return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error)
{
   /* Initialize all VME modules */
	 init_vme_modules();

   v792_DataClear(myvme, V792_BASE);
   v792_OnlineSet(myvme, V792_BASE);

	 computer_busy(0);
   return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

INT end_of_run(INT run_number, char *error)
{
   v792_OfflineSet(myvme, V792_BASE);
   v792_DataClear(myvme, V792_BASE);

   return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

INT pause_run(INT run_number, char *error)
{
   v792_OfflineSet(myvme, V792_BASE);

   return SUCCESS;
}

/*-- Resuem Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error)
{
   v792_OnlineSet(myvme, V792_BASE);

   return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/

INT frontend_loop()
{
   /* if frontend_call_loop is true, this routine gets called when
      the frontend is idle or once between every event */

   return SUCCESS;
}

/*------------------------------------------------------------------*/

/********************************************************************\

  Readout routines for different events

\********************************************************************/

/*-- Trigger event routines ----------------------------------------*/

INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
   INT i;
   DWORD lam = 0;

   for (i = 0; i < count; i++) {
      lam = v792_DataReady(myvme, V792_BASE);

      if (lam)
         if (!test)
				 {
					 computer_busy(1);
					 return lam;
				 }
   }

   return 0;
}

/*-- Interrupt configuration ---------------------------------------*/

INT interrupt_configure(INT cmd, INT source, POINTER_T adr)
{
   switch (cmd) {
   case CMD_INTERRUPT_ENABLE:
      break;
   case CMD_INTERRUPT_DISABLE:
      break;
   case CMD_INTERRUPT_ATTACH:
      break;
   case CMD_INTERRUPT_DETACH:
      break;
   }
   return SUCCESS;
}

/*-- Event readout -------------------------------------------------*/

INT read_v792(INT base, const char *bk_name, char *pevent, INT n_chn)
{
   INT i;
   INT nentry = 0; 
   DWORD data[V792_MAX_CHANNELS+3];
   WORD *pdata;

   /* event counter */
//   v792_EvtCntRead(myvme, base, &counter);

   /* read event */
   v792_EventRead(myvme, base, data, &nentry);

   /* clear ADC */
	 v792_DataClear(myvme, base);

   /* create ADC bank */
   bk_create(pevent, bk_name, TID_WORD, &pdata);

   for (i = 0; i < n_chn; i++)
      pdata[i] = 0;

   for (i = 0; i < nentry; i++) {
      DWORD w = data[i];
      if (((w >> 24) & 0x7) != 0) continue;
      INT channel = (w >> 17) & 0xF;
      INT value = (w & 0x3FFF);
      pdata[channel] = value;
   }

   pdata += n_chn;

   bk_close(pevent, pdata);

   return nentry;
}
//#endif

INT read_trigger_event(char *pevent, INT off)
{
   /* init bank structure */
   bk_init(pevent);

   read_v792(V792_BASE, "ADC0", pevent, N_ADC);

	 computer_busy(0);
   return bk_size(pevent);
}

// simple computer busy logic
INT computer_busy(int busy) 
{
	if (busy) 
		v1718_PulserStop(myvme,v1718_pulserA);
	else
		v1718_PulserStart(myvme,v1718_pulserA);
	return busy;
}
