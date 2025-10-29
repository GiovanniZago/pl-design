#include <iostream>
#include <vector>
#include <queue>

struct BxData {
    unsigned int bx;
    const uint64_t *header_ptr;
    const uint64_t *data_ptr;
    size_t data_size;

    // comparison operator for priority queue
    bool operator>(const BxData &other) const { return bx > other.bx; }
};

using MinHeap = std::priority_queue<BxData, std::vector<BxData>, std::greater<>>;

class 

