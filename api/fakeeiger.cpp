/**********************************************************
 * Eiger Performance Modeling Framework nosql collection interface
 * 
 * Ben Allan, baallan@sandia.gov
 * July 2012, Sandia National Laboratories
 * 
 * C++ to file implementation of Eiger API
 * 
 **********************************************************/

// C++ string includes
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "fakekeywords.h" 

// Eiger includes
#include "eiger.h"


///////////////////////////////////////////////////////////

namespace eiger{

	error_t err;

  struct Log{
    std::fstream f;
    std::map< std::string, int> Metric_idmap;
    std::map< std::string, int> Machine_idmap;
    std::map< std::string, int> Dataset_idmap;
    std::map< std::string, int> Application_idmap;
    std::map< std::string, int> DataCollection_idmap;
  } *log;

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

    log = new Log();
    char* tmpname = strdup("fakeeiger.log.XXXXXX");
    if(mkstemp(tmpname) == -1){
      throw "Unable to open unique output file";
    }
		log->f.open(tmpname,std::fstream::out|std::fstream::trunc); 
		log->f.precision(18);
		log->f << FEVERSION<<";2\n";
		log->f << FEFORMAT << ";" KWFORMAT "\n";
		log->f << FECONNECT << ";" << databaseLocation << ";" << databaseName << ";" <<username << ";" << password << "\n";
	}

	void Disconnect(){
		if(log != NULL) {
      log->f.close();
			delete log;
			log = NULL;
		}
	}

	//-----------------------------------------------------------------

	Metric::Metric(int ID){}

	Metric::Metric(metric_type_t type, std::string name, std::string description) : type(type), name(name), description(description) {ecs=ecs_pre;}

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

		if (!log) return;
    std::ostringstream s;
		s<<METRIC_COMMIT<<";"<< type_name << ";" << name << ";" << description ;
		std::string key = s.str();
		log->f << key ;
    int id;
		if (log->Metric_idmap.find(key) != log->Metric_idmap.end()) {
			id = log->Metric_idmap[key];
    } else {
			id = 1 + log->Metric_idmap.size();
      log->Metric_idmap[key] = id;
		}
    log->f << ";" << id << "\n";
		this->ID = id;
		this->ecs = ecs_ok;
	}


	NondeterministicMetric::NondeterministicMetric(ExecutionID executionID, MetricID metricID, double value) : executionID(executionID), metricID(metricID), value(value) {}

	void NondeterministicMetric::commit() {
		if (!log) return;
		log->f << NONDETERMINISTICMETRIC_COMMIT<<";" << executionID << " ;" << metricID << " ;" << value <<  "\n";
	}

	DeterministicMetric::DeterministicMetric(DatasetID datasetID, MetricID metricID, double value) : datasetID(datasetID), metricID(metricID), value(value) {}

	void DeterministicMetric::commit() {
		if (!log) return;
		log->f << DETERMINISTICMETRIC_COMMIT<<";" << datasetID << " ;" << metricID << " ;" << value << "\n";
	}

	MachineMetric::MachineMetric(MachineID machineID, MetricID metricID, double value) : machineID(machineID), metricID(metricID), value(value) {}

	void MachineMetric::commit() {
		if (!log) return;
		log->f<< MACHINEMETRIC_COMMIT<<";" << machineID <<" ;"<< metricID<<" ;" << value << "\n";
	}

	Execution::Execution(TrialID trialID, MachineID machineID) : trialID(trialID), machineID(machineID) {ecs=ecs_pre;}

	void Execution::commit() {
		if (!log) return;
    static int id_count = 0;
		std::ostringstream s;
		s<<EXECUTION_COMMIT<<";" << machineID << ";" << trialID << ";" << ++id_count << "\n";
		std::string key = s.str();
		log->f << key ;
    this->ID = id_count;
		this->ecs = ecs_ok;
	}

	Trial::Trial(DataCollectionID dataCollectionID, MachineID machineID,
			ApplicationID applicationID, DatasetID datasetID) : 
		dataCollectionID(dataCollectionID), machineID(machineID),
		applicationID(applicationID), datasetID(datasetID) {ecs=ecs_pre;}

	void Trial::commit() {
		if (!log) return;
    static int id_count = 0;
		std::ostringstream s;
		s<< TRIAL_COMMIT<<";" << dataCollectionID << " ;" <<
                                machineID << " ;" <<
                                applicationID << " ;" <<
                                datasetID << " ;" <<
                                ++id_count << "\n";
		std::string key = s.str();
		log->f << key ;
    this->ID = id_count;
		this->ecs = ecs_ok;
	}

	Machine::Machine(std::string name, std::string description) : name(name), description(description) {ecs=ecs_pre;}

	void Machine::commit() {
		if (!log) return;
		std::ostringstream s;
		s << FEMACHINE_COMMIT<<";" <<name <<";"<< description ;
		std::string key = s.str();
		log->f << key ;
    int id;
		if (log->Machine_idmap.find(key) != log->Machine_idmap.end()) {
			id = log->Machine_idmap[key];
    } else {
      id = log->Machine_idmap[key] = 1+log->Machine_idmap.size();
    }
    log->f << ";" << id << "\n";
		this->ID = id;
		this->ecs = ecs_ok;
	}

	Dataset::Dataset(ApplicationID applicationID, std::string name, 
			std::string description, std::string url) : 
		applicationID(applicationID), name(name),
		description(description), url(url) {ecs=ecs_pre;}

	void Dataset::commit() {
		if (!log) return;
		std::ostringstream s;
		s << DATASET_COMMIT<<";" <<applicationID <<";"<<name <<";"<< description <<";"<< url ;
		std::string key = s.str();
		log->f << key ;
    int id;
    if (log->Dataset_idmap.find(key) != log->Dataset_idmap.end()) {
			id = log->Dataset_idmap[key];
    } else {
      id = log->Dataset_idmap[key] = 1+log->Dataset_idmap.size();
    }
    log->f << ";" << id << "\n";
    this->ID = id;
		this->created = "somedaysoon";
		this->ecs = ecs_ok;
	}

	Application::Application(std::string name, std::string description) : 
		name(name), description(description) {ecs=ecs_pre;}

	void Application::commit() {
		if (!log) return;
		std::ostringstream s;
		s << APPLICATION_COMMIT<<";" <<name <<";"<< description ;
		std::string key = s.str();
		log->f << key;
    int id;
		if (log->Application_idmap.find(key) != log->Application_idmap.end()) {
			id = log->Application_idmap[key];
    } else {
      id = log->Application_idmap[key] = 1+log->Application_idmap.size();
    }
    log->f << ";" << id << "\n";
		this->ID = id;
		this->ecs = ecs_ok;
	}

	DataCollection::DataCollection(int ID) {}

	DataCollection::DataCollection(std::string name, std::string description) : 
		name(name), description(description) {ecs=ecs_pre;}

	void DataCollection::commit() {
		if (!log) return;
    std::ostringstream s;
    s << DATACOLLECTION_COMMIT<<";" <<name <<";"<< description ;
    std::string key = s.str();
    log->f << key;
    int id;
    if (log->DataCollection_idmap.find(key) != log->DataCollection_idmap.end()) {
      id = log->DataCollection_idmap[key];
    } else {
      id = log->DataCollection_idmap[key] = 1+log->DataCollection_idmap.size();
    }
    log->f << ";" << id << "\n";
		this->ID = id;
		this->created = "somedaysoon";
		this->ecs = ecs_ok;
	}

	std::string Metric::toString() {
		std::string type;
		switch(this->type){
			case RESULT:
				type = "result";
				break;
			case DETERMINISTIC:
				type = "deterministic";
				break;
			case NONDETERMINISTIC:
				type = "nondeterministic";
				break;
			case MACHINE:
				type = "machine";
				break;
			case OTHER:
				type = "other";
				break;
		}

		std::stringstream ss;
		ss << "Metric (ID: " << this->ID << ", datatype: " << type << ", name: " << this->name << ", description: " << this->description << ")";

		return ss.str();

	}

	std::string NondeterministicMetric::toString() {

		std::stringstream ss;
		ss << "NondeterministicMetric(metricID: " << this->metricID << ", executionID : " << this->executionID << ", value: " << this->value << ")";

		return ss.str();

	}

	std::string DeterministicMetric::toString() {

		std::stringstream ss;
		ss << "DeterministicMetric(metricID: " << this->metricID << ", datasetID: " << this->datasetID << ", value: " << this->value << ")";

		return ss.str();

	}

	std::string MachineMetric::toString() {

		std::stringstream ss;
		ss << "MachineMetric (metricID: " << this->metricID << ", machineID: " << this->machineID << ", value: " << this->value << ")";

		return ss.str();
	}

	std::string Execution::toString() {

		std::stringstream ss;
		ss << "Execution(ID: " << this->ID << ", trialID: " << this->trialID << ", machineID : " << this->machineID << ")";

		return ss.str();
	}

	std::string Trial::toString() {

		std::stringstream ss;
		ss << "Trial (ID: " << this->ID << ", datacollectionID: " << this->dataCollectionID << ", machineID : " << this->machineID << ", applicationID: " << this->applicationID << ", datasetID: " << this->datasetID << ")";

		return ss.str();
	}

	std::string Machine::toString() {

		std::stringstream ss;
		ss << "Machine (ID: " << this->ID << ", name: " << this->name << ", description: " << this->description << ")";

		return ss.str();
	}

	std::string Application::toString() {

		std::stringstream ss;
		ss << "Application (ID: " << this->ID << ", name: " << this->name << ", description: " << this->description << ")";

		return ss.str();
	}

	std::string Dataset::toString() {

		std::stringstream ss;
		ss << "Dataset (ID: " << this->ID << ", name: " << this->name << ", description: " << this->description << ", created: " << this->created << ", url: " << this->url << ")";

		return ss.str();
	}

	std::string DataCollection::toString() {

		std::stringstream ss;
		ss << "DataCollection (ID: " << this->ID << ", name: " << this->name << ", description: " << this->description << ", created: " << this->created << ")";

		return ss.str();
	}

} // end of namespace eiger

