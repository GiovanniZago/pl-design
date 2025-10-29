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

    ap_uint<64> headers[ORBIT_SIZE];

    for (unsigned int ii=0; ii<ORBIT_SIZE; ii++) // burst memory access
    {
        headers[ii] = ap_uint<64>(mem[ii]);
    }

    ap_uint<12> bx[ORBIT_SIZE];
    ap_uint<32> orbit[ORBIT_SIZE];
    ap_uint<12> num_cands[ORBIT_SIZE];

    for (unsigned int ii=0; ii<ORBIT_SIZE; ii++)
    {
        #pragma HLS UNROLL

        bx[ii]        = headers[ii].range(23, 12);
        orbit[ii]     = headers[ii].range(55, 24);
        num_cands[ii] = headers[ii].range(11, 0);
    }

    unsigned int event_base = 0;
    unsigned int stream_port_index = 0;

    ap_uint<64> words[MAX_CANDS];
    ap_uint<16> pt   [MAX_CANDS];
    ap_int<16>  eta  [MAX_CANDS];
    ap_int<16>  phi  [MAX_CANDS];
    ap_uint<16> pid  [MAX_CANDS];

    for (unsigned int ii=0; ii<ORBIT_SIZE; ii++)
    {
        for (unsigned int jj=0; jj<num_cands[ii]; jj++) // burst memory access
        {
            words[jj] = ap_uint<64>(mem[ORBIT_SIZE + event_base + jj]);
        }

        for (unsigned int jj=0; jj<num_cands[ii]; jj++)
        {
            #pragma HLS PIPELINE II=1

            pt[jj] = words[jj].range(13, 0); // pt is 13-bit wide

            eta[jj] = words[jj].range(25, 14); // eta is 12-bit wide
            eta[jj] = eta[jj] | ((eta[jj][11]) ? 0xF000 : 0x0000); // pad the leading bits to 0 or 1 according to the sign bit

            phi[jj] = words[jj].range(36, 26); // phi is 11-bit wide
            phi[jj] = phi[jj] | ((phi[jj][10]) ? 0xFC00 : 0x0000); // pad the leading bits to 0 or 1 according to the sign bit

            pid[jj] = words[jj].range(39, 37); // pid is 2-bit wide
        }

        bool tile_0 = (stream_port_index == 0u);
        bool tile_1 = (stream_port_index == 1u);
        
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

            switch (stream_port_index) 
            {
                case 0u: s0.write(x_pt); s1.write(x_eta); break;
                case 1u: s2.write(x_pt); s3.write(x_eta); break;
                default: s4.write(x_pt); s5.write(x_eta); break;
            }
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

            switch (stream_port_index) 
            {
                case 0u: s0.write(x_phi); s1.write(x_pid); break;
                case 1u: s2.write(x_phi); s3.write(x_pid); break;
                default: s4.write(x_phi); s5.write(x_pid); break;
            }
        }

        event_base += num_cands[ii];
        stream_port_index = (stream_port_index + 1) % 3;
    }
}