#ifndef WEIGHT_DOUBLE_BUFFER_H
#define WEIGHT_DOUBLE_BUFFER_H


template <int size, int IC0, int OC0>
class WeightDoubleBufferWriter{
public:
    WeightDoubleBufferWriter(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<PackedInt<WEIGHT_PRECISION, 4> > &din,
                        ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>, size> > &dout)
    {
        // -------------------------------
        // Your code starts here
        Params params = paramsIn.read();
        printf("[WeightDoubleBufferWriter] Params: OY1=%d, OX1=%d, OC1=%d, IC1=%d, FX=%d, FY=%d\n", (int)params.OY1, (int)params.OX1, (int)params.OC1, (int)params.IC1, (int)params.FX, (int)params.FY);

        for (uint_16 oy1 = 0; oy1 < params.OY1; oy1++){
            for (uint_16 ox1 = 0; ox1 < params.OX1; ox1++){
                for (uint_16 oc1 = 0; oc1 < params.OC1; oc1++){
                    chanStruct<PackedInt<WEIGHT_PRECISION, OC0>, size> tile;
                    PackedInt<WEIGHT_PRECISION, 4> input;
                    //define size of a weight tile (IC1*IC0*FX*FY)
                    for (uint_16 ic = 0; ic < (params.IC1*IC0); ic++){
                        for (uint_16 fy = 0; fy < params.FY; fy++){
                            for (uint_16 fx = 0; fx < params.FX; fx++){
                                for (uint_16 weight_index = 0; weight_index < OC0/4; weight_index++){
                                    input = din.read();
                                    printf("[WeightDoubleBufferWriter] Read input: ic=%d, fy=%d, fx=%d, weight_index=%d input.value=[%d,%d,%d,%d]\n",
                                        (int)ic, (int)fy, (int)fx, (int)weight_index,
                                        (int)input.value[0], (int)input.value[1], (int)input.value[2], (int)input.value[3]);
                                    for (uint_16 j = 0; j < 4; j++) {
                                        tile.data[ic*(params.FY)*(params.FX) + fy*(params.FX) + fx].value[weight_index*4+j] = input.value[j];
                                    }
                                }
                            }
                        }
                    }
                    printf("[WeightDoubleBufferWriter] Writing tile for oy1=%d, ox1=%d, oc1=%d\n", (int)oy1, (int)ox1, (int)oc1);
                    dout.write(tile);
                }
            }
        }
        // Your code ends here
        // -------------------------------
    }
};

template <int size, int IC0, int OC0>
class WeightDoubleBufferReader{
public:
    WeightDoubleBufferReader(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> > &din, 
                        ac_channel<PackedInt<WEIGHT_PRECISION, OC0> > &dout)
    {
        // -------------------------------
        // Your code starts here
        Params params = paramsIn.read();
        printf("[WeightDoubleBufferReader] Params: OY1=%d, OX1=%d, OC1=%d, IC1=%d, FX=%d, FY=%d\n", (int)params.OY1, (int)params.OX1, (int)params.OC1, (int)params.IC1, (int)params.FX, (int)params.FY);

        for (uint_16 oy1 = 0; oy1 < params.OY1; oy1++){
            for (uint_16 ox1 = 0; ox1 < params.OX1; ox1++){
                for (uint_16 oc1 = 0; oc1 < params.OC1; oc1++){
                    chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> tmp = din.read();
                    printf("[WeightDoubleBufferReader] Read tile for oy1=%d, ox1=%d, oc1=%d\n", (int)oy1, (int)ox1, (int)oc1);
                    PackedInt<WEIGHT_PRECISION, OC0> weights_going_to_systolic_array;
                    for (uint_16 ic = 0; ic < (params.IC1*IC0); ic++){
                        for (uint_16 fy = 0; fy < params.FY; fy++){
                            for (uint_16 fx = 0; fx < params.FX; fx++){
                                weights_going_to_systolic_array = tmp.data[ic*(params.FY)*(params.FX) + fy*(params.FX) + fx];
                                printf("[WeightDoubleBufferReader] Writing weights: ic=%d, fy=%d, fx=%d\n", (int)ic, (int)fy, (int)fx);
                                dout.write(weights_going_to_systolic_array);
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

template <int size, int IC0, int OC0>
class WeightDoubleBuffer{
public:
  WeightDoubleBuffer(){}

  #pragma hls_design interface
  void CCS_BLOCK(run)(ac_channel<PackedInt<WEIGHT_PRECISION, 4> > &weights_in, 
                      ac_channel<PackedInt<WEIGHT_PRECISION, OC0> > &weights_out,
                      ac_channel<Params> &paramsIn)
    {
        Params params = paramsIn.read();
        printf("[WeightDoubleBuffer] Starting run with Params: OY1=%d, OX1=%d, OC1=%d, IC1=%d, FX=%d, FY=%d\n", (int)params.OY1, (int)params.OX1, (int)params.OC1, (int)params.IC1, (int)params.FX, (int)params.FY);

        // #ifndef __SYNTHESIS__
        // ac_int<ac::log2_ceil<size>::val, false> block_size = IC0*params.IC1*params.FX*params.FY;
        // assert(block_size <= size);
        // #endif

        weightDoubleBufferReaderParams.write(params);
        weightDoubleBufferWriterParams.write(params);

        printf("[WeightDoubleBuffer] Calling Writer...\n");
        weightDoubleBufferWriter.run(weightDoubleBufferWriterParams, weights_in, mem);
        printf("[WeightDoubleBuffer] Calling Reader...\n");
        weightDoubleBufferReader.run(weightDoubleBufferReaderParams, mem, weights_out);
        printf("[WeightDoubleBuffer] Finished run.\n");
    #include <cstdio>
    }

private:
    ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> > mem;
    
    WeightDoubleBufferWriter<size, IC0, OC0> weightDoubleBufferWriter;
    ac_channel<Params> weightDoubleBufferWriterParams;
    
    WeightDoubleBufferReader<size, IC0, OC0> weightDoubleBufferReader;
    ac_channel<Params> weightDoubleBufferReaderParams;
};


#endif
