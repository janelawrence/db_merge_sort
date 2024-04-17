#include <iostream>
#include <vector>
#include "Record.h"
#include "TournamentTree.h"

bool TournamentTree::isGhostNode(int node)
{
    return tree[node] == -1;
}

TournamentTree::TournamentTree(int n)
{
    // Calculate the size of the tree based on the number of contestants
    size = 2;
    while (size < n)
    {
        size *= 2;
    }
    // Resize the tree to accommodate all matches
    tree.resize(2 * size);
    assignGhost();
}

TournamentTree::~TournamentTree()
{
}

void TournamentTree::assignGhost()
{
    for (int i = 0; i < tree.size(); i++)
    {
        // tree[i].winner = std::numeric_limits<int>::max();
        tree[i] = -1;
    }
}

// Function to update the result of a match
void TournamentTree::update(int index, Record *record)
{
    index += size;
    // record->printRecord();
    records.push_back(record);
    TRACE(true);
    int recordIdx = records.size() - 1;
    tree[index] = recordIdx;
    recIdx2TreeIdx.push_back(index);

    // Update parent nodes
    while (index > 1)
    {
        index /= 2;
        // perforam competition upward
        tree[index] = compete(index);
    }
}

void TournamentTree::insert(int index, Record *record)
{
    // - Before looking up treeIdx of this rec
    // delete the mem allo to this record from records vector
    // and then assign the new one to the released mem
    delete records[index];
    records[index] = record;

    index += size;
    TRACE(true);

    // Competing/Update parent nodes
    while (index > 1)
    {
        index /= 2;
        // perforam competition upward
        tree[index] = compete(index);
    }
}

const char *TournamentTree::getRecordKey(int node)
{
    return records[tree[node]]->getKey();
}

// Can be used to empty the tree by inserting ghost record
void TournamentTree::replaceWinner(Record *record)
{
    int winnerRecIndx = tree[1];
    if (winnerRecIndx == GHOST_KEY)
    {
        printf("Tree is empty, inserting to first place\n");
        update(0, record);
    }
    else
    {
        // The rec to be replaced is located at records[winnerRecIndx]
        // TODO: find node idx of winner in tree
        // int startIdx = recIdx2TreeIdx[winnerRecIndx];
        insert(winnerRecIndx, record);
    }
}

void TournamentTree::popWinner()
{
    int winnerRecIndx = tree[1];

    // Insert ghostKey into leaf idx of this winner

    // And start competing such that

    delete records[winnerRecIndx];
}

int TournamentTree::compete(int node)
{
    int left = 2 * node;
    int right = left + 1;
    int winner = node;
    if (right < tree.size() && left < tree.size())
    {
        bool leftIsGhost = isGhostNode(left);
        bool rightIsGhost = isGhostNode(right);
        if (leftIsGhost && !rightIsGhost)
        {
            return tree[right];
        }
        else if (!leftIsGhost && rightIsGhost)
        {
            return tree[left];
        }

        return std::strcmp(getRecordKey(left), getRecordKey(right)) < 0 ? tree[left] : tree[right];
    }
    return -1;
}

int TournamentTree::compareRecordsInNodes(int node1, int node2)
{
    Record *r1 = records[tree[node1]];
    Record *r2 = records[tree[node2]];
    return std::strcmp(r1->getKey(), r2->getKey()) > 0 ? node2 : node1;
}

// Function to get the winner of the tournament
Record *TournamentTree::getWinner() const
{
    return records[tree[1]]; // Root node contains the winner
}

// NOTES: when using it in merge, there are cases
// where the tree is not full but we want to start prunning
bool TournamentTree::isFull() const
{
    return records.size() == size;
}

bool TournamentTree::isEmpty() const
{
    return records.size() == 0;
}

void TournamentTree::printTree() const
{
    for (int i = 0; i < tree.size(); i++)
    {
        int recIdx = tree[i];
        if (recIdx != -1 && tree[recIdx] < records.size())
        {
            if (records[recIdx] == nullptr)
            {
                printf("%d, ", -1);
                continue;
            }
            printf("%s, ", records[recIdx]->getKey());
        }
        else
        {
            printf("%d, ", -1);
        }
    }
    printf("\n");
}

int main()
{
    int n = 6;
    TournamentTree tournament(n);
    // std::vector<Record *> candidates;

    std::vector<Record *> candidates = {new Record(10, "Zasdfweasd"),
                                        new Record(10, "Hasdfweasd"),
                                        new Record(10, "Yasdfweasd"),
                                        new Record(10, "Basdfweasd"),
                                        new Record(10, "Tasdfweasd"),
                                        new Record(10, "Dasdfweasd")};
    Record *rec = new Record(10, "Easdfweasd");

    std::vector<Record *> temp;
    for (int i = 0; i < n; ++i)
    {
        if (candidates[i] == nullptr)
        {
            printf("----------\n");
        }
        // tournament.printTree();
        tournament.update(i, candidates[i]);
        bool isfull = tournament.isFull();
        printf("full? %d\n", static_cast<int>(isfull));
        // tournament.printTree();
    }

    std::vector<Record *> sortedRecords;
    cout << "The winner is: Contestant ";
    tournament.getWinner()->printRecord();

    tournament.replaceWinner(rec);

    cout << "The new winner is: Contestant ";
    tournament.getWinner()->printRecord();
    // tournament.printTree();

    for (int i = 0; i < candidates.size(); i++)
    {
        delete candidates[i];
    }
    return 0;
}
