#ifndef UEB_FORCINGVARIABLES
#define UEB_FORCINGVARIABLES

#include <string>
#include <array>
#include <map>
#include "uebpgdecls.h"

#define NFORCS 13

namespace ueb { class ForcingVariables; }

std::ostream& operator<< ( std::ostream &os, ueb::ForcingVariables fv);

namespace ueb {
  class ForcingVariables {
    private:
      std::string _forcingfile;
      std::array<inpforcvar, NFORCS> _strinpforcArray;

      std::array<float*, NFORCS> _tsvarArray;

      std::array<int, NFORCS> _ntimesteps;

      void deepCopy( ForcingVariables const& f );

    public:
      ForcingVariables();
      ForcingVariables( std::string const& forcfile );

      //copy constructor
      ForcingVariables( ForcingVariables const& );

      virtual ~ForcingVariables();

      std::string getForcFile() const;
      void setForcFile( std::string const& fname );

      std::array<inpforcvar, NFORCS> getStrinpforcArray() const;

      std::array<float*, NFORCS> getTsvarArray() const;

      std::array<int, NFORCS> getNtimesteps() const;
      void setNtimesteps( std::array<int, NFORCS> const& nt );

      ForcingVariables& operator=( ForcingVariables const& f );

      static const std::array< std::string, NFORCS > forcing_var_names;
      static const std::map< std::string, std::string > forcing_var_units;

    friend std::ostream& ::operator<< ( std::ostream &os, ForcingVariables fv);
  };

};

#endif //#ifndef FORCINGVARIABLES
