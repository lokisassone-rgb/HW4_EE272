#ifndef SYSTOLIC_ARRAY_H
#define SYSTOLIC_ARRAY_H

#include "ProcessingElement.h"
#include "conv.h"
#include "Fifo.h"
#include "SystolicArrayCore.h"

// Include mc_scverify.h for CCS_* macros
#include <mc_scverify.h>

class SystolicArrayLooper
{
public:
    SystolicArrayLooper() {}

#pragma hls_design interface
void run(ac_channel<Params> &paramsIn,
         ac_channel<Params> &paramsOut,
         ac_channel<LoopIndices> &loopIndicesOut)
    {
        // -------------------------------
        // Generate the loop indices here for the systolic array.
        // Write the loop indices as well as the params out to channels.
        // Your code starts here
        Params params_tmp = paramsIn.read();
        // paramsOut.write(params_tmp);
        // loop indices over ic1, fx and fy
        for (uint_16 oy1 = 0; oy1 < params_tmp.OY1; oy1++){
            for (uint_16 ox1 = 0; ox1 < params_tmp.OX1; ox1++){
                for (uint_16 oc1 = 0; oc1 < params_tmp.OC1; oc1++){
                    for (uint_16 ic1 = 0; ic1 < params_tmp.IC1; ic1++){
                        for (uint_16 fy = 0; fy < params_tmp.FY; fy++){
                            for (uint_16 fx = 0; fx < params_tmp.FX; fx++){
                                LoopIndices loopIndices_tmp;
                                loopIndices_tmp.ic1_idx = ic1;
                                loopIndices_tmp.fy_idx = fy;
                                loopIndices_tmp.fx_idx = fx;
                                loopIndicesOut.write(loopIndices_tmp);
                                paramsOut.write(params_tmp);
                            }
                        }
                    }
                }
            }
        }
        // Your code ends here
        // -------------------------------
    }
};

template <typename IDTYPE, typename WDTYPE, typename ODTYPE, int OC0, int IC0>
class SystolicArrayWrapper
{
public:
    SystolicArrayWrapper(){}
    
#pragma hls_design interface
    void run(ac_channel<PackedInt<INPUT_PRECISION, IC0> > &input, 
             ac_channel<PackedInt<WEIGHT_PRECISION, OC0> > &weight, 
             ac_channel<PackedInt<OUTPUT_PRECISION, OC0> > &output,
             ac_channel<Params> &paramsIn)
    {
        systolicArrayLooper.run(paramsIn, paramsChannel, loopIndicesChannel);
        systolicArrayCore.run(input, weight, output, paramsChannel, loopIndicesChannel);
    }
private:
    SystolicArrayCore<IDTYPE, WDTYPE, ODTYPE, OC0, IC0> systolicArrayCore;
    SystolicArrayLooper systolicArrayLooper;
    ac_channel<Params> paramsChannel;
    ac_channel<LoopIndices> loopIndicesChannel;
};

#endif
