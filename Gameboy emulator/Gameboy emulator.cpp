#include <iostream>

#include <nlohmann/json.hpp>

#include "Cpu_state.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <unordered_map>


/*
void manipulate_json()
{
    using json = nlohmann::json;
    std::ifstream json_file{ R"(C:\Users\Michael\source\repos\Gameboy emulator\Gameboy emulator\opcodes.json)" };
    json opcode_data = json::parse(json_file);

    std::unordered_map<std::string,std::ofstream> group_files;

    const auto remove_quotes = [](std::string name)
    {
        return name.substr(1,name.size()-2);
    };

    const auto encode_parens = [](std::string name)
    {
        if(name.back()==')')
            return "i"+name.substr(1,name.size()-2);
        return name;
    };

    const auto encode_plus = [](std::string name)
    {
        if(name.back()=='+')
            return name.substr(0,name.size()-1)+"inc";
        return name;
    };

    const auto encode_operand = [&](auto op)
    {
        return encode_plus(encode_parens(remove_quotes(to_string(op)))); 
    };

    for (const auto& [key, value] : opcode_data["unprefixed"].items())
    {
        if (group_files.count(value["group"]) == 0)
        {
            auto filename = to_string(value["group"]);
            for (auto& c : filename)
                if (c == '/') c = '_';

            group_files[value["group"]] = std::ofstream{ filename.substr(1,filename.size() - 2) + "_switch.txt" };

            group_files[value["group"]]<<"switch(instruction)\n{\n";
        }

        auto enumerator_name = remove_quotes(to_string(value["mnemonic"]));
        if(value.contains("operand1"))
            enumerator_name+="_"+encode_operand(value["operand1"]);
        if(value.contains("operand2"))
            enumerator_name+="_"+encode_operand(value["operand2"]);

        group_files[value["group"]] << "\tcase opcode::" << enumerator_name <<":\n\t{\n\t\tbreak;\n\t}\n";
    }

    for(auto& [_,file]: group_files)
    {
        file<<'}\n';
    }
}
*/


void test_x8_arithmetic()
{
    Cpu_state cpu_state;

    cpu_state.run(opcode::INC_B);

    if ((cpu_state.registers.BC>>8 != 1))
        std::cerr << "You implemented inc wrongly. Shame on you!\n";

    cpu_state.registers.BC = (255<<8);
    cpu_state.run(opcode::INC_B);

    if ((cpu_state.registers.BC>>8 != 0))
        std::cerr << "You implemented inc wrongly. Shame on you!\n";

    cpu_state.registers.BC = 0;
    cpu_state.run(opcode::DEC_B);
    if (cpu_state.registers.BC >> 8 != 255)
        std::cerr << "You implemented inc wrongly. Shame on you!\n";
}

int main()
{
    test_x8_arithmetic();   
   
}
