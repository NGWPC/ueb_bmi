#ifndef UEB_OUTCONTROL
#define UEB_OUTCONTROL

#include <string>
#include <array>
#include "uebpgdecls.h"

#define NOUTPUTS  70

namespace ueb { class OutControl; }

std::ostream& operator<< ( std::ostream &os, ueb::OutControl const& fv);

namespace ueb {
  class OutControl {
    private:

      std::string _outcontrolfile;

      pointOutput* _pOut = (pointOutput*) NULL; 
      ncOutput* _ncOut = (ncOutput*)NULL;
      aggOutput* _aggOut = (aggOutput*)NULL;
      
      int _npout = 0;
      int _nncout = 0;
      int _naggout = 0;

      void deepCopy( OutControl const& o );

    public:

      OutControl();
      OutControl( std::string const& confile );

      //copy constructor
      OutControl( OutControl const& c );

      virtual ~OutControl();

      std::string getOutControlFile() const;
      void setOutControlFile( std::string const& fname );

      int getNumPointOut() const;
      int getNumNCOut() const;
      int getNumAggOut() const;

      void setNumPointOut( int const& n );
      void setNumNCOut( int const& n );
      void setNumAggOut( int const& n );

      pointOutput* getPOut() const;

      void setPOut( pointOutput* const& pout );

      ncOutput* getNCOut() const;

      void setNCOut( ncOutput* const& ncout );

      aggOutput* getAggOut() const;

      void setAggOut( aggOutput* const& aggout );

      OutControl& operator=(OutControl const& o);

      static const std::array< std::string, NOUTPUTS> output_var_names;

    friend std::ostream& ::operator<< ( std::ostream &os, OutControl const& fv);
  };

};

#endif //#ifndef OUTCONTROL
