#include "mm2s.h"

void mm2s(
    const unsigned long long* mem, 
    hls::stream<qdma_t>& s0,
    hls::stream<qdma_t>& s1,
    hls::stream<qdma_t>& s2,
    hls::stream<qdma_t>& s3,
    hls::stream<qdma_t>& s4,
    hls::stream<qdma_t>& s5
) {
    #pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem 
    #pragma HLS INTERFACE axis port=s0
    #pragma HLS INTERFACE axis port=s1
    #pragma HLS INTERFACE axis port=s2
    #pragma HLS INTERFACE axis port=s3
    #pragma HLS INTERFACE axis port=s4
    #pragma HLS INTERFACE axis port=s5

    #pragma HLS INTERFACE s_axilite port=mem bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    unsigned long long headers[ORBIT_SIZE];

    for (unsigned int ii=0; ii<ORBIT_SIZE; ii++) // burst memory access
    {
        headers[ii] = mem[ii];
    }

    unsigned int orbit[ORBIT_SIZE], bx[ORBIT_SIZE], num_cands[ORBIT_SIZE];

    for (unsigned int ii=0; ii<ORBIT_SIZE; ii++)
    {
        #pragma HLS PIPELINE II=1

        bx[ii]        = (headers[ii] >> 12) & 0xFFF;
        orbit[ii]     = (headers[ii] >> 24) & 0xFFFFFFFF;
        num_cands[ii] = headers[ii] & 0xFFF;
    }

    unsigned int event_base = 0;
    unsigned int stream_port_index = 0;

    unsigned long long words[MAX_CANDS];
    ap_uint<16> pt[MAX_CANDS];
    ap_int<16> eta[MAX_CANDS];
    ap_int<16> phi[MAX_CANDS];
    ap_uint<16> pid[MAX_CANDS];

    #pragma HLS BIND_STORAGE variable=words type=ram_1p impl=bram
    #pragma HLS BIND_STORAGE variable=pt    type=ram_1p impl=bram
    #pragma HLS BIND_STORAGE variable=eta   type=ram_1p impl=bram
    #pragma HLS BIND_STORAGE variable=phi   type=ram_1p impl=bram
    #pragma HLS BIND_STORAGE variable=pid   type=ram_1p impl=bram

    for (unsigned int ii=0; ii<ORBIT_SIZE; ii++)
    {
        #pragma HLS PIPELINE II=1

        for (unsigned int jj=0; jj<num_cands[ii]; jj++) // burst memory access
        {
            words[jj] = mem[ORBIT_SIZE + event_base + jj];
        }

        for (unsigned int jj=0; jj<num_cands[ii]; jj++)
        {
            #pragma HLS PIPELINE II=1

            pt[jj] = words[jj] & 0x1FFF; // pt is 13-bit wide

            eta[jj] = (words[jj] >> 14) & 0xFFF; // eta is 12-bit wide
            eta[jj] = eta[jj] | ((eta[jj][11]) ? 0xF000 : 0x0000); // pad the leading bits to 0 or 1 according to the sign bit

            phi[jj] = (words[jj] >> 26) & 0x7FF; // phi is 11-bit wide
            phi[jj] = phi[jj] | ((phi[jj][10]) ? 0xFC00 : 0x0000); // pad the leading bits to 0 or 1 according to the sign bit

            pid[jj] = (words[jj] >> 37) & 0x3; // pid is 2-bit wide
        }

        bool tile_0 = (stream_port_index == 0u);
        bool tile_1 = (stream_port_index == 1u);

        auto& port_0 = tile_0 ? s0 : (tile_1 ? s2 : s4);            
        auto& port_1 = tile_0 ? s1 : (tile_1 ? s3 : s5);    
        
        qdma_t x_pt, x_eta, x_phi, x_pid;
        
        for (unsigned int jj=0; jj<num_cands[ii]; jj+=2)
        {
            #pragma HLS PIPELINE II=1

            bool pair = (jj + 1 < num_cands[ii]);
            
            x_pt.data.range(15, 0) = pt[jj];
            x_pt.data.range(31,16) = pair ? pt[jj + 1] : ap_uint<16>(0);

            x_eta.data.range(15, 0) = eta[jj];
            x_eta.data.range(31,16) = pair ? eta[jj + 1] : ap_int<16>(0);

            x_pt.keep_all();
            x_eta.keep_all();

            port_0.write(x_pt);
            port_1.write(x_eta);
        }

        for (unsigned int jj=0; jj<num_cands[ii]; jj+=2)
        {
            #pragma HLS PIPELINE II=1

            bool pair = (jj + 1 < num_cands[ii]);

            x_phi.data.range(15, 0) = phi[jj];
            x_phi.data.range(31,16) = pair ? phi[jj + 1] : ap_int<16>(0);

            x_pid.data.range(15, 0) = pid[jj];
            x_pid.data.range(31,16) = pair ? pid[jj + 1] : ap_uint<16>(0);

            x_phi.keep_all();
            x_pid.keep_all();

            port_0.write(x_phi);
            port_1.write(x_phi);
        }

        event_base += num_cands[ii];
        stream_port_index = (stream_port_index + 1) % 3;
    }
}