#include "tb_utils.h"

void dump_pt_eta_stream(hls::stream<qdma_axis<32,0,0,0>> &s, 
                const std::string &fout, 
                const uint32_t steps) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }

    // read pt
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();
        int32_t out_data = out.data.to_int();
        uint16_t out_data_l = out_data & 0xFFF; 
        uint16_t out_data_h = (out_data >> 16) & 0xFFF;

        out_file << std::right << std::setw(10) << std::dec << out_data_h << std::setw(10) << std::dec << out_data_l << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
    }

    // read eta
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();
        int32_t out_data = out.data.to_int();
        int16_t out_data_l = out_data & 0xFFFF; // must keep all 16 bits otherwise, if we use 0xFFF, the top nibble is set to zero -> positive value
        int16_t out_data_h = (out_data >> 16) & 0xFFFF;

        out_file << std::right << std::setw(10) << std::dec << out_data_h << std::setw(10) << std::dec << out_data_l << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
    }
}

void dump_pid_phi_stream(hls::stream<qdma_axis<32,0,0,0>> &s, 
                const std::string &fout, 
                const uint32_t steps) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }

    // read pid
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();
        int32_t out_data = out.data.to_int();
        uint8_t out_data_l = out_data & 0xFF; 
        uint8_t out_data_h = (out_data >> 16) & 0xFF;

        out_file << std::right << std::setw(10) << std::dec << unsigned(out_data_h) << std::setw(10) << std::dec << unsigned(out_data_l) << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
    }

    // read phi
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();
        int32_t out_data = out.data.to_int();
        int16_t out_data_l = out_data & 0xFFFF; 
        int16_t out_data_h = (out_data >> 16) & 0xFFFF;

        out_file << std::right << std::setw(10) << std::dec << out_data_h << std::setw(10) << std::dec << out_data_l << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
    }
}

void dump_bx_cands_header(hls::stream<qdma_axis<32,0,0,0>> &s, const std::string &fout) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }
    
    qdma_axis<32,0,0,0> out = s.read();
    int32_t out_data = out.data.to_int();
    uint16_t bx = out_data & 0x0FFF;
    uint16_t num_cands = (out_data >> 16) & 0x0FFF;

    out_file << std::right << std::setw(10) << std::dec << num_cands << std::setw(10) << std::dec << bx << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << " --------------------\n";
}

void dump_orbit_header(hls::stream<qdma_axis<32,0,0,0>> &s, const std::string &fout) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }

    qdma_axis<32,0,0,0> out = s.read();
    int32_t orbit_data = out.data.to_int();
    uint16_t orbit_number_l = orbit_data & 0xFFFF;
    uint16_t orbit_number_h = (orbit_data >> 16) & 0xFFFF;
    uint32_t orbit = (orbit_number_h << 16) | orbit_number_l;

    out_file << std::right << std::setw(10) << std::dec << orbit_number_h << std::setw(10) << std::dec << orbit_number_l << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << " --------------------\n";
}