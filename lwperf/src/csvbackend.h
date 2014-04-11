#ifndef csvbackend_h
#define csvbackend_h

#include <string>
#include <cstdio>
#include <vector>

#include "datakind.h"

class CSVBackend{
  private:
    std::string filename_;
    FILE *f_;

  public:
    void writeheaders(const std::vector<std::pair<std::string, enum datakind> >& headers);
    void ckfile();
    void nextrow(const std::vector<std::pair<std::string, enum datakind> >& headers,
                 const std::vector<double>& row);
    void addCol(const std::string& label, const enum datakind kind) {}

    CSVBackend(std::string filename, std::string machine, std::string application, bool append=false);
    ~CSVBackend();
};

#endif
