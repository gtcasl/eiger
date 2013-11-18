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
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

// MySQL++ includes
#include <mysql++.h>
#include <ssqls.h>

// Eiger includes
#include "fakekeywords.h"
#include "eiger.h"


///////////////////////////////////////////////////////////
sql_create_2(datacollections, 1, 2, 
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_text, description);
sql_create_4(trials, 4, 0, 
             mysqlpp::sql_int, dataCollectionID,
             mysqlpp::sql_int, machineID,
             mysqlpp::sql_int, applicationID,
             mysqlpp::sql_int, datasetID);
sql_create_2(executions, 2, 0, 
             mysqlpp::sql_int, machineID,
             mysqlpp::sql_int, trialID);
sql_create_2(machines, 1, 2, 
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_text, description);
sql_create_3(machine_metrics, 2, 3, 
             mysqlpp::sql_int, machineID,
             mysqlpp::sql_int, metricID,
             mysqlpp::sql_double, metric);
sql_create_2(applications, 1, 2,
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_text, description);
sql_create_4(datasets, 2, 4,
             mysqlpp::sql_int, applicationID,
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_text, description,
             mysqlpp::sql_text, url);
sql_create_3(metrics, 2, 3,
             mysqlpp::sql_enum, type,
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_text, description);
sql_create_3(nondeterministic_metrics, 2, 3,
             mysqlpp::sql_int, executionID,
             mysqlpp::sql_int, metricID,
             mysqlpp::sql_double, metric);
sql_create_3(deterministic_metrics, 2, 3,
             mysqlpp::sql_int, datasetID,
             mysqlpp::sql_int, metricID,
             mysqlpp::sql_double, metric);

namespace eiger{

  /********
   * Static vars
   */
  static std::string dbloc;
  static std::string dbname;
  static std::string user;
  static std::string passwd;
	
  // map file local value to global db value.
	// Each time we open a new log file, we must clear the idmaps with initLocalToGlobal.
	static std::map<int, int > local2dbDataCollection;
	static std::map<int, int > local2dbApplication;
	static std::map<int, int > local2dbDataset;
	static std::map<int, int > local2dbMachine;
	static std::map<int, int > local2dbTrial;
	static std::map<int, int > local2dbExecution;
	static std::map<int, int > local2dbMetric;

  // containers for bulk insertions
#define MAKEROWVEC(X) \
  static std::vector<X> X##_rows; \
  static std::vector<int> X##_ids;
  MAKEROWVEC(datacollections)
  MAKEROWVEC(trials)
  MAKEROWVEC(executions)
  MAKEROWVEC(machines)
  MAKEROWVEC(machine_metrics)
  MAKEROWVEC(applications)
  MAKEROWVEC(datasets)
  MAKEROWVEC(metrics)
  MAKEROWVEC(nondeterministic_metrics)
  MAKEROWVEC(deterministic_metrics)

	mysqlpp::Connection* conn;
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

  std::vector<int> stableUnique(const std::vector<int>& in){
    std::vector<int> res;
    for(std::vector<int>::const_iterator it = in.begin();
        it != in.end(); ++it){
      if(std::find(res.begin(), res.end(), *it) == res.end()){
        res.push_back(*it);
      }
    }
    return res;
  }

  void resetIDs(std::vector<int>& localids, std::map<int,int>& l2d, bool make_unique,
                const std::vector<int>& globalids){
    if(make_unique){
      localids = stableUnique(localids);
    }
    int i = 0;
    for(std::vector<int>::iterator it = localids.begin();
        it != localids.end(); ++it, ++i){
      l2d[*it] = globalids[i];
    }
  }

  template<typename T>
  int insertAll(T first, T last, mysqlpp::Query& q, bool ignore){
    bool empty = true;
    std::string ignore_cmd = ignore ? " IGNORE " : " ";
    if(first == last) return 0;
    for(T it = first; it != last; ++it){
      if(empty){
        q << "INSERT" << ignore_cmd << "INTO `" << it->table() << "` (" 
          << it->field_list() << ") VALUES (";
      }else{
        q << ",(";
      }
      q << it->value_list() << ")";
      empty = false;
    }
    q.execute();
    return q.insert_id();
  }

  template<typename T>
  std::vector<int> insertAndIDByInsertID(T first, T last, mysqlpp::Query& q){
    bool empty = true;
    int first_id = insertAll(first, last, q, false);
    std::vector<int> retval;
    if(first == last) return retval;
    int i = 0;
    for(T it = first; it != last; ++it, ++i){
      retval.push_back(first_id + i);
    }
    return retval;
  }

  template<typename T>
  std::vector<int> insertAndIDByName(T first, T last, mysqlpp::Query& q){
    bool empty = true;
    insertAll(first, last, q, true);
    if(first == last) return std::vector<int>();
    for(T it = first; it != last; ++it){
      if(empty){
        q << "SELECT ID FROM `" << it->table() << "` WHERE (" << it->field_list()
          << ") IN ((";
      }else{
        q << ",(";
      }
      q << it->value_list() << ")";
      empty = false;
    }
    empty = true;
    for(T it = first; it != last; ++it){
      if(empty){
        q << ") ORDER BY FIELD(name, ";
      }else{
        q << ",";
      }
      q << "'" << it->name << "'";
      empty = false;
    }
    q << ")";
    mysqlpp::StoreQueryResult res = q.store();
    std::vector<int> retval;
    for(mysqlpp::StoreQueryResult::const_iterator it = res.begin();
        it != res.end(); ++it){
      retval.push_back((*it)[0]);
    }
    return retval;
  }

  template<typename T>
  struct nameCompare{
    std::string key_;
    nameCompare(const std::string& key) : key_(key) {}
    bool operator()(const T& yours) const { return yours.name == key_; }
  };

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
		try{
      mysqlpp::Connection conn(true);
			conn.connect(dbname.c_str(), dbloc.c_str(), user.c_str(), passwd.c_str());
      mysqlpp::Query query = conn.query();

      resetIDs(datacollections_ids, local2dbDataCollection, true,
               insertAndIDByName(datacollections_rows.begin(), 
                                 datacollections_rows.end(), query));
      resetIDs(machines_ids, local2dbMachine, true,
               insertAndIDByName(machines_rows.begin(),
                                 machines_rows.end(), query));
      std::vector<int> apgids = insertAndIDByName(applications_rows.begin(), 
                                 applications_rows.end(), query);
      resetIDs(applications_ids, local2dbApplication, true,
               apgids);
      resetIDs(metrics_ids, local2dbMetric, true,
               insertAndIDByName(metrics_rows.begin(), 
                                 metrics_rows.end(), query));
      
      for(std::vector<datasets>::iterator it = datasets_rows.begin(); 
          it != datasets_rows.end(); ++it){
        it->applicationID = local2dbApplication[it->applicationID];
      }
      std::vector<int> dsgids = insertAndIDByName(datasets_rows.begin(), 
                                 datasets_rows.end(), query);
      resetIDs(datasets_ids, local2dbDataset, true, 
               insertAndIDByName(datasets_rows.begin(), datasets_rows.end(), 
                                 query));

      for(std::vector<machine_metrics>::iterator it = machine_metrics_rows.begin(); 
          it != machine_metrics_rows.end(); ++it){
        it->machineID = local2dbMachine[it->machineID];
        it->metricID = local2dbMetric[it->metricID];
      }
      insertAll(machine_metrics_rows.begin(), machine_metrics_rows.end(), query, 
                false);

      for(std::vector<trials>::iterator it = trials_rows.begin(); 
          it != trials_rows.end(); ++it){
        it->dataCollectionID = local2dbDataCollection[it->dataCollectionID];
        it->machineID = local2dbMachine[it->machineID];
        it->applicationID = local2dbApplication[it->applicationID];
        it->datasetID = local2dbDataset[it->datasetID];
      }
      resetIDs(trials_ids, local2dbTrial, false,
               insertAndIDByInsertID(trials_rows.begin(), 
                                     trials_rows.end(), query));
      
      for(std::vector<executions>::iterator it = executions_rows.begin(); 
          it != executions_rows.end(); ++it){
        it->machineID = local2dbMachine[it->machineID];
        it->trialID = local2dbTrial[it->trialID];
      }
      resetIDs(executions_ids, local2dbExecution, false,
               insertAndIDByInsertID(executions_rows.begin(), 
                                     executions_rows.end(), query));

      for(std::vector<nondeterministic_metrics>::iterator it = nondeterministic_metrics_rows.begin(); 
          it != nondeterministic_metrics_rows.end(); ++it){
        it->executionID = local2dbExecution[it->executionID];
        it->metricID = local2dbMetric[it->metricID];
      }
      insertAll(nondeterministic_metrics_rows.begin(), 
                nondeterministic_metrics_rows.end(), query, false);

      for(std::vector<deterministic_metrics>::iterator it = deterministic_metrics_rows.begin(); 
          it != deterministic_metrics_rows.end(); ++it){
        it->datasetID = local2dbDataset[it->datasetID];
        it->metricID = local2dbMetric[it->metricID];
      }
      insertAll(deterministic_metrics_rows.begin(), 
                deterministic_metrics_rows.end(), query, false);
		}
		catch (const mysqlpp::Exception& er){
			std::cerr << "eiger::Disconnect() Error: " << er.what() << std::endl;
		}
	}

	//-----------------------------------------------------------------

	Metric::Metric(metric_type_t type, std::string name, std::string description) : type(type), name(name), description(description) { ecs = ecs_pre; }

	void Metric::commit() {

		std::string type_name;
		switch(type){
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
    std::vector<metrics>::iterator it = std::find_if(metrics_rows.begin(), 
                                                     metrics_rows.end(), 
                                                     nameCompare<metrics>(name));
    int id;
    if(it != metrics_rows.end()){
      id = std::distance(metrics_rows.begin(), it);
    } else {
      id = metrics_rows.size();
      metrics_rows.push_back(metrics(type_name, name, description));
    }
    metrics_ids.push_back(id);
    ecs = ecs_ok;
    ID = id;
	}

	NondeterministicMetric::NondeterministicMetric(ExecutionID executionID, MetricID metricID, double value) :  executionID(executionID), metricID(metricID), value(value) {}

	void NondeterministicMetric::commit() {
    nondeterministic_metrics_rows.push_back(nondeterministic_metrics(executionID, metricID, value));
	}

	DeterministicMetric::DeterministicMetric(DatasetID datasetID, MetricID metricID, double value) : datasetID(datasetID), metricID(metricID), value(value) {}

	void DeterministicMetric::commit() {
    deterministic_metrics_rows.push_back(deterministic_metrics(datasetID, metricID, value));
	}

	MachineMetric::MachineMetric(MachineID machineID, MetricID metricID, double value) : machineID(machineID), metricID(metricID), value(value) {}

	void MachineMetric::commit() {
    machine_metrics_rows.push_back(machine_metrics(machineID, metricID, value));
	}

	Execution::Execution(TrialID trialID, MachineID machineID) : trialID(trialID), machineID(machineID){ ecs = ecs_pre;}

	void Execution::commit() {
    int id = executions_rows.size();
    executions_ids.push_back(id);
    executions_rows.push_back(executions(machineID, trialID));
    ecs = ecs_ok;
    ID = id;
	}

	Trial::Trial(DataCollectionID dataCollectionID, MachineID machineID,
			ApplicationID applicationID, DatasetID datasetID) :
		dataCollectionID(dataCollectionID), machineID(machineID),
		applicationID(applicationID), datasetID(datasetID) { ecs = ecs_pre; }

	void Trial::commit() {
    int id  = trials_rows.size();
    trials_ids.push_back(id);
    trials_rows.push_back(trials(dataCollectionID, machineID, applicationID, datasetID));
    ID = id;
    ecs = ecs_ok;
	}

	Machine::Machine(std::string name, std::string description) : name(name), description(description) { ecs = ecs_pre; }

	void Machine::commit() {
    std::vector<machines>::iterator it = std::find_if(machines_rows.begin(), 
                                                      machines_rows.end(), 
                                                      nameCompare<machines>(name));
    int id;
    if(it != machines_rows.end()){
      id = std::distance(machines_rows.begin(), it);
    } else {
      id = machines_rows.size();
      machines_rows.push_back(machines(name, description));
    }
    machines_ids.push_back(id);
    ecs = ecs_ok;
    ID = id;
	}

	Dataset::Dataset(ApplicationID applicationID, std::string name, 
			std::string description, std::string url) : 
		applicationID(applicationID), name(name),
		description(description), url(url) { ecs = ecs_pre; }

	void Dataset::commit() {
    std::vector<datasets>::iterator it = std::find_if(datasets_rows.begin(), 
                                                      datasets_rows.end(), 
                                                      nameCompare<datasets>(name));
    int id;
    if(it != datasets_rows.end()){
      id = std::distance(datasets_rows.begin(), it);
    } else {
      id = datasets_rows.size();
      datasets_rows.push_back(datasets(applicationID, name, description, url));
    }
    datasets_ids.push_back(id);
    ecs = ecs_ok;
    ID = id;
	}

	Application::Application(std::string name, std::string description) : 
		name(name), description(description) { ecs = ecs_pre; }

	void Application::commit() {
    std::vector<applications>::iterator it = std::find_if(applications_rows.begin(), 
                                                          applications_rows.end(), 
                                                          nameCompare<applications>(name));
    int id;
    if(it != applications_rows.end()){
      id = std::distance(applications_rows.begin(), it);
    } else {
      id = applications_rows.size();
      applications_rows.push_back(applications(name, description));
    }
    applications_ids.push_back(id);
    ecs = ecs_ok;
    ID = id;
	}

	DataCollection::DataCollection(std::string name, std::string description) : 
		name(name), description(description) { ecs = ecs_pre; }

	void DataCollection::commit() {
    std::vector<datacollections>::iterator it = std::find_if(datacollections_rows.begin(), 
                                                             datacollections_rows.end(), 
                                                             nameCompare<datacollections>(name));
    int id;
    if(it != datacollections_rows.end()){
      id = std::distance(datacollections_rows.begin(), it);
    } else {
      id = datacollections_rows.size();
      datacollections_rows.push_back(datacollections(name, description));
    }
    datacollections_ids.push_back(id);
    ecs = ecs_ok;
    ID = id;
	}

	void Metric::print(std::ostream& str) {

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

	void NondeterministicMetric::print(std::ostream& str) {
		str << NONDETERMINISTICMETRIC_COMMIT << ";" << executionID << ";" 
        << metricID << ";" << value;
	}

	void DeterministicMetric::print(std::ostream& str) {
		str << DETERMINISTICMETRIC_COMMIT << ";" << datasetID << ";" << metricID 
        << ";" << value;
	}

	void MachineMetric::print(std::ostream& str) {
		str << MACHINEMETRIC_COMMIT << ";" << machineID <<";"<< metricID << ";" 
        << value;
	}

	void Execution::print(std::ostream& str) {
		str << EXECUTION_COMMIT << ";" << trialID << ";" << machineID << ";" 
        << ID;
	}

	void Trial::print(std::ostream& str) {
		str << TRIAL_COMMIT << ";" << dataCollectionID << ";" << machineID << ";" 
        << applicationID << ";" << datasetID << ";" << ID;
	}

	void Machine::print(std::ostream& str) {
		str << FEMACHINE_COMMIT << ";" << name << ";" << description << ";" << ID;
	}

	void Application::print(std::ostream& str) {
		str << APPLICATION_COMMIT << ";" << name << ";" << description << ";" << ID;
	}

	void Dataset::print(std::ostream& str) {
		str << DATASET_COMMIT << ";" << applicationID << ";" << name << ";" 
      << description << ";" << url << ";" << ID;
	}

	void DataCollection::print(std::ostream& str) {
    str << DATACOLLECTION_COMMIT << ";" << name << ";" << description << ";" 
        << ID;
	}
} // end of namespace eiger

