/********************************************************************\
Example frontend, which writes out a slow control bank of format TID_STRUCT.
Used as example for debugging sqlite problem.
\********************************************************************/

#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <algorithm>
#include <stdlib.h>
#include "midas.h"
#include <stdint.h>
#include <math.h>

/* make frontend functions callable from the C framework */
#ifdef __cplusplus
extern "C" {
#endif

/*-- Globals -------------------------------------------------------*/

char const *frontend_name = "readoutfe_";
char const *frontend_file_name = __FILE__;
BOOL frontend_call_loop = FALSE;
INT display_period = 000;
INT max_event_size_frag = 100000;
INT max_event_size = 256000;
INT event_buffer_size = 52 * 10000;


/*************** Baselines ****************/
///Structure of baselines bank
typedef struct { 
  double mean1;
  double mean2;
  double mean3;
  double mean4;
  double mean5;
  double mean6;
} Baselines_bank;



///String representation of Baselines struct needed for database creation
char* Baselines_bank_str[] = {
  "mean1 = DOUBLE : \n",
  "mean2 = DOUBLE : \n",
  "mean3 = DOUBLE : \n",
  "mean4 = DOUBLE : \n",
  "mean5 = DOUBLE : \n",
  "mean6 = DOUBLE : \n",
  NULL /*needed to close out array */
};

/*************** Overall definition of all banks************************/
BANK_LIST readoutfe_history_bank_list[] = {
  {"BSLN", TID_STRUCT, sizeof(Baselines_bank), 
   Baselines_bank_str},
  {""},
};


/*-- Function declarations -----------------------------------------*/

INT frontend_init();
INT frontend_exit();
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();


  INT process_history_event_v2(char* pevent, INT off);


/*-- Equipment list ------------------------------------------------*/

#undef USE_INT

EQUIPMENT equipment[] = {

   {"readouthistory%02d",     /* equipment name */
    {1, 0,                    /* event ID, trigger mask */
     "",                      /* event buffer (none) */
     EQ_PERIODIC,             /* equipment type */
     1,                       /* interrupt source, not used */
     "MIDAS",                 /* format */
      
     TRUE,                    /* enabled - switch to FALSE to disable */
     RO_RUNNING,              /* read only when running */
     10000,                   /* read every 10 s */
     0,                       /* stop run after this event limit */
     0,                       /* number of sub events */
     1,                       /* enable history */
     "", "", "",},
    process_history_event_v2,    /* readout routine */
    NULL,
    NULL,
    readoutfe_history_bank_list,
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

/*-- Frontend Init -------------------------------------------------*/
// Upon init, read ODB settings and write them to DCRC
INT frontend_init()
{
    

  return SUCCESS;
}





void things_to_do_when_starting_frontend_or_run (int initflag) {
    
    printf ("Done with this initialization\n");
    
}






/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
  return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/
// Upon run start, read ODB settings and write them to DCRC
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

/*-- Resume Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/

INT frontend_loop()
{
   /* if frontend_call_loop is true, this routine gets called when
      the frontend is idle or once between every event */
   return SUCCESS;
}


/*-- Trigger event routines ----------------------------------------*/
extern "C" { INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
  
  
  if (test) {
    for (int i = 0; i < count; i++) {
      usleep(2000);
    }
    return (FALSE);
  }
  
  return (FALSE);
  
}// end of poll_event
} // end of exertn C that goes with poll_event

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
  
  
  INT process_history_event_v2(char* pevent, INT off)
  {
    printf("In process_history_event!\n");
    INT status = SUCCESS;
    
    bk_init32(pevent);
    
    Baselines_bank* pbslbank;
    bk_create(pevent, "BSLN", TID_STRUCT, (void**)&pbslbank); 
    
    double rand_phonon_mean = rand()%100;    
    pbslbank->mean1 = rand_phonon_mean;
    pbslbank->mean2 = rand_phonon_mean;
    pbslbank->mean3 = rand_phonon_mean;
    pbslbank->mean4 = rand_phonon_mean;
    pbslbank->mean5 = rand_phonon_mean;
    pbslbank->mean6 = rand_phonon_mean;
      
    bk_close(pevent, pbslbank+1);    
    
    if(status != SUCCESS){
      cm_msg(MERROR,frontend_name,"fill_baseline_bank returned %d",status);
      return status;
    }
    
    return bk_size(pevent);
  };
   
  
#ifdef __cplusplus
}
#endif
