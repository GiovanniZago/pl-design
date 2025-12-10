#include <adf.h>
#include "stream_mux/src/kernels.h"

using namespace adf;

class TestGraph : public graph 
{
    private:
        kernel stream_mux_k;

    public:
        input_plio in0;
        input_plio in1;
        output_plio out;

        TestGraph() 
        {
            stream_mux_k = kernel::create(stream_mux);

            in0 = input_plio::create("in0", plio_32_bits, "data0.txt", 360);
            in1 = input_plio::create("in1", plio_32_bits, "data1.txt", 360);
            out = output_plio::create("out", plio_32_bits, "out.txt", 360);

            // PL inputs
            connect<stream>(in0.out[0], stream_mux_k.in[0]);
            connect<stream>(in1.out[0], stream_mux_k.in[1]);

            // PL outputs
            connect<stream>(stream_mux_k.out[0], out.in[0]);

            // sources and runtime ratios
            source(stream_mux_k) = "stream_mux/src/kernels.cc";
            runtime<ratio>(stream_mux_k) = 1;
        }
};