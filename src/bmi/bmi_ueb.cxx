#include <stdio.h>

#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>

#include <bmi.hxx>
#include "bmi_ueb.hxx"


void ueb::BmiUEB::
Initialize (std::string config_file)
{
  if (config_file.compare("") != 0 )
  {
     _confile = ControlFile( config_file );
     _ws = Watershed( _confile.getWatershedFile(),
		      _confile.getWsvarName(),
		      _confile.getWsycorName(),
		      _confile.getWsxcorName() );
     _parms = Parameters( _confile.getParamFile() );
     _sitevars = SiteVariables( _confile.getSitevarFile() );
     _forcings = ForcingVariables( _confile.getInputconFile() );
     _outcontrol = OutControl( _confile.getOutputconFile() );
     _currentModelDateTime = this->GetStartTime();

     this->setStatev();

     //
     //initialize cumulation values
     //
     auto numOfActiveCells = 
                _ws.getActiveCells( _sitevars.getSiteVars().data() ).size();

     _Ws1.resize( numOfActiveCells, 0.f); 
     _Wc1.resize( numOfActiveCells, 0.f); 
     _cumP.resize( numOfActiveCells, 0.f); 
     _cumEs.resize( numOfActiveCells, 0.f); 
     _cumEc.resize( numOfActiveCells, 0.f); 
     _cumMr.resize( numOfActiveCells, 0.f); 
     _cumGm.resize( numOfActiveCells, 0.f); 
     _cumEg.resize( numOfActiveCells, 0.f); 
    

     int numTotalTs = _confile.getModelTotalTimeSteps();
     int nstepinaDay = _confile.getStepsInADay();

     _tsprevday.resize( numOfActiveCells );
     _taveprevday.resize( numOfActiveCells );

     float Us,Ws, Wc, Apr, cg, rhog, de, tave, WGT;
      // WGT=WATER EQUIVALENT GLACIER THICKNESS

     auto Param = _parms.getParams();

     _outvarArray.resize( numOfActiveCells );

     for ( int cell = 0; cell < numOfActiveCells; ++cell)
     {
         auto sitev = this->getSitevForCell(  cell );
	 auto SiteState = this->getSiteState( cell );
         float ts_last = SiteState[30];

	 _Ws1[ cell ] = _statev[ cell ][ 1 ];
	 _Wc1[ cell ] = _statev[ cell ][ 3 ];

         if (sitev[9] ==0 || sitev[9] ==3)
	    WGT = 0.0;
         else
	    WGT = 1.0;      

         if(sitev[9] != 3)  //  Only do this work for non accumulation cells where model is run
	 {
            _tsprevday[cell].resize( nstepinaDay, -9999.f );
            _taveprevday[cell].resize( nstepinaDay, -9999.f );

            // Take surface temperature as 0 where it is unknown the previous time step
	    // This is for first day of the model to get the force restore going
	    //#$#$#$#$#_is this all the time steps or the last time?
	    if(ts_last <= -9999)
		_tsprevday[cell][nstepinaDay-1] = 0;
 	    else 
		_tsprevday[cell][nstepinaDay-1] = ts_last;

     // compute Ave.Temp for previous day 
            Us   = _statev[cell][0]; // Ub in UEB
            Ws   = _statev[cell][1]; // W in UEB
            Wc   = _statev[cell][3]; // Canopy SWE
            Apr  = sitev[1];  // Atm. Pressure  [PR in UEB]
            cg   = Param[3];  // Ground heat capacity [nominally 2.09 KJ/kg/C]
            rhog = Param[7];  // Soil Density [nominally 1700 kg/m^3]
            de   = Param[10]; // Thermally active depth of soil (0.1 m)

    	    tave = TAVG(Us,Ws+WGT,Rho_w,C_s,T_0, rhog, de, cg, H_f);    
   	    _taveprevday[cell][nstepinaDay-1] = tave;      
         }

	 _outvarArray[ cell ].resize( numOut );
         for ( auto& v : _outvarArray[ cell ] )
	 {
		 v.resize( numTotalTs ); 
	 }
     }
  }
}


void ueb::BmiUEB::
Update()
{
     int Year, Month, Day, MYear, MMonth, MDay; 	  
     double dHour;
     float fHour, fModeldt;
     double modelDT = this->GetTimeStep ();
     calendardate(_currentModelDateTime,Year, Month, Day, dHour);
     double UTCHour = dHour - _confile.getModelUTCOffset();

     auto activeCells = _ws.getActiveCells( _sitevars.getSiteVars().data() );

     auto Param = _parms.getParams();

     float bca = _parms.getParArray()[ 30 ];
     float bcc = _parms.getParArray()[ 31 ];

     float dtbar[12];
     float Ta, P, V, RH, Trange, Qsiobs, Qg, Qli;
     float Qnetob,Snowalb,atff, Ema,Eacl,errMB, HRI;

     float Inpt[niv];

     int istep = std::round( ( this->GetCurrentTime() - this->GetStartTime() ) 
		             * 24 / this->GetTimeStep() );
     
     auto tsvarArray = _forcings.getTsvarArray();

     int irad = _parms.getIrad();
     int ireadalb=_parms.getParArray()[ 1 ];

     int numTotalTs = _confile.getModelTotalTimeSteps();
     
     int nstepinaDay = _confile.getStepsInADay();

     float mtime[4], outv[nov], OutArr[53];  

     std::fill( outv, outv + nov, 0.f );
     std::fill( OutArr, OutArr + 53, 0.f );

     for ( int cell = 0; cell < activeCells.size(); ++cell )
     {
	uebCellY = activeCells[cell].first;
   	uebCellX = activeCells[cell].second;

        auto sitev = this->getSitevForCell( cell );
        auto SiteState = this->getSiteState( cell );
        float slope = SiteState[12];
	float azi=SiteState[13];
	float lat=SiteState[14];
        int subtype= (int)SiteState[16];

        Param[11]=SiteState[15];
        Param[21]=SiteState[17];

	for(int i=0;i<12;i++)
	     dtbar[i] = SiteState[i+18];

	float lon = SiteState[31];          

        if(sitev[9] != 3)
	{            
           this->prepareInputForPoint( UTCHour,        //input
				    dHour,            //input
		                    lon,              //input
				    Year,             //input
			            Month,            //input
				    Day,              //input
				    modelDT,          //input
		                    nstepinaDay,      //input
				    istep,            //input
				    numTotalTs,       //input
			    	    irad,             //input
                                    tsvarArray,       //input
				    MYear,            //output
				    MMonth,           //output
				    MDay,             //output
				    fHour,            //output
				    fModeldt,         //output
		                    sitev,            //input/output
			            Trange,	      //output
		                    Qsiobs,           //output
				    Qli,              //output
				    Qnetob,           //output
				    Qg,               //output
			            Ta,               //output
				    P,                //output
				    V,                //output
		                    RH,               //output
				    Snowalb );        //output
       
           this->runPointUEB( 
                             cell,               //input
	                     Year,               //input
                    	     Month,              //input
	                     Day,                //input
	                     MYear,              //input
	                     MMonth,             //input
	                     MDay,               //input
	                     fHour,              //input
	                     dHour,              //input
	                     fModeldt,           //input
	                     nstepinaDay,        //input
                             slope,    	         //input
                             azi,                //input
                             lat,                //input
	                     irad,               //input
	                     ireadalb,           //input
	                     sitev,              //input/output
	                     Trange,             //input
	                     Qsiobs,             //input
	                     Qli,                //input
	                     Qnetob,             //input
	                     Qg,                 //input
	                     Ta,                 //input
	                     P,                  //input
	                     V,                  //input
	                     RH,                 //input
	                     Snowalb,            //input
                             dtbar,              //array of 12, input
                             bca,	         //input
                             bcc,	         //input
                             Param,              //Input
	                     Inpt,         //array of niv elements, output 
	                     atff,               //output
                             HRI,                //output
       	                     Ema,                //output
                             Eacl,               //output,
                             OutArr );           //array of 53 elements, output     
          this->updatOutVars( 
                             cell,               //input
                             istep,              //input
	                     Year,               //input
	                     Month,              //input
	                     Day,                //input
	                     dHour,           //input
	                     Inpt,         //array of niv elements, input 
	                     atff,                  //input
                             HRI,                    //input
       	                     Ema,                    //input
                             Eacl,                   //input,
                             OutArr );      //array of 53 elements,  input     
		  
           }
	   else //sitev[9] ==3 // this block entered only if sitev(10)= 3
	   {
                errMB = 0.0;
		for (int i=0;i<53;i++)
		      OutArr[i] = 0.0;
		for (int i= 0;i <70;i++)
		      _outvarArray[cell][i][istep] = 0.0;              
	   }     
     }

     UPDATEtime(Year, Month, Day, dHour,modelDT);  			
     _currentModelDateTime = julian(Year, Month, Day, dHour); 
}


void ueb::BmiUEB::
UpdateUntil(double t)
{
  double time;

  time = this->GetCurrentTime();
  double dt = this->GetTimeStep() / 24; //convert hour to day

  {
    double n_steps = (t - time) / dt + 1;
    double frac;

    for (int n=0; n<int(n_steps); n++)
    {
      this->Update();
    }

 //   frac = n_steps - int(n_steps);
//    this->_model.dt = frac * dt;
//    this->_model.advance_in_time();
//    this->_model.dt = dt;
  }
}


void ueb::BmiUEB::
Finalize()
{
     auto activeCells = _ws.getActiveCells( _sitevars.getSiteVars().data() );
     int numTimeStep = _confile.getModelTotalTimeSteps();

     auto pOut = _outcontrol.getPOut();
     int npout = _outcontrol.getNumPointOut();

     for (int irank = 0; irank < activeCells.size(); ++irank )
     {
		uebCellY = activeCells[irank].first;
   	        uebCellX = activeCells[irank].second;

		//point outputs
		for (int ipout = 0; ipout < npout; ipout++)
		{
			if (uebCellY == pOut[ipout].ycoord && uebCellX == pOut[ipout].xcoord)
			{
				std::cerr << "output point " << uebCellY
					<< ", " << uebCellX << std::endl;
				FILE* pointoutFile = fopen((const char*)pOut[ipout].outfName, "w");
				for (int istep = 0; istep < numTimeStep; istep++)
				{
					fprintf(pointoutFile, "\n %d %d %d %8.3f ", (int)_outvarArray[irank][0][istep], (int)_outvarArray[irank][1][istep], (int)_outvarArray[irank][2][istep], _outvarArray[irank][3][istep]);
					for (int vnum = 4; vnum < 70; vnum++)
						fprintf(pointoutFile, " %16.6f ", _outvarArray[irank][vnum][istep]);
				}
				fclose(pointoutFile);
			}
		}
       } // 
}


int ueb::BmiUEB::
GetVarGrid(std::string name)
{
  if (name.compare("plate_surface__temperature") == 0)
    return 0;
  else
    return -1;
}


std::string ueb::BmiUEB::
GetVarType(std::string name)
{
  if (name.compare("plate_surface__temperature") == 0)
    return "double";
  else
    return "";
}


int ueb::BmiUEB::
GetVarItemsize(std::string name)
{
  if (name.compare("plate_surface__temperature") == 0)
    return sizeof(double);
  else
    return 0;
}


std::string ueb::BmiUEB::
GetVarUnits(std::string name)
{
  if (name.compare("plate_surface__temperature") == 0)
    return "K";
  else
    return "";
}


int ueb::BmiUEB::
GetVarNbytes(std::string name)
{
  int itemsize;
  int gridsize;

  itemsize = this->GetVarItemsize(name);
  gridsize = this->GetGridSize(this->GetVarGrid(name));
  
  return itemsize * gridsize;
}


std::string ueb::BmiUEB::
GetVarLocation(std::string name)
{
  if (name.compare("plate_surface__temperature") == 0)
    return "node";
  else
    return "";
}


void ueb::BmiUEB::
GetGridShape(const int grid, int *shape)
{
/*
  if (grid == 0) {
    shape[0] = this->_model.shape[0];
    shape[1] = this->_model.shape[1];
  }
  */

     std::cerr << "to be implemented" << std::endl;
}


void ueb::BmiUEB::
GetGridSpacing (const int grid, double * spacing)
{
//  if (grid == 0) {
//    spacing[0] = this->_model.spacing[0];
//    spacing[1] = this->_model.spacing[1];
//  }
     std::cerr << "to be implemented" << std::endl;
}


void ueb::BmiUEB::
GetGridOrigin (const int grid, double *origin)
{
//  if (grid == 0) {
//    origin[0] = this->_model.origin[0];
//    origin[1] = this->_model.origin[1];
//  }
     std::cerr << "to be implemented" << std::endl;
}


int ueb::BmiUEB::
GetGridRank(const int grid)
{
  if (grid == 0)
    return 2;
  else
    return -1;
}


int ueb::BmiUEB::
GetGridSize(const int grid)
{
	/*
  if (grid == 0)
    return this->_model.shape[0] * this->_model.shape[1];
  else
    return -1;
    */
     std::cerr << "to be implemented" << std::endl;
     return -1;
}


std::string ueb::BmiUEB::
GetGridType(const int grid)
{
  if (grid == 0)
    return "uniform_rectilinear";
  else
    return "";
}


void ueb::BmiUEB::
GetGridX(const int grid, double *x)
{
  throw NotImplemented();
}


void ueb::BmiUEB::
GetGridY(const int grid, double *y)
{
  throw NotImplemented();
}


void ueb::BmiUEB::
GetGridZ(const int grid, double *z)
{
  throw NotImplemented();
}


int ueb::BmiUEB::
GetGridNodeCount(const int grid)
{
	/*
  if (grid == 0)
    return this->_model.shape[0] * this->_model.shape[1];
  else
    return -1;
	*/
	return -1;
}


int ueb::BmiUEB::
GetGridEdgeCount(const int grid)
{
  throw NotImplemented();
}


int ueb::BmiUEB::
GetGridFaceCount(const int grid)
{
  throw NotImplemented();
}


void ueb::BmiUEB::
GetGridEdgeNodes(const int grid, int *edge_nodes)
{
  throw NotImplemented();
}


void ueb::BmiUEB::
GetGridFaceEdges(const int grid, int *face_edges)
{
  throw NotImplemented();
}


void ueb::BmiUEB::
GetGridFaceNodes(const int grid, int *face_nodes)
{
  throw NotImplemented();
}


void ueb::BmiUEB::
GetGridNodesPerFace(const int grid, int *nodes_per_face)
{
  throw NotImplemented();
}


void ueb::BmiUEB::
GetValue (std::string name, void *dest)
{
  void * src = NULL;
  int nbytes = 0;

  src = this->GetValuePtr(name);
  nbytes = this->GetVarNbytes(name);

  memcpy (dest, src, nbytes);
}


void *ueb::BmiUEB::
GetValuePtr (std::string name)
{
  if (name.compare("plate_surface__temperature") == 0)
    //return (void*)this->_model.z[0];
    return (void*)NULL;
  else
    return NULL;
}


void ueb::BmiUEB::
GetValueAtIndices (std::string name, void *dest, int *inds, int len)
{
  void * src = NULL;

  src = this->GetValuePtr(name);

  if (src) {
    int i;
    int itemsize = 0;
    int offset;
    char *ptr;

    itemsize = this->GetVarItemsize(name);

    for (i=0, ptr=(char *)dest; i<len; i++, ptr+=itemsize) {
      offset = inds[i] * itemsize;
      memcpy(ptr, (char *)src + offset, itemsize);
    }
  }
}


void ueb::BmiUEB::
SetValue (std::string name, void *src)
{
  void * dest = NULL;

  dest = this->GetValuePtr(name);

  if (dest) {
    int nbytes = 0;
    nbytes = this->GetVarNbytes(name);
    memcpy(dest, src, nbytes);
  }
}


void ueb::BmiUEB::
SetValueAtIndices (std::string name, int * inds, int len, void *src)
{
  void * dest = NULL;

  dest = this->GetValuePtr(name);

  if (dest) {
    int i;
    int itemsize = 0;
    int offset;
    char *ptr;

    itemsize = this->GetVarItemsize(name);

    for (i=0, ptr=(char *)src; i<len; i++, ptr+=itemsize) {
      offset = inds[i] * itemsize;
      memcpy((char *)dest + offset, ptr, itemsize);
    }
  }
}


std::string ueb::BmiUEB::
GetComponentName()
{
  return "The Utah Energy Balance Snow Accumulation and Melt Model";
}


int ueb::BmiUEB::
GetInputItemCount()
{
  return Parameters::npar + 
	  NSITEVARS +
          NFORCS;
}


int ueb::BmiUEB::
GetOutputItemCount()
{
  return NOUTPUTS;
}


std::vector<std::string> ueb::BmiUEB::
GetInputVarNames()
{
  std::vector<std::string> names;

  for (int i=0; i<Parameters::npar; i++)
    names.push_back(Parameters::parameter_names[i]);

  for (int i=0; i<NFORCS; i++)
    names.push_back(ForcingVariables::forcing_var_names[i]);

  for (int i=0; i<NSITEVARS; i++)
    names.push_back(SiteVariables::site_var_names[i]);

  return names;
}


std::vector<std::string> ueb::BmiUEB::
GetOutputVarNames()
{
  std::vector<std::string> names;

  for (int i=0; i<NOUTPUTS; i++)
    names.push_back(OutControl::output_var_names[i]);

  return names;
}


double ueb::BmiUEB::
GetStartTime () {
   auto ModelStartDate = _confile.getModelStartDate();
   int Year = ModelStartDate[0];
   int Month = ModelStartDate[1];
   int Day = ModelStartDate[2];          
   double sHour = _confile.getModelStartHour();
   double  currentModelDateTime = julian(Year, Month, Day, sHour);
   return currentModelDateTime;
}


double ueb::BmiUEB::
GetEndTime () {
   auto ModelEndDate = _confile.getModelEndDate();
   double dHour = _confile.getModelEndHour();
   double EJD = julian(ModelEndDate[0], ModelEndDate[1], ModelEndDate[2],dHour);     
   return EJD;
}


double ueb::BmiUEB::
GetCurrentTime () {
  return _currentModelDateTime;
}


std::string ueb::BmiUEB::
GetTimeUnits() {
  return "h";
}


double ueb::BmiUEB::
GetTimeStep () {
  double modelDT = _confile.getModelDt();
  int stepinaDay= (int) (24.0/modelDT +0.5);  // closest rounding
  modelDT = 24.0/stepinaDay;
  return modelDT;
}

void ueb::BmiUEB::
GetCurrentTimeInGregorianCalendar( int& y, int& m, int& d, int& h )
{
     double dHour;
     calendardate(_currentModelDateTime,y, m, d, dHour);
     h = std::round( dHour );
}

void ueb::BmiUEB::
GetEndTimeInGregorianCalendar( int& y, int& m, int& d, int& h ) {
   double dHour;
   double start = this->GetEndTime();
   calendardate( start, y, m, d, dHour);
   h = std::round( dHour );
}

void ueb::BmiUEB::
GetStartTimeInGregorianCalendar( int& y, int& m, int& d, int& h ) {
   double dHour;
   double start = this->GetStartTime();
   calendardate( start, y, m, d, dHour);
   h = std::round( dHour );
}

std::array<float, NSITEVARS> ueb::BmiUEB::getSiteState( int const& cell )
{
     std::array< float, NSITEVARS > SiteState;

     std::array<sitevar, NSITEVARS> strsvArray = _sitevars.getSiteVars();
     auto activeCells = _ws.getActiveCells( strsvArray.data() );
     int uebCellY = activeCells[cell].first;
     int uebCellX = activeCells[cell].second;

     for (int is = 0; is < NSITEVARS; is++)
     {
	if (strsvArray[is].svType == 1)
	     SiteState[is] = strsvArray[is].svArrayValues[uebCellY][uebCellX];
	else
    	     SiteState[is] = strsvArray[is].svdefValue;
     }
     return SiteState;
}

void ueb::BmiUEB::setStatev()
{
    auto activeCells = _ws.getActiveCells( _sitevars.getSiteVars().data() );

    _statev.resize( activeCells.size() );
    for ( int i = 0; i < activeCells.size(); ++i )
    {
         auto SiteState = this->getSiteState( i );
         for ( int j = 0; j < 4; ++j )
	 {
           _statev[i][j] = SiteState[j];
	 }
    }
}

std::array< float, ueb::BmiUEB::nsv >  ueb::BmiUEB::getSitevForCell( 
		                                            int const& i )
{
    std::array< float, ueb::BmiUEB::nsv > sitev;

    auto SiteState = this->getSiteState( i );
    sitev[0] = SiteState[4];
    sitev[1]   = SiteState[5];
    for (int j = 3; j<9; j++)
       sitev[j]=SiteState[j+3];
    sitev[9]=SiteState[16];

    return sitev;
}

void  ueb::BmiUEB::prepareInputForPoint( double const& UTCHour,   //input
					 double const& dHour,     //input
		                         float const& lon,        //input
					 int const& Year,         //input
					 int const& Month,        //input
					 int const& Day,          //input
					 double const& modelDT,   //input
		                         int const& nstepinaDay,  //input
				         int const& istep,        //input
					 int const& numTotalTs,   //input
					 int const& irad,         //input
              std::array<float*, NFORCS> const& tsvarArray,       //input
					 int& MYear,              //output
					 int& MMonth,             //output
					 int& MDay,               //output
					 float& fHour,            //output
					 float& fModeldt,         //output
		 std::array< float, ueb::BmiUEB::nsv >& sitev,    //input/output
				         float& Trange,           //output
		                         float& Qsiobs,           //output
					 float& Qli,              //output
					 float& Qnetob,           //output
					 float& Qg,               //output
					 float& Ta,               //output
					 float& P,                //output
					 float& V,                //output
					 float& RH,               //output
					 float& Snowalb )         //output
{

          float Vp, Tmin = 0.0, Tmax = 0.0;
          int nb, nf, nbstart, nfend;

	  double OHour = UTCHour + lon/15.0;
 	  double UTCJulDat = julian(Year, Month, Day ,OHour);
	  double MHour;
	  calendardate(UTCJulDat, MYear, MMonth, MDay, MHour);
	  fHour = (float) MHour;
	  fModeldt = (float) modelDT;

	  //      Map from wrapper input variables to UEB variables     
	  if ( _confile.getInpDailyorSubdaily() == 0)       //inputs are given for each time step (sub-daily time step)--in m/hr units 
	  {	
	     P = tsvarArray[0][istep];  // / 24000;   #12.19.14 --Daymet prcp in mm/day				             
	     Ta =tsvarArray[1][istep];
	     V = tsvarArray[4][istep];
	     //get min max temp
	     Tmin = tsvarArray[2][istep]; 
	     Tmax = tsvarArray[3][istep];
				   
	     Trange = 8;
	     //get max/min temperature during the day 
	     nb = (dHour - 0)/modelDT;	//number of time steps before current time within same day
	     nf = (24 - dHour)/modelDT;      //no of time steps after current time within the same day
	     //#_TBC 9.13.13 look for better method for the following
	     if(dHour > 23)                  //to take care of hour 24, <=>0hr
	     {
	        nb = 0;
	        nf = 24/modelDT;
	     }
	     nbstart = findMax(istep-nb,0);  //to guard against going out of lower bound near start time when the start time is not 0 hr (istep < nb )
	     nfend = findMin(istep + nf, numTotalTs - 1); //don't go out of upper limit		
				
	     for (int it = nbstart; it < nfend; ++it)
	     {
	        if (tsvarArray[1][it] <= Tmin)					
		   Tmin = tsvarArray[1][it];
		if (tsvarArray[1][it] >= Tmax)					
		   Tmax = tsvarArray[1][it];
	     }
	     Trange = Tmax - Tmin;
	     //cout<<Trange<<endl;
	     if (Trange <= 0)
	     {
	        if (snowdgtvariteflag==1)	
		{
		   cout<<"Input Diurnal temperature range is less than or equal to 0 which is unrealistic "<<endl;
		   cout<< "Diurnal temperature range is assumed as 8 degree celsius on "<<endl;
		   cout<< Year<<" "<< Month<<" "<<Day<<endl;
		}
		Trange = 8.0;
	     }

		//Flag to control radiation (irad)
  //!     0 is no measurements - radiation estimated from diurnal temperature range
  //!     1 is incoming shortwave radiation read from file (measured), incoming longwave estimated
  //!     2 is incoming shortwave and longwave radiation read from file (measured)
  //!     3 is net radiation read from file (measured)
   	     switch(irad)
	     {
	       case 0:
		Qsiobs = tsvarArray[8][1];
                Qli = tsvarArray[9][1];
	        Qnetob = tsvarArray[10][1];						
	        break;
	       case 1:
	        Qsiobs = tsvarArray[8][istep];                         // *3.6; // Daymet srad in W/m^2
		Qli = tsvarArray[9][1];
		Qnetob = tsvarArray[10][1];
		break;
	       case 2:
		Qsiobs = tsvarArray[8][istep];                         // *3.6; // Daymet srad in W/m^2
		Qli = tsvarArray[9][istep];
		Qnetob = tsvarArray[10][1];
		break;
	       case 3:
		Qsiobs = tsvarArray[8][1];
		Qli = tsvarArray[9][1];
		Qnetob = tsvarArray[10][istep];
		break;
	       default:
	        cout<<" The radiation flag is not the right number; must be between 0 and 3"<<endl;
	        getchar();
	        break;				
	    }
	    //atm. pressure from netcdf 10.30.13
	    //this may need revision 		//####TBC_6.20.13
	    if(tsvarArray[7][0] == 2)
	        sitev[1] = tsvarArray[7][1];
	    else
	        sitev[1] = tsvarArray[7][istep];

	   //this may need revision 		//####TBC_6.20.13
	    if(tsvarArray[11][0] == 2)
	       Qg = tsvarArray[11][1];
	    else
	       Qg = tsvarArray[11][istep];
	    //!     Flag to control albedo (ireadalb)  
	    //!     0 is no measurements - albedo estimated internally
            //!     1 is albedo read from file (provided: measured or obtained from another model)
	    //these need revision //####TBC_6.20.13
	    if(tsvarArray[12][0] == 2)
	       Snowalb = tsvarArray[12][1];
	    else
	       Snowalb = tsvarArray[12][istep];	

	    //12.18.14 Vapor pressure of air
	    if(tsvarArray[6][0] == 2)
          	Vp = tsvarArray[6][1];
	    else
	        Vp = tsvarArray[6][istep];
	    //relative humidity computed or read from file
	    //#12.18.14 may need revision
	    if (tsvarArray[5][0] == 2)
	    {
	        RH = tsvarArray[5][1];					   
	    }
	    else if (tsvarArray[5][0] == -1)          //RH computed internally 
	    {
	        float eSat = 611 * exp(17.27*Ta / (Ta + 237.3)); //Pa
	        RH = Vp / eSat;
	    }
	    else
	        RH = tsvarArray[5][istep];
	    if (RH > 1)
	    {
	        //cout<<"relative humidity >= 1 at time step "<<istep<<endl;
	        RH = 0.99;
	    }
	  }   //if ( inpDailyorSubdaily == 0)
	  else		//inputs are given as AVERAGE daily values, precip unit is always m/hr, Tmax and Tmin are required
	  {
	    //average daily value of precip in units of m/hr 4.22.14
	    P = tsvarArray[0][istep / nstepinaDay];            // /24000 #12.19.14 Daymet prcp in mm/day           
	    V = tsvarArray[4][istep/nstepinaDay];
				    
	    //get min max temp
	    Tmin = tsvarArray[2][istep/nstepinaDay];				  
	    Tmax = tsvarArray[3][istep/nstepinaDay];
	    //cout << "Tmin = " << Tmin << "Tmax = " << Tmax << " ";

	    Trange = Tmax - Tmin;
	    //cout<<Trange<<endl;
	    if (Trange <= 0)
	    {
	       if (snowdgtvariteflag==1)	
	       {
		cout<<"Input Diurnal temperature range is less than or equal to 0 which is unrealistic "<<endl;
		cout<< "Diurnal temperature range is assumed as 8 degree celsius on "<<endl;
		cout<< Year<<" "<< Month<<" "<<Day<<endl;
	       }
	       Trange = 8.0;
	    }
	    //sin curve describes the diel temperature fluctuations with max at 15 hrs and min at 3 hrs
	    Ta = Tmin + 0.5*Trange + 0.5*Trange*sin(2*P_i*(fHour + 15.0)/24);
	    //! Flag to control radiation (irad)
	    //!     0 is no measurements - radiation estimated from diurnal temperature range
	    //!     1 is incoming shortwave radiation read from file (measured), incoming longwave estimated
	    //!     2 is incoming shortwave and longwave radiation read from file (measured)
	    //!     3 is net radiation read from file (measured)
	    switch(irad)
	    {
	        case 0:
		   Qsiobs = tsvarArray[8][1];
		   Qli = tsvarArray[9][1];
		   Qnetob = tsvarArray[10][1];						
		   break;
		case 1:
		   Qsiobs = tsvarArray[8][istep / nstepinaDay];                 // *3.6; // Daymet srad in W/m^2
		   Qli = tsvarArray[9][1];
		   Qnetob = tsvarArray[10][1];
		   break;
		case 2:
		   Qsiobs = tsvarArray[8][istep / nstepinaDay];                 // *3.6; // Daymet srad in W/m^2
		   Qli = tsvarArray[9][istep/nstepinaDay];
		   Qnetob = tsvarArray[10][1];
		   break;
		case 3:
		   Qsiobs = tsvarArray[8][1];
		   Qli = tsvarArray[9][1];
		   Qnetob = tsvarArray[10][istep/nstepinaDay];
		   break;
		default:
		   cout<<" The radiation flag is not the right number; must be between 0 and 3"<<endl;
		   getchar();
		   break;				
	      }
	      //atm. pressure from netcdf 10.30.13
	      //this may need revision 		//####TBC_6.20.13
	      if(tsvarArray[7][0] == 2)
	        sitev[1] = tsvarArray[7][1];
	      else
	        sitev[1] = tsvarArray[7][istep/nstepinaDay];

	      //this may need revision 		//####TBC_6.20.13
	      if(tsvarArray[11][0] == 2)
		Qg = tsvarArray[11][1];
	      else
	        Qg = tsvarArray[11][istep/nstepinaDay];
	      //!     Flag to control albedo (ireadalb)  
	      //!     0 is no measurements - albedo estimated internally
	      //!     1 is albedo read from file (provided: measured or obtained from another model)
	      //these need revision //####TBC_6.20.13
	      if(tsvarArray[12][0] == 2)
		Snowalb = tsvarArray[12][1];
  	      else
	        Snowalb = tsvarArray[12][istep/nstepinaDay];
	      //12.18.14 Vapor pressure of air
	      if (tsvarArray[6][0] == 2)
		Vp = tsvarArray[6][1];
	      else
		Vp = tsvarArray[6][istep/nstepinaDay];
	      //relative humidity computed or read from file
	      //#12.18.14 may need revision
	      if (tsvarArray[5][0] == 2)
	      {
		RH = tsvarArray[5][1];
	      }
	      else if (tsvarArray[5][0] == -1)          //RH computed internally 
	      {
	         float eSat = 611 * exp(17.27*Ta / (Ta + 237.3)); //Pa
		 RH = Vp / eSat;
	      }
	      else
	          RH = tsvarArray[5][istep/nstepinaDay];
	      if (RH > 1)
	      {
	          //cout<<"relative humidity >= 1 at time step "<<istep<<endl;
		  RH = 0.99;
	      }
	}	
}

void  ueb::BmiUEB::runPointUEB( 
         int const& cell,               //input
	 int const& Year,               //input
	 int const& Month,              //input
	 int const& Day,                //input
	 int const& MYear,              //input
	 int const& MMonth,             //input
	 int const& MDay,               //input
	 float const& fHour,            //input
	 double const& dHour,           //input
	 float const& fModeldt,         //input
	 int&       nstepinaDay,        //input
         float const& slope,	        //input
         float const& azi,              //input
         float const& lat,              //input
	 int const& irad,               //input
	 int const& ireadalb,           //input
	 std::array< float, ueb::BmiUEB::nsv >& sitev,    //input/output
	 float const& Trange,           //input
	 float const& Qsiobs,           //input
	 float const& Qli,              //input
	 float const& Qnetob,           //input
	 float const& Qg,               //input
	 float const& Ta,               //input
	 float const& P,                //input
	 float const& V,                //input
	 float const& RH,               //input
	 float const& Snowalb,          //input
         float* dtbar,                 //array of 12, input
         float const& bca,	        //input
         float const& bcc,	        //input
         std::array<float, NPARS>& Param, //Input
	 float* Inpt,         //array of niv elements, output 
	 float&  atff,                  //output
         float& HRI,                    //output
       	 float& Ema,                    //output
         float& Eacl,                   //output,
         float* OutArr )      //array of 53 elements,  output     
{				  //

        float HRI0, cosZen;
	float atfimplied;
	float cf;
	float QLif;
	float mtime[4], outv[nov];
        int iradfl, iflag[6] = {0, 0, 0, 0, 0, 0}; 

        std::fill( outv, outv + nov, 0.f );

        float as = Param[27];
        float bs = Param[28];
	//  Below is code from point UEB 
	sitev[2]= Qg;   
	Inpt[0] =Ta;
	Inpt[1]=P;
	Inpt[2]=V;
	Inpt[3]=RH;
	Inpt[6] = Qnetob;         
			
        //Radiation Input Parameterization  
	hyri(MYear, MMonth, MDay, fHour, fModeldt,slope, azi, lat, HRI, cosZen); 
	Inpt[7] = cosZen;                
	if (irad <= 2)
	{   
	   atf(atff,Trange, Month,dtbar,bca,bcc);      
	   // We found that Model reanalysis and dowscaled data may produce some unreasonably negative solar radiation. this is simply bad data and it is generally better policy to try to give a model good data. 
	   // If this is not possible, then the UEB checks will avoid the model doing anything too bad, it handles negative solar radiation in following way:
	   // "no data in radiation would be to switch to the temperature method just for time steps when radiation is negative." 

	    if( irad ==0 || Qsiobs < 0)     //  For cases where input is strictly negative we calculate QSI from HRI and air temp range.  This covers the case of missing data being flagged with negative number, i.e. -9999.                 
	    {   
		Inpt[4] = atff* Io *HRI;
		cloud(as, bs, atff,cf);   // For cloudiness fraction
	    }
	    else   // Here incoming solar is input
	    {
	       //      Need to call HYRI for horizontal surface to perform horizontal measurement adjustment
	       hyri(MYear, MMonth, MDay, fHour, fModeldt, 0.0, azi, 
			       lat, HRI0, cosZen);
	        //      If HRI0 is 0 the sun should have set so QSIOBS should be 0.  If it is
		//      not it indicates a potential measurement problem. i.e. moonshine
		if(HRI0 > 0) 
		{
		   //cout<<Qsiobs;
		   atfimplied =  findMin(Qsiobs/(HRI0*Io),0.9); // To avoid unreasonably large radiation when HRI0 is small
		   Inpt[4] = atfimplied * HRI * Io;
		 }
		 else
		 {
		     Inpt[4] = Qsiobs; 
		     if(Qsiobs != 0)
		     {
		       if (radwarnflag < 3)   //leave this warning only three times--enough to alert to non- -ve night time solar rad
		       {
			   cout<<"Warning: you have nonzero nightime incident radiation of "<<Qsiobs<<endl;
			   cout<<"at date "<<Year<<"   "<< Month<<"   "<< Day<<"     "<<dHour<<endl;
			   ++radwarnflag;
			}
		      }
	          }//else
  		  cloud(as,bs,atff,cf);   // For cloudiness fraction  This is more theoretically correct
	      } //else        
	      if(irad < 2)
	      {
		qlif(Ta, RH, T_k, SB_c, Ema,Eacl,cf,QLif);
		Inpt[5] = QLif;
	      }
	      else 
	      {
	         Ema = -9999;  //  These values are not evaluated but may need to be written out so are assigned for completeness
		 Eacl = -9999;
		 Inpt[5] = Qli;
	      } 
	      iradfl = 0;  
	   }   // Long wave or shortwave either measured and calculated
	   else
	   {
	        iradfl = 1;                    // This case is when given IRAD =3 (From Net Radiation)  
	        Inpt[6] = Qnetob;           
	    }        
	
    	    //      set control flags
	     iflag[0] = iradfl;   // radiation [0=radiation is shortwave in col 5 and longwave in col 6, else = net radiation in column 7]
				//  In the code above radiation inputs were either computed or read from input files
	     iflag[1] = 0;        // no 0 [/yes 1] printing
	     //iflag[2] = outFile;        // Output unit to which to print
	    if(ireadalb == 0)
	      iflag[3] = 1;        // Albedo Calculation [a value 1 means albedo is calculated, otherwise statev[3] is albedo
	    else
	    {
	    	iflag[3]=0;
		_statev[cell][2]=Snowalb;
	     }  
				
	     /*	if (istep >=48)
	     {
		snowdgtvariteflag = 1;
		snowdgtvariteflag2 = 1;
		snowdgtvariteflag2 = 1;
		getchar();
	     }*/

	     //added 9.16.13
	     iflag[4] = 4;
	     mtime[0] = Year;
	     mtime[1] = Month;
	     mtime[2] = Day;
	     mtime[3] = dHour;
				
	     SNOWUEB2(fModeldt, Inpt, sitev.data(), _statev[cell].data(),
					       	_tsprevday[cell].data(), 
						_taveprevday[cell].data(),
					       	nstepinaDay, 
						Param.data(),
						iflag,
					        _cumP[cell], 
						_cumEs[cell], 
						_cumEc[cell], 
						_cumMr[cell], 
						_cumGm[cell], 
						_cumEg[cell], 
						 outv, mtime, 
						  atff, cf, OutArr);     
		  
}

void  ueb::BmiUEB::updatOutVars( 
         int const& cell,               //input
	 int const& istep,              //input
	 int const& Year,               //input
	 int const& Month,              //input
	 int const& Day,                //input
	 double const& dHour,           //input
	 float const* Inpt,         //array of niv elements, input 
	 float const&  atff,                  //input
         float const& HRI,                    //input
       	 float const& Ema,                    //input
         float const& Eacl,                   //input,
         float const* OutArr )      //array of 53 elements,  input     
{

      float dStorage = _statev[cell][1]-_Ws1[cell]+ _statev[cell][3]-_Wc1[cell];
      float errMB= _cumP[cell]-_cumMr[cell]-_cumEs[cell]-_cumEc[cell] 
		    -dStorage+_cumGm[cell] - _cumEg[cell]; 
				
      _outvarArray[cell][0][istep]= Year;
      _outvarArray[cell][1][istep]=Month;
      _outvarArray[cell][2][istep]=Day;
      _outvarArray[cell][3][istep]=dHour;
      _outvarArray[cell][4][istep]=atff;
      _outvarArray[cell][5][istep]=HRI;
      _outvarArray[cell][6][istep]=Eacl;
      _outvarArray[cell][7][istep]=Ema;
      _outvarArray[cell][8][istep]=Inpt[7]; //cosZen
      _outvarArray[cell][9][istep]=Inpt[0];
      _outvarArray[cell][10][istep]=Inpt[1];
      _outvarArray[cell][11][istep]=Inpt[2];
      _outvarArray[cell][12][istep]=Inpt[3];
      _outvarArray[cell][13][istep]=Inpt[4];
      _outvarArray[cell][14][istep]=Inpt[5];
      _outvarArray[cell][15][istep]=Inpt[6];	
							
      for (int i=16;i<69;i++)
      {		   
 	  _outvarArray[cell][i][istep] = OutArr[i-16];					
      }
      _outvarArray[cell][69][istep] = errMB;

      if (snowdgt_outflag == 1)             //if debug mode 
      {
	       for(int uit = 0; uit<70; uit++)
	       cout<<_outvarArray[cell][uit][istep]<<" ";
	       cout<<endl;
               cout<<endl;
      }
}

extern "C"
{
    /**
    * Construct this BMI instance as a normal C++ object, to be returned to the framework.
    *
    * @return A pointer to the newly allocated instance.
    */
	ueb::BmiUEB *bmi_model_create()
	{
		return new ueb::BmiUEB();
	}

    /**
     * @brief Destroy/free an instance created with @see bmi_model_create
     * 
     * @param ptr 
     */
	void bmi_model_destroy(ueb::BmiUEB *ptr)
	{
		delete ptr;
	}
}
