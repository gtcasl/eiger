/**********************************************************
* Eiger Performance Modeling Framework
* 
* Eric Anger <eanger@gatech.edu>
* September 19, 2011
* 
* C++ to MySQL implementation of Eiger API
* 
**********************************************************/

#ifndef EIGER_H_INCLUDED
#define EIGER_H_INCLUDED

// C++ string includes
#include <string>

// STL includes
#include <vector>


///////////////////////////////////////////////////////////

namespace eiger{

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

// base for typesafe int ID types, used with covariant return getID.
// An int wrapped in struct notation that prevents passing the wrong IDs
// in the wrong slots.
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

// the proper override for getID that must be used in each class derived from EigerIdentifiedClass
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
      Metric(int ID);
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
      // Constructor reads in datacollection from DB
      DataCollection(int ID);
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
      int propertyID; // always 0

      // Constructors
      Properties(std::string name, TrialID trialID, PropertiesID propertyID);
      Properties(int ID);

      // Methods
      std::string toString();
      void commit();
      COVARIANT_GETID(PropertiesID);

  };


} // end namespace eiger

#endif

