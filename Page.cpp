#include "Page.h"
#include "Record.h"

Page::Page(int i, int s, int pageSize):idx(i), MAX_RECORDS(s), size(pageSize) {}


 
void Page::addRecord(Record * record) {
    records.push_back(record);
    bytes+=record->getSize();
}

Page* Page::clone(){
    Page* clonedPage = new Page(idx, MAX_RECORDS, size);
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
    bytes-=records.front()->getSize();
    records.pop_front();
}

/**Return a deep copy of the first Record in Page*/
Record* Page::getFirstRecord() {
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
    bytes = 0;
    size = 0;
    prev = nullptr;
    next = nullptr;
    idx = 0;
}

void Page::print(bool listRecord) const {
    printf("Page %d : has %lu records, total bytes %d \n", idx, records.size(), bytes);
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

bool Page::isFull() const {
    return records.size() == MAX_RECORDS;
}

Page* Page::getPrev() const { return prev; }
Page* Page::getNext() const { return next; }
int Page::getIdx() const {return idx;};
int Page::getBytes() const {return bytes;};





// Setters
void Page::setPrev(Page* prevPage) { prev = prevPage; }
void Page::setNext(Page* nextPage) { next = nextPage; }
void Page::setIdx(int newIdx) { idx = newIdx;};
void Page::setBytes(int newBytes) { bytes = newBytes;};



