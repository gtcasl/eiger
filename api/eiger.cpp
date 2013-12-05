/**********************************************************
 * Eiger Performance Modeling Framework
 * 
 * Eric Anger <eanger@gatech.edu>
 * September 19, 2011
 * 
 * C++ to MySQL implementation of Eiger API
 * 
 **********************************************************/

// C++ string includes
#include <ostream>
#include <string>
#include <vector>
#include <algorithm>

// Eiger includes
#include "fakekeywords.h"
#include "eiger.h"

using std::string;
using std::vector;

namespace eiger{

  void do_disconnect(const string& dbloc, const string& dbname, 
                     const string& user, const string& passwd,
                     const vector<DataCollection>& datacollections,
                     const vector<Application>& applications,
                     const vector<Dataset>& datasets,
                     const vector<Machine>& machines,
                     const vector<Trial>& trials,
                     const vector<Execution>& executions,
                     const vector<Metric>& metrics,
                     const vector<NondeterministicMetric>& nondet_metrics,
                     const vector<DeterministicMetric>& det_metrics,
                     const vector<MachineMetric>& machine_metrics);
  
  template<typename T>
  struct nameCompare{
    std::string key_;
    nameCompare(const std::string& key) : key_(key) {}
    bool operator()(const T& yours) const { return yours.name == key_; }
  };

  /********
   * Static vars
   */
  static std::string dbloc;
  static std::string dbname;
  static std::string user;
  static std::string passwd;
  static vector<DataCollection> datacollections;
  static vector<Application> applications;
  static vector<Dataset> datasets;
  static vector<Machine> machines;
  static vector<Trial> trials;
  static vector<Execution> executions;
  static vector<Metric> metrics;
  static vector<NondeterministicMetric> nondet_metrics;
  static vector<DeterministicMetric> det_metrics;
  static vector<MachineMetric> machine_metrics;
	
	error_t err;

	error_t getLastError(){
		return err;
	}

	std::string getErrorString(error_t error) {

		std::string errorString;

		switch(error) {
			case SUCCESS:
				errorString = "Last operation successful.";
				break;
			case CONNECT_FAILURE:
				errorString = "Failure connecting to Eiger.";
				break;
			case CREATE_DATA_COLLECTION_FAILURE:
				errorString = "Failure inserting new data collection.";
				break;
			case CREATE_TRIAL_FAILURE:
				errorString = "Failure inserting new trial.";
				break;
			case CREATE_DYNAMIC_METRIC_FAILURE:
				errorString = "Failure inserting new dynamic metric.";
				break;
			default:
				errorString = "Unknown error type.";
				break;
		}

		return errorString;

	}

	void Connect(std::string databaseLocation,
			std::string databaseName,
			std::string username,
			std::string password) {
    dbloc = databaseLocation;
    dbname = databaseName;
    user = username;
    passwd = password;
	}

	void Disconnect(){
    do_disconnect(dbloc, dbname, user, passwd, datacollections, applications,
                  datasets, machines, trials, executions, metrics,
                  nondet_metrics, det_metrics, machine_metrics);
    dbloc.clear();
    dbname.clear();
    user.clear();
    passwd.clear();
    datacollections.clear();
    applications.clear();
    datasets.clear();
    machines.clear();
    trials.clear();
    executions.clear();
    metrics.clear();
    nondet_metrics.clear();
    det_metrics.clear();
    machine_metrics.clear();
	}

	//-----------------------------------------------------------------

	Metric::Metric(metric_type_t type, std::string name, std::string description) : type(type), name(name), description(description) { ecs = ecs_pre; }

	void Metric::commit() {
    vector<Metric>::iterator it = std::find_if(metrics.begin(), 
                                               metrics.end(), 
                                               nameCompare<Metric>(name));
    if(it != metrics.end()){
      ID = std::distance(metrics.begin(), it);
    } else {
      ID = metrics.size();
      metrics.push_back(*this);
    }
    ecs = ecs_ok;
	}

	NondeterministicMetric::NondeterministicMetric(ExecutionID executionID, MetricID metricID, double value) :  executionID(executionID), metricID(metricID), value(value) {}

	void NondeterministicMetric::commit() {
    nondet_metrics.push_back(*this);
	}

	DeterministicMetric::DeterministicMetric(DatasetID datasetID, MetricID metricID, double value) : datasetID(datasetID), metricID(metricID), value(value) {}

	void DeterministicMetric::commit() {
    det_metrics.push_back(*this);
	}

	MachineMetric::MachineMetric(MachineID machineID, MetricID metricID, double value) : machineID(machineID), metricID(metricID), value(value) {}

	void MachineMetric::commit() {
    machine_metrics.push_back(*this);
	}

	Execution::Execution(TrialID trialID, MachineID machineID) : trialID(trialID), machineID(machineID){ ecs = ecs_pre;}

	void Execution::commit() {
    ID = executions.size();
    executions.push_back(*this);
    ecs = ecs_ok;
	}

	Trial::Trial(DataCollectionID dataCollectionID, MachineID machineID,
			ApplicationID applicationID, DatasetID datasetID) :
		dataCollectionID(dataCollectionID), machineID(machineID),
		applicationID(applicationID), datasetID(datasetID) { ecs = ecs_pre; }

	void Trial::commit() {
    ID = trials.size();
    trials.push_back(*this);
    ecs = ecs_ok;
	}

	Machine::Machine(std::string name, std::string description) : name(name), description(description) { ecs = ecs_pre; }

	void Machine::commit() {
    vector<Machine>::iterator it = std::find_if(machines.begin(), 
                                                machines.end(), 
                                                nameCompare<Machine>(name));
    if(it != machines.end()){
      ID = std::distance(machines.begin(), it);
    } else {
      ID = machines.size();
      machines.push_back(*this);
    }
    ecs = ecs_ok;
	}

	Dataset::Dataset(ApplicationID applicationID, std::string name, 
			std::string description, std::string url) : 
		applicationID(applicationID), name(name),
		description(description), url(url) { ecs = ecs_pre; }

	void Dataset::commit() {
    vector<Dataset>::iterator it = std::find_if(datasets.begin(), 
                                                datasets.end(), 
                                                nameCompare<Dataset>(name));
    if(it != datasets.end()){
      ID = std::distance(datasets.begin(), it);
    } else {
      ID = datasets.size();
      datasets.push_back(*this);
    }
    ecs = ecs_ok;
	}

	Application::Application(std::string name, std::string description) : 
		name(name), description(description) { ecs = ecs_pre; }

	void Application::commit() {
    vector<Application>::iterator it = std::find_if(applications.begin(), 
                                                    applications.end(), 
                                                    nameCompare<Application>(name));
    if(it != applications.end()){
      ID = std::distance(applications.begin(), it);
    } else {
      ID = applications.size();
      applications.push_back(*this);
    }
    ecs = ecs_ok;
	}

	DataCollection::DataCollection(std::string name, std::string description) : 
		name(name), description(description) { ecs = ecs_pre; }

	void DataCollection::commit() {
    vector<DataCollection>::iterator it = std::find_if(datacollections.begin(), 
                                                       datacollections.end(), 
                                                       nameCompare<DataCollection>(name));
    if(it != datacollections.end()){
      ID = std::distance(datacollections.begin(), it);
    } else {
      ID = datacollections.size();
      datacollections.push_back(*this);
    }
    ecs = ecs_ok;
	}

	void Metric::print(std::ostream& str) const {

		std::string type_name;
		switch(this->type){
			case RESULT:
				type_name = "result";
				break;
			case DETERMINISTIC:
				type_name = "deterministic";
				break;
			case NONDETERMINISTIC:
				type_name = "nondeterministic";
				break;
			case MACHINE:
				type_name = "machine";
				break;
			case OTHER:
				type_name = "other";
				break;
		}
		str << METRIC_COMMIT << ";" << type_name << ";" << name << ";" 
        << description << ";" << ID;
	}

	void NondeterministicMetric::print(std::ostream& str) const {
		str << NONDETERMINISTICMETRIC_COMMIT << ";" << executionID << ";" 
        << metricID << ";" << value;
	}

	void DeterministicMetric::print(std::ostream& str) const {
		str << DETERMINISTICMETRIC_COMMIT << ";" << datasetID << ";" << metricID 
        << ";" << value;
	}

	void MachineMetric::print(std::ostream& str) const {
		str << MACHINEMETRIC_COMMIT << ";" << machineID <<";"<< metricID << ";" 
        << value;
	}

	void Execution::print(std::ostream& str) const {
		str << EXECUTION_COMMIT << ";" << trialID << ";" << machineID << ";" 
        << ID;
	}

	void Trial::print(std::ostream& str) const {
		str << TRIAL_COMMIT << ";" << dataCollectionID << ";" << machineID << ";" 
        << applicationID << ";" << datasetID << ";" << ID;
	}

	void Machine::print(std::ostream& str) const {
		str << FEMACHINE_COMMIT << ";" << name << ";" << description << ";" << ID;
	}

	void Application::print(std::ostream& str) const {
		str << APPLICATION_COMMIT << ";" << name << ";" << description << ";" << ID;
	}

	void Dataset::print(std::ostream& str) const {
		str << DATASET_COMMIT << ";" << applicationID << ";" << name << ";" 
      << description << ";" << url << ";" << ID;
	}

	void DataCollection::print(std::ostream& str) const {
    str << DATACOLLECTION_COMMIT << ";" << name << ";" << description << ";" 
        << ID;
	}
} // end of namespace eiger

