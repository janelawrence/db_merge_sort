#include "Run.h"


Run::Run(std::vector<Record*> res): records(res){}

std::vector<Record*> Run::getRecords() const {
    return records;
}


void Run::printRun(bool listRecord){
    printf("Run: has %d records \n", records.size());
    if(listRecord) {
        for(int i = 0; i < records.size(); i++) {
            records[i]->printRecord();
        }
    }
}
