#!/usr/bin/python2.7
from __future__ import division
from pyfaidx import Fasta
import argparse, timeit, regex, csv, sys
import timeit
import regex
import csv
import sys
import numpy as np
import pyopencl as cl
import os
import datetime
import time
import math
from operator import attrgetter
from collections import namedtuple

os.environ['PYOPENCL_COMPILER_OUTPUT'] = '1'

class CL:
    def __init__(self):
        self.ctx = cl.create_some_context(False)
        self.queue = cl.CommandQueue(self.ctx)
        self.loadOpenCL()

    def loadOpenCL(self):
        filename = "OpenCL\Kernel.cl"
        #f = open(filename, 'r')
        #fstr = "".join(f.readlines())
        with open(filename, "r") as kernel_file:
            kernel_src = kernel_file.read()
        self.program = cl.Program(self.ctx, kernel_src).build()

    def GenerateFFA(self, pattern):
        mf = cl.mem_flags
        global_size=(len(pattern),)#8388608
        local_size=(2,)

        counter = np.int32(-1)
        self.matches = np.zeros(len(pattern)).astype(np.str)
        self.stack = np.zeros(len(pattern)).astype(np.int32)

        d_pat = cl.Buffer(self.ctx, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=pattern.encode())
        counterBuffer = cl.Buffer(self.ctx, cl.mem_flags.READ_WRITE | cl.mem_flags.COPY_HOST_PTR, hostbuf=counter)
        stackBuffer = cl.Buffer(self.ctx, cl.mem_flags.READ_WRITE, self.stack.nbytes)
        self.resultsBuffer = cl.Buffer(self.ctx, cl.mem_flags.WRITE_ONLY, self.matches.nbytes)

        regex = self.program.PostRegex
        regex.set_scalar_arg_dtypes([None, None, None, None, int])
        event = regex(self.queue, global_size, local_size, d_pat, counterBuffer, stackBuffer, self.resultsBuffer, len(pattern))
        event.wait()
        #elapsed = 1e-9*(event.profile.end - event.profile.start)  # Calculate the time it took to execute the kernel
        #print("GPU Kernel Time: {0} s Data Size: {1} kb".format(elapsed, sys.getsizeof(seqText)/1024))  # Print the time it took to execute the kernel

    def execute(self):
        pattern = "(a.(b.(c)))".strip()

        self.GenerateFFA(pattern)
        self.queue.finish()
        cl.enqueue_copy(self.queue, self.matches, self.resultsBuffer)
        for match in self.matches:
            print match


def main():
    cl = CL()
    cl.execute()




if __name__ == '__main__':
    main()
