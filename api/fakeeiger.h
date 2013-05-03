/**********************************************************
* Fake Eiger Performance Modeling Framework
* 
* Ben Allan
* July 2012
* 
* C++ to flat file impl of eiger for later replay to mysql.
* In particular, the output of this can be read with the
* loader class on a system where mysql is supported.
* 
* Produces a log file which assumes all app calls are correct.
*
* if real eiger header is seen first, only the replay class
* (FakeEigerLoader) of this file is seen. They are headered
* together because the input/output format must be consistent.
**********************************************************/
#ifndef EIGER_H_INCLUDED

// define fake eiger interface, else define loader interface.

#ifndef FAKEEIGER_H_INCLUDED
#define FAKEEIGER_H_INCLUDED

// C++ string includes
#include <string>

// STL includes
#include <vector>
#include <fstream>
#include <map>

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
		f.open("fakeeiger.log",std::fstream::out|std::fstream::app); 
		f.precision(18);
		// could conceivable make some sort of consistency check; if offsets, log must contain data...
		std::ifstream offsets("fakeeiger.offsets");
		if (offsets.is_open())
		{
			std::string line;
			getline (offsets,line);
			std::cout << "offsets:" << line <<std::endl;
			offsets.close();
			std::istringstream s(line);
			s>> Execution_idoffset;
			s>> Trial_idoffset;
			std::cout << "offsets read:" << Execution_idoffset << " " << Trial_idoffset  <<std::endl;
		} else {
			Execution_idoffset = 0;
			Trial_idoffset = 0;
		}

	}

	~Log() { 
		f.close();
		std::ofstream offsets("fakeeiger.offsets");
		if (offsets.is_open())
		{
			offsets << Execution_idmap.size()+Execution_idoffset << " " << Trial_idmap.size()+Trial_idoffset << "\n";
			offsets.close();
		} else {
			std::cout << "ERROR writing fakeeiger.offsets" <<std::endl;
			std::cerr << "ERROR writing fakeeiger.offsets" <<std::endl;
		}
	}

	void connect(const char *databaseLocation,
                const char *databaseName,
                const char *username,
                const char *password) 
	{
		f << "CONNECT" << ";" << databaseLocation << ";" << databaseName << ";" <<username << ";" << password << "\n";
	}

	void Metric(metric_type_t type, std::string name, std::string description) {
		f << "Metric;" << type << " ;" << name << ";" << description << "\n";
	}
	int Metric_commit(std::string type, std::string name, std::string description) {
		std::ostringstream s;
		s<<"Metric_commit;"<< type << ";" << name << ";" << description ;
		std::string key = s.str();
		f << key << "\n";
		if (Metric_idmap.find(key) != Metric_idmap.end()) {
			return Metric_idmap[key];
                } else {
			Metric_idmap[key] = 1 + Metric_idmap.size();
			return Metric_idmap[key];
		}
	}

	void NondeterministicMetric(int executionID, MetricID metricID, double value) {
		f << "NondeterministicMetric;" << executionID << " ;" << metricID << " ;" << value << "\n";
	}
	void NondeterministicMetric_commit(int executionID, int metricID, double value) {
		f << "NondeterministicMetric_commit;" << executionID << " ;" << metricID << " ;" << value << "\n";
	}
	void DeterministicMetric(int datasetID, int metricID, double value) {
		f << "DeterministicMetric;" << datasetID << " ;" << metricID << " ;" << value << "\n";
	}
	void DeterministicMetric_commit(int datasetID, int metricID, double value) {
		f << "DeterministicMetric_commit;" << datasetID << " ;" << metricID << " ;" << value << "\n";
	}
	void MachineMetric(int machineID, int metricID, double value) {
		f<< "MachineMetric;" << machineID <<" ;"<< metricID<<" ;" << value << "\n";
	}
	void MachineMetric_commit(int machineID, int metricID, double value) {
		f<< "MachineMetric_commit;" << machineID <<" ;"<< metricID<<" ;" << value << "\n";
	}
	void Execution(int trialID, int machineID) {
		f<< "Execution;" << trialID << " ;"<< machineID << "\n";
	}
	int Execution_commit(int trialID, int machineID) {
		std::ostringstream s;
		s<<"Execution_commit;" << trialID << " ;"<< machineID ;
		std::string key = s.str();
		f << key ;
		if (Execution_idmap.find(key) != Execution_idmap.end()) {
			return Execution_idmap[key];
                } else {
			Execution_idmap[key] = 1+Execution_idmap.size() + Execution_idoffset;
			return Execution_idmap[key];
		}
	}
	void Trial(int dataCollectionID, int machineID,
                        int  applicationID, int datasetID,
                        int propertiesID) {
		f<< "Trial;" << dataCollectionID << " ;" <<
                                machineID << " ;" <<
                                applicationID << " ;" <<
                                datasetID << " ;" <<
                                propertiesID << "\n";
	}
	int Trial_commit(int dataCollectionID, int machineID,
                        int  applicationID, int datasetID,
                        int propertiesID) {
		std::ostringstream s;
		s<< "Trial_commit;" << dataCollectionID << " ;" <<
                                machineID << " ;" <<
                                applicationID << " ;" <<
                                datasetID << " ;" <<
                                propertiesID;
		std::string key = s.str();
		f << key << "\n";
		if (Trial_idmap.find(key) != Trial_idmap.end()) {
			return Trial_idmap[key];
                } else {
			Trial_idmap[key] = 1+Trial_idmap.size() + Trial_idoffset;
			return Trial_idmap[key];
		}
	}
	void Machine(std::string name, std::string description) {
		f << "Machine;" <<name <<";"<< description << "\n";
	}
	int Machine_commit(std::string name, std::string description) {
		std::ostringstream s;
		s << "Machine_commit;" <<name <<";"<< description << "\n";
		std::string key = s.str();
		f << key;
		if (Machine_idmap.find(key) != Machine_idmap.end()) {
                        return Machine_idmap[key];
                } else {
                        Machine_idmap[key] = 1+Machine_idmap.size();
                        return Machine_idmap[key];
                }
	}
	void Dataset(int applicationID, std::string name,
                        std::string description, std::string url) {
		f << "Dataset;" <<applicationID <<";"<<name <<";"<< description <<";"<< url << "\n";
	}
	int Dataset_commit(int applicationID, std::string name,
                        std::string description, std::string url) {
		std::ostringstream s;
		s << "Dataset_commit;" <<applicationID <<";"<<name <<";"<< description <<";"<< url << "\n";
		std::string key = s.str();
		f << key;
                if (Dataset_idmap.find(key) != Dataset_idmap.end()) {
                        return Dataset_idmap[key];
                } else {
                        Dataset_idmap[key] = 1+Dataset_idmap.size();
                        return Dataset_idmap[key];
                }
	}
	void Application(std::string name, std::string description) {
		f << "Application;" <<name <<";"<< description << "\n";
	}
	int Application_commit(std::string name, std::string description) {
		std::ostringstream s;
		s << "Application_commit;" <<name <<";"<< description << "\n";
		std::string key = s.str();
		f << key;
		if (Application_idmap.find(key) != Application_idmap.end()) {
                        return Application_idmap[key];
                } else {
                        Application_idmap[key] = 1+Application_idmap.size();
                        return Application_idmap[key];
                }
	}
	void DataCollection(std::string name, std::string description) {
		f << "DataCollection;" <<name <<";"<< description << "\n";
	}
	int DataCollection_commit(std::string name, std::string description) {
		std::ostringstream s;
		s << "DataCollection_commit;" <<name <<";"<< description << "\n";
		std::string key = s.str();
		f << key;
		if (DataCollection_idmap.find(key) != DataCollection_idmap.end()) {
                        return DataCollection_idmap[key];
                } else {
                        DataCollection_idmap[key] = 1+DataCollection_idmap.size();
                        return DataCollection_idmap[key];
                }
	}
	void Properties(std::string name, int trialID, int propertyID) {
		f << "Properties;" <<name <<";"<<trialID <<";"<<propertyID <<"\n";
	}
	int Properties_commit(std::string name, int trialID, int propertyID) {
		std::ostringstream s;
		s << "Properties_commit;" <<name <<";"<<trialID <<";"<<propertyID ;
		std::string key = s.str();
		f << key<< "\n";
		if (Properties_idmap.find(key) != Properties_idmap.end()) {
                        return Properties_idmap[key];
                } else {
                        Properties_idmap[key] = 1+Properties_idmap.size();
                        return Properties_idmap[key];
                }
	}


  private:
	std::fstream f;
	// these maps will all have int values in runtime commit order.
	// exec and trial number across all runs, so in a single run have offset
	std::map< std::string, int> Execution_idmap;
	std::map< std::string, int> Trial_idmap;
	int Execution_idoffset;
	int Trial_idoffset;
	std::map< std::string, int> Metric_idmap;
	std::map< std::string, int> Machine_idmap;
	std::map< std::string, int> Dataset_idmap;
	std::map< std::string, int> Application_idmap;
	std::map< std::string, int> DataCollection_idmap;
	std::map< std::string, int> Properties_idmap;
  };

} // end namespace eiger
#endif // FAKEEIGER_H_INCLUDED

#else // EIGER_H_INCLUDED seen before this file.
// define loader interface.
#include <iostream>
#include <boost/algorithm/string.hpp>
// C++ string includes
#include <string>

// STL includes
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
namespace eiger {
/*
This loads a datafile using a real eiger mysqlpp library.
*/
class FakeEigerLoader {

  public:
	FakeEigerLoader(): connected(false) {
		initmaps();
	}
	void parse() {
		std::string line;
		std::ifstream myfile ("fakeeiger.log");
		if (myfile.is_open())
		{
			while ( myfile.good() )
			{
				getline (myfile,line);
				one(line);
			}
			myfile.close();
		}
	}

	private:
	// the following works provided enum-from-0 semantics in the standard and no changes to metric_type_t
	bool connected;
	std::vector<std::string> connargs;
	std::map< std::string, metric_type_t> mttmap;

	enum dispatch {
	Properties_commit,
	Properties,
	DataCollection_commit,
	DataCollection,
	Application_commit,
	Application,
	Dataset_commit,
	Dataset,
	Machine_commit,
	Machine,
	Trial_commit,
	Trial,
	Execution_commit,
	Execution,
	MachineMetric_commit,
	MachineMetric,
	DeterministicMetric_commit,
	DeterministicMetric,
	NondeterministicMetric_commit,
	NondeterministicMetric,
	Metric_commit,
	Metric,
	CONNECT
  };
  std::map< std::string, dispatch > domap;
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
		for ( int i = 0; i < v.size(); i++) {
			std::cout << v[i] << "\n";
		}
		dispatch d;
		if (domap.find(v[0]) != domap.end()) {
			d = domap[v[0]];
		} else {
			std::cerr << "unknown fakeeiger keyword "<< v[0] << "\n";
			return;
		}
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
				eiger::TrialID ti; ti = toInt(v[2]);
				eiger::PropertiesID pi; pi = toInt(v[3]);
				eiger::Properties p(v[1], ti , pi);
				p.commit();
			}
		break;
		case DataCollection_commit:
			{
				eiger::DataCollection dc(v[1],v[2]);
				dc.commit();
			}
			break;
		case Application_commit:
			{
				eiger::Application ap(v[1],v[2]);
				ap.commit();
			}
			break;
		case Dataset_commit:
			{
				eiger::ApplicationID ai; ai = toInt(v[1]);
				eiger::Dataset ds(ai ,v[2],v[3],v[4]);
				ds.commit();
			}
			break;
		case Machine_commit:
			{
				eiger::Machine m(v[1],v[2]);
				m.commit();
			}
			break;
		case Trial_commit:
			{
				eiger::DataCollectionID dci; dci  = toInt(v[1]);
				eiger::MachineID mi; mi = toInt(v[2]);
				eiger::ApplicationID ai; ai = toInt(v[3]);
				eiger::DatasetID dsi; dsi = toInt(v[4]);
				eiger::PropertiesID pi; pi = toInt(v[5]);
				eiger::Trial t(dci,mi, ai, dsi, pi);
				t.commit();
			}
			break;
		case Execution_commit:
			{
				eiger::TrialID ti; ti = toInt(v[1]); 
				eiger::MachineID mi; mi = toInt(v[2]);
				eiger::Execution e(ti, mi);
				e.commit();
			}
			break;
		case MachineMetric_commit:
			{
				eiger::MachineID mai; mai = toInt(v[1]);
				eiger::MetricID mi; mi = toInt(v[2]);
				eiger::MachineMetric mm(mai,mi,toDouble(v[3]));
				mm.commit();
			}
			break;
		case DeterministicMetric_commit:
			{
				eiger::DatasetID dsi; dsi = toInt(v[1]);
				eiger::MetricID mi; mi = toInt(v[2]);
				eiger::DeterministicMetric dm(dsi,mi,toDouble(v[3]));
				dm.commit();
			}
			break;
		case NondeterministicMetric_commit:
			{
				eiger::ExecutionID ei; ei = toInt(v[1]);
				eiger::MetricID mi; mi = toInt(v[2]);
				eiger::NondeterministicMetric nm(ei,mi,toDouble(v[3]));
				nm.commit();
			}
			break;
		case Metric_commit:
			{
				eiger::Metric me(toType(v[1]),v[2],v[3]);
				me.commit();
			}
			break;
		} // end switch
	} // end one()

  // set up a hash map to convert switching over strings into switching on enum.
  void initmaps() {
	domap["Properties_commit"]=Properties_commit;
	domap["Properties"]=Properties;
	domap["DataCollection_commit"]=DataCollection_commit;
	domap["DataCollection"]=DataCollection;
	domap["Application_commit"]=Application_commit;
	domap["Application"]=Application;
	domap["Dataset_commit"]=Dataset_commit;
	domap["Dataset"]=Dataset;
	domap["Machine_commit"]=Machine_commit;
	domap["Machine"]=Machine;
	domap["Trial_commit"]=Trial_commit;
	domap["Trial"]=Trial;
	domap["Execution_commit"]=Execution_commit;
	domap["Execution"]=Execution;
	domap["MachineMetric_commit"]=MachineMetric_commit;
	domap["MachineMetric"]=MachineMetric;
	domap["DeterministicMetric_commit"]=DeterministicMetric_commit;
	domap["DeterministicMetric"]=DeterministicMetric;
	domap["NondeterministicMetric_commit"]=NondeterministicMetric_commit;
	domap["NondeterministicMetric"]=NondeterministicMetric;
	domap["Metric_commit"]=Metric_commit;
	domap["Metric"]=Metric;
	domap["CONNECT"]=CONNECT;

	mttmap["result"] = RESULT;
	mttmap["deterministic"] =DETERMINISTIC;
	mttmap["nondeterministic"]= NONDETERMINISTIC;
	mttmap["machine"] = MACHINE;
	mttmap["other"] = OTHER;
  }
	
}; // end FakeEigerLoader
}
#endif // EIGER_H_INCLUDED

