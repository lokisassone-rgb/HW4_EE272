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
        
        for (int oy1 = 0; oy1 < params.OY1; oy1++){
            for (int ox1 = 0; ox1 < params.OX1; ox1++){
                for (int oc1 = 0; oc1 < params.OC1; oc1++){
                    chanStruct<PackedInt<WEIGHT_PRECISION, OC0>, size> temp;
                    for (int ic1 = 0; ic1 < params.IC1; ic1++){
                        for (int fy = 0; fy < params.FY; fy++){
                            for (int fx = 0; fx < params.FX; fx++){
                                int buffer_add = ic1 * (int(params.FY) * int(params.FX)) + fy * int(params.FX) + fx; //calculate address linearly
                                PackedInt<WEIGHT_PRECISION, 4> tempdinread = din.read();
                                for (int oc0_block = 0; oc0_block < OC0/4; oc0_block++){
                                    PackedInt<WEIGHT_PRECISION, OC0> tempdinwrite;
                                    tempdinwrite.value[oc0_block*4] = tempdinread.value[0];
                                    tempdinwrite.value[oc0_block*4+1] = tempdinread.value[1];
                                    tempdinwrite.value[oc0_block*4+2] = tempdinread.value[2];
                                    tempdinwrite.value[oc0_block*4+3] = tempdinread.value[3];
                                    temp.data[buffer_add * (OC0/4) + oc0_block] = tempdinwrite;
                                }
                            }
                        }
                    }
                    dout.write(temp);
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

        for (int oy1 = 0; oy1 < params.OY1; oy1++){
            for (int ox1 = 0; ox1 < params.OX1; ox1++){
                chanStruct<PackedInt<WEIGHT_PRECISION, OC0>, size> temp = din.read(); //create new temp tile 
                for (int oc1 = 0; oc1 < params.OC1; oc1++){
                    for (int ic1 = 0; ic1 < params.IC1; ic1++){
                        for (int fy = 0; fy < params.FY; fy++){
                            for (int fx = 0; fx < params.FX; fx++){
                                int buffer_add = ic1 * (int(params.FY) * int(params.FX)) + fy * int(params.FX) + fx; //calculate address linearly
                                for (int oc0_block = 0; oc0_block < OC0/4; oc0_block++){
                                    dout.write( temp.data[buffer_add * (OC0/4) + oc0_block] );
                                }
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

        // #ifndef __SYNTHESIS__
        // ac_int<ac::log2_ceil<size>::val, false> block_size = IC0*params.IC1*params.FX*params.FY;
        // assert(block_size <= size);
        // #endif

        weightDoubleBufferReaderParams.write(params);
        weightDoubleBufferWriterParams.write(params);

        weightDoubleBufferWriter.run(weightDoubleBufferWriterParams, weights_in, mem);
        weightDoubleBufferReader.run(weightDoubleBufferReaderParams, mem, weights_out);
    }

private:
    ac_channel<chanStruct<PackedInt<WEIGHT_PRECISION, OC0>,size> > mem;
    
    WeightDoubleBufferWriter<size, IC0, OC0> weightDoubleBufferWriter;
    ac_channel<Params> weightDoubleBufferWriterParams;
    
    WeightDoubleBufferReader<size, IC0, OC0> weightDoubleBufferReader;
    ac_channel<Params> weightDoubleBufferReaderParams;
};


#endif
