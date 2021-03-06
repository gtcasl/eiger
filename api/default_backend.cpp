#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include "sqlite3.h"

#include "eiger.h"

using namespace std;

namespace eiger{

int db_check_callback(void* is_db_already, int num_cols, char** col_text, char** col_name){
  if(num_cols > 0){
    if(stoi(col_text[0]) != 0){
      *(bool*)is_db_already = true;
    }
  }

  return 0;
}

void do_disconnect(const string& dbname, 
                   const vector<DataCollection>& datacollections_e,
                   const vector<Application>& applications_e,
                   const vector<Dataset>& datasets_e,
                   const vector<Machine>& machines_e,
                   const vector<Trial>& trials_e,
                   const vector<Metric>& metrics_e,
                   const vector<NondeterministicMetric>& nondet_metrics_e,
                   const vector<DeterministicMetric>& det_metrics_e,
                   const vector<MachineMetric>& machine_metrics_e){
  vector<DataCollection> datacollections = datacollections_e;
  vector<Application> applications = applications_e;
  vector<Dataset> datasets = datasets_e;
  vector<Machine> machines = machines_e;
  vector<Trial> trials = trials_e;
  vector<Metric> metrics = metrics_e;
  vector<NondeterministicMetric> nondet_metrics = nondet_metrics_e;
  vector<DeterministicMetric> det_metrics = det_metrics_e;
  vector<MachineMetric> machine_metrics = machine_metrics_e;

  sqlite3* db;
  int err = sqlite3_open(dbname.c_str(), &db);
  if(err != SQLITE_OK){
    cerr << sqlite3_errstr(err) << endl;
    exit(-1);
  }
  bool is_db_already = false;
  err = sqlite3_exec(db, "pragma schema_version;", db_check_callback, (void*)&is_db_already, NULL);
  if(err != SQLITE_OK){
    cerr << sqlite3_errstr(err) << endl;
    exit(-1);
  }

  // Only reload schema if the db file isn't valid
  if(!is_db_already){
    ifstream ifs(SCHEMAFILE);
    stringstream schema;
    schema << ifs.rdbuf();
    err = sqlite3_exec(db, schema.str().c_str(), NULL, NULL, NULL);
    if(err != SQLITE_OK){
      cerr << sqlite3_errstr(err) << endl;
      exit(-1);
    }
  }

  sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);

  vector<int> dc_ids, machine_ids, app_ids, metric_ids, dataset_ids, trial_ids;
  sqlite3_stmt* insert_statement;
  sqlite3_stmt* select_statement;

  sqlite3_prepare_v2(db, 
                     "INSERT OR IGNORE INTO datacollections"
                     "(name, description) VALUES(?,?)", 
                     -1, &insert_statement, NULL);
  sqlite3_prepare_v2(db,
                     "SELECT ID FROM datacollections WHERE name=?",
                     -1, &select_statement, NULL);
  for(const auto& dc : datacollections){
    sqlite3_bind_text(insert_statement, 1, dc.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_statement, 2, dc.description.c_str(), -1, 
                      SQLITE_STATIC);
    sqlite3_step(insert_statement);
    sqlite3_reset(insert_statement);

    sqlite3_bind_text(select_statement, 1, dc.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(select_statement);
    dc_ids.push_back(sqlite3_column_int(select_statement, 0));
    sqlite3_reset(select_statement);
  }
  sqlite3_finalize(insert_statement);
  sqlite3_finalize(select_statement);

  sqlite3_prepare_v2(db, 
                     "INSERT OR IGNORE INTO machines"
                     "(name, description) VALUES(?,?)", 
                     -1, &insert_statement, NULL);
  sqlite3_prepare_v2(db,
                     "SELECT ID FROM machines WHERE name=?",
                     -1, &select_statement, NULL);
  for(const auto& ma : machines){
    sqlite3_bind_text(insert_statement, 1, ma.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_statement, 2, ma.description.c_str(), -1, 
                      SQLITE_STATIC);
    sqlite3_step(insert_statement);
    sqlite3_reset(insert_statement);

    sqlite3_bind_text(select_statement, 1, ma.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(select_statement);
    machine_ids.push_back(sqlite3_column_int(select_statement, 0));
    sqlite3_reset(select_statement);
  }
  sqlite3_finalize(insert_statement);
  sqlite3_finalize(select_statement);

  sqlite3_prepare_v2(db, 
                     "INSERT OR IGNORE INTO applications"
                     "(name, description) VALUES(?,?)", 
                     -1, &insert_statement, NULL);
  sqlite3_prepare_v2(db,
                     "SELECT ID FROM applications WHERE name=?",
                     -1, &select_statement, NULL);
  for(const auto& ap : applications){
    sqlite3_bind_text(insert_statement, 1, ap.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_statement, 2, ap.description.c_str(), -1, 
                      SQLITE_STATIC);
    sqlite3_step(insert_statement);
    sqlite3_reset(insert_statement);

    sqlite3_bind_text(select_statement, 1, ap.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(select_statement);
    app_ids.push_back(sqlite3_column_int(select_statement, 0));
    sqlite3_reset(select_statement);
  }
  sqlite3_finalize(insert_statement);
  sqlite3_finalize(select_statement);

  sqlite3_prepare_v2(db, 
                     "INSERT OR IGNORE INTO metrics"
                     "(type, name, description) VALUES(?,?,?)", 
                     -1, &insert_statement, NULL);
  sqlite3_prepare_v2(db,
                     "SELECT ID FROM metrics WHERE name=?",
                     -1, &select_statement, NULL);
  for(const auto& me : metrics){
    switch(me.type){
      case DETERMINISTIC:
        sqlite3_bind_text(insert_statement, 1, "deterministic", -1, SQLITE_STATIC);
        break;
      case NONDETERMINISTIC:
        sqlite3_bind_text(insert_statement, 1, "nondeterministic", -1, SQLITE_STATIC);
        break;
      case MACHINE:
        sqlite3_bind_text(insert_statement, 1, "machine", -1, SQLITE_STATIC);
        break;
      default:
        throw "BAAAD metric type";
    }
    sqlite3_bind_text(insert_statement, 2, me.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_statement, 3, me.description.c_str(), -1, 
                      SQLITE_STATIC);
    sqlite3_step(insert_statement);
    sqlite3_reset(insert_statement);

    sqlite3_bind_text(select_statement, 1, me.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(select_statement);
    metric_ids.push_back(sqlite3_column_int(select_statement, 0));
    sqlite3_reset(select_statement);
  }
  sqlite3_finalize(insert_statement);
  sqlite3_finalize(select_statement);

  for(auto& dset : datasets){
    dset.applicationID = app_ids[dset.applicationID];
  }
  sqlite3_prepare_v2(db, 
                     "INSERT OR IGNORE INTO datasets"
                     "(applicationID, name, description, created, url) "
                     "VALUES(?,?,?,?,?)", 
                     -1, &insert_statement, NULL);
  sqlite3_prepare_v2(db,
                     "SELECT ID FROM datasets WHERE name=?",
                     -1, &select_statement, NULL);
  for(const auto& ds : datasets){
    sqlite3_bind_int(insert_statement, 1, ds.applicationID);
    sqlite3_bind_text(insert_statement, 2, ds.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_statement, 3, ds.description.c_str(), -1, 
                      SQLITE_STATIC);
    sqlite3_bind_text(insert_statement, 4, ds.created.c_str(), -1, 
                      SQLITE_STATIC);
    sqlite3_bind_text(insert_statement, 5, ds.url.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(insert_statement);
    sqlite3_reset(insert_statement);

    sqlite3_bind_text(select_statement, 1, ds.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(select_statement);
    dataset_ids.push_back(sqlite3_column_int(select_statement, 0));
    sqlite3_reset(select_statement);
  }
  sqlite3_finalize(insert_statement);
  sqlite3_finalize(select_statement);

  for(auto& mach_met : machine_metrics){
    mach_met.machineID = machine_ids[mach_met.machineID];
    mach_met.metricID = metric_ids[mach_met.metricID];
  }
  sqlite3_prepare_v2(db, 
                     "INSERT OR IGNORE INTO machine_metrics"
                     "(machineID, metricID, metric) "
                     "VALUES(?,?,?)", 
                     -1, &insert_statement, NULL);
  for(const auto& mmet : machine_metrics){
    sqlite3_bind_int(insert_statement, 1, mmet.machineID);
    sqlite3_bind_int(insert_statement, 2, mmet.metricID);
    sqlite3_bind_double(insert_statement, 3, mmet.value);
    sqlite3_step(insert_statement);
    sqlite3_reset(insert_statement);
  }
  sqlite3_finalize(insert_statement);

  for(auto& trial : trials){
    trial.dataCollectionID = dc_ids[trial.dataCollectionID];
    trial.machineID = machine_ids[trial.machineID];
    trial.applicationID = app_ids[trial.applicationID];
    trial.datasetID = dataset_ids[trial.datasetID];
  }
  sqlite3_prepare_v2(db, 
                     "INSERT OR IGNORE INTO trials"
                     "(dataCollectionID, machineID, applicationID, datasetID) "
                     "VALUES(?,?,?,?)", 
                     -1, &insert_statement, NULL);
  for(const auto& trial : trials){
    sqlite3_bind_int(insert_statement, 1, trial.dataCollectionID);
    sqlite3_bind_int(insert_statement, 2, trial.machineID);
    sqlite3_bind_int(insert_statement, 3, trial.applicationID);
    sqlite3_bind_int(insert_statement, 4, trial.datasetID);
    sqlite3_step(insert_statement);
    sqlite3_reset(insert_statement);

    trial_ids.push_back(sqlite3_last_insert_rowid(db));
  }
  sqlite3_finalize(insert_statement);

  for(auto& ndm : nondet_metrics){
    ndm.trialID = trial_ids[ndm.trialID];
    ndm.metricID = metric_ids[ndm.metricID];
  }
  sqlite3_prepare_v2(db, 
                     "INSERT OR IGNORE INTO nondeterministic_metrics"
                     "(trialID, metricID, metric) "
                     "VALUES(?,?,?)", 
                     -1, &insert_statement, NULL);
  for(const auto& ndmet : nondet_metrics){
    sqlite3_bind_int(insert_statement, 1, ndmet.trialID);
    sqlite3_bind_int(insert_statement, 2, ndmet.metricID);
    sqlite3_bind_double(insert_statement, 3, ndmet.value);
    sqlite3_step(insert_statement);
    sqlite3_reset(insert_statement);
  }
  sqlite3_finalize(insert_statement);

  for(auto& dm : det_metrics){
    dm.datasetID = dataset_ids[dm.datasetID];
    dm.metricID = metric_ids[dm.metricID];
  }
  sqlite3_prepare_v2(db, 
                     "INSERT OR IGNORE INTO deterministic_metrics"
                     "(datasetID, metricID, metric) "
                     "VALUES(?,?,?)", 
                     -1, &insert_statement, NULL);
  for(const auto& dmet : det_metrics){
    sqlite3_bind_int(insert_statement, 1, dmet.datasetID);
    sqlite3_bind_int(insert_statement, 2, dmet.metricID);
    sqlite3_bind_double(insert_statement, 3, dmet.value);
    sqlite3_step(insert_statement);
    sqlite3_reset(insert_statement);
  }
  sqlite3_finalize(insert_statement);

  sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
  sqlite3_close(db);
}

} // namespace eiger

