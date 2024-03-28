#ifndef RUN_H
#define RUN_H
#include "Record.h"
#include <vector>


class Run {
    private:
        std::vector<Record*> records;
    public:
        Run(std::vector<Record*> res);

        void printRun(bool listRecord = true);
        // Getters
        std::vector<Record*> getRecords() const;
};


#endif //RUN_H

