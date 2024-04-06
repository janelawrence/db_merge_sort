#include "Page.h"
#include "Record.h"

Page::Page(int i, int s):idx(i), MAX_RECORDS(s){}


 
void Page::addRecord(Record * record) {
    records.push_back(record);
}

Page* Page::clone(){
    Page* clonedPage = new Page(idx, size);
    for (std::list<Record*>::iterator it = records.begin(); it != records.end(); ++it) {
        Record * r = (*it);
        clonedPage->addRecord(new Record(*r));
    }
    return clonedPage;
}

void Page::removeFisrtRecord() {
    if(records.empty()) {
        printf("Can't remove, Page is empty");
        return;
    }
    records.pop_front();
}

/**Return a deep copy of the first Record in Page*/
Record* Page::getFirst() {
    Record* firstRecord;
    if(records.empty()) {
        printf("List is empty, can't get fist from the list");
        return firstRecord;
    }
    return new Record(*records.front());
}

void Page::clear(){
    std::list<Record*> newRecords;
    records.swap(newRecords);
}

void Page::print(bool listRecord) const {
    printf("Page %d : has %lu records \n", idx, records.size());
    if(listRecord) {
        for (std::list<Record*>::const_iterator it = records.begin(); it != records.end(); ++it) {
            (*it)->printRecord();
        }
    }
}

//  Getters
std::list<Record*> Page::getRecords() const {
    return records;
}

int Page::getNumRecords() const {
    return records.size();
}

bool Page::isEmpty() const {
    return records.empty();
}

Page* Page::getPrev() const { return prev; }
Page* Page::getNext() const { return next; }


// Setters
void Page::setPrev(Page* prevPage) { prev = prevPage; }
void Page::setNext(Page* nextPage) { next = nextPage; }
