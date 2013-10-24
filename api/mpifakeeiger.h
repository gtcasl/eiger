/**********************************************************
* Fake Eiger Performance Modeling Framework
* 
* Ben Allan
* Oct 2012
* 
* MPI/C++ to flat file impl of eiger for later replay to mysql.
* In particular, the output of this can be read with the
* loader class on a system where mysql is supported.
* 
* Produces a log file which assumes all app calls are correct.
*
* if real eiger header is seen first, only the replay class
* (FakeEigerLoader) of this file is seen. They are headered
* together because the input/output format must be consistent.
*
* Special notes:
* Datasets are intended to clump similar data together
* in some way. however, dataset names are not
* constrained by eiger to any particular format.
* To keep all the data matching a particular parameter set
* across an entire parallel application, the dataset name
* must uniquely encode the parameter set. This is not 
* enforceable here, but a profiling convention can enforce
* it to yield consistent results across multiple ranks/threads.
**********************************************************/
#include "fakekeywords.h" // needed in all cases
#ifndef EIGER_H_INCLUDED


// define fake eiger interface, else define loader interface.
#ifndef MPIFAKEEIGER_H_INCLUDED
#define MPIFAKEEIGER_H_INCLUDED

// C++ string includes
#include <string>

// STL includes
#include <vector>
#include <fstream>
#include <map>

#ifndef USING_SSTMAC
#include <mpi.h>
#else
#include <sstmac/sstmpi.h>
#endif


// turn off features requiring sql reads rather than writes
#define NEEDSQLREAD 0

///////////////////////////////////////////////////////////

namespace eiger{

  class Log;

  enum error_t {
    SUCCESS,
    CONNECT_FAILURE,
    CREATE_DATA_COLLECTION_FAILURE,
    CREATE_TRIAL_FAILURE,
    CREATE_DYNAMIC_METRIC_FAILURE
  };

  error_t getLastError();

  std::string getErrorString(error_t error);

  void Connect(std::string databaseLocation,
               std::string databaseName,
               std::string username,
               std::string password);

  void Disconnect();

  //-----------------------------------------------------------------

  class Trial;
  class Dataset;
  class Machine;
  class DataCollection;
  class Application;
  class Properties;

  enum metric_type_t{ RESULT,
                      DETERMINISTIC,
                      NONDETERMINISTIC,
                      MACHINE,
                      OTHER};

  class EigerClass {
    public: 
      virtual std::string toString() = 0;
			virtual void commit() = 0;
  };
/** It is very easy to pass the wrong int ID in the original version of
this ID API. We can fix this with type checking.
An int wrapped in struct notation that prevents passing the wrong IDs
in the wrong slots. Converting an int into one of these must be done
 with an explicit '=' operation to avoid inline and automatic casting accidents.
Requires implementing covariant return getID.
sizeof(EigerID) should be sizeof(int);
*/
  struct EigerID {
	int ID;
	// no default constructor path from *other* EigerID derived types or from int
	explicit EigerID(const int t_) : ID(t_) {}
        EigerID() : ID(0) {}
        EigerID(const EigerID & t_) : ID(t_.ID){}
        EigerID & operator=(const EigerID & rhs) { ID = rhs.ID; return *this; }
        EigerID & operator=(const int & rhs) { ID = rhs; return *this; }
        operator const int & () const {return ID; }
        operator int & () { return ID; }
        bool operator==(const EigerID & rhs) const { return ID == rhs.ID; }
        bool operator<(const EigerID & rhs) const { return ID < rhs.ID; }
  };
#define DERIVED_ID(D) \
  struct D : public EigerID { \
	explicit D(const int t_, int nodefaultconvert) { ID = t_; (void)nodefaultconvert;} \
        D() { ID=0; } \
        D(const D & t_) { ID= t_.ID; } \
        D & operator=(const D & rhs) { ID = rhs.ID; return *this; } \
        D & operator=(const int & rhs) { ID = rhs; return *this; } \
        operator const int & () const {return ID; } \
        operator int & () { return ID; } \
        bool operator==(const D & rhs) const { return ID == rhs.ID; } \
        bool operator<(const D & rhs) const { return ID < rhs.ID; } \
  }

// The proper override for getID that must be used in each class derived from EigerIdentifiedClass
// and used as an argument to DERIVED_ID.
// @param D the name of the corresponding DERIVED_ID type.
#define COVARIANT_GETID(D) \
      D getID() { if (ecs == ecs_ok) return D(ID,0); throw #D " requested from uncommited/failed object."; }

DERIVED_ID(ExecutionID);
DERIVED_ID(MetricID);
DERIVED_ID(TrialID);
DERIVED_ID(MachineID);
DERIVED_ID(DatasetID);
DERIVED_ID(ApplicationID);
DERIVED_ID(DataCollectionID);
DERIVED_ID(PropertiesID);

  enum commit_status {
  	ecs_pre, // allocated but not committed
	ecs_ok, // committed ok (only in this status is getID ok on EigerIdentifiedClass objects.
	ecs_fail, // commit failed
  };
	
  
  class EigerIdentifiedClass : EigerClass {
    protected:
      int ID;
      commit_status ecs;
    public:
      // base of a covariant return in the subclasses to check that ids are used correctly.
      EigerID getID(){ if (ecs == ecs_ok) return EigerID(ID); throw "EigerIdentifiedClass::getID called on uncommitted/failed object."; }
  };

  class Metric : public EigerIdentifiedClass {
    public:
			// Members
      metric_type_t type;
      std::string name;
      std::string description;

			// Constructors
#if NEEDSQLREAD
      Metric(int ID);
#endif
      Metric(metric_type_t type, std::string name, std::string description);

			// Methods
	std::string toString();
	void commit();

      COVARIANT_GETID(MetricID);
  };

  class NondeterministicMetric: EigerClass {
    public:
			// Members
      int executionID;
      int metricID;
      double value;

			// Constructors
      NondeterministicMetric(ExecutionID executionID, MetricID metricID, double value);

			// Methods
      std::string toString();
			void commit();

  };

  class DeterministicMetric: EigerClass {
    public:
			// Members
      int datasetID;
      int metricID;
      double value;

			// Constructors
      DeterministicMetric(DatasetID datasetID, MetricID metricID, double value);

			// Methods
      std::string toString();
			void commit();

  };

  class MachineMetric : EigerClass {
    public:
			// Members
      int machineID;
      int metricID;
      double value;

			// Constructors
      MachineMetric(MachineID machineID, MetricID metricID, double value);

			// Methods
      std::string toString();
			void commit();

  };

  class Execution : public EigerIdentifiedClass {
  	public:
		  	// Members
	  int trialID;
	  int machineID;

	  		// Constructors
	  Execution(TrialID trialID, MachineID machineID);

	  		// Methods
	  std::string toString();
	  void commit();
	COVARIANT_GETID(ExecutionID);

  };


  class Trial : public EigerIdentifiedClass {
    public:
			// Members
      int dataCollectionID;
      int machineID;
      int applicationID;
      int datasetID;
      int propertiesID;

			// Constructors
      Trial(DataCollectionID dataCollectionID, MachineID machineID, 
            ApplicationID applicationID, DatasetID datasetID,
            PropertiesID propertiesID);

			// Methods
      //std::vector<DynamicMetric>* getDynamicMetrics();
      std::string toString();
			void commit();
        COVARIANT_GETID(TrialID);


  };

  class Machine : public EigerIdentifiedClass {
    public:
			// Members
      std::string name;
      std::string description;

			// Constructors
      Machine(std::string name, std::string description);
			
			// Methods
      //std::vector<MachineMetric>* getMachineMetrics();
      std::string toString();
			void commit();
        COVARIANT_GETID(MachineID);
      
  };

  class Dataset : public EigerIdentifiedClass { 
    public:
			// Members
      int applicationID;
      std::string name;
      std::string description;
      std::string created;
      std::string url;

			// Constructors
	/** For parallel fakeeiger use
	name and/or description must somehow uniquely encode the 
	set of integer values associated with the dataset, or the
	correct mapping cannot be from local to global ids is not
	possible.
	*/
      Dataset(ApplicationID applicationID, std::string name, 
              std::string description, std::string url);

			// Methods
      //std::vector<StaticMetric>* getStaticMetrics();
      std::string toString();
			void commit();
        COVARIANT_GETID(DatasetID);

  };

  class Application : public EigerIdentifiedClass {
    public:
			// Members
      std::string name;
      std::string description;

			// Constructors
      Application(std::string name, std::string description);

			// Methods
      //std::vector<Dataset>* getDatasets();
      std::string toString();
			void commit();
        COVARIANT_GETID(ApplicationID);

  };

  class DataCollection : public EigerIdentifiedClass {
    public:
			// Members
      std::string name;
      std::string description;
      std::string created;

			// Constructors
#if NEEDSQLREAD
      // Constructor reads in datacollection from DB
      DataCollection(int ID);
#endif
      // Constructor makes new datacollection, sends to DB
      DataCollection(std::string name, std::string description);

			// Methods
      //std::vector<Trial>* getTrials();
      std::string toString();
			void commit();
        COVARIANT_GETID(DataCollectionID);

  };

  class Properties : public EigerIdentifiedClass {
    public:
			// Members
      std::string name;
			int trialID;
			int propertyID;

			// Constructors
			Properties(std::string name, TrialID trialID, PropertiesID propertyID);
#if NEEDSQLREAD
			Properties(int ID);
#endif

			// Methods
			std::string toString();
			void commit();
        COVARIANT_GETID(PropertiesID);

  };


  /** logger so we can replay to mysql later. */
  class Log { //  ; delimited log file.
  public:
	Log() {  
		int minit=0;
		MPI_Initialized(&minit);
		if (!minit) {
			throw "mpifakeeiger cannot be used without MPI_Init in the application";
		}
                MPI_Comm_rank(MPI_COMM_WORLD,&rank);
		std::ostringstream fn;
		fn << "mpifakeeiger." << rank <<".log";
		std::string fns = fn.str();
		f.open(fns.c_str(),std::fstream::out|std::fstream::trunc); 
		f.precision(18);
	}

	~Log() { 
		f.close();
	}

	void connect(const char *databaseLocation,
                const char *databaseName,
                const char *username,
                const char *password) 
	{
		f << FEVERSION<<";2\n";
		f << FEFORMAT << ";" KWFORMAT "\n";
		f << FECONNECT << ";" << databaseLocation << ";" << databaseName << ";" <<username << ";" << password << "\n";
	}

	void disconnect() {
		f << FEVERSION<<";2" << std::endl;
	}

	void Metric(metric_type_t type, std::string name, std::string description) {
//		f << FEMETRIC<<";" << type << " ;" << name << ";" << description << "\n";
	}
	int Metric_commit(std::string type, std::string name, std::string description) {
		std::ostringstream s;
		s<<METRIC_COMMIT<<";"<< type << ";" << name << ";" << description ;
		std::string key = s.str();
		f << key ;
		if (Metric_idmap.find(key) != Metric_idmap.end()) {
			int id = Metric_idmap[key];
			f << ";" << id << "\n";
			return id;
                } else {
			Metric_idmap[key] = 1 + Metric_idmap.size();
			int id = Metric_idmap[key];
			f << ";" << id << "\n";
			return id;
		}
	}

	void NondeterministicMetric(int executionID, MetricID metricID, double value) {
//		f << NONDETERMINISTICMETRIC<<";" << executionID << " ;" << metricID << " ;" << value << "\n";
	}
	void NondeterministicMetric_commit(int executionID, int metricID, double value) {
		f << NONDETERMINISTICMETRIC_COMMIT<<";" << executionID << " ;" << metricID << " ;" << value <<  "\n";
	}
	void DeterministicMetric(int datasetID, int metricID, double value) {
//		f << DETERMINISTICMETRIC<<";" << datasetID << " ;" << metricID << " ;" << value << "\n";
	}
	void DeterministicMetric_commit(int datasetID, int metricID, double value) {
		f << DETERMINISTICMETRIC_COMMIT<<";" << datasetID << " ;" << metricID << " ;" << value << "\n";
	}
	void MachineMetric(int machineID, int metricID, double value) {
//		f<< MACHINEMETRIC<<";" << machineID <<" ;"<< metricID<<" ;" << value << "\n";
	}
	void MachineMetric_commit(int machineID, int metricID, double value) {
		f<< MACHINEMETRIC_COMMIT<<";" << machineID <<" ;"<< metricID<<" ;" << value << "\n";
	}
	void Execution(int trialID, int machineID) {
//		f<< EXECUTION<<";" << trialID << " ;"<< machineID << "\n";
	}
	int Execution_commit(int trialID, int machineID) {
		std::ostringstream s;
		s<<EXECUTION_COMMIT<<";" << trialID << " ;"<< machineID ;
		std::string key = s.str();
		f << key ;
		if (Execution_idmap.find(key) != Execution_idmap.end()) {
			int id = Execution_idmap[key];
			f << ";" << id << "\n";
			return id;
                } else {
			Execution_idmap[key] = 1+Execution_idmap.size() ;
			int id = Execution_idmap[key];
			f << ";" << id << "\n";
			return id;
		}
	}
	void Trial(int dataCollectionID, int machineID,
                        int  applicationID, int datasetID,
                        int propertiesID) {
#if 0
		f<< TRIAL<<";" << dataCollectionID << " ;" <<
                                machineID << " ;" <<
                                applicationID << " ;" <<
                                datasetID << " ;" <<
                                propertiesID << "\n";
#endif
	}
	int Trial_commit(int dataCollectionID, int machineID,
                        int  applicationID, int datasetID,
                        int propertiesID) {
		std::ostringstream s;
		s<< TRIAL_COMMIT<<";" << dataCollectionID << " ;" <<
                                machineID << " ;" <<
                                applicationID << " ;" <<
                                datasetID << " ;" <<
                                propertiesID;
		std::string key = s.str();
		f << key ;
		if (Trial_idmap.find(key) != Trial_idmap.end()) {
			int id = Trial_idmap[key];
			f << ";" << id << "\n";
			return id;
                } else {
			Trial_idmap[key] = 1+Trial_idmap.size() ;
			int id = Trial_idmap[key];
			f << ";" << id << "\n";
			return id;
		}
	}
	void Machine(std::string name, std::string description) {
//		f << FEMACHINE<<";" <<name <<";"<< description << "\n";
	}
	int Machine_commit(std::string name, std::string description) {
		std::ostringstream s;
		s << FEMACHINE_COMMIT<<";" <<name <<";"<< description ;
		std::string key = s.str();
		f << key ;
		if (Machine_idmap.find(key) != Machine_idmap.end()) {
			int id = Machine_idmap[key];
			f << ";" << id << "\n";
			return id;
                } else {
                        Machine_idmap[key] = 1+Machine_idmap.size();
			int id = Machine_idmap[key];
			f << ";" << id << "\n";
			return id;
                }
	}
	void Dataset(int applicationID, std::string name,
                        std::string description, std::string url) {
//		f << DATASET<<";" <<applicationID <<";"<<name <<";"<< description <<";"<< url << "\n";
	}
	int Dataset_commit(int applicationID, std::string name,
                        std::string description, std::string url) {
		std::ostringstream s;
		s << DATASET_COMMIT<<";" <<applicationID <<";"<<name <<";"<< description <<";"<< url ;
		std::string key = s.str();
		f << key ;
                if (Dataset_idmap.find(key) != Dataset_idmap.end()) {
			int id = Dataset_idmap[key];
			f << ";" << id << "\n";
			return id;
                } else {
                        Dataset_idmap[key] = 1+Dataset_idmap.size();
			int id = Dataset_idmap[key];
			f << ";" << id << "\n";
			return id;
                }
	}
	void Application(std::string name, std::string description) {
//		f << APPLICATION<<";" <<name <<";"<< description << "\n";
	}
	int Application_commit(std::string name, std::string description) {
		std::ostringstream s;
		s << APPLICATION_COMMIT<<";" <<name <<";"<< description ;
		std::string key = s.str();
		f << key;
		if (Application_idmap.find(key) != Application_idmap.end()) {
			int id = Application_idmap[key];
			f << ";" << id << "\n";
			return id;
                } else {
                        Application_idmap[key] = 1+Application_idmap.size();
			int id = Application_idmap[key];
			f << ";" << id << "\n";
			return id;
                }
	}
	void DataCollection(std::string name, std::string description) {
//		f << DATACOLLECTION<<";" <<name <<";"<< description << "\n";
	}
	int DataCollection_commit(std::string name, std::string description) {
		std::ostringstream s;
		s << DATACOLLECTION_COMMIT<<";" <<name <<";"<< description ;
		std::string key = s.str();
		f << key;
		if (DataCollection_idmap.find(key) != DataCollection_idmap.end()) {
			int id = DataCollection_idmap[key];
			f << ";" << id << "\n";
			return id;
                } else {
                        DataCollection_idmap[key] = 1+DataCollection_idmap.size();
			int id = DataCollection_idmap[key];
			f << ";" << id << "\n";
			return id;
                }
	}
	void Properties(std::string name, int trialID, int propertyID) {

//		f << PROPERTIES<<";" <<name <<";"<<trialID <<";"<<propertyID <<"\n";
	}
	int Properties_commit(std::string name, int trialID, int propertyID) {
		std::ostringstream s;
		s << PROPERTIES_COMMIT<<";" <<name <<";"<<trialID <<";"<<propertyID ;
		std::string key = s.str();
		f << key;
		if (Properties_idmap.find(key) != Properties_idmap.end()) {
			int id = Properties_idmap[key];
			f << ";" << id << "\n";
			return id;
                } else {
                        Properties_idmap[key] = 1+Properties_idmap.size();
			int id = Properties_idmap[key];
			f << ";" << id << "\n";
			return id;
                }
	}


  private:
	int rank;
	std::fstream f;
	// these maps will all have int values in runtime commit order.
	// across multiple ranks or runs, these are not unique and must
	// be rationalized before feeding database.
	std::map< std::string, int> Execution_idmap;
	std::map< std::string, int> Trial_idmap;
	std::map< std::string, int> Metric_idmap;
	std::map< std::string, int> Machine_idmap;
	std::map< std::string, int> Dataset_idmap;
	std::map< std::string, int> Application_idmap;
	std::map< std::string, int> DataCollection_idmap;
	std::map< std::string, int> Properties_idmap;
  };

} // end namespace eiger
#endif // MPIFAKEEIGER_H_INCLUDED

#else // EIGER_H_INCLUDED seen before this file.
// define loader interface.
#include <iostream>
#include <boost/algorithm/string.hpp>
// C++ string includes
#include <string>

// STL includes
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
// posix
#include <glob.h>
namespace eiger {
/*
This loads a datafile using a real eiger mysqlpp library.
*/
class FakeEigerLoader {

  public:
	FakeEigerLoader(): connected(false) {
		initmaps();
	}
	~FakeEigerLoader() { }
	void parse() {
	dcmax = amax= dsmax= machmax = tmax = emax = mmax = pmax = -1;
		std::string line;
		std::vector< std::string> infiles = glob_logs("mpifakeeiger.*.log");
		std::vector< std::string>::size_type nf = infiles.size();
		for (std::vector< std::string>::size_type i = 0; i < nf; i++) {
			std::ifstream myfile (infiles[i].c_str());
			if (myfile.is_open())
			{
				initLocalToGlobal();
				std::cout << "parsing " << infiles[i] <<"\n";
				while ( myfile.good() )
				{
					getline (myfile,line);
					one(line);
				}
				myfile.close();
			}
		}
		std::cout << "Upper bounds from eiger db:\n" <<
			"MachineID          " << machmax << "\n" <<
			"ApplicationID      " << amax << "\n" <<
			"MetricID           " << mmax << "\n" <<
			"DataCollectionID   " << dcmax << "\n" <<
			"DatasetID          " << dsmax << "\n" <<
			"TrialID            " << tmax << "\n" <<
			"ExecutionID        " << emax << "\n" <<
			"PropertiesID       " << pmax << std::endl;
	}

	private:
	static const int version = 2;
	bool connected;
	std::vector<std::string> connargs;
	std::map< std::string, metric_type_t> mttmap;

	enum dispatch {
		Metric,
		Metric_commit,
		Properties,
		Properties_commit,
		DataCollection,
		DataCollection_commit,
		Application,
		Application_commit,
		Dataset,
		Dataset_commit,
		Machine,
		Machine_commit,
		Trial,
		Trial_commit,
		Execution,
		Execution_commit,

		MachineMetric,
		MachineMetric_commit,
		DeterministicMetric,
		DeterministicMetric_commit,
		NondeterministicMetric,
		NondeterministicMetric_commit,
		CONNECT,
		LVERSION,
		LFORMAT,
	};
	// log file object/step name string to enum for switching
	std::map< std::string, dispatch > domap;

	// map enum to int-id maps. id maps map file local int value to global db value.
	// When any object is committed, we map its log file id int to the eiger-returned value.
	// Each time we open a new log file, we must clear the idmaps with initLocalToGlobal.
	std::map<int, DataCollectionID > local2dbDataCollection;
	std::map<int, ApplicationID >   local2dbApplication;
	std::map<int, DatasetID >   local2dbDataset;
	std::map<int, MachineID >   local2dbMachine;
	std::map<int, TrialID >   local2dbTrial;
	std::map<int, ExecutionID >   local2dbExecution;
	std::map<int, MetricID >   local2dbMetric;
	std::map<int, PropertiesID >   local2dbProperties;

	int dcmax;
	int amax;
	int dsmax;
	int machmax;
	int tmax;
	int emax;
	int mmax;
	int pmax;

	// define l2dMetric, etc. mainly for usage error checking.
#define MAKEL2D(x) \
	x##ID l2d##x(int id) { \
		if (!id) { return eiger::x##ID(0,0); } \
		std::map<int, x##ID >::const_iterator it = local2db##x.find(id); \
		if ( it != local2db##x.end()) { return it->second;} \
		throw "attempt to use undefined " #x " index"; \
	}
	MAKEL2D( DataCollection )
	MAKEL2D( Application )
	MAKEL2D( Dataset )
	MAKEL2D( Machine )
	MAKEL2D( Trial )
	MAKEL2D( Execution )
	MAKEL2D( Metric )
	MAKEL2D( Properties )

	enum metric_type_t toType(std::string& s) {
		return mttmap[s];
	}
	double toDouble(std::string s) {
		std::istringstream is(s);
		double i;
		is >> i;
		return i;
	}
	int toInt(std::string s) {
		std::istringstream is(s);
		int i;
		is >> i;
		return i;
	}
	void one(std::string s) {
		std::vector<std::string> v;
		if (s.size()==0 ) return;
		boost::split(v,s,boost::is_any_of( ";" ) );
#if 0 // make a debug flag for this
		for ( int i = 0; i < v.size(); i++) {
			std::cout << v[i] << "\n";
		}
#endif
		dispatch d;
		if (domap.find(v[0]) != domap.end()) {
			d = domap[v[0]];
		} else {
			std::cerr << "unknown fakeeiger keyword "<< v[0] << "\n";
			return;
		}
#define EIGCOUNT(imax,x) if ( x.getID() > imax) imax = x.getID()
		// we will merrily assume enough and correct args
		switch (d) {
		case CONNECT:
			// we should check that for given dataset, it is empty in the database
			// otherwise, we're likely to have id# problems.
			if (! connected) {
				eiger::Connect(v[1],v[2],v[3],v[4]);
				connargs.push_back(v[1]);
				connargs.push_back(v[2]);
				connargs.push_back(v[3]);
				connargs.push_back(v[4]);
				connected = true;
			} else {
				for (int i=0;i<4;i++) {
					if (connargs[i] != v[i+1]) {	
						std::cerr << "CONNECT in log file with mismatched argument " << v[0] << "!=" << connargs[i] << "\n";
						std::cout << "CONNECT: quitting on likely user error\n";
					}
				}
			}
			break;
		case LVERSION:
			{
				int filever = toInt(v[1]);
				if (filever != version) {
					std::cout << "VERSION: " << version <<" != " << filever <<"\n";
					throw "mpifakeeiger data file version mismatch.";
				}
			}
			break;
		case LFORMAT:
			{
				if (v[1] != KWFORMAT) {
					std::cout << FEFORMAT ": " << v[1] <<" does not match compiled in format " KWFORMAT "\n";
					throw "mpifakeeiger data file format mismatch.";
				}
			}
			break;
		/* this batch don't really need to do anything yet. ctors. */
		case Properties:
		case DataCollection:
		case Application:
		case Dataset:
		case Machine:
		case Trial:
		case Execution:
		case MachineMetric:
		case DeterministicMetric:
		case NondeterministicMetric:
		case Metric:
			break;
		/* these do something */
		case Properties_commit:
			{
				int id = toInt(v[4]); // id as logged
				eiger::TrialID ti; ti = l2dTrial(toInt(v[2]));
				eiger::PropertiesID pi; pi = l2dProperties(toInt(v[3])); 
				eiger::Properties p(v[1], ti , pi);
				p.commit();
				local2dbProperties[id] = p.getID();
				EIGCOUNT(pmax, p);
			}
			break;
		case DataCollection_commit:
			{
				int id = toInt(v[3]); // id as logged
				eiger::DataCollection dc(v[1],v[2]);
				dc.commit();
				local2dbDataCollection[id] = dc.getID(); 
				EIGCOUNT(dcmax, dc);
			}
			break;
		case Application_commit:
			{
				int id = toInt(v[3]); // id as logged
				eiger::Application ap(v[1],v[2]);
				ap.commit();
				local2dbApplication[id] = ap.getID(); 
				EIGCOUNT(amax, ap);
			}
			break;
		case Dataset_commit:
			{
				int id = toInt(v[5]); // id as logged
				eiger::ApplicationID ai; ai = l2dApplication(toInt(v[1]));
				eiger::Dataset ds(ai ,v[2],v[3],v[4]);
				ds.commit();
				local2dbDataset[id] = ds.getID(); 
				EIGCOUNT(dsmax, ds);
			}
			break;
		case Machine_commit:
			{
				int id = toInt(v[3]); // id as logged
				eiger::Machine m(v[1],v[2]);
				m.commit();
				local2dbMachine[id] = m.getID(); 
				EIGCOUNT(machmax, m);
			}
			break;
		case Trial_commit:
			{
				int id = toInt(v[6]); // id as logged
				eiger::DataCollectionID dci; dci  = l2dDataCollection(toInt(v[1]));
				eiger::MachineID mi; mi = l2dMachine(toInt(v[2])); 
				eiger::ApplicationID ai; ai = l2dApplication(toInt(v[3]));
				eiger::DatasetID dsi; dsi = l2dDataset(toInt(v[4]));
				eiger::PropertiesID pi; pi = l2dProperties(toInt(v[5]));
				eiger::Trial t(dci,mi, ai, dsi, pi);
				t.commit();
				local2dbTrial[id] = t.getID(); 
				EIGCOUNT(tmax, t);
			}
			break;
		case Execution_commit:
			{
				int id = toInt(v[3]); // id as logged
				eiger::TrialID ti; ti = l2dTrial(toInt(v[1])); 
				eiger::MachineID mi; mi = l2dMachine(toInt(v[2]));
				eiger::Execution e(ti, mi);
				e.commit();
				local2dbExecution[id] = e.getID(); 
				EIGCOUNT(emax, e);
			}
			break;
		case MachineMetric_commit:
			{
				eiger::MachineID mai; mai = l2dMachine(toInt(v[1]));
				eiger::MetricID mi; mi = l2dMetric(toInt(v[2]));
				eiger::MachineMetric mm(mai,mi,toDouble(v[3]));
				mm.commit();
			}
			break;
		case DeterministicMetric_commit:
			{
				eiger::DatasetID dsi; dsi = l2dDataset(toInt(v[1]));
				eiger::MetricID mi; mi = l2dMetric(toInt(v[2]));
				eiger::DeterministicMetric dm(dsi,mi,toDouble(v[3]));
				dm.commit();
			}
			break;
		case NondeterministicMetric_commit:
			{
				eiger::ExecutionID ei; ei = l2dExecution(toInt(v[1]));
				eiger::MetricID mi; mi = l2dMetric(toInt(v[2]));
				eiger::NondeterministicMetric nm(ei,mi,toDouble(v[3]));
				nm.commit();
			}
			break;
		case Metric_commit:
			{
				int id = toInt(v[4]); // id as logged
				eiger::Metric me(toType(v[1]),v[2],v[3]);
				me.commit();
				local2dbMetric[id] = me.getID(); 
				EIGCOUNT(mmax, me);
			}
			break;
		default:
			throw "unexpected enum value in one() handling";
		} // end switch
	} // end one()

  // set up a hash map to convert switching over strings into switching on enum.
  void initmaps() {
	domap[PROPERTIES_COMMIT]=Properties_commit;
	domap[PROPERTIES]=Properties;
	domap[DATACOLLECTION_COMMIT] =DataCollection_commit;
	domap[DATACOLLECTION] =DataCollection;
	domap[APPLICATION_COMMIT] =Application_commit;
	domap[APPLICATION] =Application;
	domap[DATASET_COMMIT] =Dataset_commit;
	domap[DATASET] =Dataset;
	domap[FEMACHINE_COMMIT] =Machine_commit;
	domap[FEMACHINE] =Machine;
	domap[TRIAL_COMMIT] =Trial_commit;
	domap[TRIAL] =Trial;
	domap[EXECUTION_COMMIT]=Execution_commit;
	domap[EXECUTION]=Execution;
	domap[MACHINEMETRIC_COMMIT]=MachineMetric_commit;
	domap[MACHINEMETRIC]=MachineMetric;
	domap[DETERMINISTICMETRIC_COMMIT]=DeterministicMetric_commit;
	domap[DETERMINISTICMETRIC]=DeterministicMetric;
	domap[NONDETERMINISTICMETRIC_COMMIT]=NondeterministicMetric_commit;
	domap[NONDETERMINISTICMETRIC]=NondeterministicMetric;
	domap[METRIC_COMMIT]=Metric_commit;
	domap[FEMETRIC]=Metric;
	domap[FECONNECT]=CONNECT;
	domap[FEVERSION]=LVERSION;
	domap[FEFORMAT]=LFORMAT;

	mttmap["result"] = RESULT;
	mttmap["deterministic"] =DETERMINISTIC;
	mttmap["nondeterministic"]= NONDETERMINISTIC;
	mttmap["machine"] = MACHINE;
	mttmap["other"] = OTHER;
  }

  void initLocalToGlobal() {
	local2dbProperties.clear();
	local2dbDataCollection.clear();
	local2dbApplication.clear();
	local2dbDataset.clear();
	local2dbMachine.clear();
	local2dbTrial.clear();
	local2dbExecution.clear();
	local2dbMetric.clear();
  }

  inline std::vector<std::string> glob_logs(const std::string& pat){
    glob_t glob_result;
    glob(pat.c_str(),GLOB_TILDE,NULL,&glob_result);
    std::vector<std::string> ret;
    for(unsigned int i=0;i<glob_result.gl_pathc;++i){
        ret.push_back(std::string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return ret;
}
	
}; // end FakeEigerLoader
}
#endif // EIGER_H_INCLUDED

