#include <iostream>

#include <nlohmann/json.hpp>

#include "Cpu_state.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <unordered_map>


void test_x8_arithmetic()
{
    Cpu_state cpu_state;

    cpu_state.run(opcode::INC_B);
    if ((cpu_state.registers.BC>>8 != 1))
        std::cerr << "You implemented inc wrongly. Shame on you!\n";

    cpu_state.registers.BC = (1 << 8);
    cpu_state.run(opcode::DEC_B);
    if (cpu_state.registers.BC >> 8 != 0)
        std::cerr << "You implemented dec wrongly. Shame on you!\n";

    cpu_state.registers.accumulator_and_flags = (1 << 8);
    cpu_state.run(opcode::CPL);
    if (cpu_state.registers.accumulator_and_flags >> 8 != 0b11111110)
        std::cerr << "You implemented cpl wrongly. Shame on you!\n";

    cpu_state.registers.BC = (1 << 8);
    cpu_state.registers.accumulator_and_flags = 0;
    cpu_state.run(opcode::ADD_A_B);
    if (cpu_state.registers.accumulator_and_flags >> 8 != 1)
        std::cerr << "You implemented add wrongly. Shame on you!\n";

    cpu_state.registers.BC = (1 << 8);
    cpu_state.registers.accumulator_and_flags = 2 << 8;
    cpu_state.run(opcode::SUB_B);
    if (cpu_state.registers.accumulator_and_flags >> 8 != 1)
        std::cerr << "You implemented sub wrongly. Shame on you!\n";

    cpu_state.registers.BC = (1 << 8);
    cpu_state.registers.accumulator_and_flags = 5<<8;
    cpu_state.set_flags(Flags::carry);
    cpu_state.run(opcode::SBC_A_B);
    if (cpu_state.registers.accumulator_and_flags >> 8 != 3)
        std::cerr << "You implemented sbc wrongly. Shame on you!\n";

}

int main()
{
    //test_x8_arithmetic();   
   
    std::ifstream in{R"(C:\Users\Michael\Downloads\gb-test-roms-master\gb-test-roms-master\cpu_instrs\individual\06-ld r,r.gb)"};
   
    if(!in) std::cerr<<"Failed to load file\n";

    Cpu_state cpu_state;
    {
        int pos = 0x0;
        while(in.read(reinterpret_cast<char*>(&cpu_state.memory[pos++]),1));
    }    

    cpu_state.registers.program_counter = 0x100;
    int i = 0;
    while(true)
    {
        //std::cout<<std::hex<<"Instruction at address "<<cpu_state.registers.program_counter<<" is "<<static_cast<int>(cpu_state.memory[cpu_state.registers.program_counter])<<'\n';
        cpu_state.run(opcode{ cpu_state.memory[cpu_state.registers.program_counter++] });

        if(i==0)
            std::cin>>i;
        else
            --i;
        //char c;
        //std::cin>>c;

    }
}
