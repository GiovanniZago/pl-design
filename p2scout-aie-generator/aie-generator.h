#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <fstream>

namespace aie {

struct BxData {
    unsigned int bx;
    const uint64_t *header_ptr;
    const uint64_t *data_ptr;
    size_t data_size;

    // comparison operator for priority queue
    bool operator>(const BxData &other) const { return bx > other.bx; }
};

using mem_t = std::vector<uint64_t>;
using MinHeap = std::priority_queue<BxData, std::vector<BxData>, std::greater<>>;

class AIEGenerator {
    public:
        AIEGenerator(const std::string &fname) : fname_(std::move(fname)) {};

        inline const mem_t& orbit_view() {
            mem_t h_data;
            mem_t p_data;

            h_data.clear();
            p_data.clear();

            MinHeap min_heap;

            // read raw data inside char vector
            std::vector<unsigned char> buffer = read_raw_data(fname_);

            // get begin and end position pointers of the buffer
            const auto buffer_begin = reinterpret_cast<const uint64_t *>(buffer.data());
            const auto buffer_end = reinterpret_cast<const uint64_t *>(buffer.data() + buffer.size());

            for (auto ptr = buffer_begin; ptr < buffer_end;) {
                if (*ptr == 0) {
                    ++ptr;
                    continue;
                }
                
                // the first 64-bit word pointed by ptr should be an header
                // first, unpack the bx number, then unpack the event size
                unsigned int bx = ((*ptr) >> 12) & 0xFFF;  
                auto event_size = (*ptr) & 0xFFF; 
                ++ptr;
                
                // calculate how many bytes are left from the current position to 
                const size_t space_left = buffer_end - ptr;

                // get the min between space_left and event_size, in order not to overshoot the actual file size
                const size_t effective_size = std::min<size_t>(event_size, space_left);

                // push the struct inside the min heap
                min_heap.push({bx, ptr - 1, ptr, effective_size});

                // update the position of the pointer
                ptr += effective_size;
            }

            while (!min_heap.empty()) {
                const auto &bx_data = min_heap.top();
                h_data.push_back(*(bx_data.header_ptr));                                               // store header
                p_data.insert(p_data.end(), bx_data.data_ptr, bx_data.data_ptr + bx_data.data_size);  // copy payload
                min_heap.pop();
            }

            // move the content of the two vectors inside a mem_ vector
            mem_.clear();
            mem_.reserve(h_data.size() + p_data.size());
            mem_.insert(mem_.end(), h_data.begin(), h_data.end());
            mem_.insert(mem_.end(), p_data.begin(), p_data.end());

            // return 
            return mem_;
        }   

        inline const mem_t& get_orbit_view() const noexcept { return mem_; }

    private:
        // file name
        std::string fname_;
        
        // vector with data inside
        mem_t mem_;

        std::vector<unsigned char> read_raw_data(const std::string& fname) {
            std::ifstream file(fname, std::ios::binary | std::ios::ate); // set position to the end
            if (!file) {
                throw std::runtime_error("Failed to open file: " + fname);
            }
            
            // get size of the file (in bytes)
            std::streamsize size = file.tellg();
            if (size < 0) {
                throw std::runtime_error("Failed to determine file size: " + fname);
            }
            
            // align position to the start
            file.seekg(0, std::ios::beg);

            // allocate vector and read content
            std::vector<unsigned char> buffer(size);
            
            if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
                throw std::runtime_error("Failed to read the file: " + fname);
            }
            
            return buffer;
        };
};

} // namespace aie