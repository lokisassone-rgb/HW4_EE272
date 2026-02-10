set blockname [file rootname [file tail [info script] ]]

source scripts/common.tcl

directive set -DESIGN_HIERARCHY "
    {WeightDoubleBuffer<8192, ${ARRAY_DIMENSION}, ${ARRAY_DIMENSION}>} 
"

go compile

source scripts/set_libraries.tcl

go libraries
directive set -CLOCKS $clocks

go assembly

# -------------------------------
# Set the correct word widths and the stage replication
# Your code starts here
set word_width [expr ${ARRAY_DIMENSION} * 8]
set block_size ${ARRAY_DIMENSION}

directive set /WeightDoubleBuffer<8192,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/mem:cns -STAGE_REPLICATION 2
directive set /WeightDoubleBuffer<8192,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/weightDoubleBufferWriter/tmp -BLOCK_SIZE $block_size
directive set /WeightDoubleBuffer<8192,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/weightDoubleBufferReader/tmp -BLOCK_SIZE $block_size

directive set /WeightDoubleBuffer<8192,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/weightDoubleBufferReader/din -WORD_WIDTH $word_width
directive set /WeightDoubleBuffer<8192,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/weightDoubleBufferWriter/dout -WORD_WIDTH $word_width
directive set /WeightDoubleBuffer<8192,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/mem -WORD_WIDTH $word_width
directive set /WeightDoubleBuffer<8192,${ARRAY_DIMENSION},${ARRAY_DIMENSION}>/.../temp.data.value -match glob -WORD_WIDTH $word_width


# Your code ends here
# -------------------------------

go architect

go allocate
go extract
