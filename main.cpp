#include<stdio.h>
#include<stdlib.h>

using namespace std;
using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;

struct Mem{
    static constexpr u32 MAX_MEM = 1024*64;
    Byte Data[MAX_MEM];
    
    void initialize(){
        for(u32 i=0;i<MAX_MEM;i++){
            Data[i] = 0;
        }
    }

    //read 1 byte
    Byte operator[](u32 address) const {
        //assert here address is < MAX_LEN
        return Data[address];
    }

    //Write 1 byte
    Byte& operator[](u32 address) {
        //assert here address is < MAX_LEN
        return Data[address];
    }

    void WriteWord(Word Data){
        
    }


};

struct CPU{
    Word PC;                //program counter
    Word SP;                //Stack pointer

    Byte A, X, Y;           //Registers

    Byte C:1;               //Carry Flag
    Byte Z:1;               //Zero Flag
    Byte I:1;               //Interrupt Disable
    Byte D:1;               //Decimal Mode
    Byte B:1;               //Break Command
    Byte V:1;               //Overflow Flag
    Byte N:1;               //Negetive Flag

    void Reset( Mem &memory){
        PC = 0xFFFC;
        SP = 0x0100;
        C=Z=I=B=V=D=N = 0;
        A = X = Y = 0;
        memory.initialize();
    }

    // Fetches a byte of memory into the register
    Byte FetchByte(u32 &Cycles, Mem &memory){
        Byte Data = memory[PC];
        PC++;
        Cycles--;
        return Data;
    }

    Word FetchWord(u32 &Cycles, Mem &memory){
        Word Data = memory[PC];
        PC++;
        Cycles--;

        Data |= (memory[PC] << 8);
        Cycles--;
        PC++;
        return Data;
    }

    // Reads a byte of memory
    Byte ReadByte(Byte Address, u32 &Cycles, Mem &memory){
        Byte Data = memory[Address];
        Cycles--;
        return Data;
    }

    //Sets the flags in Load Accumulator
    void LDASetStatus(){
        Z = (A==0);
        N = (A & 0b10000000) > 0;
    }

    //Opcodes
    static constexpr Byte
        INS_JMP_ABS = 0x4C,         //Jump - Absolute
        INS_LDA_IM = 0xA9,          //Load Accumulator Immediate
        INS_LDA_ZP = 0xA5,          //Load Accumulator Zero Page
        INS_LDA_ZPX = 0xB5;         //Load Accumulator Zero Page,X

    void Execute(u32 Cycles, Mem &memory){
        while(Cycles > 0){
            Byte Ins = FetchByte(Cycles, memory);
            switch(Ins){
                case INS_LDA_IM:{
                    Byte Value = FetchByte(Cycles, memory);
                    A = Value;
                    LDASetStatus();
                }break;
                case INS_LDA_ZP:{
                    Byte ZeroPageAddress = FetchByte(Cycles, memory);
                    A = ReadByte(ZeroPageAddress, Cycles, memory);
                    LDASetStatus();
                }break;
                case INS_LDA_ZPX:{
                    Byte ZeroPageAddress = FetchByte(Cycles, memory);
                    ZeroPageAddress += X;
                    Cycles--;
                    A = ReadByte(ZeroPageAddress, Cycles, memory);
                    LDASetStatus();
                }break;
                case INS_JMP_ABS:{
                    Word SubAddr = FetchWord(Cycles, memory);
                    memory[SP] = PC - 1;
                    Cycles--; 
                    PC = SubAddr;
                    Cycles--;
                }break;
                default:{
                    printf("Instruction not handled!");
                }
            }
        }
    }

};


int main(){
    //printf("Size of unsigned char: %zu bytes\n", sizeof(unsigned char));
    //printf("Size of unsigned short: %zu bytes\n", sizeof(unsigned short));

    CPU cpu;
    Mem mem;
    cpu.Reset(mem);

    mem[0xFFFC] = CPU::INS_LDA_ZP;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x84;

    cpu.Execute(3, mem);
    return 0;
}