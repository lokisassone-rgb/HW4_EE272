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
        std::cout << "[WeightDoubleBufferWriter] Params: OY1=" << params.OY1 << ", OX1=" << params.OX1 << ", OC1=" << params.OC1 << ", IC1=" << params.IC1 << ", FX=" << params.FX << ", FY=" << params.FY << std::endl;

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
                                    std::cout << "[WeightDoubleBufferWriter] Read input: ic=" << ic << ", fy=" << fy << ", fx=" << fx << ", weight_index=" << weight_index << " input.value=[";
                                    for (uint_16 j = 0; j < 4; j++) {
                                        std::cout << input.value[j] << (j < 3 ? "," : "]\n");
                                        tile.data[ic*(params.FY)*(params.FX) + fy*(params.FX) + fx].value[weight_index*4+j] = input.value[j];
                                    }
                                }
                            }
                        }
                    }
                    std::cout << "[WeightDoubleBufferWriter] Writing tile for oy1=" << oy1 << ", ox1=" << ox1 << ", oc1=" << oc1 << std::endl;
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
        std::cout << "[WeightDoubleBufferReader] Params: OY1=" << params.OY1 << ", OX1=" << params.OX1 << ", OC1=" << params.OC1 << ", IC1=" << params.IC1 << ", FX=" << params.FX << ", FY=" << params.FY << std::endl;

        for (uint_16 oy1 = 0; oy1 < params.OY1; oy1++){
            for (uint_16 ox1 = 0; ox1 < params.OX1; ox1++){
                for (uint_16 oc1 = 0; oc1 < params.OC1; oc1++){
                    chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> tmp = din.read();
                    std::cout << "[WeightDoubleBufferReader] Read tile for oy1=" << oy1 << ", ox1=" << ox1 << ", oc1=" << oc1 << std::endl;
                    PackedInt<WEIGHT_PRECISION, OC0> weights_going_to_systolic_array;
                    for (uint_16 ic = 0; ic < (params.IC1*IC0); ic++){
                        for (uint_16 fy = 0; fy < params.FY; fy++){
                            for (uint_16 fx = 0; fx < params.FX; fx++){
                                weights_going_to_systolic_array = tmp.data[ic*(params.FY)*(params.FX) + fy*(params.FX) + fx];
                                std::cout << "[WeightDoubleBufferReader] Writing weights: ic=" << ic << ", fy=" << fy << ", fx=" << fx << std::endl;
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
        std::cout << "[WeightDoubleBuffer] Starting run with Params: OY1=" << params.OY1 << ", OX1=" << params.OX1 << ", OC1=" << params.OC1 << ", IC1=" << params.IC1 << ", FX=" << params.FX << ", FY=" << params.FY << std::endl;

        // #ifndef __SYNTHESIS__
        // ac_int<ac::log2_ceil<size>::val, false> block_size = IC0*params.IC1*params.FX*params.FY;
        // assert(block_size <= size);
        // #endif

        weightDoubleBufferReaderParams.write(params);
        weightDoubleBufferWriterParams.write(params);

        std::cout << "[WeightDoubleBuffer] Calling Writer..." << std::endl;
        weightDoubleBufferWriter.run(weightDoubleBufferWriterParams, weights_in, mem);
        std::cout << "[WeightDoubleBuffer] Calling Reader..." << std::endl;
        weightDoubleBufferReader.run(weightDoubleBufferReaderParams, mem, weights_out);
        std::cout << "[WeightDoubleBuffer] Finished run." << std::endl;
    }

private:
    ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> > mem;
    
    WeightDoubleBufferWriter<size, IC0, OC0> weightDoubleBufferWriter;
    ac_channel<Params> weightDoubleBufferWriterParams;
    
    WeightDoubleBufferReader<size, IC0, OC0> weightDoubleBufferReader;
    ac_channel<Params> weightDoubleBufferReaderParams;
};


#endif
