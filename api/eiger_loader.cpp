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
// C++ string includes
#include <string>
// STL includes
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

#include "fakekeywords.h" 
#include "eiger.h"

static const int version = 2;

enum dispatch {
  Metric,
  Metric_commit,
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
  MachineMetric,
  MachineMetric_commit,
  DeterministicMetric,
  DeterministicMetric_commit,
  NondeterministicMetric,
  NondeterministicMetric_commit,
  CONNECT,
  LVERSION,
  LFORMAT,
  DISCONNECT
};

std::map<std::string,dispatch> domap;

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
  std::stringstream sstream{s};
  std::string elem;
  while(std::getline(sstream, elem, ';')){
    v.push_back(elem);
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
    eiger::Connect(v[1]);
    break;
  case DISCONNECT:
    eiger::Disconnect();
    break;
  case LVERSION:
    {
      int filever = toInt(v[1]);
      if (filever != version) {
        std::cout << "VERSION: " << version <<" != " << filever <<"\n";
        throw "fakeeiger log file version mismatch.";
      }
    }
    break;
  case LFORMAT:
    {
      if (v[1] != KWFORMAT) {
        std::cout << FEFORMAT ": " << v[1] <<" does not match compiled in format " KWFORMAT "\n";
        throw "fakeeiger log file format mismatch.";
      }
    }
    break;
  /* this batch don't really need to do anything yet. ctors. */
  case DataCollection:
  case Application:
  case Dataset:
  case Machine:
  case Trial:
  case MachineMetric:
  case DeterministicMetric:
  case NondeterministicMetric:
  case Metric:
    break;
  /* these do something */
  case DataCollection_commit:
    eiger::DataCollection(v[1], v[2]).commit();
    break;
  case Application_commit:
    eiger::Application(v[1], v[2]).commit();
    break;
  case Dataset_commit:
    {
      eiger::ApplicationID ai(toInt(v[1]),0);
      eiger::Dataset(ai, v[2], v[3], v[4]).commit();
    }
    break;
  case Machine_commit:
    eiger::Machine(v[1], v[2]).commit();
    break;
  case Trial_commit:
    {
      eiger::DataCollectionID dci(toInt(v[1]),0);
      eiger::MachineID mi(toInt(v[2]),0);
      eiger::ApplicationID ai(toInt(v[3]),0);
      eiger::DatasetID dsi(toInt(v[4]),0);
      eiger::Trial(dci,mi,ai,dsi).commit();
    }
    break;
  case MachineMetric_commit:
    {
      eiger::MachineID mai(toInt(v[1]),0);
      eiger::MetricID mi(toInt(v[2]),0);
      eiger::MachineMetric(mai,mi,toDouble(v[3])).commit();
    }
    break;
  case DeterministicMetric_commit:
    {
      eiger::DatasetID dsi(toInt(v[1]),0);
      eiger::MetricID mi(toInt(v[2]),0);
      eiger::DeterministicMetric(dsi,mi,toDouble(v[3])).commit();
    }
    break;
  case NondeterministicMetric_commit:
    {
      eiger::TrialID ti(toInt(v[1]),0);
      eiger::MetricID mi(toInt(v[2]),0);
      eiger::NondeterministicMetric(ti,mi,toDouble(v[3])).commit();
    }
    break;
  case Metric_commit:
    eiger::metric_type_t type;
    if(v[1].compare("deterministic") == 0) type = eiger::DETERMINISTIC;
    if(v[1].compare("nondeterministic") == 0) type = eiger::NONDETERMINISTIC;
    if(v[1].compare("machine") == 0) type = eiger::MACHINE;
    eiger::Metric(type,v[2],v[3]).commit();
    break;
  default:
    throw "unexpected enum value in one() handling";
  } // end switch
} // end one()

void parse(std::vector<std::string> filenames) {
  std::string line;
  std::vector< std::string>::size_type nf = filenames.size();
  std::cout << "Parsing " << nf << " logs" << std::endl;
  for (std::vector< std::string>::size_type i = 0; i < nf; i++) {
    std::ifstream myfile (filenames[i].c_str());
    if (myfile.is_open())
    {
      std::cout << "parsing " << filenames[i] <<"\n";
      while ( myfile.good() )
      {
        getline (myfile,line);
        one(line);
      }
      myfile.close();
    }
  }
}

// set up a hash map to convert switching over strings into switching on enum.
void initmaps() {
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
  domap[MACHINEMETRIC_COMMIT]=MachineMetric_commit;
  domap[MACHINEMETRIC]=MachineMetric;
  domap[DETERMINISTICMETRIC_COMMIT]=DeterministicMetric_commit;
  domap[DETERMINISTICMETRIC]=DeterministicMetric;
  domap[NONDETERMINISTICMETRIC_COMMIT]=NondeterministicMetric_commit;
  domap[NONDETERMINISTICMETRIC]=NondeterministicMetric;
  domap[METRIC_COMMIT]=Metric_commit;
  domap[FEMETRIC]=Metric;
  domap[FECONNECT]=CONNECT;
  domap[FEDISCONNECT]=DISCONNECT;
  domap[FEVERSION]=LVERSION;
  domap[FEFORMAT]=LFORMAT;
}

int main(int argc, char **argv){
  if(argc == 1){
    std::cerr << "Error: Must provide file names to parse. Exiting..." << std::endl;
    return -1;
  }
  std::vector<std::string> names(argv+1, argv+argc);
  std::cout << "Initializing" << std::endl;
  // log file object/step name string to enum for switching
  initmaps();
  parse(names);
  return 0;
}

