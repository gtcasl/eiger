#include <vector>
#include <cassert>

#include "csvbackend.h"

enum datakind;

CSVBackend::CSVBackend(std::string filename, std::string machine, bool append) 
  : filename_(filename) 
{
    f_ = fopen(filename_.c_str(),(append?"a+":"w+"));
    if(!f_){
      throw "csvbackend: cannot open file";
    }
}

CSVBackend::~CSVBackend(){
  if(f_) fclose(f_); 
  f_=0; 
}

void CSVBackend::writeheaders(const std::vector<std::pair<std::string, enum datakind> >& headers) {
  fprintf(f_,"\"%s\"\n",filename_.c_str());
  for(std::vector<std::pair<std::string, enum datakind> >::const_iterator it = headers.begin();
      it != headers.end(); ++it){
    fprintf(f_,"\"%s\",",it->first.c_str());
  }
  fprintf(f_,"\n");
}

void CSVBackend::nextrow(const std::vector<std::pair<std::string, enum datakind> >& headers,
                         const std::vector<double>& row){
  std::vector<std::pair<std::string, enum datakind> >::size_type len = headers.size();
  assert(row.size() == len && 0 != "csvbackend::nextrow called with incomplete data");
  for(std::vector<double>::const_iterator it = row.begin();
      it != row.end(); ++it){
    fprintf(f_, "%.18g",*it);
    fprintf(f_,", ");
  }
  fprintf(f_,"\n");
}

