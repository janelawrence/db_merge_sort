#include "Run.h"


Run::Run(std::vector<Record*> res): records(res){};

std::vector<Record*> Run::getRecords() const {
    return records;
};
