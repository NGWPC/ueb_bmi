#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <bmi.hxx>

#include "ControlFile.hxx"
#include "Watershed.hxx"
#include "Parameters.hxx"
#include "SiteVariables.hxx"
#include "ForcingVariables.hxx"
#include "OutControl.hxx"
#include "bmi_ueb.hxx"


int main(int argc, char *argv[])
{

  if (argc != 2) {
    printf("Usage: run_bmiuebcxx CONTROL_FILE\n\n");
    printf("Run the uebcxx model through its BMI with a control file.\n");
    return bmi::BMI_SUCCESS;
  }

  int year, month, day, hour;

  //create the model object
  ueb::BmiUEB* model = bmi_model_create();

  std::cout << "Model name: " << model->GetComponentName() << std::endl;

  //setup the model
  std::cout << "Setup the model." << std::endl;
  model->Initialize(argv[1]);

  void* ts = new float();

  model->GetValue( std::string( "ts" ), ts );
  std::cout << "ts = " << *(float*)ts << std::endl;
/*
  float newts = 2.f;
  model->SetValue( std::string( "ts" ), &newts );
  model->GetValue( std::string( "ts" ), ts );
  std::cout << "New ts = " << *(float*)ts << std::endl;
*/

  //get the start time
  model->GetStartTimeInGregorianCalendar( year, month, day, hour );

  std::cout << "Model start datetime is " << year << '/' << month << '/'
	                                  << day << ' ' << hour << std::endl;
  //get the end time
  model->GetEndTimeInGregorianCalendar( year, month, day, hour );

  std::cout << "Model end datetime is " << year << '/' << month << '/'
	                                  << day << ' ' << hour << std::endl;
  //advance the model one timestep and pause
  
  std::cout << "Advance the model one timestep and pause." << std::endl;
  model->Update();

  //get the current time
  model->GetCurrentTimeInGregorianCalendar( year, month, day, hour );

  std::cout << "Model current datetime is " << year << '/' << month << '/'
	                                  << day << ' ' << hour << std::endl;

  double currenttime = model->GetCurrentTime();

  double tendaysafter = currenttime + 10.0 * 24 * 3600;

  //advance the model 10 days and  pause
  //
  std::cout << "Advance the model ten days and pause." << std::endl;
  model->UpdateUntil( tendaysafter );
  model->GetCurrentTimeInGregorianCalendar( year, month, day, hour );
  std::cout << "Model current datetime is " << year << '/' << month << '/'
	                                  << day << ' ' << hour << std::endl;

  double endtime = model->GetEndTime();

  //advance to the end
  std::cout << "Advance the model to the end." << std::endl;
  model->UpdateUntil( endtime );

  model->GetCurrentTimeInGregorianCalendar( year, month, day, hour );
  std::cout << "Model current datetime is " << year << '/' << month << '/'
	                                  << day << ' ' << hour << std::endl;
  //cleaning up
  std::cout << "Output and clean up." << std::endl;
  model->Finalize();

  //destroy the mdoel
  bmi_model_destroy( model );

  return bmi::BMI_SUCCESS;
}
