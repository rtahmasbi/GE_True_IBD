# GE True IBD
This programs calculats the shared true IBD segments based on GeneEvolve output files.

There's a makefile as well. The log file with be formed automatically with the name <inputfilename>.log


# Flags used:
    --in <filename.int>
    --out <somefilename>
    --min_bp 100000


# To compile:
    make all


# To run:
    ./GE_True_IBD --in out1.pop1.gen20.chr10.int --out testoutput --min_bp 100000

