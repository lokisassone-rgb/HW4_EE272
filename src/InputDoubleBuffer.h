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
        int numberofTiles = params.OX1*params.OY1;
        int ix0 = (params.OX0 -1 )*params.STRIDE + params.FX;
        int iy0 = (params.OY0 -1 )*params.STRIDE + params.FY;
        int sizeofDoubleBuffer = ix0*iy0*params.IC1;

        chanStruct<PackedInt<INPUT_PRECISION,IC0>,size> temp;
        PackedInt<INPUT_PRECISION, 4> tempdinread;
        PackedInt<INPUT_PRECISION, IC0> tempdinwrite;

        for (int i=0; i < numberofTiles; i++){
            printf("[DEBUG] Tile i=%d\n", i);
            for (int j=1; j <= sizeofDoubleBuffer; j++){
                for (int idx = 0; idx < IC0; idx++) {
                    tempdinwrite.value[idx] = 0;
                }
                for (int k=0; k<IC0/4; k++){
                    tempdinread = din.read();
                    printf("[DEBUG] din.read() j=%d k=%d: ", j, k);
                    for (int v=0; v<4; v++) printf("%d ", (int)tempdinread.value[v]);
                    printf("\n");
                    tempdinwrite.value[k*4] = tempdinread.value[0];
                    tempdinwrite.value[k*4+1] = tempdinread.value[1];
                    tempdinwrite.value[k*4+2] = tempdinread.value[2];
                    tempdinwrite.value[k*4+3] = tempdinread.value[3];
                }
                printf("[DEBUG] tempdinwrite for j=%d: ", j);
                for (int v=0; v<IC0; v++) printf("%d ", (int)tempdinwrite.value[v]);
                printf("\n");
                temp.data[j-1] = tempdinwrite;
            }
            // Print temp.data before writing
            printf("[DEBUG] temp.data before dout.write for tile %d:\n", i);
            for (int jj=0; jj<sizeofDoubleBuffer; jj++) {
                printf("  j=%d: ", jj);
                for (int v=0; v<IC0; v++) printf("%d ", (int)temp.data[jj].value[v]);
                printf("\n");
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
        int numberofTiles = params.OX1*params.OY1;
        int ix0 = (params.OX0 -1 )*params.STRIDE + params.FX;
        int iy0 = (params.OY0 -1 )*params.STRIDE + params.FY;
        int sizeofDoubleBuffer = ix0*iy0*params.IC1;

        chanStruct<PackedInt<INPUT_PRECISION,IC0>,size> temp;

        for (int i = 0; i < numberofTiles; i++){
            temp = din.read();
            for (int j = 0; j < sizeofDoubleBuffer; j++){
                dout.write( temp.data[j] );
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
