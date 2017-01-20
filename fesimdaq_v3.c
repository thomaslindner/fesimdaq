/********************************************************************\
Example period frontend
 Thomas Lindner (TRIUMF)

Example MIDAS frontend, testing writing TID_STRUCT banks to ODB
\********************************************************************/


#include <vector>
#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include "midas.h"
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <unistd.h>

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
char *frontend_name = "fesimdaq_v3";
/* The frontend file name, don't change it */
char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = TRUE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 0;

/* maximum event size produced by this frontend */
INT max_event_size =  3 * 1024 * 1024;

/* maximum event size for fragmented events (EQ_FRAGMENTED) --- not really used here */
INT max_event_size_frag = 2 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 20 * 1000000;
 void **info;
char  strin[256];
HNDLE hDB, hSet;






/*-- Function declarations -----------------------------------------*/

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();


#define MAX_CHARGE_CHANNEL 2
#define MAX_PHONON_CHANNEL 5
#define SUM_CHANNELS       7
#ifndef xstr
#define str(s) #s
#define xstr(s) str(s)
#endif

///String representation of test struct needed for database creation
char* Assorted_bank_str[] = {
  "assorted1   = DOUBLE : \n",
  "assorted2   = INT : \n",
  "assorted3   = INT[2] : \n",
  "assorted4   = DOUBLE[3] : \n",
  NULL /*needed to close out array */
};
///String representation of Baselines struct needed for database creation
char* Baselines_bank_str[] = {
  "phonon_mean = DOUBLE[" xstr(MAX_PHONON_CHANNEL) "] : \n",
  "phonon_rms  = DOUBLE[" xstr(MAX_PHONON_CHANNEL) "] : \n",
  "charge_mean = DOUBLE[" xstr(MAX_CHARGE_CHANNEL) "] : \n",
  "charge_rms  = DOUBLE[" xstr(MAX_CHARGE_CHANNEL) "] : \n",
  NULL /*needed to close out array */
};

/*************** Baselines ****************/
///Structure of assorted bank
typedef struct { 
  double assorted1;
  int assorted2;
  int assorted3[2];
  double assorted4[3];
} Assorted_bank;

///Structure of baselines bank
typedef struct { 
  double phonon_mean[MAX_PHONON_CHANNEL];
  double phonon_rms [MAX_PHONON_CHANNEL];
  double charge_mean[MAX_CHARGE_CHANNEL];
  double charge_rms [MAX_CHARGE_CHANNEL];
} Baselines_bank;

/*************** Overall definition of all banks************************/
BANK_LIST history_bank_list[] = {
  {"ASOT", TID_STRUCT, sizeof(Assorted_bank), 
   Assorted_bank_str},
  {"BSLN", TID_STRUCT, sizeof(Baselines_bank), 
   Baselines_bank_str},
  {""},
};

INT process_history_event(char *pevent, INT off);

/*-- Equipment list ------------------------------------------------*/

#undef USE_INT

EQUIPMENT equipment[] = {

   {"SIMDAQ3",               /* equipment name */
    {1, 0,                   /* event ID, trigger mask */
     "",               /* event buffer */
     EQ_PERIODIC,              /* equipment type */
     1,        /* event source crate 0, all stations */
     "MIDAS",                /* format */
     TRUE,                   /* enabled */
     RO_RUNNING,           /* read only when running */
     4000,                    /* poll for 1000ms */
     0,                      /* stop run after this event limit */
     0,                      /* number of sub events */
     1,                      /* enable history */
     "", "", "",},
    process_history_event,      /* readout routine */
    NULL,
    NULL,
    history_bank_list,
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

void seq_callback(INT h, INT hseq, void *info)
{

}

/*-- Frontend Init -------------------------------------------------*/
INT frontend_init()
{


  return SUCCESS;
}




/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
   return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/
// Upon run stasrt, read ODB settings and write them to DCRC
INT begin_of_run(INT run_number, char *error)
{

   return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

INT end_of_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

INT pause_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Resuem Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/

INT frontend_loop()
{
   /* if frontend_call_loop is true, this routine gets called when
      the frontend is idle or once between every event */
  usleep(10000);
  return SUCCESS;
}

/*------------------------------------------------------------------*/

/********************************************************************\

  Readout routines for different events

\********************************************************************/

/*-- Trigger event routines ----------------------------------------*/
// Not currently used for DCRC readout
extern "C" { INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
   int i;

   for (i = 0; i < count; i++) {
         if (!test)
            return 1;
   }

   usleep(1000);
   return 0;
}
}

/*-- Interrupt configuration ---------------------------------------*/
// This is not currently used by the DCRC readout
extern "C" { INT interrupt_configure(INT cmd, INT source, POINTER_T adr)
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
}

#ifdef __cplusplus
extern "C" {
#endif


  INT process_history_event(char* pevent, INT off)
  {
    printf("In process_history_event!\n");
    
    bk_init32(pevent);
    
    Assorted_bank* asotbank;
    bk_create(pevent, "ASOT", TID_STRUCT, (void**)&asotbank);
    asotbank->assorted1 = rand()%100;
    asotbank->assorted2 = rand()%100;
    asotbank->assorted3[0] = rand()%100;
    asotbank->assorted3[1] = asotbank->assorted3[0] +1;
    asotbank->assorted4[0] = rand()%100;
    asotbank->assorted4[1] = asotbank->assorted4[0] +1;
    asotbank->assorted4[2] = asotbank->assorted4[0] + 2;

    bk_close(pevent, asotbank+1);

   

    Baselines_bank* pbslbank;
    bk_create(pevent, "BSLN", TID_STRUCT, (void**)&pbslbank);
    double rand_phonon_mean = rand()%100;
    double rand_charge_mean = rand_phonon_mean; //rand()%100;
    double rand_phonon_rms = rand()%10;
    double rand_charge_rms = rand_phonon_rms; //rand()%10;
        
    for(int ch=0; ch<MAX_PHONON_CHANNEL; ch++)
    {
      pbslbank->phonon_mean[ch] = rand_phonon_mean;
      pbslbank->phonon_rms[ch] = rand_phonon_rms;
    }    
    for(int ch=0; ch<MAX_CHARGE_CHANNEL; ch++)
    {
      pbslbank->charge_mean[ch] = rand_charge_mean;
      pbslbank->charge_rms[ch] = rand_charge_rms;
    }

    bk_close(pevent, pbslbank+1);
    
    return bk_size(pevent);
  };
  
  
  
#ifdef __cplusplus
}
#endif



