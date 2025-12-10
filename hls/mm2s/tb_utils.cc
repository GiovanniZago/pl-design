#include "tb_utils.h"

void dump_pt_eta_stream(hls::stream<qdma_axis<32,0,0,0>> &s, 
                const std::string &fout, 
                const uint32_t steps, 
                const bool split,
                const bool verbose) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }

    // read pt
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();
        int32_t out_data = out.data.to_int();

        if (!split) {
            if (verbose) {
                out_file << std::right << std::setw(10) << std::dec << out_data << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
            } else {
                out_file << std::right << std::setw(10) << std::dec << out_data << "\n";
            }
        } else {
            uint16_t out_data_l = out_data & 0xFFF; 
            uint16_t out_data_h = (out_data >> 16) & 0xFFF;
            if (verbose){
                out_file << std::right << std::setw(10) << std::dec << out_data_h << std::setw(10) << std::dec << out_data_l << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
            } else {
                out_file << std::right << std::setw(10) << std::dec << out_data_h << std::setw(10) << std::dec << out_data_l << "\n";
            }
        }
    }

    // read eta
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();
        int32_t out_data = out.data.to_int();

        if (!split) {
            if (verbose) {
                out_file << std::right << std::setw(10) << std::dec << out_data << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
            } else {
                out_file << std::right << std::setw(10) << std::dec << out_data << "\n";
            }
        } else {
            uint16_t out_data_l = out_data & 0xFFFF; // must keep all 16 bits otherwise, if we use 0xFFF, the top nibble is set to zero -> positive value
            uint16_t out_data_h = (out_data >> 16) & 0xFFFF; // must keep all 16 bits otherwise, if we use 0xFFF, the top nibble is set to zero -> positive value
            if (verbose){
                out_file << std::right << std::setw(10) << std::dec << out_data_h << std::setw(10) << std::dec << out_data_l << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
            } else {
                out_file << std::right << std::setw(10) << std::dec << out_data_h << std::setw(10) << std::dec << out_data_l << "\n";
            }
        }
    }
}

void dump_pid_phi_stream(hls::stream<qdma_axis<32,0,0,0>> &s, 
                const std::string &fout, 
                const uint32_t steps, 
                const bool split,
                const bool verbose) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }

    // read pid
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();
        int32_t out_data = out.data.to_int();

        if (!split) {
            if (verbose) {
                out_file << std::right << std::setw(10) << std::dec << out_data << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
            } else {
                out_file << std::right << std::setw(10) << std::dec << out_data << "\n";
            }
        } else {
            uint16_t out_data_l = out_data & 0xFF; 
            uint16_t out_data_h = (out_data >> 16) & 0xFF; 
            if (verbose){
                out_file << std::right << std::setw(10) << std::dec << unsigned(out_data_h) << std::setw(10) << std::dec << unsigned(out_data_l) << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
            } else {
                out_file << std::right << std::setw(10) << std::dec << unsigned(out_data_h) << std::setw(10) << std::dec << unsigned(out_data_l) << "\n";
            }
        }
    }

    // read phi
    for (unsigned int i = 0; i < steps; ++i) {
        qdma_axis<32,0,0,0> out = s.read();
        int32_t out_data = out.data.to_int();

        if (!split) {
            if (verbose) {
                out_file << std::right << std::setw(10) << std::dec << out_data << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
            } else {
                out_file << std::right << std::setw(10) << std::dec << out_data << "\n";
            }
        } else {
            uint16_t out_data_l = out_data & 0xFFFF; // must keep all 16 bits otherwise, if we use 0xFFF, the top nibble is set to zero -> positive value
            uint16_t out_data_h = (out_data >> 16) & 0xFFFF; // must keep all 16 bits otherwise, if we use 0xFFF, the top nibble is set to zero -> positive value
            if (verbose){
                out_file << std::right << std::setw(10) << std::dec << out_data_h << std::setw(10) << std::dec << out_data_l << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
            } else {
                out_file << std::right << std::setw(10) << std::dec << out_data_h << std::setw(10) << std::dec << out_data_l << "\n";
            }
        }
    }
}

void dump_bx_cands_header(hls::stream<qdma_axis<32,0,0,0>> &s, 
                        const std::string &fout,
                        const bool split, 
                        const bool verbose) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }
    
    qdma_axis<32,0,0,0> out = s.read();
    int32_t out_data = out.data.to_int();

    if (!split) {
        if (verbose) {
            out_file << std::right << std::setw(10) << std::dec << out_data << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
        } else {
            out_file << std::right << std::setw(10) << std::dec << out_data << "\n";
        }
    } else {
        uint16_t bx = out_data & 0x0FFF;
        uint16_t num_cands = (out_data >> 16) & 0x0FFF;
        if (verbose) {
            out_file << std::right << std::setw(10) << std::dec << num_cands << std::setw(10) << std::dec << bx << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << " --------------------\n";
        } else {
            out_file << std::right << std::setw(10) << std::dec << num_cands << std::setw(10) << std::dec << bx << "\n";
        }
    }


}

void dump_orbit_header(hls::stream<qdma_axis<32,0,0,0>> &s, 
                        const std::string &fout, 
                        const bool split,
                        const bool verbose) {
    std::ofstream out_file(fout, std::ios::app);

    if (!out_file) throw std::runtime_error("Error in opening the out file");

    if (s.empty()) {
        throw std::runtime_error("s is an empty stream");
    }

    qdma_axis<32,0,0,0> out = s.read();
    int32_t out_data = out.data.to_int();

    if (!split) {
        if (verbose) {
            out_file << std::right << std::setw(10) << std::dec << out_data << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << "\n";
        } else {
            out_file << std::right << std::setw(10) << std::dec << out_data << "\n";
        }
    } else {
        uint16_t orbit_number_l = out_data & 0xFFFF;
        uint16_t orbit_number_h = (out_data >> 16) & 0xFFFF;
        if (verbose) {
            out_file << std::right << std::setw(10) << std::dec << orbit_number_h << std::setw(10) << std::dec << orbit_number_l << std::setw(10) << std::hex << out.last << std::setw(10) << std::hex << out.keep << " --------------------\n";
        } else {
            out_file << std::right << std::setw(10) << std::dec << orbit_number_h << std::setw(10) << std::dec << orbit_number_l << "\n";
        }
    }


}