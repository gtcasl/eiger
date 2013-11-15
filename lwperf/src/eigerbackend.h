#ifndef eigerbackend_h
#define eigerbackend_h

#include <vector>

#include "eiger.h"

#include "datakind.h"

class EigerBackend{
  private:
  eiger::Application app_;
  eiger::Machine machine_;
	std::string sitename_;
	std::vector<eiger::Metric> erow_;
	eiger::DataCollection dc_;


  public:
    void writeheaders(const std::vector<std::pair<std::string, enum datakind> >& headers) {}
    void nextrow(const std::vector<std::pair<std::string, enum datakind> >& headers,
                 const std::vector<double>& row);

    void addCol(const std::string& label, const enum datakind k);

    EigerBackend(std::string sitename, std::string machine, bool append=false);
};

#endif
