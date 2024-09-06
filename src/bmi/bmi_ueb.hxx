#ifndef BMI_UEB_H_INCLUDED
#define BMI_UEB_H_INCLUDED

#include <string>
#include <iostream>

#include <bmi.hxx>

#include "ControlFile.hxx"
#include "Watershed.hxx"
#include "Parameters.hxx"
#include "OutControl.hxx"
#include "SiteVariables.hxx"
#include "ForcingVariables.hxx"

namespace ueb { class Parameters; class NotImplemented; }

namespace ueb {

class NotImplemented : public std::logic_error {
  public:
  NotImplemented() : std::logic_error("Not Implemented") { };
};


class BmiUEB : public bmi::Bmi {
  public:
    BmiUEB() {
#ifdef DEBUG_UEB_BMI
	  std::cerr << "BmiUEB()!" << std::endl;
#endif //if DEBUG_UEB_BMI
    };
    virtual ~BmiUEB(){};
    void Initialize(std::string config_file) override;
    void Update() override;
    void UpdateUntil(double time) override;
    void Finalize() override;

    std::string GetComponentName() override;
    int GetInputItemCount() override;
    int GetOutputItemCount() override;
    std::vector<std::string> GetInputVarNames() override;
    std::vector<std::string> GetOutputVarNames() override;

    int GetVarGrid(std::string name) override;
    std::string GetVarType(std::string name) override;
    int GetVarItemsize(std::string name) override;
    std::string GetVarUnits(std::string name) override;
    int GetVarNbytes(std::string name) override;
    std::string GetVarLocation(std::string name) override;

    double GetCurrentTime() override;
    double GetStartTime() override;
    double GetEndTime() override;
    std::string GetTimeUnits() override;
    double GetTimeStep() override;

    void GetValue(std::string name, void *dest) override;
    void *GetValuePtr(std::string name) override;
    void GetValueAtIndices(std::string name, void *dest, int *inds, int count) override;

    void SetValue(std::string name, void *src) override;
    void SetValueAtIndices(std::string name, int *inds, int len, void *src) override;

    int GetGridRank(const int grid) override;
    int GetGridSize(const int grid) override;
    std::string GetGridType(const int grid) override;

    void GetGridShape(const int grid, int *shape) override;
    void GetGridSpacing(const int grid, double *spacing) override;
    void GetGridOrigin(const int grid, double *origin) override;

    void GetGridX(const int grid, double *x) override;
    void GetGridY(const int grid, double *y) override;
    void GetGridZ(const int grid, double *z) override;

    int GetGridNodeCount(const int grid) override;
    int GetGridEdgeCount(const int grid) override;
    int GetGridFaceCount(const int grid) override;

    void GetGridEdgeNodes(const int grid, int *edge_nodes) override;
    void GetGridFaceEdges(const int grid, int *face_edges) override;
    void GetGridFaceNodes(const int grid, int *face_nodes) override;
    void GetGridNodesPerFace(const int grid, int *nodes_per_face) override;


    void GetStartTimeInGregorianCalendar( int& y, int& m, 
		                             int& d, int& h ); 
    void GetEndTimeInGregorianCalendar( int& y, int& m, int& d, int& h ); 
    void GetCurrentTimeInGregorianCalendar( int& y, int& m, 
		                             int& d, int& h ); 

    static const int nxv = 6;
    static const int nsv = 10;
    static const int niv = 8;
    static const int nov = 14;
    static const int numOut = 70;

  private:

    ControlFile _confile;
    Watershed   _ws;
    Parameters _parms;
    SiteVariables _sitevars;
    ForcingVariables _forcings;
    OutControl _outcontrol;

    double _currentModelDateTime; //this is Julian date in days

    std::vector< std::array< float, nxv> > _statev;

    std::vector< float > _Ws1;
    std::vector< float > _Wc1;
    std::vector< float > _cumP;
    std::vector< float > _cumEs;
    std::vector< float > _cumEc;
    std::vector< float > _cumMr;
    std::vector< float > _cumGm;
    std::vector< float > _cumEg;

    std::vector< std::vector< float > > _tsprevday;
    std::vector< std::vector< float > > _taveprevday;

    //
    //use 1d array to store 3D data to obtain
    //a contiguous block of memory
    //The formular to obtain the value for the ith forcing, jth timestep, 
    //and kth cell is
    //i * (ntimesteps * ncells)  + j * ncells + k 
    std::vector< float > _outvarArray;

    void setStatev();

    void  prepareInputForPoint( double const& UTCHour,   //input
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
				int const& cell,         //input
              std::array<float**, NFORCS> const& tsvarArray,       //input
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
				float& Snowalb );         //output

void  runPointUEB( 
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
         float* dtbar,                  //array of 12, input
         float const& bca,	        //input
         float const& bcc,	        //input
         std::array<float, NPARS>& Param, //Input
	 float* Inpt,         //array of niv elements, output 
	 float&  atff,                  //output
         float& HRI,                    //output
       	 float& Ema,                    //output
         float& Eacl,                   //output,
         float* OutArr );      //array of 53 elements,  output     
			       //
void  updateOutVars( 
         int const& totalNumOfSteps,    //input
         int const& totalNumOfCells,    //input
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
         float const* OutArr );      //array of 53 elements,  input     
				    //
    void  outputAggregratedFiles();
    void  outputNcFiles();
    void  outputPointFiles();
    //
    //Get the UEB start time, this is different form the GetStartTime API.
    //The NextGen frameword require the GetStartTime returns 0.
    //Here it returns the time in the unit used internaly by UEB.
    //
    double getUEBStartTime();
    double getUEBEndTime();

    std::array< float, nsv > getSitevForCell( int const& cell);

    std::array<float, NSITEVARS> getSiteState( int const& cell );
};

};

extern "C"
{
    /**
    * Construct this BMI instance as a normal C++ object, to be returned to the framework.
    *
    * @return A pointer to the newly allocated instance.
    */
	ueb::BmiUEB *bmi_model_create();
/*	{
		return new ueb::BmiUEB();
	}
*/

    /**
     * @brief Destroy/free an instance created with @see bmi_model_create
     * 
     * @param ptr 
     */
	void bmi_model_destroy(ueb::BmiUEB *ptr);
/*	{
		delete ptr;
	}
*/
}

#endif
