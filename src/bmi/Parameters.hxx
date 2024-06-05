#ifndef UEB_PARAMETERS
#define UEB_PARAMETERS

#include <string>
#include <array>

#define NPARS 32

namespace ueb { class Parameters; }

std::ostream& operator<< ( std::ostream &os, ueb::Parameters p);

namespace ueb {
  class Parameters {
    private:
      std::string _parfile;
      std::array<float, NPARS> _parArray;

    public:
      static const int npar = NPARS;

      Parameters();
      Parameters( std::string const& parfile );

      //copy constructor
      Parameters( Parameters const& );

      virtual ~Parameters();

      std::string getParFile() const;
      void setParFile( std::string const& fname );

      std::array<float, NPARS> getParArray() const;
      void setParArray( std::array<float, NPARS> const& parms );

      std::array<float, NPARS> getParams() const;

      int getIrad() const;

      static const std::array< std::string, NPARS> parameter_names;

    friend std::ostream& ::operator<< ( std::ostream &os, Parameters p);
  };

};

#endif //#ifndef PARAMETERS
