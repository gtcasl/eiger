#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <iterator>

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
                   const vector<Metric>& metrics,
                   const vector<NondeterministicMetric>& nondet_metrics,
                   const vector<DeterministicMetric>& det_metrics,
                   const vector<MachineMetric>& machine_metrics){
  char* tmpname = strdup("fakeeiger.log.XXXXXX");
  if(mkstemp(tmpname) == -1){
    throw "Unable to open unique output file";
  }
  std::fstream fake_log;
  fake_log.open(tmpname,std::fstream::out|std::fstream::trunc); 
  fake_log.precision(18);
  fake_log << FEVERSION<<";2\n";
  fake_log << FEFORMAT << ";" KWFORMAT "\n";
  fake_log << FECONNECT << ";" << dbloc << ";" << dbname << ";" <<user << ";" 
            << passwd << "\n";

  std::copy(datacollections.begin(), datacollections.end(),
            std::ostream_iterator<DataCollection>(fake_log, "\n"));
  std::copy(applications.begin(), applications.end(),
            std::ostream_iterator<Application>(fake_log, "\n"));
  std::copy(datasets.begin(), datasets.end(),
            std::ostream_iterator<Dataset>(fake_log, "\n"));
  std::copy(machines.begin(), machines.end(),
            std::ostream_iterator<Machine>(fake_log, "\n"));
  std::copy(trials.begin(), trials.end(),
            std::ostream_iterator<Trial>(fake_log, "\n"));
  std::copy(metrics.begin(), metrics.end(),
            std::ostream_iterator<Metric>(fake_log, "\n"));
  std::copy(nondet_metrics.begin(), nondet_metrics.end(),
            std::ostream_iterator<NondeterministicMetric>(fake_log, "\n"));
  std::copy(det_metrics.begin(), det_metrics.end(),
            std::ostream_iterator<DeterministicMetric>(fake_log, "\n"));
  std::copy(machine_metrics.begin(), machine_metrics.end(),
            std::ostream_iterator<MachineMetric>(fake_log, "\n"));
  
  fake_log << FEDISCONNECT << "\n";
  fake_log.close();
}

} // namespace eiger

