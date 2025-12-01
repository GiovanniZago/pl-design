#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <iomanip>
#include <limits>
#include <numeric>
#include <assert.h>


#include "puppi-unpackers.h"

namespace aie {

struct BxData {
    unsigned int bx;
    const uint64_t *header_ptr;
    const uint64_t *data_ptr;
    size_t data_size;

    // comparison operator for priority queue
    bool operator>(const BxData &other) const { return bx > other.bx; }
};

struct BxLookup {
    std::vector<uint32_t> bx;
    std::vector<uint32_t> offset;
};

struct CandidatesCollection {
    std::vector<uint32_t> bx;
    std::vector<uint16_t> pt;
    std::vector<int16_t> eta;
    std::vector<int16_t> phi;
    std::vector<uint8_t> pid;
};

using mem_t = std::vector<uint64_t>;
using MinHeap = std::priority_queue<BxData, std::vector<BxData>, std::greater<>>;
static constexpr uint32_t NBX = 3564;

class AIEGenerator {
    public:
        AIEGenerator(const std::string &fname) : fname_(std::move(fname)) {
            mem_t h_data;
            mem_t p_data;

            h_data.clear();
            p_data.clear();

            MinHeap min_heap;

            // read raw data inside char vector
            const auto buffer = read_raw_data(fname_);

            // get begin and end position pointers of the buffer
            const auto buffer_begin = reinterpret_cast<const uint64_t *>(buffer.data());
            const auto buffer_end = reinterpret_cast<const uint64_t *>(buffer.data() + buffer.size());

            // set beginning of association map
            asmap_.offset.push_back(0);  

            for (auto ptr = buffer_begin; ptr < buffer_end;) {
                if (*ptr == 0) {
                    ++ptr;
                    continue;
                }
                
                // the first 64-bit word pointed by ptr should be an header
                // first, unpack the bx number
                unsigned int bx = ((*ptr) >> 12) & 0xFFF;  

                // push back bx index
                asmap_.bx.push_back(bx);
                
                // unpack the event size
                auto event_size = (*ptr) & 0xFFF; 

                // push back previous total size + current event size
                asmap_.offset.push_back(asmap_.offset.back() + event_size);
                
                // update pointer
                ++ptr;
                
                // calculate how many bytes are left from the current position to the end of the whole data block
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
                h_data.push_back(*(bx_data.header_ptr));                                              // store header
                p_data.insert(p_data.end(), bx_data.data_ptr, bx_data.data_ptr + bx_data.data_size);  // copy payload
                min_heap.pop();
            }

            // move the content of the two vectors inside a mem_ vector
            mem_.clear();
            mem_.reserve(h_data.size() + p_data.size());
            mem_.insert(mem_.end(), h_data.begin(), h_data.end());
            mem_.insert(mem_.end(), p_data.begin(), p_data.end());
        }   

        inline const mem_t& get_orbit_view() const noexcept { return mem_; }

        inline const BxLookup& get_association_map() const noexcept { return asmap_; }

        void dump_aiesim_files(const std::string &fout0, 
                                const std::string &fout1, 
                                const uint32_t bx = 0u, 
                                const uint32_t bx_end = std::numeric_limits<uint32_t>::max(),
                                const bool use_tlast = false);

    private:
        // file name
        std::string fname_;
        
        // vector with data inside
        mem_t mem_;

        // association map
        BxLookup asmap_;

        // private helpers
        const std::vector<unsigned char> read_raw_data(const std::string& fname); 
        const CandidatesCollection unpack_events(const uint32_t bx = 0u, 
                                                const uint32_t bx_end = std::numeric_limits<uint32_t>::max());
};

const std::vector<unsigned char> AIEGenerator::read_raw_data(const std::string& fname) {
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

    file.close();
    
    return buffer;
}

const CandidatesCollection AIEGenerator::unpack_events(const uint32_t bx, const uint32_t bx_end) {
    auto ptr = mem_.data();
    ptr += NBX;

    std::vector<uint32_t> bx_range;
    if (bx_end < std::numeric_limits<uint32_t>::max()) {
        bx_range.resize(bx_end - bx);
        if (bx_end > 3563) {
            throw std::runtime_error("bx_end must be in the range [0, 3563]");
        }
        std::iota(bx_range.begin(), bx_range.end(), bx);
    } else {
        bx_range.push_back(bx);
    }

    CandidatesCollection cands;

    for (auto bx_idx : bx_range) {
        cands.bx.push_back(bx_idx);

        auto bx_begin = asmap_.offset[bx_idx];
        auto bx_end = asmap_.offset[bx_idx + 1];
        auto bx_size = bx_end - bx_begin;

        mem_t bx_data{ptr + bx_begin, ptr + bx_end};

        uint16_t pt;
        int16_t eta, phi;
        uint8_t pid;

        for (unsigned int i = 0; i < bx_data.size(); ++i) {
            l1puppiUnpack::readall(bx_data[i], pt, eta, phi, pid);
            cands.pt.push_back(pt);
            cands.eta.push_back(eta);
            cands.phi.push_back(phi);
            cands.pid.push_back(pid);
        }
    }

    return cands;
}

void AIEGenerator::dump_aiesim_files(const std::string &fout0, 
                                        const std::string &fout1, 
                                        const uint32_t bx, 
                                        const uint32_t bx_end, 
                                        const bool use_tlast) {
    std::ofstream out0(fout0, std::ios::out);
    std::ofstream out1(fout1, std::ios::out);

    if ((!out0) | (!out1)) throw std::runtime_error("Error in opening the out files");

    auto ptr = mem_.data(); // get pointer to begin of header block of data
    ptr += NBX; // skip all headers

    std::vector<uint32_t> bx_range;
    if (bx_end < std::numeric_limits<uint32_t>::max()) {
        if (bx_end > 3563) {
            throw std::runtime_error("bx_end must be in the range [0, 3563]");
        }
        bx_range.resize(bx_end - bx);
        std::iota(bx_range.begin(), bx_range.end(), bx);
    } else {
        bx_range.push_back(bx);
    }

    for (auto bx_idx : bx_range) {
        auto bx_begin = asmap_.offset[bx_idx];
        auto bx_end = asmap_.offset[bx_idx + 1];
        auto bx_size = bx_end - bx_begin;

        mem_t bx_data{ptr + bx_begin, ptr + bx_end};

        if ((bx_size % 2) != 0) 
            bx_data.push_back(0u);

        uint16_t pt0, pt1;
        int16_t eta0, eta1, phi0, phi1;
        uint8_t pid0, pid1;

        for (unsigned int i = 0; i < bx_data.size(); i += 2) {
            l1puppiUnpack::readall(bx_data[i], pt0, eta0, phi0, pid0);
            l1puppiUnpack::readall(bx_data[i + 1], pt1, eta1, phi1, pid1);

            out0 << std::right << std::setw(10) << pt0 << std::setw(10) << pt1 << "\n";
            out1 << std::right << std::setw(10) << phi0 << std::setw(10) << phi1 << "\n";
        }

        for (unsigned int i = 0; i < bx_data.size(); i += 2) {
            l1puppiUnpack::readall(bx_data[i], pt0, eta0, phi0, pid0);
            l1puppiUnpack::readall(bx_data[i + 1], pt1, eta1, phi1, pid1);

            out0 << std::right << std::setw(10) << eta0 << std::setw(10) << eta1 << "\n";
            out1 << std::right << std::setw(10) << pid0 << std::setw(10) << pid1 << "\n";
        }
    }
    
    out0.close();
    out1.close();
}

} // namespace aie