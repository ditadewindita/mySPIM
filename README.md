# mySPIM
  Computer Logic and Organization (CDA3103C) Spring 2017 Project
  
Author/Team member(s): Haerunnisa Dewindita

MySPIM is a mini processor simulator that demonstrates some of the functions of a MIPS processor. MySPIM reads in MIPS machine
codes from an .asc file and executes the instructions, as well as providing several commands to showcase the process. This simulator 
exhibits a single-cycle datapath and the various control signals of the original MIPS processor.

The MIPS pipeline is outlined as so:
  1. **Instruction Fetch** - Fetch instruction from memory
  2. **Instruction Decode** - Read registers while decoding instruction
  3. **Execution** - Perform operation or calculate an address
  4. **Memory Access** - Access an operand in data memory
  5. **Write Back** - Write results to a register
  
MySPIM handles 32 general purpose registers, has a 64kB memory size containing addresses 0x0000 through 0xffff, assumes all programs
start at memory location 0x4000, and halts when coming across illegal instructions (such as unaligned words).

## How to Use
Clone repository to use, then compile and run:

```
gcc -o spimcore spimcore.c project.c && ./spimcore <input_file_name>.asc 
```

To run script, use the command:

```
bash run-spim-tests.sh
```

MySPIM takes several commands during execution that allows it to work like a debugger. The commands are listed below:

| Command | Description |
| --- | --- |
| `r`    | dumps register contents |
| `m`    | dumps memory contents |
| `s[n]` | step to simulate the next *n* instruction(s), if *n* is not typed, 1 step is assumed |
| `c`    | continue to step until program halts |
| `h`    | check if program has halted |
| `d [adds1] [adds2]` | hexadecimal dump from adds1 to adds2 |
| `i` | print memory size |
| `p` | print input file |
| `g` | display all control signals |
| `q` | quit/end simulator |

## Credits
Project already includes *spimcore.c*, *spimcore.h*, and *input_file.asc*.
<br>This repository includes a script to run three test cases through the mySPIM simulator, credit to this <a href="https://github.com/gazingatnavel/ucf-spring2017-cda3103-spim-project-tests">repository</a>.
