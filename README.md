# DAC-CPU

About
--------
The Dark Assassin Core CPU (DAC-CPU) is a CPU inspired
the ARM architecture. This repo simulates how a the CPU works
and interacts with memory. Example programs written in its 
assembly to test the functionality can be found in the
<code>example_programs</code> folder of 
this repo.


Usage
--------
Once you build the executable with the provided makefile, simply pass the 
<code>.dasm</code> file of your choice as an argument
```bash
./dac-cpu example_programs/hello_world.dasm
```

To see what is going on under the hood and in each register and at each
memory address, add the -v (verbose) flag
```bash
./dac-cpu example_programs/example1.dasm
```
