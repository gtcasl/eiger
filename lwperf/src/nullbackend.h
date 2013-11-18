#ifndef nullbackend_h
#define nullbackend_h
#include "datakind.h"

class NullBackend{
  public:
    void writeheaders(const std::vector<std::pair<std::string, enum datakind> >& headers) {}
    void nextrow(const std::vector<std::pair<std::string, enum datakind> >& headers,
                 const std::vector<double>& row) {}

    void addCol(const std::string& label, const enum datakind k) {}

    NullBackend(std::string sitename, std::string machine, bool append=false) {}
};

#endif
