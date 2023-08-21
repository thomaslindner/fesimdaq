
/*******************************************************************\

  Name:         fesimdaq_tmfe.cxx
  Created by:   T. Lindner

  Contents:     simulated FE in TMFE format

\********************************************************************/

#undef NDEBUG // midas required assert() to be always enabled

#include <stdio.h>
#include <math.h> // M_PI

#include <iostream>

#include "tmfe.h"
#include "odbxx.h"
#include <stdio.h>
#include <stdlib.h>
#include <array>


#include "math.h" // for RAND, and rand
double sampleNormal() {
    double u = ((double) rand() / (RAND_MAX)) * 2 - 1;
    double v = ((double) rand() / (RAND_MAX)) * 2 - 1;
    double r = u * u + v * v;
    if (r == 0 || r > 1) return sampleNormal();
    double c = sqrt(-2 * log(r) / r);
    return u * c;
}

class EqEverything :
   public TMFeEquipment
{
public:
   EqEverything(const char* eqname, const char* eqfilename) // ctor
      : TMFeEquipment(eqname, eqfilename)
   {
      printf("fesimdaq constructure\n");
      fMfe->Msg(MINFO, "Cnst", "fesimdaq constructure");
      
      // configure the equipment here:
      fEqConfEventID = 1;
      fEqConfPeriodMilliSec = 1000;
      fEqConfLogHistory = 5;
      fEqConfWriteEventsToOdb = true;
      fEqConfEnablePeriodic = true;
      fEqConfEnablePoll     = false;
      fEqConfReadOnlyWhenRunning = true; // overwrite ODB Common RO_RUNNING to false
      fEqConfWriteEventsToOdb = true; // overwrite ODB Common RO_ODB to true
   }

   ~EqEverything() // dtor
   {
      printf("EqEverything::dtor!\n");
   }

   void HandleUsage()
   {
      printf("EqEverything::HandleUsage!\n");
   }

   TMFeResult HandleInit(const std::vector<std::string>& args)
   {

       //  midas::odb::set_debug(true);

     if(1){
       char names1[200];
       sprintf(names1,"Names BRV0");
       
       midas::odb o = {
	 {"host", "brb00"},
	 {"port", 40},
	 {names1, std::array<std::string, 6>{}},
       };
       
       
       // Set the names for the ODB keys
       o[names1][0] = "+6V Amp Current";
       o[names1][1] = "+6V Amp Voltage";
       o[names1][2] = "-5V PMT Current";
       o[names1][3] = "-5V PMT Voltage";
       o[names1][4] = "+1.8V ADC Current";
       o[names1][5] = "+1.8V ADC Voltage";
       
       o.connect("/Equipment/fesimdaq_tmfe/Settings");
     }
   
     printf("EqEverything::HandleInit!\n");
     EqSetStatus("Started...", "white");
     return TMFeOk();
   }


   TMFeResult HandleBeginRun(int run_number)
   {
      fMfe->Msg(MINFO, "HandleBeginRun", "fesimdaq Begin run %d!", run_number);
      EqSetStatus("Running", "#00FF00");
      return TMFeOk();
   }

   TMFeResult HandleEndRun(int run_number)
   {
      fMfe->Msg(MINFO, "HandleEndRun", "End run %d!", run_number);
      EqSetStatus("Stopped", "#FFFFFF");
      return TMFeOk();
   }



   void HandlePeriodic()
   {
      char buf[10000];
      ComposeEvent(buf, sizeof(buf));
      BkInit(buf, sizeof(buf));

      // Make ADC bank
      uint32_t* ptr = (uint32_t*)BkOpen(buf, "ADC0", TID_DWORD);
      
      // Add a header, with number of words in event.
      // Use the top two bits to indicate different control words.
      // 11 -> 0xcXXXXXXX  : overall header
      // 01 -> 0x4XXXXXXX  : trigger header
      // 00 -> 0x0XXXXXXX  : channel header
      // 10 -> 0x8XXXXXXX  : trailer
      
      // Write the bank header to the bank, containing the number of triggers
      *ptr++ = 0xfa000200;
      
      int sample = floor(sampleNormal()*80)+1200;
      if((rand() % 100) > 80) sample += 200;
      *ptr++ = 0xf800406b + sample;
      
      int sample2 = floor(sampleNormal()*120)+1000;
      *ptr++ = 0xf801405e + sample2;
      
      int sample3 = floor(sampleNormal()*100)+1000;
      *ptr++ = 0xf802405e + sample3;
      
      int sample4 = floor(sampleNormal()*20)+950;
      *ptr++ = 0xf803405e + sample4;
      
      int sample5 = floor(sampleNormal()*50)+1300;
      *ptr++ = 0xf804405e + sample5;
      
      int sample6 = floor(sampleNormal()*200)+2000;
      *ptr++ = 0xf805405e + sample6;



      *ptr++ = 0xfcfd57e8;

      BkClose(buf, ptr);
      
      
      // Make ADC bank
      uint32_t *pdata720  = (uint32_t*)BkOpen(buf, "W200", TID_DWORD);
 	
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
      
      BkClose(buf, pdata720);   
      
      EqSendEvent(buf);
      


   }

};


class EqTemperature :
   public TMFeEquipment
{
public:
  EqTemperature(const char* eqname, const char* eqfilename) // ctor
    : TMFeEquipment(eqname, eqfilename)
  {
    fMfe->Msg(MINFO, "Cnst", "temperature sensor constructur");
    
    // configure the equipment here:
    fEqConfEventID = 1;
    fEqConfPeriodMilliSec = 3764;
    fEqConfLogHistory = 5;
    fEqConfWriteEventsToOdb = true;
    fEqConfEnablePeriodic = true;
    fEqConfEnablePoll     = false;
    fEqConfReadOnlyWhenRunning = false; // overwrite ODB Common RO_RUNNING to false
    fEqConfWriteEventsToOdb = true; // overwrite ODB Common RO_ODB to true
  }

  ~EqTemperature(){}

   void HandleUsage()
   {
      printf("EqEverything::HandleUsage!\n");
   }

   TMFeResult HandleInit(const std::vector<std::string>& args)
   {

       //  midas::odb::set_debug(true);

     if(0){
       char names1[200];
       sprintf(names1,"Names TEMP");
       
       midas::odb o2 = {
	 {names1, std::array<std::string, 3>{}},
       };
       
       
       // Set the names for the ODB keys
       o2[names1][0] = "ADC Temperature";
       o2[names1][1] = "FPGA Temperature";
       o2[names1][2] = "Clock Cleaner Temperature";
       
       o2.connect("/Equipment/Temperature/Settings");
       
     }

     EqSetStatus("Running", "#00FF00");
     return TMFeOk();
   }


   TMFeResult HandleBeginRun(int run_number)
   {
     EqSetStatus("Running", "#00FF00");
      return TMFeOk();
   }

   TMFeResult HandleEndRun(int run_number)
   {
      return TMFeOk();
   }



   void HandlePeriodic()
   {
      char buf[10000];
      ComposeEvent(buf, sizeof(buf));
      BkInit(buf, sizeof(buf));

      // Make ADC bank
      double* ptr = (double*)BkOpen(buf, "TEMP", TID_DOUBLE);
      
      // Write the bank header to the bank, containing the number of triggers
      *ptr++ = (36.3  + sampleNormal()*0.4);
      *ptr++ = (43.3 + sampleNormal()*1.2);
      *ptr++ = (31.3 + sampleNormal()*2);
      
      BkClose(buf, ptr);
      
      EqSendEvent(buf);

   }

};



// example frontend

class FeEverything: public TMFrontend
{
public:
   FeEverything() // ctor
   {
      printf("FeEverything::ctor!\n");
      FeSetName("fesimdaq_tmfe");
      FeAddEquipment(new EqEverything("fesimdaq_tmfe", __FILE__));
      FeAddEquipment(new EqTemperature("Temperature", __FILE__));
   }

   void HandleUsage()
   {
      printf("FeEverything::HandleUsage!\n");
   };
   
   TMFeResult HandleArguments(const std::vector<std::string>& args)
   {
      printf("FeEverything::HandleArguments!\n");
      return TMFeOk();
   };
   
   TMFeResult HandleFrontendInit(const std::vector<std::string>& args)
   {
      printf("FeEverything::HandleFrontendInit!\n");
      return TMFeOk();
   };
   
   TMFeResult HandleFrontendReady(const std::vector<std::string>& args)
   {
      printf("FeEverything::HandleFrontendReady!\n");
      return TMFeOk();
   };
   
   void HandleFrontendExit()
   {
      printf("FeEverything::HandleFrontendExit!\n");
   };
};

// boilerplate main function

int main(int argc, char* argv[])
{
   FeEverything fe_everything;
   return fe_everything.FeMain(argc, argv);
}

