#include <string>
#include <array>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <iterator>
#include <ios>
#include "Parameters.hxx"
#include "uebpgdecls.h"

ueb::Parameters::Parameters(){ }

ueb::Parameters::Parameters( std::string const& parfile )
{
   _parfile = parfile;

   float* parArray = (float*)NULL; 

   readParams(parfile.c_str(), parArray, npar );

   std::copy_n( parArray, npar, _parArray.begin() );
   delete[] parArray;
}

//copy constructor
ueb::Parameters::Parameters( Parameters const& p)
{
   _parfile = p.getParFile();
   _parArray = p.getParArray();

}

ueb::Parameters::~Parameters(){ }

std::string ueb::Parameters::getParFile() const
{
    return _parfile;
}

void ueb::Parameters::setParFile( std::string const& parfile)
{
    _parfile = parfile;
}

std::array<float, NPARS> ueb::Parameters::getParArray() const
{
    return _parArray;
}

void ueb::Parameters::setParArray( std::array<float, NPARS> const& parms)
{
    _parArray = parms;
}

std::array<float, NPARS> ueb::Parameters::getParams() const
{
    std::array< float, NPARS > Param;
    for(int i=0;i<11;i++)
		Param[i] = _parArray[i+2];
    for(int i=12;i<18;i++)
		Param[i] = _parArray[i+1];      
    Param[18]=-9999;
    Param[19]=-9999;
    Param[20]=_parArray[19];
    for(int i=22;i<32;i++)
	Param[i] = _parArray[i-2];      

    return Param;
}

int ueb::Parameters::getIrad() const
{
    return (int)_parArray[0];
}	

/*
 * Model Parameters
 *
irad:  Radiation control flag (0=from ta, 1= input qsi, 2= input qsi,qli 3= input qnet)
ireadalb:  Albedo reading control flag (0=albedo is computed internally, 1 albedo is read)
tr: Temperature above which all is rain (3 C)
ts: Temperature below which all is snow (-1 C)
ems: Emissivity of snow (nominally 0.99)
cg:  Ground heat capacity (nominally 2.09 KJ/kg/C)
z: Nominal meas. heights for air temp. and humidity (2m)
zo:  Surface aerodynamic roughness (m)
rho: Snow Density (Nominally 450 kg/m^3)
rhog:  Soil Density (nominally 1700 kg/m^3)
lc: Liquid holding capacity of snow (0.05)
ks:  Snow Saturated hydraulic conductivity (20 m/hr)
de:  Thermally active depth of soil (0.1 m)
avo:  Visual new snow albedo (0.95)
anir0: NIR new snow albedo (0.65)
lans: The thermal conductivity of fresh (dry) snow (W/m-K)
lang: the thermal conductivity of soil (W/m-K)
wlf:  Low frequency fluctuation in deep snow/soil layer 
rd1: Amplitude correction coefficient of heat conduction (1)
dnews:  The threshold depth of for new snow (0.001 m)
emc:   Emissivity of canopy
alpha: Scattering coefficient for solar radiation
alphal:   Scattering coefficient for long wave radiation
g: leaf orientation with respect to zenith angle
uc:  Unloading rate coefficient (Per hour) (Hedstrom and Pomeroy, 1998)
as:  Fraction of extraterrestrial radiation on cloudy day, Shuttleworth (1993)  
Bs:     (as+bs):Fraction of extraterrestrial radiation on clear day, Shuttleworth 
lambda: Ratio of direct atm radiation to diffuse, worked out from Dingman 
rimax:  Maximum value of Richardson number for stability correction
wcoeff: Wind decay coefficient for the forest
a: A in Bristow-Campbell formula for atmospheric transmittance
c: C in Bristow-Campbell formula for atmospheric transmittance
*
*/
const std::array< std::string, NPARS> ueb::Parameters::parameter_names{
	std::string( "irad"), //  Radiation control flag (0=from ta, 1= input qsi, 2= input qsi,qli 3= input qnet)
    "ireadalb",  //  Albedo reading control flag (0=albedo is computed internally, 1 albedo is read)
    "tr",  // Temperature above which all is rain (3 C)
    "ts",  // Temperature below which all is snow (-1 C)
    "ems",  // Emissivity of snow (nominally 0.99)
    "cg",  //  Ground heat capacity (nominally 2.09 KJ/kg/C)
    "z",   // Nominal meas. heights for air temp. and humidity (2m)
    "zo",   //  Surface aerodynamic roughness (m)
    "rho",   // Snow Density (Nominally 450 kg/m^3)
    "rhog",   //  Soil Density (nominally 1700 kg/m^3)
    "lc",   // Liquid holding capacity of snow (0.05)
    "ks",   //  Snow Saturated hydraulic conductivity (20 m/hr)
    "de",   //  Thermally active depth of soil (0.1 m)
    "avo",   //  Visual new snow albedo (0.95)
    "anir0",   // NIR new snow albedo (0.65)
    "lans",   // The thermal conductivity of fresh (dry) snow (W/m-K)
    "lang",   // the thermal conductivity of soil (W/m-K)
    "wlf",   //  Low frequency fluctuation in deep snow/soil layer 
    "rd1",   // Amplitude correction coefficient of heat conduction (1)
    "dnews",   //  The threshold depth of for new snow (0.001 m)
    "emc",   //   Emissivity of canopy
    "alpha",   // Scattering coefficient for solar radiation
    "alphal",   //   Scattering coefficient for long wave radiation
    "g",   // leaf orientation with respect to zenith angle
    "uc",   //  Unloading rate coefficient (Per hour) (Hedstrom and Pomeroy, 1998)
    "as",   //  Fraction of extraterrestrial radiation on cloudy day, Shuttleworth (1993)  
    "Bs",   //     (as+bs):Fraction of extraterrestrial radiation on clear day, Shuttleworth 
    "lambda",   // Ratio of direct atm radiation to diffuse, worked out from Dingman 
    "rimax",   //  Maximum value of Richardson number for stability correction
    "wcoeff",   // Wind decay coefficient for the forest
    "a",   // A in Bristow-Campbell formula for atmospheric transmittance
    "c",   // C in Bristow-Campbell formula for atmospheric transmittance
};

std::ostream& operator<< ( std::ostream &os, ueb::Parameters p)
{ //operator<<
   os << "Param file: " << p._parfile << std::endl;

   os << "_parArray: " ;
   std::copy( p._parArray.begin(), p._parArray.end(),
		   std::ostream_iterator<float>(std::cout, ", " ));
   os << std::endl;

   return ( os );
} //operator<<
