/********************************************************************\

  Name:         frontend.c
  Created by:   Jimmy Ngai

  Date:         May 9, 2010

  Contents:     Experiment specific readout code (user part) of
                Midas frontend.
		Supported VME modules:
		CAEN V1718 VME-USB Bridge
		CAEN V792N 16 CH QDC

  $Id: $

\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "midas.h"
#include "mcstd.h"
#include "mvmestd.h"
//#include "experim.h"
#include "vme/v792n.h"

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
DWORD V1718_BASE  = 0x12000000;
DWORD V792N_BASE  = 0x32100000;

/*-- Function declarations -----------------------------------------*/

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();

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

   v792n_SoftReset(myvme, V792N_BASE);
   v792n_Setup(myvme, V792N_BASE, 2);
//   v792n_Status(myvme, V792N_BASE);

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

   /* initialize all VME modules */
   init_vme_modules();

   v792n_OfflineSet(myvme, V792N_BASE);
   v792n_DataClear(myvme, V792N_BASE);

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
//   init_vme_modules();

   v792n_DataClear(myvme, V792N_BASE);
   v792n_OnlineSet(myvme, V792N_BASE);

   return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

INT end_of_run(INT run_number, char *error)
{
   v792n_OfflineSet(myvme, V792N_BASE);
   v792n_DataClear(myvme, V792N_BASE);

   return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

INT pause_run(INT run_number, char *error)
{
   v792n_OfflineSet(myvme, V792N_BASE);

   return SUCCESS;
}

/*-- Resuem Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error)
{
   v792n_OnlineSet(myvme, V792N_BASE);

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
      lam = v792n_DataReady(myvme, V792N_BASE);

      if (lam)
         if (!test)
            return lam;
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

INT read_v792n(INT base, const char *bk_name, char *pevent, INT n_chn)
{
   INT i;
   INT nentry = 0; 
   DWORD data[V792N_MAX_CHANNELS+2];
   WORD *pdata;

   /* event counter */
//   v792n_EvtCntRead(myvme, base, &counter);

   /* read event */
   v792n_EventRead(myvme, base, data, &nentry);

   /* clear ADC */
//   v792n_DataClear(myvme, base);

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

   read_v792n(V792N_BASE, "ADC0", pevent, N_ADC);

   return bk_size(pevent);
}
