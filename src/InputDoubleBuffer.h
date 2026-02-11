#ifndef INPUT_DOUBLE_BUFFER_H
#define INPUT_DOUBLE_BUFFER_H

template <int size, int IC0, int OC0>
class InputDoubleBufferWriter{
public:
    InputDoubleBufferWriter(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<PackedInt<INPUT_PRECISION, 4> > &din,
                        ac_channel<chanStruct<PackedInt<INPUT_PRECISION,IC0>,size> > &dout)
    {
        // -------------------------------
        // Your code starts here
        Params params = paramsIn.read();
        int numberofTiles = params.OX1 * params.OY1;
        int ix0 = (params.OX0 -1 )*params.STRIDE + params.FX;
        int iy0 = (params.OY0 -1 )*params.STRIDE + params.FY;
        int sizeofDoubleBuffer = ix0*iy0*params.IC1;


        chanStruct<PackedInt<INPUT_PRECISION,IC0>,size> temp;
        PackedInt<INPUT_PRECISION, 4> tempdinread;
        PackedInt<INPUT_PRECISION, IC0> tempdinwrite;

        #pragma hls_pipeline_init_interval 1
        for (int i=0; i < numberofTiles; i++){
            for (int j=0; j < sizeofDoubleBuffer; j++){
                #pragma hls_unroll yes
                for (int idx = 0; idx < IC0; idx++) {
                    tempdinwrite.value[idx] = 0;
                }
                #pragma hls_unroll yes
                for (int k=0; k<IC0/4; k++){
                    tempdinread = din.read();
                    tempdinwrite.value[k*4] = tempdinread.value[0];
                    tempdinwrite.value[k*4+1] = tempdinread.value[1];
                    tempdinwrite.value[k*4+2] = tempdinread.value[2];
                    tempdinwrite.value[k*4+3] = tempdinread.value[3];
                }
                
                temp.data[j] = tempdinwrite;
            }
            
            dout.write(temp);
        }
        // Your code ends here
        // -------------------------------
    }
};

template <int size, int IC0, int OC0>
class InputDoubleBufferReader{
public:
    InputDoubleBufferReader(){}

    #pragma hls_design interface
    void CCS_BLOCK(run)(ac_channel<Params> &paramsIn,
                        ac_channel<chanStruct<PackedInt<INPUT_PRECISION, IC0>,size> > &din, 
                        ac_channel<PackedInt<INPUT_PRECISION, IC0> > &dout)
    {
        // -------------------------------
        // Your code starts here
        Params params = paramsIn.read();
        int ix0 = (params.OX0 -1 )*params.STRIDE + params.FX; //calculate input width
        int iy0 = (params.OY0 -1 )*params.STRIDE + params.FY; //calculate input height
        int numberofTiles = params.OX1 * params.OY1;

        chanStruct<PackedInt<INPUT_PRECISION,IC0>,size> temp; //initialize tile struct

        for (int oy1 = 0; oy1 < params.OY1; oy1++){ //read in inputs in same input tiling as mentioned in review session week 3
            for (int ox1 = 0; ox1 < params.OX1; ox1++){
                temp = din.read(); //create new tile 
                for (int oc1 = 0; oc1 < params.OC1; oc1++){
                    for (int ic1 = 0; ic1 < params.IC1; ic1++){
                        for (int fy = 0; fy < params.FY; fy++){
                            for (int fx = 0; fx < params.FX; fx++){
                                for (int oy0 = 0; oy0 < params.OY0; oy0++){
                                    #pragma hls_pipeline_init_interval 1
                                    for (int ox0 = 0; ox0 < params.OX0; ox0++){ //skip IC0 as unrolled in inputs
                                        int iy = oy0 * params.STRIDE + fy;
                                        int ix = ox0 * params.STRIDE + fx;
                                        int buffer_add = ic1 * (iy0 * ix0) + iy * ix0 + ix; //calculate address linearly
                                        dout.write(temp.data[buffer_add]);
                                    }
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
class InputDoubleBuffer{
public:
  InputDoubleBuffer(){}

  #pragma hls_design interface
  void CCS_BLOCK(run)(ac_channel<PackedInt<INPUT_PRECISION, 4> > &inputs_in, 
                      ac_channel<PackedInt<INPUT_PRECISION, IC0> > &inputs_out,
                      ac_channel<Params> &paramsIn)
    {

        Params params = paramsIn.read();

        inputDoubleBufferReaderParams.write(params);
        inputDoubleBufferWriterParams.write(params);

        inputDoubleBufferWriter.run(inputDoubleBufferWriterParams, inputs_in, mem);

        inputDoubleBufferReader.run(inputDoubleBufferReaderParams, mem, inputs_out);
    }

private:
    ac_channel<chanStruct<PackedInt<INPUT_PRECISION, IC0>,size> > mem;
    
    InputDoubleBufferWriter<size, IC0, OC0> inputDoubleBufferWriter;
    ac_channel<Params> inputDoubleBufferWriterParams;
    
    InputDoubleBufferReader<size, IC0, OC0> inputDoubleBufferReader;
    ac_channel<Params> inputDoubleBufferReaderParams;
};

#endif


