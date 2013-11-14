/**********************************************************
* Eiger Loader
* 
* Eric Anger
* Nov 2013
* 
* When supplied a list of files, each is read and parsed, 
* and the appropriate commands are executed.
**********************************************************/
#include <iostream>
#include <boost/algorithm/string.hpp>
// C++ string includes
#include <string>
// STL includes
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
// MySQL++
#include <mysql++.h>
#include <ssqls.h>

#include "fakekeywords.h" 

sql_create_3(datacollections, 1, 2, 
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_text, description,
             mysqlpp::sql_datetime, created);
sql_create_5(trials, 4, 0, 
             mysqlpp::sql_int, dataCollectionID,
             mysqlpp::sql_int, machineID,
             mysqlpp::sql_int, applicationID,
             mysqlpp::sql_int, datasetID,
             mysqlpp::sql_int, propertiesID);
sql_create_2(executions, 2, 0, 
             mysqlpp::sql_int, machineID,
             mysqlpp::sql_int, trialID);
sql_create_3(properties, 3, 0, 
             mysqlpp::sql_int, trialID,
             mysqlpp::sql_text, propertyName,
             mysqlpp::sql_int, property);
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
sql_create_5(datasets, 2, 4,
             mysqlpp::sql_int, applicationID,
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_text, description,
             mysqlpp::sql_text, url,
             mysqlpp::sql_datetime, created);
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

namespace eiger {
/*
This loads a datafile using a real eiger mysqlpp library.
*/
class FakeEigerLoader {

  public:
	FakeEigerLoader(): connected(false), conn(true) {
    std::cout << "Initializing" << std::endl;
		initmaps();
	}
	~FakeEigerLoader() { }
	void parse(std::vector<std::string> filenames) {
		std::string line;
		std::vector< std::string>::size_type nf = filenames.size();
    std::cout << "Parsing " << nf << " logs" << std::endl;
		for (std::vector< std::string>::size_type i = 0; i < nf; i++) {
			std::ifstream myfile (filenames[i].c_str());
			if (myfile.is_open())
			{
				initLocalToGlobal();
				std::cout << "parsing " << filenames[i] <<"\n";
				while ( myfile.good() )
				{
					getline (myfile,line);
					one(line);
				}
				myfile.close();
			}
		}
    finalize();
	}

	private:
	static const int version = 2;
	bool connected;
	std::vector<std::string> connargs;
  mysqlpp::Connection conn;

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

	// map file local value to global db value.
	// Each time we open a new log file, we must clear the idmaps with initLocalToGlobal.
	std::map<int, int > local2dbDataCollection;
	std::map<int, int >   local2dbApplication;
	std::map<int, int >   local2dbDataset;
	std::map<int, int >   local2dbMachine;
	std::map<int, int >   local2dbTrial;
	std::map<int, int >   local2dbExecution;
	std::map<int, int >   local2dbMetric;

  // containers for bulk insertions
#define MAKEROWVEC(X) \
  std::vector<X> X##_rows; \
  std::vector<int> X##_ids;
  MAKEROWVEC(datacollections)
  MAKEROWVEC(trials)
  MAKEROWVEC(executions)
  MAKEROWVEC(properties)
  MAKEROWVEC(machines)
  MAKEROWVEC(machine_metrics)
  MAKEROWVEC(applications)
  MAKEROWVEC(datasets)
  MAKEROWVEC(metrics)
  MAKEROWVEC(nondeterministic_metrics)
  MAKEROWVEC(deterministic_metrics)

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
        // takes name,loc,user,passwd, but given loc,name,user,passwd
				conn.connect(v[2].c_str(),v[1].c_str(),v[3].c_str(),v[4].c_str());
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
		case DataCollection_commit:
			{
				datacollections_ids.push_back(toInt(v[3])); // id as logged
        datacollections_rows.push_back(datacollections(v[1],v[2]));
			}
			break;
		case Application_commit:
			{
				applications_ids.push_back(toInt(v[3])); // id as logged
				applications_rows.push_back(applications(v[1],v[2]));
			}
			break;
		case Dataset_commit:
			{
				datasets_ids.push_back(toInt(v[5])); // id as logged
				datasets_rows.push_back(datasets(toInt(v[1]),v[2],v[3],v[4]));
			}
			break;
		case Machine_commit:
			{
				machines_ids.push_back(toInt(v[3])); // id as logged
				machines_rows.push_back(machines(v[1],v[2]));
			}
			break;
		case Trial_commit:
			{
				trials_ids.push_back(toInt(v[6])); // id as logged
				int dci  = toInt(v[1]);
				int mi = toInt(v[2]); 
				int ai = toInt(v[3]);
				int dsi = toInt(v[4]);
				trials_rows.push_back(trials(dci,mi, ai, dsi));
			}
			break;
		case Execution_commit:
			{
				executions_ids.push_back(toInt(v[3])); // id as logged
				int ti = toInt(v[1]); 
				int mi = toInt(v[2]);
				executions_rows.push_back(executions(ti, mi));
			}
			break;
		case MachineMetric_commit:
			{
				int mai = toInt(v[1]);
				int mi = toInt(v[2]);
				machine_metrics_rows.push_back(machine_metrics(mai,mi,toDouble(v[3])));
			}
			break;
		case DeterministicMetric_commit:
			{
				int dsi = toInt(v[1]);
				int mi = toInt(v[2]);
				deterministic_metrics_rows.push_back(deterministic_metrics(dsi,mi,toDouble(v[3])));
			}
			break;
		case NondeterministicMetric_commit:
			{
				int ei = toInt(v[1]);
				int mi = toInt(v[2]);
				nondeterministic_metrics_rows.push_back(nondeterministic_metrics(ei,mi,toDouble(v[3])));
			}
			break;
		case Metric_commit:
			{
				metrics_ids.push_back(toInt(v[4])); // id as logged
				metrics_rows.push_back(metrics(v[1],v[2],v[3]));
			}
			break;
		default:
			throw "unexpected enum value in one() handling";
		} // end switch
	} // end one()

  // Go through and actually commit everything, making sure to replace local 
  // IDs with db IDs.
  void finalize(){
    mysqlpp::Query query = conn.query();
#define INSERTANDRESET(n, l2d) \
    resetIDs(n##_ids, local2db##l2d, insertAll(n##_rows.begin(), n##_rows.end(), query));
#define INSERT(n) \
    insertAll(n##_rows.begin(), n##_rows.end(), query);

    INSERTANDRESET(datacollections, DataCollection)
    INSERTANDRESET(machines, Machine)
    INSERTANDRESET(applications, Application)
    INSERTANDRESET(metrics, Metric)
    
    for(std::vector<datasets>::iterator it = datasets_rows.begin(); 
        it != datasets_rows.end(); ++it){
      it->applicationID = local2dbApplication[it->applicationID];
    }
    INSERTANDRESET(datasets, Dataset)

    for(std::vector<machine_metrics>::iterator it = machine_metrics_rows.begin(); 
        it != machine_metrics_rows.end(); ++it){
      it->machineID = local2dbMachine[it->machineID];
      it->metricID = local2dbMetric[it->metricID];
    }
    INSERT(machine_metrics)

    for(std::vector<trials>::iterator it = trials_rows.begin(); 
        it != trials_rows.end(); ++it){
      it->dataCollectionID = local2dbDataCollection[it->dataCollectionID];
      it->machineID = local2dbMachine[it->machineID];
      it->applicationID = local2dbApplication[it->applicationID];
      it->datasetID = local2dbDataset[it->datasetID];
    }
    INSERTANDRESET(trials, Trial)
    
    for(std::vector<executions>::iterator it = executions_rows.begin(); 
        it != executions_rows.end(); ++it){
      it->machineID = local2dbMachine[it->machineID];
      it->trialID = local2dbTrial[it->trialID];
    }
    INSERTANDRESET(executions, Execution)

    for(std::vector<nondeterministic_metrics>::iterator it = nondeterministic_metrics_rows.begin(); 
        it != nondeterministic_metrics_rows.end(); ++it){
      it->executionID = local2dbExecution[it->executionID];
      it->metricID = local2dbMetric[it->metricID];
    }
    INSERT(nondeterministic_metrics)

    for(std::vector<deterministic_metrics>::iterator it = deterministic_metrics_rows.begin(); 
        it != deterministic_metrics_rows.end(); ++it){
      it->datasetID = local2dbDataset[it->datasetID];
      it->metricID = local2dbMetric[it->metricID];
    }
    INSERT(deterministic_metrics)

  }

  void resetIDs(std::vector<int>& ids, std::map<int,int>& l2d, int first_id){
    int i = 0;
    for(std::vector<int>::iterator it = ids.begin();
        it != ids.end(); ++it, ++i){
      l2d[*it] = first_id + i;
    }
  }

  template<typename T>
  int insertAll(T first, T last, mysqlpp::Query& q){
    bool empty = true;
    if(first == last){std::cerr << "empty set" << std::endl; return 0;}
    for(T it = first; it != last; ++it){
      if(empty){
        q << "INSERT INTO `" << it->table() << "` (" << it->field_list()
          << ") VALUES (";
      }else{
        q << ",(";
      }
      q << it->value_list() << ")";
      empty = false;
    }
    q.execute();
    return q.insert_id();
  }

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
  }

  void initLocalToGlobal() {
    local2dbDataCollection.clear();
    local2dbApplication.clear();
    local2dbDataset.clear();
    local2dbMachine.clear();
    local2dbTrial.clear();
    local2dbExecution.clear();
    local2dbMetric.clear();
  }

}; // end FakeEigerLoader
}

int main(int argc, char **argv){
  if(argc == 1){
    std::cerr << "Error: Must provide file names to parse. Exiting..." << std::endl;
    return -1;
  }
  std::vector<std::string> names(argv+1, argv+argc);
  eiger::FakeEigerLoader f;
  f.parse(names);

  return 0;
}

