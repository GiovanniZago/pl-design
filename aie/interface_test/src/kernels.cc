#include "kernels.h"

void interface_test(input_stream<int16> * __restrict in0, input_stream<int16> * __restrict in1, output_stream<int32> * __restrict out) {
    // read event metadata
    const int16_t num_cands = readincr(in0);
    const int16_t bx = readincr(in0);
    const int16_t orbit_h = readincr(in1);
    const int16_t orbit_l = readincr(in1);
    const int32_t orbit = (orbit_h << 16) | orbit_l;

    // vectors
    div_t res = div(num_cands, VEC_SIZE);
    const int16_t num_vects = (res.quot == 0) ? 1 : (res.rem == 0) ? res.quot : res.quot + 1;
    aie::vector<int16_t, VEC_SIZE> pt[num_vects] = { aie::broadcast<int16_t, VEC_SIZE>(0) };
    aie::vector<int16_t, VEC_SIZE> eta[num_vects] = { aie::broadcast<int16_t, VEC_SIZE>(0) };
    aie::vector<int16_t, VEC_SIZE> phi[num_vects] = { aie::broadcast<int16_t, VEC_SIZE>(0) };
    aie::vector<int16_t, VEC_SIZE> pid[num_vects] = { aie::broadcast<int16_t, VEC_SIZE>(0) };
    
    // buffers
    const int16_t num_ele = num_vects * VEC_SIZE;
    int16_t pt_buff[num_ele] = { 0 };
    int16_t eta_buff[num_ele] = { 0 };
    int16_t phi_buff[num_ele] = { 0 };
    int16_t pid_buff[num_ele] = { 0 };
    
    const int16_t num_read = (num_cands % 2) ? num_cands : num_cands + 1;
    
    for (int32_t ii = 0; ii < num_read; ++i) {
        pt_buff[ii] = readincr(in0);
        pid_buff[ii] = readincr(in1);
    }

    for (int32_t ii = 0; ii < num_read; ++i) {
        eta_buff[ii] = readincr(in0);
        phi_buff[ii] = readincr(in1);
    }

    // vector pointers
    aie::vector<int16_t, VEC_SIZE> *pt_ptr = (aie::vector<int16_t, VEC_SIZE> *) pt_buff;
    aie::vector<int16_t, VEC_SIZE> *eta_ptr = (aie::vector<int16_t, VEC_SIZE> *) eta_buff;
    aie::vector<int16_t, VEC_SIZE> *phi_ptr = (aie::vector<int16_t, VEC_SIZE> *) phi_buff;
    aie::vector<int16_t, VEC_SIZE> *pid_ptr = (aie::vector<int16_t, VEC_SIZE> *) pid_buff;
    
    // fill vectors
    for (unsigned int vec_idx = 0; vec_idx < num_vects; ++vec_idx) {
        pt[vec_idx].load_v<VEC_SIZE>(pt_ptr);
        eta[vec_idx].load_v<VEC_SIZE>(eta_ptr);
        ++vec_ptr0;
        ++vec_ptr1;
    }

    // restore variables back to initial position
    memset(buf0, (int16) 0, MAX_CANDS * sizeof(buf0[0]))
    memset(buf1, (int16) 0, MAX_CANDS * sizeof(buf1[0]))
    s0_handler{0, false};
    s1_handler{0, false};
    ii = 0;
    vec_ptr0 -= NUM_VECT_A;
    vec_ptr1 -= NUM_VECT_A;

    // read phi and pid
    while ((!s0_handler.t_last) & (!s1_handler.t_last))
    {   
        // read pt
        s0 >> s0_handler;
        buf0[ii] = s0_handler.value;

        // read eta
        s1 >> s1_handler;
        buf1[ii] = s1_handler.value;

        ++ii;
    }

    div_t result_b = div(ii, (unsigned int) 32);
    
    if (result_b.quot == 0) {
        const int NUM_VECT_B = 1;
    } else {
        const int NUM_VECT_B = (result_b.rem == 0) ? result_b.quot : result_b.quot + 1;
    }

    // check consistency between the two streams
    if (NUM_VECT_A != NUM_VECT_B) {
        throw std::runtime_error("The number of int16 values read for pt, eta and phi, pid should be the same, but it is not.\n");
    }

    aie::vector<int16, 32> phi[NUM_VECT_B] = { aie::broadcast<int16, 32>(0) };
    aie::vector<int16, 32> pid[NUM_VECT_B] = { aie::broadcast<int16, 32>(0) };

    for (unsigned int vidx = 0; vidx < NUM_VECT_B; vidx++) {
        phi[vidx].load(vec_ptr0);
        pid[vidx].load(vec_ptr1);
        ++vec_ptr0;
        ++vec_ptr1;
    }

    // data loopback
    for (unsigned int vidx = 0; vidx < NUM_VECT_A; vidx++) {
        writeincr(out, pt[vidx]);
    }

    for (unsigned int vidx = 0; vidx < NUM_VECT_A; vidx++) {
        writeincr(out, eta[vidx]);
    }

    for (unsigned int vidx = 0; vidx < NUM_VECT_B; vidx++) {
        writeincr(out, phi[vidx]);
    }

    for (unsigned int vidx = 0; vidx < NUM_VECT_B; vidx++) {
        writeincr(out, pid[vidx]);
    }
}