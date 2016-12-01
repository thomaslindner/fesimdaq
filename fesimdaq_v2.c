/********************************************************************\
Example period frontend
 Thomas Lindner (TRIUMF)
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
char *frontend_name = "fesimdaq";
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

INT read_trigger_event(char *pevent, INT off);
INT read_scaler_event(char *pevent, INT off);


/*-- Equipment list ------------------------------------------------*/

#undef USE_INT

EQUIPMENT equipment[] = {

   {"SIMDAQ",               /* equipment name */
    {1, 0,                   /* event ID, trigger mask */
     "SYSTEM",               /* event buffer */
#ifdef USE_INT
     EQ_INTERRUPT,           /* equipment type */
#else
     EQ_PERIODIC,              /* equipment type */
#endif
     LAM_SOURCE(0, 0xFFFFFF),        /* event source crate 0, all stations */
     "MIDAS",                /* format */
     TRUE,                   /* enabled */
     RO_RUNNING,           /* read only when running */
     1000,                    /* poll for 1000ms */
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

BOOL finished_readout=FALSE;

int nremaining = -1;
// Deferred transition where we take 10 extra events...
BOOL wait_end_cycle(int transition, BOOL first)
 {
   if(first){
     std::cout << "Starting deferred transition" << std::endl;
     nremaining = 10;
   }

   if (finished_readout) {
     return TRUE;
   }
   
   return FALSE;
 }


#define SIMDAQSETTINGS_STR(_name) const char *_name[] = {\
"[.]",\
"Descrip = STRING : [256] ",\
"Parameter AAA = INT : 0",\
"Parameter BB2 = INT : 0",			\
"Parameter CC22 = INT : 0",\
"",\
NULL }

SIMDAQSETTINGS_STR(simdaqsettings_str);


struct SIMDAQSETTINGS {
  char  comment[256];
  INT id;   
  INT family;
  INT  enable;
};

void seq_callback(INT h, INT hseq, void *info)
{

}


int is_utf8(const char * string)
{
    if(!string)
        return 0;

    const unsigned char * bytes = (const unsigned char *)string;
    while(*bytes)
    {
        if( (// ASCII
             // use bytes[0] <= 0x7F to allow ASCII control characters
                bytes[0] == 0x09 ||
                bytes[0] == 0x0A ||
                bytes[0] == 0x0D ||
                (0x20 <= bytes[0] && bytes[0] <= 0x7E)
            )
        ) {
            bytes += 1;
            continue;
        }

        if( (// non-overlong 2-byte
                (0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF)
            )
        ) {
            bytes += 2;
            continue;
        }

        if( (// excluding overlongs
                bytes[0] == 0xE0 &&
                (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// straight 3-byte
                ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
                    bytes[0] == 0xEE ||
                    bytes[0] == 0xEF) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            ) ||
            (// excluding surrogates
                bytes[0] == 0xED &&
                (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF)
            )
        ) {
            bytes += 3;
            continue;
        }

        if( (// planes 1-3
                bytes[0] == 0xF0 &&
                (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// planes 4-15
                (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
                (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            ) ||
            (// plane 16
                bytes[0] == 0xF4 &&
                (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
                (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                (0x80 <= bytes[3] && bytes[3] <= 0xBF)
            )
        ) {
            bytes += 4;
            continue;
        }

        return 0;
    }

    return 1;
}



/*-- Frontend Init -------------------------------------------------*/
INT frontend_init()
{

  HNDLE directoryKey, keyHandle;  
  int status = db_find_key(hDB,0,"/Equipment/SIMDAQ",&directoryKey);
  
  char key[16];
  key[0] = 0x45;
  key[1] = 0x75;
  key[2] = 0x72;
  key[3] = 0x7;
  key[4] = 0xe2;
  key[5] = 0x82;
  key[6] = 0xac;
  key[7] = 0x00;
  
  printf("Key %s\n",key);
  
  char keyName[160];
  sprintf(keyName,"/Equipment/SIMDAQ/%s",key);
  printf("Finding %s\n",keyName);
  
  //  HNDLE keyHandle;
  //status = db_find_key(hDB,0,"/Equipment/SIMDAQ",&directoryKey);
  //status = db_find_key(hDB,0,"/Equipment/SIMDAQ",&directoryKey); // step 2
  status = db_find_key(hDB,0,keyName,&keyHandle); // step 2

  
  printf("findkey status %i\n",status);


  
  if(status == DB_NO_KEY){
    printf("Key doesn't exist\n");
  } else if(status == DB_SUCCESS){
    status = db_delete_key(hDB,keyHandle,FALSE);    
    printf("Key exists; deletion %i\n",status);   
  }
    
  if(is_utf8(key))
    printf("%s is valid utf-8\n", key);
  else
    printf("%s is invalid utf-8\n", key);

  
  printf("creating new key\n");
  status = db_create_key(hDB,directoryKey,key,TID_INT);
  printf("Create key status = %i\n",status);
  

  for(int i = 0; i < 10; i++){
    printf("open record %i\n",i);

    char set_this_str[256];
    sprintf(set_this_str,"[.]\n\
Descrip = STRING : [256]\n\
Parameter AAA = INT : 0\n\
Parameter BB2 = INT : 0\n\
Parameter CC22 = INT : 0");
    
    char namm[256];
    sprintf(namm,"/Equipment/SIMDAQ/DFSettings_%i_DSFSFSD",i);
    //int status = db_create_record(hDB, 0, namm, strcomb(simdaqsettings_str));
    int status = db_create_record(hDB, 0, namm, set_this_str);
    

    HNDLE settings_handle_adc_;
    status = db_find_key (hDB, 0, namm, &settings_handle_adc_);
    int size = sizeof(SIMDAQSETTINGS);
    
    status = db_open_record(hDB, settings_handle_adc_, &simdaqsettings_str, size, MODE_READ, seq_callback, NULL);
    
    if (status != DB_SUCCESS){
      cm_msg(MERROR,"SetBoardRecord","Couldn't create hotlink for %s. Return code: %d", "BLAH", status);
      return status;
    }

  }

  printf("FFFF\n");
  //char mystring[32];
  //i/nt size = sizeof(mystring); 
  //int status = db_get_value(hDB, 0,"/test2/mystring", &mystring, &size, TID_STRING, 
  //                    FALSE);
    
  cm_register_deferred_transition(TR_STOP, wait_end_cycle);
  printf("Success\n");
  return SUCCESS;
}




/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
  printf("Exiting fedcrc!\n");
   return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/
// Upon run stasrt, read ODB settings and write them to DCRC
INT begin_of_run(INT run_number, char *error)
{

  nremaining = -1;
  finished_readout=FALSE;
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
//      cam_lam_read(LAM_SOURCE_CRATE(source), &lam);

//      if (lam & LAM_SOURCE_STATION(source))
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

#include "math.h" // for RAND, and rand
double sampleNormal() {
    double u = ((double) rand() / (RAND_MAX)) * 2 - 1;
    double v = ((double) rand() / (RAND_MAX)) * 2 - 1;
    double r = u * u + v * v;
    if (r == 0 || r > 1) return sampleNormal();
    double c = sqrt(-2 * log(r) / r);
    return u * c;
}
#include <sys/time.h>
/*-- Event readout -------------------------------------------------*/
INT read_trigger_event(char *pevent, INT off)
{

  /* init bank structure */
  bk_init32(pevent);
  
  uint32_t *pdata32;
  /* create structured ADC0 bank of double words (i.e. 4-byte words)  */
  bk_create(pevent, "ADC0", TID_DWORD, (void **)&pdata32);
 
  // Add a header, with number of words in event.
  // Use the top two bits to indicate different control words.
  // 11 -> 0xcXXXXXXX  : overall header
  // 01 -> 0x4XXXXXXX  : trigger header
  // 00 -> 0x0XXXXXXX  : channel header
  // 10 -> 0x8XXXXXXX  : trailer
  
  // Write the bank header to the bank, containing the number of triggers
  *pdata32++ = 0xfa000200;

  int sample = floor(sampleNormal()*80)+1200;
  if((rand() % 100) > 80) sample += 200;
  *pdata32++ = 0xf800406b + sample;

  int sample2 = floor(sampleNormal()*120)+1000;
  *pdata32++ = 0xf801405e + sample2;

  int sample3 = floor(sampleNormal()*100)+1000;
  *pdata32++ = 0xf802405e + sample3;

  int sample4 = floor(sampleNormal()*20)+950;
  *pdata32++ = 0xf803405e + sample4;

  int sample5 = floor(sampleNormal()*50)+1300;
  *pdata32++ = 0xf804405e + sample5;

  int sample6 = floor(sampleNormal()*200)+2000;
  *pdata32++ = 0xf805405e + sample6;



  *pdata32++ = 0xfcfd57e8;

  
  int size2 = bk_close(pevent, pdata32);    
  

  uint32_t *pdata322;
  bk_create(pevent, "ADC1", TID_DWORD, (void **)&pdata322);
 

  //for(int i = 0; i < 100000; i++)
    //*pdata322++ = i;
  
  size2 = bk_close(pevent, pdata322);    

  uint32_t *pdata720;
  bk_create(pevent, "W200", TID_DWORD, (void **)&pdata720);
 

  *pdata720++ = 0xa00001f8;
  *pdata720++ = 0xfff703;
  *pdata720++ = 0x0;
  *pdata720++ = 0x13132f;
  
  int pulse_position = floor(sampleNormal()*80) + 200;
  for(int j = 0; j < 2; j++){

    if (j==1) pulse_position+=  5;
    int pulse_height = floor(sampleNormal()*30) +200;  
    pulse_height = pulse_height * (1.0 - ((double)pulse_position)/600.0);

    for(int i = 0; i < 504; i++){
      uint32_t word = 0;
      uint32_t sample = 400 + floor(sampleNormal()*2);
      if(i == pulse_position) sample += 0.2 * pulse_height;
      if(i == pulse_position+1) sample += 0.8 * pulse_height;
      if(i == pulse_position+2) sample += 1.0 * pulse_height;
      if(i == pulse_position+3) sample += 0.9 * pulse_height;
      if(i == pulse_position+4) sample += 0.5 * pulse_height;
      if(i == pulse_position+5) sample += 0.3 * pulse_height;
      if(i == pulse_position+6) sample += 0.1 * pulse_height;    
      //std::cout << i << " " << pulse_position << " " << pulse_height << " " << sample << std::endl;
      word = sample;
      i++;
      sample = 400 + floor(sampleNormal()*2);
      if(i == pulse_position) sample += 0.2 * pulse_height;
      if(i == pulse_position+1) sample += 0.8 * pulse_height;
      if(i == pulse_position+2) sample += 1.0 * pulse_height;
      if(i == pulse_position+3) sample += 0.9 * pulse_height;
      if(i == pulse_position+4) sample += 0.5 * pulse_height;
      if(i == pulse_position+5) sample += 0.3 * pulse_height;
      if(i == pulse_position+6) sample += 0.1 * pulse_height;    
      //std::cout << j << " " << i << " " << pulse_position << " " << pulse_height << " " << sample << std::endl;
      word += (sample << 16);
      *pdata720++ = word;
    }  
  }
  
  

  size2 = bk_close(pevent, pdata720);    



  struct timeval start,end;
  gettimeofday(&start,NULL);
  if(0)printf ("simdaq request: %f\n",start.tv_sec
      + 0.000001*start.tv_usec); 
  // close the bank

  // handle the deferred transition
  if(nremaining > 0){ std::cout << "deferring " << nremaining << std::endl; nremaining--;
  };

  if(nremaining == 0){
    finished_readout=TRUE;
    std::cout << "Finished readout... " << std::endl;
  }

    /*  
  printf("Start sleep\n");
  bool done = false;
  double time;
  while(!done){
    sleep(5);
    gettimeofday(&end,NULL);
    time = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)*0.000001;
    if(time > 10) done = true;
  }
  
      //}
  printf("End sleep. time %f\n", time);
  */
  
  usleep(10000);
  return bk_size(pevent);
}



