#include <sstream>
#include <map>
#include <cassert>
#include <eiger.h>

#include "datakind.h"
#include "eigerbackend.h"

EigerBackend::EigerBackend(std::string sitename, std::string machine, bool append)
  : sitename_(sitename), dc_(sitename, sitename), app_(sitename, sitename), machine_(machine, machine)
{
		dc_.commit();
    app_.commit();
    machine_.commit();
}

void EigerBackend::addCol(const std::string& label, const enum datakind kind){
  eiger::metric_type_t etype;
  std::string slabel = sitename_ + "_"+label;
  switch(kind){
    case RESULT:
      etype = eiger::RESULT;
      break;
    case DETERMINISTIC:
      etype = eiger::DETERMINISTIC;
			break;
    case NONDETERMINISTIC:
      etype = eiger::NONDETERMINISTIC;
			break;
		default:
			throw "eigerbackend: addCol unhandled datakind";
  }
  eiger::Metric metric(etype, slabel, slabel);
  metric.commit();
  erow_.push_back(metric);
}

void EigerBackend::nextrow(const std::map<std::string, enum datakind>& headers,
                           const std::vector<double>& row){
  // declare a trial, exec, etc and do all commits, then 
  // do text since text handles row.clear().
  std::ostringstream dsbuf; 
  std::ostringstream ddbuf; 
  dsbuf << sitename_;
  ddbuf << sitename_;
  std::map<std::string,enum datakind>::size_type len = headers.size();
  assert(row.size() == len && 0 != "eigerbackend::nextrow called with incomplete data");
  int i = 0;
  for(std::map<std::string,enum datakind>::const_iterator it = headers.begin();
      it != headers.end(); ++it, ++i){
    dsbuf << "_" << row.at(i);
    ddbuf << " " << it->first <<  "=" << row.at(i);
  } 
  std::string dsname = dsbuf.str();
  std::string dsdesc = ddbuf.str();

  eiger::ApplicationID aid = app_.getID();
  eiger::Dataset ds(aid, dsname, dsdesc, "nURL");
  ds.commit();
  eiger::DatasetID dsid = ds.getID();

  eiger::MachineID machid = machine_.getID();
  eiger::Trial trial(dc_.getID(), machid, aid, dsid, eiger::PropertiesID(0,0));
  trial.commit();
  eiger::Execution exec(trial.getID(),machid);
  exec.commit();
  eiger::ExecutionID eid = exec.getID();
  int j = 0;
  for(std::map<std::string,enum datakind>::const_iterator it = headers.begin();
      it != headers.end(); ++it, ++j){
    eiger::MetricID mid = (erow_.at(j)).getID();
    switch(it->second){
    case DETERMINISTIC: {
      eiger::DeterministicMetric dmD(dsid, mid, row.at(j));
      dmD.commit();
      break;
      }
    case NONDETERMINISTIC: {
      eiger::NondeterministicMetric ndmD(eid, mid, row.at(j));
      ndmD.commit();
      break;
      }
    case RESULT: {
      eiger::NondeterministicMetric rdmD(eid, mid, row.at(j));
      rdmD.commit();
      break;
      }
    default:
      throw "unhandled htype in nextrow";
    } 
  }
}

