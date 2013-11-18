#include <string>
#include <vector>
// MySQL++ includes
#include <mysql++.h>
#include <ssqls.h>

#include "eiger.h"

using std::string;
using std::vector;

sql_create_2(datacollections, 2, 0, 
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
sql_create_2(machines, 2, 0, 
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_text, description);
sql_create_3(machine_metrics, 3, 0, 
             mysqlpp::sql_int, machineID,
             mysqlpp::sql_int, metricID,
             mysqlpp::sql_double, metric);
sql_create_2(applications, 2, 0,
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_text, description);
sql_create_4(datasets, 4, 0,
             mysqlpp::sql_int, applicationID,
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_text, description,
             mysqlpp::sql_text, url);
sql_create_3(metrics, 3, 0,
             mysqlpp::sql_enum, type,
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_text, description);
sql_create_3(nondeterministic_metrics, 3, 0,
             mysqlpp::sql_int, executionID,
             mysqlpp::sql_int, metricID,
             mysqlpp::sql_double, metric);
sql_create_3(deterministic_metrics, 3, 0,
             mysqlpp::sql_int, datasetID,
             mysqlpp::sql_int, metricID,
             mysqlpp::sql_double, metric);

namespace eiger{

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

void do_disconnect(const string& dbloc, const string& dbname, 
                   const string& user, const string& passwd,
                   const vector<DataCollection>& datacollections_e,
                   const vector<Application>& applications_e,
                   const vector<Dataset>& datasets_e,
                   const vector<Machine>& machines_e,
                   const vector<Trial>& trials_e,
                   const vector<Execution>& executions_e,
                   const vector<Metric>& metrics_e,
                   const vector<NondeterministicMetric>& nondet_metrics_e,
                   const vector<DeterministicMetric>& det_metrics_e,
                   const vector<MachineMetric>& machine_metrics_e){
#define MAKEROWVEC(X) vector<X> X##_rows;
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
  for(vector<DataCollection>::const_iterator it = datacollections_e.begin();
      it != datacollections_e.end(); ++it){
    datacollections_rows.push_back(datacollections(it->name, it->description));
  }
  for(vector<Application>::const_iterator it = applications_e.begin();
      it != applications_e.end(); ++it){
    applications_rows.push_back(applications(it->name, it->description));
  }
  for(vector<Dataset>::const_iterator it = datasets_e.begin();
      it != datasets_e.end(); ++it){
    datasets_rows.push_back(datasets(it->applicationID, it->name, 
                                     it->description, it->url));
  }
  for(vector<Machine>::const_iterator it = machines_e.begin();
      it != machines_e.end(); ++it){
    machines_rows.push_back(machines(it->name, it->description));
  }
  for(vector<Trial>::const_iterator it = trials_e.begin();
      it != trials_e.end(); ++it){
    trials_rows.push_back(trials(it->dataCollectionID, it->machineID, 
                                 it->applicationID, it->datasetID));
  }
  for(vector<Execution>::const_iterator it = executions_e.begin();
      it != executions_e.end(); ++it){
    executions_rows.push_back(executions(it->machineID, it->trialID));
  }
  for(vector<Metric>::const_iterator it = metrics_e.begin();
      it != metrics_e.end(); ++it){
		std::string type_name;
		switch(it->type){
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
    metrics_rows.push_back(metrics(type_name, it->name, it->description));
  }
  for(vector<NondeterministicMetric>::const_iterator it = nondet_metrics_e.begin();
      it != nondet_metrics_e.end(); ++it){
    nondeterministic_metrics_rows.push_back(nondeterministic_metrics(it->executionID, 
                                                                     it->metricID, 
                                                                     it->value));
  }
  for(vector<DeterministicMetric>::const_iterator it = det_metrics_e.begin();
      it != det_metrics_e.end(); ++it){
    deterministic_metrics_rows.push_back(deterministic_metrics(it->datasetID, 
                                                               it->metricID, 
                                                               it->value));
  }
  for(vector<MachineMetric>::const_iterator it = machine_metrics_e.begin();
      it != machine_metrics_e.end(); ++it){
    machine_metrics_rows.push_back(machine_metrics(it->machineID, it->metricID, 
                                                   it->value));
  }

  try{
    mysqlpp::Connection conn(true);
    conn.connect(dbname.c_str(), dbloc.c_str(), user.c_str(), passwd.c_str());
    mysqlpp::Query query = conn.query();

    vector<int> dc_ids = insertAndIDByName(datacollections_rows.begin(), 
                                           datacollections_rows.end(), query);
    vector<int> machine_ids = insertAndIDByName(machines_rows.begin(),
                                                machines_rows.end(), query);
    vector<int> app_ids = insertAndIDByName(applications_rows.begin(), 
                                            applications_rows.end(), query);
    vector<int> metric_ids = insertAndIDByName(metrics_rows.begin(), 
                                               metrics_rows.end(), query);
    
    for(vector<datasets>::iterator it = datasets_rows.begin(); 
        it != datasets_rows.end(); ++it){
      it->applicationID = app_ids[it->applicationID];
    }
    vector<int> dataset_ids = insertAndIDByName(datasets_rows.begin(), 
                                                datasets_rows.end(), query);

    for(vector<machine_metrics>::iterator it = machine_metrics_rows.begin(); 
        it != machine_metrics_rows.end(); ++it){
      it->machineID = machine_ids[it->machineID];
      it->metricID = metric_ids[it->metricID];
    }
    insertAll(machine_metrics_rows.begin(), machine_metrics_rows.end(), query, 
              false);

    for(vector<trials>::iterator it = trials_rows.begin(); 
        it != trials_rows.end(); ++it){
      it->dataCollectionID = dc_ids[it->dataCollectionID];
      it->machineID = machine_ids[it->machineID];
      it->applicationID = app_ids[it->applicationID];
      it->datasetID = dataset_ids[it->datasetID];
    }
    vector<int> trial_ids = insertAndIDByInsertID(trials_rows.begin(), 
                                                  trials_rows.end(), query);
    
    for(vector<executions>::iterator it = executions_rows.begin(); 
        it != executions_rows.end(); ++it){
      it->machineID = machine_ids[it->machineID];
      it->trialID = trial_ids[it->trialID];
    }
    vector<int> execution_ids = insertAndIDByInsertID(executions_rows.begin(), 
                                                      executions_rows.end(), 
                                                      query);

    for(vector<nondeterministic_metrics>::iterator it = nondeterministic_metrics_rows.begin(); 
        it != nondeterministic_metrics_rows.end(); ++it){
      it->executionID = execution_ids[it->executionID];
      it->metricID = metric_ids[it->metricID];
    }
    insertAll(nondeterministic_metrics_rows.begin(), 
              nondeterministic_metrics_rows.end(), query, false);

    for(vector<deterministic_metrics>::iterator it = deterministic_metrics_rows.begin(); 
        it != deterministic_metrics_rows.end(); ++it){
      it->datasetID = dataset_ids[it->datasetID];
      it->metricID = metric_ids[it->metricID];
    }
    insertAll(deterministic_metrics_rows.begin(), 
              deterministic_metrics_rows.end(), query, false);
  }
  catch (const mysqlpp::Exception& er){
    std::cerr << "MySQL Backend Error: " << er.what() << std::endl;
  }
}

} // namespace eiger

