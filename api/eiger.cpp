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

// MySQL++ includes
#include <mysql++.h>

// Eiger includes
#include <api/eiger.h>


///////////////////////////////////////////////////////////

namespace eiger{

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

	void Connect(std::string databaseLocation,
			std::string databaseName,
			std::string username,
			std::string password) {

		try{
			conn = new mysqlpp::Connection(true);
			conn->connect(databaseName.c_str(), databaseLocation.c_str(), username.c_str(), password.c_str());
		}
		catch (const mysqlpp::Exception& er){
			std::cerr << "eiger::Connect() Error: " << er.what() << std::endl;
		}
	}

	void Disconnect(){
		if(conn != NULL)
			delete conn;
	}

	//-----------------------------------------------------------------

	Metric::Metric(int ID) {

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

		try{
			mysqlpp::Query q = conn->query();
			q << "INSERT IGNORE INTO metrics (type, name, description) VALUES(" << mysqlpp::quote << type_name << "," << mysqlpp::quote << name << "," << mysqlpp::quote << description << ");";
			mysqlpp::SimpleResult res = q.execute();

			q << "SELECT ID FROM metrics WHERE name=" << mysqlpp::quote << name <<";";
			mysqlpp::StoreQueryResult myID = q.store();
			this->ID = (int) myID[0]["ID"];
			ecs = ecs_ok;
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::Metric::commit() Error: " << er.what() << std::endl;
			ecs = ecs_fail;
		}

	}

// used to control formatting of insert stream for double values. 
#define EIGERDB_FP_PREC 18

	NondeterministicMetric::NondeterministicMetric(ExecutionID executionID, MetricID metricID, double value) :  executionID(executionID), metricID(metricID), value(value) {}

	void NondeterministicMetric::commit() {
		try{
			std::ostringstream fpout;
			fpout.precision(EIGERDB_FP_PREC); 
			fpout << value;
			mysqlpp::Query q = conn->query();
			q << "INSERT INTO nondeterministic_metrics (executionID, metricID, metric) VALUES(" << executionID << "," << metricID << "," << fpout.str() << ");";

			mysqlpp::SimpleResult res = q.execute();
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::NondeterministicMetric::commit() Error: " << er.what() << std::endl;
		}

	}

	DeterministicMetric::DeterministicMetric(DatasetID datasetID, MetricID metricID, double value) : datasetID(datasetID), metricID(metricID), value(value) {}

	void DeterministicMetric::commit() {
		try{
			std::ostringstream fpout;
			fpout.precision(EIGERDB_FP_PREC); 
			fpout << value;
			mysqlpp::Query q = conn->query();
			q << "INSERT INTO deterministic_metrics (datasetID, metricID, metric) VALUES(" << datasetID << "," << metricID << "," << fpout.str() << ");";

			mysqlpp::SimpleResult res = q.execute();
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::DeterministicMetric::commit() Error: " << er.what() << std::endl;
		}


	}

	MachineMetric::MachineMetric(MachineID machineID, MetricID metricID, double value) : machineID(machineID), metricID(metricID), value(value) {}

	void MachineMetric::commit() {
		try{
			mysqlpp::Query q = conn->query();

			q << "SELECT COUNT(*) FROM machine_metrics WHERE machineID=" << machineID << " AND metricID=" << metricID <<";";
			mysqlpp::StoreQueryResult exists = q.store();
			//std::cout << "exists? " << (int) exists[0]["COUNT(*)"] << std::endl;
			if((int) exists[0]["COUNT(*)"] == 0){
				std::ostringstream fpout;
				fpout.precision(EIGERDB_FP_PREC); 
				fpout << value;
				q << "INSERT INTO machine_metrics (machineID, metricID, metric) VALUES(" << machineID << "," << metricID << "," << fpout.str() << ");";
				mysqlpp::SimpleResult res = q.execute();
			}
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::MachineMetric::commit() Error: " << er.what() << std::endl;
		}

	}

	Execution::Execution(TrialID trialID, MachineID machineID) : trialID(trialID), machineID(machineID){ ecs = ecs_pre;}

	void Execution::commit() {
		try{
			mysqlpp::Query q = conn->query();
			q << "INSERT INTO executions (trialID, machineID) VALUES(" << 
				trialID << "," <<
				machineID << ");";

			mysqlpp::SimpleResult res = q.execute();

			this->ID = q.insert_id();
			ecs = ecs_ok;
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::Execution::commit() Error: " << er.what() << std::endl;
			ecs = ecs_fail;
		}
	}

	Trial::Trial(DataCollectionID dataCollectionID, MachineID machineID,
			ApplicationID applicationID, DatasetID datasetID,
			PropertiesID propertiesID) : 
		dataCollectionID(dataCollectionID), machineID(machineID),
		applicationID(applicationID), datasetID(datasetID), propertiesID(propertiesID) { ecs = ecs_pre; }

	void Trial::commit() {
		try{
			mysqlpp::Query q = conn->query();
			q << "INSERT INTO trials (dataCollectionID, machineID, applicationID, datasetID, propertiesID) VALUES(" << 
				dataCollectionID << "," <<
				machineID << "," <<
				applicationID << "," <<
				datasetID << "," <<
				propertiesID << ");";

			mysqlpp::SimpleResult res = q.execute();

			this->ID = q.insert_id();
			ecs = ecs_ok;
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::Trial::commit() Error: " << er.what() << std::endl;
			ecs = ecs_fail;
		}
	}

	/*
	   std::vector<DynamicMetric>* Trial::getDynamicMetrics() {

	   }
	 */

	Machine::Machine(std::string name, std::string description) : name(name), description(description) { ecs = ecs_pre; }

	void Machine::commit() {

		try{
			mysqlpp::Query q = conn->query();
			q << "INSERT IGNORE INTO machines (name, description) VALUES(" << mysqlpp::quote << name << "," << mysqlpp::quote << description << ");";
			mysqlpp::SimpleResult res = q.execute();

			q << "SELECT ID FROM machines WHERE name=" << mysqlpp::quote << name <<";";
			mysqlpp::StoreQueryResult myID = q.store();
			this->ID = (int) myID[0]["ID"];
			ecs = ecs_ok; 
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::Machine::commit() Error: " << er.what() << std::endl;
			ecs = ecs_fail; 
		}

	}

	/*
	   std::vector<MachineMetric>* Machine::getMachineMetrics() {

	   }
	 */

	Dataset::Dataset(ApplicationID applicationID, std::string name, 
			std::string description, std::string url) : 
		applicationID(applicationID), name(name),
		description(description), url(url) { ecs = ecs_pre; }

	void Dataset::commit() {

		try{
			mysqlpp::Query q = conn->query();
			q << "SELECT COUNT(*) FROM datasets WHERE applicationID=" << applicationID << " AND name=" << mysqlpp::quote << name <<";";
			mysqlpp::StoreQueryResult exists = q.store();

			if((int) exists[0]["COUNT(*)"] != 0){
				q << "SELECT ID from datasets where applicationID=" << applicationID << " AND name=" << mysqlpp::quote << name << ";";
				mysqlpp::StoreQueryResult dID = q.store();
				this->ID = (int) dID[0]["ID"];
			}
			else{
				q << "INSERT IGNORE INTO datasets (applicationID, name, description, created, url) VALUES(" << applicationID << "," << mysqlpp::quote << name << "," << mysqlpp::quote << description << ",NOW()," << mysqlpp::quote << url << ");";
				mysqlpp::SimpleResult res = q.execute();

				q << "SELECT ID,created FROM datasets WHERE applicationID=" << applicationID << " AND name=" << mysqlpp::quote << name <<";";
				mysqlpp::StoreQueryResult myrow = q.store();
				this->ID = (int) myrow[0]["ID"];
				this->created = (std::string) myrow[0]["created"];
			}
			ecs = ecs_ok;
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::Dataset::commit() Error: " << er.what() << std::endl;
			ecs = ecs_fail;
		}

	}

	/*
	   std::vector<StaticMetric>* Dataset::getStaticMetrics() {

	   }
	 */

	Application::Application(std::string name, std::string description) : 
		name(name), description(description) { ecs = ecs_pre; }

	void Application::commit() {

		try{
			mysqlpp::Query q = conn->query();
			q << "INSERT IGNORE INTO applications (name, description) VALUES(" << mysqlpp::quote << name << "," << mysqlpp::quote << description << ");";
			mysqlpp::SimpleResult res = q.execute();

			q << "SELECT ID FROM applications WHERE name=" << mysqlpp::quote << name <<";";
			mysqlpp::StoreQueryResult myID = q.store();
			this->ID = (int) myID[0]["ID"];
			ecs = ecs_ok;
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::Application::commit() Error: " << er.what() << std::endl;
			ecs = ecs_fail;
		}

	}

	/*
	   std::vector<Dataset>* Application::getDatasets() {

	   }
	 */

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

	DataCollection::DataCollection(std::string name, std::string description) : 
		name(name), description(description) { ecs = ecs_pre; }

	void DataCollection::commit() {

		try{
			mysqlpp::Query q = conn->query();
			q << "INSERT IGNORE INTO datacollections (name, description, created) VALUES(" << mysqlpp::quote << name << "," << mysqlpp::quote << description << ", NOW());";
			mysqlpp::SimpleResult res = q.execute();

			q << "SELECT ID,created FROM datacollections WHERE name=" << mysqlpp::quote << name <<";";
			mysqlpp::StoreQueryResult myrow = q.store();
			this->ID = (int) myrow[0]["ID"];
			this->created = (std::string) myrow[0]["created"];
			ecs = ecs_ok;
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::DataCollection::commit() Error: " << er.what() << std::endl;
			ecs = ecs_fail;
		}

	}

	/*
	   std::vector<Trial>* DataCollection::getTrials() {

	   }
	 */

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

	Properties::Properties(std::string name, TrialID trialID, PropertiesID propertyID) : name(name), trialID(trialID), propertyID(propertyID) { ecs = ecs_pre; }

	void Properties::commit() {
		try{
			mysqlpp::Query q = conn->query();
			q << "INSERT IGNORE INTO properties (trialID, propertyName, property) VALUES(" << trialID << "." << mysqlpp::quote << name << "," << propertyID << ");";
			mysqlpp::SimpleResult res = q.execute();

			q << "SELECT ID,created FROM properties WHERE propertyName=" << mysqlpp::quote << name <<";";
			mysqlpp::StoreQueryResult myrow = q.store();
			this->ID = (int) myrow[0]["ID"];
			this->ecs = ecs_ok;
		}
		catch(const mysqlpp::BadQuery& er){
			std::cerr << "eiger::::commit() Error: " << er.what() << std::endl;
			this->ecs = ecs_fail;
		}

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

