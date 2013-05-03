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
#include <iostream>
#include <sstream>
#include <string>

#if NEEDSQLREAD
// MySQL++ includes
#include <mysql++.h>
#endif

// Eiger includes
#include <api/fakeeiger.h>


///////////////////////////////////////////////////////////

namespace eiger{

	error_t err;

	Log *log;

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
			log->connect(databaseLocation.c_str(), databaseName.c_str(),  username.c_str(), password.c_str());
	}

	void Disconnect(){
		if(log != NULL)
			delete log; log = NULL;
	}

	//-----------------------------------------------------------------

#if NEEDSQLREAD
	Metric::Metric(int ID){

		try{
			mysqlpp::Query q = conn->query();
			q << "SELECT type, name, description FROM metrics WHERE ID=" << ID << ";";
			mysqlpp::StoreQueryResult res = q.store();

			this->ecs = ecs_ok;
			this->ID = ID;
			this->name = (std::string) res[0]["name"];
			this->description = (std::string) res[0]["description"];

			std::string type_name = (std::string) res[0]["type"];
			if(!type_name.compare("result"))
				this->type = RESULT;
			else if(!type_name.compare("deterministic"))
				this->type = DETERMINISTIC;
			else if(!type_name.compare("nondeterministic"))
				this->type = NONDETERMINISTIC;
			else if(!type_name.compare("machine"))
				this->type = MACHINE;
			else if(!type_name.compare("other"))
				this->type = OTHER;
		}
		catch(const mysqlpp::BadQuery& er) {
			std::cerr << "eiger::Metric() Error: " << er.what() << std::endl;
			this->ecs = ecs_fail;
		}

	}
#endif

	Metric::Metric(metric_type_t type, std::string name, std::string description) : type(type), name(name), description(description) { if (!log) return; log->Metric(type, name, description); }

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
		this->ID = log->Metric_commit(type_name, name, description);
		this->ecs = ecs_ok;

	}


	NondeterministicMetric::NondeterministicMetric(ExecutionID executionID, MetricID metricID, double value) : executionID(executionID), metricID(metricID), value(value) { if (!log) return; log->NondeterministicMetric(executionID, metricID,value); }

	void NondeterministicMetric::commit() {
		if (!log) return;
		log->NondeterministicMetric_commit(executionID, metricID,value);
	}

	DeterministicMetric::DeterministicMetric(DatasetID datasetID, MetricID metricID, double value) : datasetID(datasetID), metricID(metricID), value(value) { if (!log) return; log->DeterministicMetric(datasetID, metricID, value); }

	void DeterministicMetric::commit() {
		if (!log) return;
		log->DeterministicMetric_commit(datasetID, metricID, value);
	}

	MachineMetric::MachineMetric(MachineID machineID, MetricID metricID, double value) : machineID(machineID), metricID(metricID), value(value) { if (!log) return; log->MachineMetric(machineID, metricID,  value);}

	void MachineMetric::commit() {
		if (!log) return;
		log->MachineMetric_commit(machineID,metricID, value);
	}

	Execution::Execution(TrialID trialID, MachineID machineID) : trialID(trialID), machineID(machineID){ if (!log) return; log->Execution(trialID,machineID);  ecs = ecs_pre; }

	void Execution::commit() {
		if (!log) return;
		this->ID = log->Execution_commit(trialID,machineID);
		this->ecs = ecs_ok;
	}

	Trial::Trial(DataCollectionID dataCollectionID, MachineID machineID,
			ApplicationID applicationID, DatasetID datasetID,
			PropertiesID propertiesID) : 
		dataCollectionID(dataCollectionID), machineID(machineID),
		applicationID(applicationID), datasetID(datasetID), propertiesID(propertiesID) { if (!log) return; log->Trial(dataCollectionID,machineID, applicationID, datasetID, propertiesID); ecs = ecs_pre; }

	void Trial::commit() {
		if (!log) return;
		this->ID = log->Trial_commit(dataCollectionID,machineID,applicationID,datasetID,propertiesID);
		this->ecs = ecs_ok;
	}

	/*
	   std::vector<DynamicMetric>* Trial::getDynamicMetrics() {

	   }
	 */

	Machine::Machine(std::string name, std::string description) : name(name), description(description) { if (!log) return; log->Machine(name,description); ecs = ecs_pre; }

	void Machine::commit() {
		if (!log) return;
		this->ID = log->Machine_commit(name,description);
		this->ecs = ecs_ok;
	}

	/*
	   std::vector<MachineMetric>* Machine::getMachineMetrics() {

	   }
	 */

	Dataset::Dataset(ApplicationID applicationID, std::string name, 
			std::string description, std::string url) : 
		applicationID(applicationID), name(name),
		description(description), url(url) { if (!log) return; log->Dataset(applicationID,name,description,url); ecs = ecs_pre; }

	void Dataset::commit() {
		if (!log) return;
		this->created = "somedaysoon";
		this->ID = log->Dataset_commit(applicationID,name,description,url);
		this->ecs = ecs_ok;
	}

	/*
	   std::vector<StaticMetric>* Dataset::getStaticMetrics() {

	   }
	 */

	Application::Application(std::string name, std::string description) : 
		name(name), description(description) { if (!log) return; log->Application(name, description); ecs = ecs_pre; }

	void Application::commit() {
		if (!log) return;
		this->ID = log->Application_commit(name,description);
		this->ecs = ecs_ok;
	}

	/*
	   std::vector<Dataset>* Application::getDatasets() {

	   }
	 */
#if NEEDSQLREAD

	DataCollection::DataCollection(int ID){

		try{
			mysqlpp::Query q = conn->query();
			q << "SELECT name, description, created FROM datacollections WHERE ID=" << ID << ";";
			mysqlpp::StoreQueryResult res = q.store();

			this->ID = ID;
			this->name = (std::string) res[0]["name"];
			this->description = (std::string) res[0]["description"];
			this->created = (std::string) res[0]["created"];
			ecs = ecs_ok;
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::DataCollection() Error: " << er.what() << std::endl;
			ecs = ecs_fail;
		}

	}
#endif

	DataCollection::DataCollection(std::string name, std::string description) : 
		name(name), description(description) { if (!log) return; log->DataCollection(name,description); ecs = ecs_pre; }

	void DataCollection::commit() {
		if (!log) return;
		this->ID = log->DataCollection_commit(name,description);
		this->created = "somedaysoon";
		this->ecs = ecs_ok;
	}

	/*
	   std::vector<Trial>* DataCollection::getTrials() {

	   }
	 */

#if NEEDSQLREAD

	Properties::Properties(int ID) {
		try{
			mysqlpp::Query q = conn->query();
			q << "SELECT trialID, propertyName, property FROM properties WHERE ID=" << ID << ";";
			mysqlpp::StoreQueryResult res = q.store();

			this->ID = ID;
			this->trialID = (int) res[0]["trialID"];
			this->propertyID = (int) res[0]["property"];
			this->name = (std::string) res[0]["propertyName"];
			ecs = ecs_ok;
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::Properties() Error: " << er.what() << std::endl;
			ecs = ecs_fail;
		}

	}
#endif

	Properties::Properties(std::string name, TrialID trialID, PropertiesID propertyID) : name(name), trialID(trialID), propertyID(propertyID) { if (!log) return; log->Properties(name,trialID,propertyID); ecs = ecs_pre; }

	void Properties::commit() {
		if (!log) return;
		this->ID = log->Properties_commit(name,trialID,propertyID);
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
		ss << "Trial (ID: " << this->ID << ", datacollectionID: " << this->dataCollectionID << ", machineID : " << this->machineID << ", applicationID: " << this->applicationID << ", datasetID: " << this->datasetID << ", propertiesID: " << this->propertiesID << ")";

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

	std::string Properties::toString() {

		std::stringstream ss;
		ss << "Property (name: " << this->name << ", trialID: " << this->trialID << ", propertyID: " << this->propertyID << ")";

		return ss.str();
	}
























} // end of namespace eiger

