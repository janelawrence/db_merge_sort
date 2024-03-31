#include "Run.h"


Run::Run(){}



void Run::add(Record * record) {
    records.push_back(record);
}

Run* Run::clone(){
    Run* clonedRun = new Run();
    for (std::list<Record*>::iterator it = records.begin(); it != records.end(); ++it) {
        Record * r = (*it);
        clonedRun->add(new Record(*r));
    }
    return clonedRun;
}

void Run::removeFisrt() {
    if(records.size() == 0) {
        printf("Can't remove, run is empty");
        return;
    }
    records.pop_front();
}

void Run::clear(){
    std::list<Record*> newRecords;
    records.swap(newRecords);
}

void Run::print(bool listRecord) const {
    printf("Run: has %lu records \n", records.size());
    if(listRecord) {
        for (std::list<Record*>::const_iterator it = records.begin(); it != records.end(); ++it) {
            (*it)->printRecord();
        }
    }
}

std::list<Record*> Run::getRecords() const {
    return records;
}

int Run::getSize() const {
    return records.size();
}

bool Run::isEmpty() const {
    return records.size() == 0;
}

