#include "spimcore.h"

/* NOTES */
/*
  PC is index of Mem[]
  Addresses must be a multiple of 4
  Shift address right by 2 to get correct index in memory
  ALUresult is an address if MemWrite or MemRead is asserted, not just if it's divisible by 4
  Set less than has two types, unsigned and signed, so cast to int when necessary
*/

/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero){

  switch(ALUControl){
    // Z = A + B
    case 0x0:
      *ALUresult = A + B;
      break;

    // Z = A - B
    case 0x1:
      *ALUresult = A - B;
      break;

    // Z = 1 iff A < B, otherwise Z = 0 (cast to int since this slt instruction is not unsigned)
    case 0x2:
      *ALUresult = ((int)A < (int)B) ? 1 : 0;
      break;

    // Z = 1 iff A < B, otherwise Z = 0 (unsigned)
    case 0x3:
      *ALUresult = (A < B) ? 1 : 0;
      break;

    // Z = A AND B
    case 0x4:
      *ALUresult = A & B;
      break;

    // Z = A OR B
    case 0x5:
      *ALUresult = A | B;
      break;

    // Shift left B by 16 bits
    case 0x6:
      *ALUresult = B << 16;
      break;

    // Z = NOT A
    case 0x7:
      *ALUresult = ~A;
      break;
  }

  // If result is 0, set Zero to 1, or 0 otherwise
  *Zero = (*ALUresult == 0) ? 1 : 0;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction)
{
  // If word isn't aligned, send halt condition
  if(PC % 4 != 0)
    return 1;

  *instruction = Mem[PC >> 2];
  return 0;
}

/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec){
  // 1111 1100 0000 0000 0000 0000 0000 0000 --> instruction [31-26]
  *op = (instruction & 0xfc000000) >> 26;

  // 0000 0011 1110 0000 0000 0000 0000 0000 --> instruction [25-21]
  *r1 = (instruction & 0x03e00000) >> 21;

  // 0000 0000 0001 1111 0000 0000 0000 0000 --> instruction [20-16]
  *r2 = (instruction & 0x001f0000) >> 16;

  // 0000 0000 0000 0000 1111 1000 0000 0000 --> instruction [15-11]
  *r3 = (instruction & 0x0000f800) >> 11;

  // 0000 0000 0000 0000 0000 0000 0011 1111 --> instruction [5-0]
  *funct = instruction & 0x0000003f;

  // 0000 0000 0000 0000 1111 1111 1111 1111 --> instruction [15-0]
  *offset = instruction & 0x0000ffff;

  // 0000 0011 1111 1111 1111 1111 1111 1111 --> instruction [25-0]
  *jsec = instruction & 0x03ffffff;
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls){
  // Initially set controls to 0
  controls->RegDst = 0;
	controls->Jump = 0;
	controls->Branch = 0;
	controls->MemRead = 0;
	controls->MemtoReg = 0;
	controls->ALUOp = 0;
	controls->MemWrite = 0;
	controls->ALUSrc = 0;
	controls->RegWrite = 0;

  switch(op){
    // R-type instruction --> 0000 0000 [(Arithmetic) add, sub (Logic) and, or (Conditional branch) slt, sltu]
    case 0x0:
      // Destination register given by instruction [15-11] --> according to figure 2, leads to mux path 1
      controls->RegDst = 1;
      // R-type instruction means ALUOp of 111 (binary) or 7 (decimal)
      controls->ALUOp = 7;
      // Multiplexer path 1 leads to write register according to figure 2, so enable it
      controls->RegWrite = 1;
      break;

    // Add immediate --> 0000 1000
    case 0x8:
      controls->RegWrite = 1;
      controls->ALUSrc = 1;
      break;

    // Load word (lw) --> 0010 0011
    case 0x23:
      controls->RegWrite = 1;
      controls->MemRead = 1;
      controls->MemtoReg = 1;
      controls->ALUSrc = 1;
      break;

    // Store word (sw) --> 0010 1011
    case 0x2b:
      controls->MemWrite = 1;
      controls->RegDst = 2; // ?
      controls->MemtoReg = 2; // ?
      controls->ALUSrc = 1;
      break;

    // Load upper immediate (lui) --> 0000 1111
    case 0xf:
      controls->RegWrite = 1;
      // Requires upper 16 bits, so set ALU operation to shift
      controls->ALUOp = 6;
      controls->ALUSrc = 1;
      break;

    // Branch on equal (beq) --> 0000 0100
    case 0x4:
      // PC updates for branch to multiplexer path 1
      controls->Branch = 1;
      controls->RegDst = 2;
      controls->MemtoReg = 2;
      controls->ALUSrc = 1; // 2? 0?
      // Branching requires subtraction
      controls->ALUOp = 1;
      break;

    // Set less than immediate (slti) --> 0000 1010
    case 0xa:
      // Set ALU operation for 'set less than'
      controls->ALUOp = 2;
      controls->RegWrite = 1;
      controls->ALUSrc = 1;
      break;

    // Set less than immediate unsigned (sltiu) --> 0000 1011
    case 0xb:
      // Set ALU operation for 'set less than unsigned'
      controls->ALUOp = 3;
      controls->RegWrite = 1;
      controls->ALUSrc = 1;
      break;

    // Jump (j) --> 0000 0010
    case 0x2:
      controls->Jump = 1;
      controls->RegDst = 2;
      controls->Branch = 2;
      controls->MemtoReg = 2;
      controls->ALUSrc = 2;
      controls->ALUOp = 2;
      break;

    // If none of the cases apply, halt condition occurs, so return 1
    default:
      return 1;
  }
  // No halt condition = successful encoding
  return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
  // Get values of registers r1 and r2 from register array, and fill in the data
  *data1 = Reg[r1];
  *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
  // Check if offset is negative
  if((offset >> 15) == 1)
    // Fill with 1s if constant is negative
    *extended_value = offset | 0xffff0000;
  else
    // Fill with 0s if constant is positive
    *extended_value = offset & 0x0000ffff;

}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
  unsigned ALUControl = ALUOp;

  // Send halt condition if improper instruction is passed
  if(ALUOp < 0 || ALUOp > 7)
    return 1;

  // If R-type, determine instruction by funct field
  if(ALUOp == 7){
    switch(funct){
      // Add
      case 0x20:
        ALUControl = 0;
        break;
      // Subtract
      case 0x22:
        ALUControl = 1;
        break;
      // And
      case 0x24:
        ALUControl = 4;
        break;
      // Or
      case 0x25:
        ALUControl = 5;
        break;
      // Set less than
      case 0x2a:
        ALUControl = 2;
        break;
      // Set less than unsigned
      case 0x2b:
        ALUControl = 3;
        break;
      // Return a halt condition if instruction is invalid
      default:
        return 1;
    }
  }

  // If second ALU input comes from sign extend unit, set second data to the extended value
  unsigned B = (ALUSrc == 1) ? extended_value : data2;

  ALU(data1, B, ALUControl, ALUresult, Zero);

  return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
  // ALUresult is an address if MemWrite/MemRead is asserted AND divisible by 4, so check if MemWrite/MemRead is asserted first

  // Write value of data2 to memory location addressed by ALUresult
  if(MemWrite == 1){
    if(ALUresult % 4 == 0)
      Mem[ALUresult >> 2] = data2;
    else
      return 1;
  }

  // Read content of memory location addressed by ALUresult to memdata
  if(MemRead == 1){
    if(ALUresult % 4 == 0)
      *memdata = Mem[ALUresult >> 2];
    else
      return 1;
  }

  return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
  if(RegWrite == 1){
    // If information comes from memory, it is an I-type instruction and therefore has RegDst of 0
    if(MemtoReg == 1)
      Reg[r2] = memdata;

    // If information comes from a register, determine which one
    else if(MemtoReg == 0){
      // If it's an R-type instruction, write to r3
      if(RegDst == 1)
        Reg[r3] = ALUresult;
      // If an I-type instruction, write to r2
      else
        Reg[r2] = ALUresult;
    }
  }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
  // Update PC by 4
  *PC += 4;

  // If branch is taken and zero signal from ALU is received, further increment PC by the offset
  if(Branch == 1 && Zero == 1)
    *PC += (extended_value << 2);

  // If given a jump instruction, use upper 4 bits of PC and left shift bits of jsec by 2
  if(Jump == 1)
    *PC = (*PC & 0xf000000) | (jsec << 2);
}
