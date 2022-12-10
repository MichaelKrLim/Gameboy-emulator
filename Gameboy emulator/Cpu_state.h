#include "opcode.h"

#include <array>
#include <cstdint>

enum class Flags
{
    zero = 1 << 7,
    subtraction = 1 << 6,
    half_carry = 1 << 5,
    carry = 1<<4
};

struct Registers
{
    std::uint16_t accumulator_and_flags{};
    std::uint16_t BC{};
    std::uint16_t DE{};
    std::uint16_t HL{};
    std::uint16_t stack_pointer{};
    std::uint16_t program_counter{};
};

struct Cpu_state
{

public:
    Registers registers;
    static constexpr size_t memory_size = 65536;
    std::array<std::uint8_t, memory_size> memory;

    void run(opcode instruction)
    {
        x8_Rotate_and_Shift_Bits(instruction);
        x8_Arithmetic_Logic_Unit(instruction);
        control(instruction);
        x16_Load_Store_Move(instruction);
        x8_Load_Store_Move(instruction);
    }

//private:

    void x8_Rotate_and_Shift_Bits(opcode instruction)
    {
        switch (instruction)
        {
            case opcode::RLCA:
            {  
                auto A = get_upper(registers.accumulator_and_flags);

                if ((A & 0b10000000) > 0)
                {
                    set_flags(Flags::carry);
                }
                else
                    unset_flags(Flags::carry);

                set_upper(registers.accumulator_and_flags, (A << 1) | (A >> 7));
                break;
            }
            case opcode::RRCA:
            {
                auto A = get_upper(registers.accumulator_and_flags);

                if ((A & 1) == 1)
                {
                    set_flags(Flags::carry);
                }
                else
                    unset_flags(Flags::carry);

                set_upper(registers.accumulator_and_flags, (A >> 1) | (A << 7));
                break;
            }
            case opcode::RLA:                                      
            {                                                            
                auto A = get_upper(registers.accumulator_and_flags);
                auto past_carry = is_flag_set(Flags::carry) ? 1 : 0;

                if ((A & 0b10000000) > 0)
                {
                    set_flags(Flags::carry);
                }
                else
                    unset_flags(Flags::carry);

                set_upper(registers.accumulator_and_flags, (A << 1) | (past_carry));
                break;                                                            
            }                                                                 
            case opcode::RRA:       
            {
                auto A = get_upper(registers.accumulator_and_flags);
                auto past_carry = is_flag_set(Flags::carry) ? 1 : 0;

                if((A&1) == 1)
                {
                    set_flags(Flags::carry);
                }
                else 
                    unset_flags(Flags::carry);
                    
                set_upper(registers.accumulator_and_flags, (A>>1) | (past_carry << 7));
                break;
            }
        }
    }

    void push_to_stack(uint16_t val)
    {
        const auto f = (val & 0xFF00)>>8;
        const auto s = (val & 0x00FF);
        --registers.stack_pointer;
        write_to_memory(registers.stack_pointer,f);
        --registers.stack_pointer;
        write_to_memory(registers.stack_pointer,s);
        //--(--registers.stack_pointer); // -=2 --(*reinterpret_cast<&uint16_t>(reinterpret_cast<void *>));
    }
 
    uint16_t pop_from_stack()
    {
        std::uint16_t lower = read_from_memory(registers.stack_pointer);
        std::uint16_t higher = read_from_memory(++registers.stack_pointer);

        return (higher<<8) | lower;
    }

    std::uint16_t read_16b_value()
    {
        const int lower = read_from_memory(registers.program_counter++);
        const int upper = read_from_memory(registers.program_counter++);

        return lower | (upper << 8);
    }

    void compare_flags(uint8_t result, uint8_t decrement, uint8_t reg)
    {
        if (result == 0) set_flags(Flags::zero);
        set_flags(Flags::subtraction);
        if ((get_upper(registers.accumulator_and_flags) & 0xf) > (decrement & 0xf))
            set_flags(Flags::half_carry);
        if (reg > decrement) set_flags(Flags::carry);
    }

    void logically_compare_accumulator(uint8_t reg)
    {
        auto accumulator = get_upper(registers.accumulator_and_flags);
        uint8_t result = accumulator - reg;
        compare_flags(result, reg, accumulator);
    }

    void logically_or_accumulator(uint8_t reg)
    {
        uint8_t A = get_upper(registers.accumulator_and_flags) | reg;
        registers.accumulator_and_flags = set_upper(registers.accumulator_and_flags, A);
    }

    void logically_xor_accumulator(uint8_t reg)
    {
        uint8_t A = get_upper(registers.accumulator_and_flags) ^ reg; 
        registers.accumulator_and_flags = set_upper(registers.accumulator_and_flags, A);
    }

    void logically_and_accumulator(uint8_t reg)
    {
        uint8_t A = get_upper(registers.accumulator_and_flags) & reg;
        registers.accumulator_and_flags = set_upper(registers.accumulator_and_flags, A);
    }

    void decrease_accumulator(uint8_t decrement)
    {
        uint8_t A = get_upper(registers.accumulator_and_flags) - decrement;
        registers.accumulator_and_flags = set_upper(registers.accumulator_and_flags, A);
    }

    void increase_accumulator(uint8_t increment)
    {
        uint8_t A = get_upper(registers.accumulator_and_flags) + increment;
        registers.accumulator_and_flags = set_upper(registers.accumulator_and_flags,A);
    }

    bool is_flag_set(Flags flag)
    {
        return (registers.accumulator_and_flags& static_cast<int> (flag)) != 0; 
    }
    
    void set_flags(Flags flags)
    {
        registers.accumulator_and_flags |= static_cast<int> (flags);
    }

    void invert_flag(Flags flags)
    {
        registers.accumulator_and_flags ^= static_cast<int> (flags);
    }

    void unset_flags(Flags flags)
    {
        registers.accumulator_and_flags &= ~static_cast<int> (flags);
    }

    uint8_t get_lower(uint16_t register_)
    {
        return register_ & 0b0000000011111111;
    }

    uint8_t get_upper(uint16_t register_)
    {
        return register_ >> 8;
    }

    uint16_t set_upper(uint16_t register_, uint8_t upper)
    {
        register_ &= 0b0000000011111111;
        return register_ + (upper << 8);
    }

    uint16_t set_lower(uint16_t register_, uint8_t lower)
    {
        register_ &= 0b1111111100000000;
        return register_ + lower;
    }

    uint16_t increment_upper(uint16_t register_)
    {
        uint8_t upper = get_upper(register_);
        ++upper;
        if (upper == 0) set_flags(Flags::zero);
        else unset_flags(Flags::zero);
        return set_upper(register_,upper);
    }

    uint16_t increment_lower(uint16_t register_)
    {
        uint8_t lower = get_lower(register_);
        ++lower;
        if (lower == 0) set_flags(Flags::zero);
        else unset_flags(Flags::zero);
        return set_lower(register_, lower);
    }

    uint16_t decrement_upper(uint16_t register_)
    {
        uint8_t upper = get_upper(register_);
        --upper;
        if (upper == 0) set_flags(Flags::zero);
        else unset_flags(Flags::zero);
        return set_upper(register_, upper);
    }

    uint16_t decrement_lower(uint16_t register_)
    {
        uint8_t lower = get_lower(register_);
        --lower;
        if (lower == 0) set_flags(Flags::zero);
        else unset_flags(Flags::zero);
        return set_lower(register_, lower);
    }

    void check_and_toggle_z_flag()
    {
        if (get_upper(registers.accumulator_and_flags) == 0)
            set_flags(Flags::zero);
        else
        {
            unset_flags(Flags::zero);
        }
    }

    void subtract_with_carry(uint8_t val)
    {
        auto decrement = val + is_flag_set(Flags::carry);
        if (get_upper(registers.accumulator_and_flags) > decrement)
            set_flags(Flags::carry);
        else
            unset_flags(Flags::carry);

        if ((get_upper(registers.accumulator_and_flags) & 0xf) > (decrement & 0xf))
            set_flags(Flags::half_carry);
        else
            unset_flags(Flags::half_carry);

        decrease_accumulator(decrement);

        check_and_toggle_z_flag();
        set_flags(Flags::subtraction);
    }

    void and_flags()
    {
        check_and_toggle_z_flag();
        unset_flags(Flags::subtraction);
        unset_flags(Flags::carry);
        set_flags(Flags::half_carry);
    }

    void xor_flags()
    {
        check_and_toggle_z_flag();
        unset_flags(Flags::carry);
        unset_flags(Flags::half_carry);
        unset_flags(Flags::subtraction);
    }

    void or_flags()
    {
        check_and_toggle_z_flag();
        unset_flags(Flags::carry);
        unset_flags(Flags::half_carry);
        unset_flags(Flags::subtraction);
    }

    void x8_Arithmetic_Logic_Unit(opcode instruction)
    {
        switch (instruction)
        {
            case opcode::INC_B:
            {
                registers.BC = increment_upper(registers.BC);
                break;
            }
            case opcode::DEC_B:
            {
                registers.BC = decrement_upper(registers.BC);
                break;
            }
            case opcode::INC_C:
            {
                
                registers.BC = increment_lower(registers.BC);
                break;
            }
            case opcode::DEC_C:
            {
                registers.BC = decrement_lower(registers.BC);
                break;
            }
            case opcode::INC_D:
            {
                registers.DE = increment_upper(registers.DE);
                break;
            }
            case opcode::DEC_D:
            {
                registers.DE = decrement_upper(registers.DE);
                break;
            }
            case opcode::INC_E:
            {
                registers.DE = increment_lower(registers.DE);
                break;
            }
            case opcode::DEC_E:
            {
                registers.DE = decrement_lower(registers.DE);
                break;
            }
            case opcode::INC_H:
            {
                registers.HL = increment_upper(registers.HL);
                break;
            }
            case opcode::DEC_H:
            {
                registers.HL = decrement_upper(registers.HL);
                break;
            }
            case opcode::DAA:
            {

                break;
            }
            case opcode::INC_L:
            {
                registers.HL = increment_lower(registers.HL);
                break;
            }
            case opcode::DEC_L:
            {
                registers.HL = decrement_lower(registers.HL);
                break;
            }
            case opcode::CPL:
            {
                uint8_t accumulator = get_upper(registers.accumulator_and_flags);
                registers.accumulator_and_flags = set_upper(registers.accumulator_and_flags,~accumulator);
                set_flags(Flags::subtraction);
                set_flags(Flags::half_carry);
                break;
            }
            case opcode::INC_iHL:
            {
                memory[registers.HL] += 1;
                break;
            }
            case opcode::DEC_iHL:
            {
                memory[registers.HL] -= 1;
                break;
            }
            case opcode::SCF:
            {
                set_flags(Flags::carry);
                unset_flags(Flags::subtraction);
                unset_flags(Flags::half_carry);
                break;
            }
            case opcode::INC_A:
            {
                registers.accumulator_and_flags = increment_upper(registers.accumulator_and_flags);
                break;
            }
            case opcode::DEC_A:
            {
                registers.accumulator_and_flags = decrement_upper(registers.accumulator_and_flags);
                break;
            }
            case opcode::CCF:
            {
                invert_flag(Flags::carry);
                unset_flags(Flags::half_carry);
                unset_flags(Flags::subtraction);
                break;
            }
            case opcode::ADD_A_B:
            {
                increase_accumulator(get_upper(registers.BC));
                break;
            }
            case opcode::ADD_A_C:
            {
                increase_accumulator(get_lower(registers.BC));
                break;//break;break;break;break;break;break;break;break;break;break;break;break;break;break;break;break;break;break;break;break;break;break;break;
            }
            case opcode::ADD_A_D:
            {
                increase_accumulator(get_upper(registers.DE));
                break;
            }
            case opcode::ADD_A_E:
            {
                increase_accumulator(get_lower(registers.DE));
                break;
            }
            case opcode::ADD_A_H:
            {
                increase_accumulator(get_upper(registers.HL));
                break;
            }
            case opcode::ADD_A_L:
            {
                increase_accumulator(get_lower(registers.HL));
                break;
            }
            case opcode::ADD_A_iHL:
            {
                increase_accumulator(memory[registers.HL]);
                break;
            }
            case opcode::ADD_A_A:
            {
                increase_accumulator(get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::ADC_A_B:
            {
                std::uint8_t to_add = get_upper(registers.BC);
                if(is_flag_set(Flags::carry))
                    ++to_add;
                increase_accumulator(to_add);
                break;
            }
            case opcode::ADC_A_C:
            {
                std::uint8_t to_add = get_lower(registers.BC);
                if (is_flag_set(Flags::carry))
                    ++to_add;
                increase_accumulator(to_add);
                break;
            }
            case opcode::ADC_A_D:
            {
                std::uint8_t to_add = get_upper(registers.DE);
                if (is_flag_set(Flags::carry))
                    ++to_add;
                increase_accumulator(to_add);
                break;
            }
            case opcode::ADC_A_E:
            {
                std::uint8_t to_add = get_lower(registers.DE);
                if (is_flag_set(Flags::carry))
                    ++to_add;
                increase_accumulator(to_add);
                break;
            }
            case opcode::ADC_A_H:
            {
                std::uint8_t to_add = get_upper(registers.HL);
                if (is_flag_set(Flags::carry))
                    ++to_add;
                increase_accumulator(to_add);
                break;
            }
            case opcode::ADC_A_L:
            {
                std::uint8_t to_add = get_lower(registers.HL);
                if (is_flag_set(Flags::carry))
                    ++to_add;
                increase_accumulator(to_add);
                break;
            }
            case opcode::ADC_A_iHL:
            {
                std::uint8_t to_add = memory[registers.HL];
                if (is_flag_set(Flags::carry))
                    ++to_add;
                increase_accumulator(to_add);
                break;
            }
            case opcode::ADC_A_A:
            {
                std::uint8_t to_add = get_upper(registers.accumulator_and_flags);
                if (is_flag_set(Flags::carry))
                    ++to_add;
                increase_accumulator(to_add);
                break;
            }
            case opcode::SUB_B:
            {
                decrease_accumulator(get_upper(registers.BC));
                break;
            }
            case opcode::SUB_C:
            {
                decrease_accumulator(get_lower(registers.BC));
                break;
            }
            case opcode::SUB_D:
            {
                decrease_accumulator(get_upper(registers.DE));
                break;
            }
            case opcode::SUB_E:
            {
                decrease_accumulator(get_lower(registers.DE));
                break;
            }
            case opcode::SUB_H:
            {
                decrease_accumulator(get_upper(registers.HL));
                break;
            }
            case opcode::SUB_L:
            {
                decrease_accumulator(get_lower(registers.HL));
                break;
            }
            case opcode::SUB_iHL:
            {
                decrease_accumulator(memory[registers.HL]);
                break;
            }
            case opcode::SUB_A:
            {
                decrease_accumulator(get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::SBC_A_B:
            {
                subtract_with_carry(get_upper(registers.BC));
                break;
            }
            case opcode::SBC_A_C:
            {
                subtract_with_carry(get_lower(registers.BC));
                break;
            }
            case opcode::SBC_A_D:
            {
                subtract_with_carry(get_upper(registers.DE));
                break;
            }
            case opcode::SBC_A_E:
            {
                subtract_with_carry(get_lower(registers.DE));
                break;
            }
            case opcode::SBC_A_H:
            {
                subtract_with_carry(get_upper(registers.HL));
                break;
            }
            case opcode::SBC_A_L:
            {
                subtract_with_carry(get_lower(registers.HL));
                break;
            }
            case opcode::SBC_A_iHL:
            {
                subtract_with_carry(memory[registers.HL]);
                break;
            }
            case opcode::SBC_A_A:
            {
                subtract_with_carry(get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::AND_B:
            {
                logically_and_accumulator(get_upper(registers.BC));
                and_flags();
                break;
            }
            case opcode::AND_C:
            {
                logically_and_accumulator(get_lower(registers.BC));
                and_flags();
                break;
            }
            case opcode::AND_D:
            {
                logically_and_accumulator(get_upper(registers.DE));
                and_flags();
                break;
            }
            case opcode::AND_E:
            {
                logically_and_accumulator(get_lower(registers.DE));
                and_flags();
                break;
            }
            case opcode::AND_H:
            {
                logically_and_accumulator(get_upper(registers.HL));
                and_flags();
                break;
            }
            case opcode::AND_L:
            {
                logically_and_accumulator(get_lower(registers.HL));
                and_flags();
                break;
            }
            case opcode::AND_iHL:
            {
                logically_and_accumulator(memory[registers.HL]);
                and_flags();
                break;
            }
            case opcode::AND_A:
            {
                logically_and_accumulator(get_upper(registers.accumulator_and_flags));
                and_flags();
                break;
            }
            case opcode::XOR_B:
            {
                logically_xor_accumulator(get_upper(registers.BC));
                xor_flags();
                break;
            }
            case opcode::XOR_C:
            {
                logically_xor_accumulator(get_lower(registers.BC));
                xor_flags();
                break;
            }
            case opcode::XOR_D:
            {
                logically_xor_accumulator(get_upper(registers.DE));
                xor_flags();
                break;
            }
            case opcode::XOR_E:
            {
                logically_xor_accumulator(get_lower(registers.DE));
                xor_flags();
                break;
            }
            case opcode::XOR_H:
            {
                logically_xor_accumulator(get_upper(registers.HL));
                xor_flags();
                break;
            }
            case opcode::XOR_L:
            {
                logically_xor_accumulator(get_lower(registers.HL));
                xor_flags();
                break;
            }
            case opcode::XOR_iHL:
            {
                logically_xor_accumulator(memory[registers.HL]);
                xor_flags();
                break;
            }
            case opcode::XOR_A:
            {
                logically_xor_accumulator(get_upper(registers.accumulator_and_flags));
                xor_flags();
                break;
            }
            case opcode::OR_B:
            {
                logically_or_accumulator(get_upper(registers.BC));
                or_flags();
                break;
            }
            case opcode::OR_C:
            {
                logically_or_accumulator(get_lower(registers.BC));
                or_flags();
                break;
            }
            case opcode::OR_D:
            {
                logically_or_accumulator(get_upper(registers.DE));
                or_flags();
                break;
            }
            case opcode::OR_E:
            {
                logically_or_accumulator(get_lower(registers.DE));
                or_flags();
                break;
            }
            case opcode::OR_H:
            {
                logically_or_accumulator(get_upper(registers.HL));
                or_flags();
                break;
            }
            case opcode::OR_L:
            {
                logically_or_accumulator(get_lower(registers.HL));
                or_flags();
                break;
            }
            case opcode::OR_iHL:
            {
                logically_or_accumulator(memory[registers.HL]);
                or_flags();
                break;
            }
            case opcode::OR_A:
            {
                logically_or_accumulator(get_upper(registers.accumulator_and_flags));
                or_flags();
                break;
            }
            case opcode::CP_B:
            {
                logically_compare_accumulator(get_upper(registers.BC));
                break;
            }
            case opcode::CP_C:
            {
                logically_compare_accumulator(get_lower(registers.BC));
                break;
            }
            case opcode::CP_D:
            {
                logically_compare_accumulator(get_upper(registers.DE));
                break;
            }
            case opcode::CP_E:
            {
                logically_compare_accumulator(get_lower(registers.DE));
                break;
            }
            case opcode::CP_H:
            {
                logically_compare_accumulator(get_upper(registers.HL));
                break;
            }
            case opcode::CP_L:
            {
                logically_compare_accumulator(get_lower(registers.HL));
                break;
            }
            case opcode::CP_iHL:
            {
                logically_compare_accumulator(memory[registers.HL]);
                break;
            }
            case opcode::CP_A:
            {
                logically_compare_accumulator(get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::ADD_A_d8:
            {
                increase_accumulator(memory[registers.program_counter]);
                ++registers.program_counter;
                break;
            }
            case opcode::ADC_A_d8:
            {
                std::uint8_t to_add = memory[registers.program_counter];
                if (is_flag_set(Flags::carry))
                    ++to_add;
                increase_accumulator(to_add);
                ++registers.program_counter;
                break;
            }
            case opcode::SUB_d8:
            {
                decrease_accumulator(memory[registers.program_counter]);
                ++registers.program_counter;
                break;
            }
            case opcode::SBC_A_d8:
            {
                subtract_with_carry(memory[registers.program_counter]);
                ++registers.program_counter;
                break;
            }
            case opcode::AND_d8:
            {
                logically_and_accumulator(memory[registers.program_counter]);
                and_flags();
                ++registers.program_counter;
                break;
            }
            case opcode::XOR_d8:
            {
                logically_xor_accumulator(memory[registers.program_counter]);
                xor_flags();
                ++registers.program_counter;
                break;
            }
            case opcode::OR_d8:
            {
                logically_or_accumulator(memory[registers.program_counter]);
                or_flags();
                ++registers.program_counter;
                break;
            }
            case opcode::CP_d8:
            {
                logically_compare_accumulator(memory[registers.program_counter]);
                ++registers.program_counter;
                break;
            }
        }
    }

    void write_to_memory(std::uint16_t address, uint8_t value)
    { 
        if (address == 0x0FF01)
            std::cout << static_cast<char> (value);
        else
            memory[address] = value;
    }
    std::uint8_t read_from_memory(std::uint16_t address)
    {
        return memory[address];
    }

    void x8_Load_Store_Move(opcode instruction)
    {
        switch (instruction)
        {
            case opcode::LD_iBC_A:
            {
                write_to_memory(registers.BC, get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LD_B_d8:
            {
                set_upper(registers.BC,read_from_memory(registers.program_counter));
                ++registers.program_counter;
                break;
            }
            case opcode::LD_A_iBC:
            {
                set_upper(registers.accumulator_and_flags, read_from_memory(registers.BC));
                break;
            }
            case opcode::LD_C_d8:
            {
                set_lower(registers.BC,read_from_memory(registers.program_counter));
                ++registers.program_counter;
                break;
            }
            case opcode::LD_iDE_A:
            {
                write_to_memory(registers.DE, get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LD_D_d8:
            {
                set_upper(registers.DE,read_from_memory(registers.program_counter));
                ++registers.program_counter;
                break;
            }
            case opcode::LD_A_iDE:
            {
                set_upper(registers.accumulator_and_flags, read_from_memory(registers.DE));
                break;
            }
            case opcode::LD_E_d8:
            {
                set_lower(registers.DE,read_from_memory(registers.program_counter));
                ++registers.program_counter;
                break;
            }
            case opcode::LD_iHLinc_A:
            {
                write_to_memory(registers.HL, get_upper(registers.accumulator_and_flags));
                ++registers.HL;
                break;
            }
            case opcode::LD_H_d8:
            {
                set_upper(registers.HL,read_from_memory(registers.program_counter));
                ++registers.program_counter;
                break;
            }
            case opcode::LD_A_iHLinc:
            {
                set_upper(registers.accumulator_and_flags, read_from_memory(registers.HL));
                ++registers.HL;
                break;
            }
            case opcode::LD_L_d8:
            {
                set_lower(registers.HL, read_from_memory(registers.program_counter));
                ++registers.program_counter;
                break;
            }
            case opcode::LD_iHLdec_A:
            {
                write_to_memory(registers.HL, get_upper(registers.accumulator_and_flags));
                --registers.HL;
                break;
            }
            case opcode::LD_iHL_d8:
            {
                write_to_memory(registers.HL, get_upper(registers.accumulator_and_flags));
                ++registers.program_counter;
                break;
            }
            case opcode::LD_A_iHLdec:
            {
                set_upper(registers.accumulator_and_flags, read_from_memory(registers.HL));
                --registers.HL;
                break;
            }
            case opcode::LD_A_d8:
            {
                set_upper(registers.accumulator_and_flags, read_from_memory(registers.program_counter));
                ++registers.program_counter;
                break;
            }
            case opcode::LD_B_B:
            {
                //done
                break;
            }
            case opcode::LD_B_C:
            {
                set_upper(registers.BC, get_lower(registers.BC));
                break;
            }
            case opcode::LD_B_D:
            {
                set_upper(registers.BC,get_upper(registers.DE));
                break;
            }
            case opcode::LD_B_E:
            {
                set_upper(registers.BC,get_lower(registers.DE));
                break;
            }
            case opcode::LD_B_H:
            {
                set_upper(registers.BC,get_upper(registers.HL));
                break;
            }
            case opcode::LD_B_L:
            {
                set_upper(registers.BC, get_lower(registers.HL));
                break;
            }
            case opcode::LD_B_iHL:
            {
                set_upper(registers.BC,read_from_memory(registers.HL));
                break;
            }
            case opcode::LD_B_A:
            {
                set_upper(registers.BC,get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LD_C_B:
            {
                set_lower(registers.BC,get_upper(registers.BC));
                break;
            }
            case opcode::LD_C_C:
            {
                //done
                break;
            }
            case opcode::LD_C_D:
            {
                set_lower(registers.BC, get_upper(registers.DE));
                break;
            }
            case opcode::LD_C_E:
            {
                set_lower(registers.BC,get_lower(registers.DE));
                break;
            }
            case opcode::LD_C_H:
            {
                set_lower(registers.BC,get_upper(registers.HL));
                break;
            }
            case opcode::LD_C_L:
            {
                set_lower(registers.BC,get_lower(registers.HL));
                break;
            }
            case opcode::LD_C_iHL:
            {
                set_lower(registers.BC, memory[registers.HL]);
                break;
            }
            case opcode::LD_C_A:
            {
                set_lower(registers.BC,get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LD_D_B:
            {
                set_upper(registers.DE,get_upper(registers.BC));
                break;
            }
            case opcode::LD_D_C:
            {
                set_upper(registers.DE, get_lower(registers.BC));
                break;
            }
            case opcode::LD_D_D:
            {
                //done
                break;
            }
            case opcode::LD_D_E:
            {
                set_upper(registers.DE,get_lower(registers.DE));
                break;
            }
            case opcode::LD_D_H:
            {
                set_upper(registers.DE,get_upper(registers.HL));
                break;
            }
            case opcode::LD_D_L:
            {
                set_upper(registers.DE,get_lower(registers.HL));
                break;
            }
            case opcode::LD_D_iHL:
            {
                set_upper(registers.DE,read_from_memory(registers.HL));
                break;
            }
            case opcode::LD_D_A:
            {
                set_upper(registers.DE, get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LD_E_B:
            {
                set_lower(registers.DE, get_upper(registers.BC));
                break;
            }
            case opcode::LD_E_C:
            {
                break;
            }
            case opcode::LD_E_D:
            {
                set_lower(registers.DE,get_upper(registers.DE));
                break;
            }
            case opcode::LD_E_E:
            {
                //done
                break;
            }
            case opcode::LD_E_H:
            {
                set_lower(registers.DE,get_upper(registers.HL));
                break;
            }
            case opcode::LD_E_L:
            {
                set_lower(registers.DE,get_lower(registers.HL));
                break;
            }
            case opcode::LD_E_iHL:
            {
                set_lower(registers.DE,get_lower(registers.DE));
                break;
            }
            case opcode::LD_E_A:
            {
                set_lower(registers.DE,get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LD_H_B:
            {
                set_upper(registers.HL,get_upper(registers.BC));
                break;
            }
            case opcode::LD_H_C:
            {
                set_upper(registers.HL,get_lower(registers.BC));
                break;
            }
            case opcode::LD_H_D:
            {
                set_upper(registers.HL,get_upper(registers.DE));
                break;
            }
            case opcode::LD_H_E:
            {
                set_upper(registers.HL,get_lower(registers.DE));
                break;
            }
            case opcode::LD_H_H:
            {
                //done
                break;
            }
            case opcode::LD_H_L:
            {
                set_upper(registers.HL, get_lower(registers.HL));
                break;
            }
            case opcode::LD_H_iHL:
            {
                set_upper(registers.HL, memory[registers.HL]);
                break;
            }
            case opcode::LD_H_A:
            {
                set_upper(registers.HL, get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LD_L_B:
            {
                set_lower(registers.HL,get_upper(registers.BC));
                break;
            }
            case opcode::LD_L_C:
            {
                set_lower(registers.HL,get_lower(registers.BC));
                break;
            }
            case opcode::LD_L_D:
            {
                set_lower(registers.HL,get_upper(registers.DE));
                break;
            }
            case opcode::LD_L_E:
            {
                set_lower(registers.HL,get_lower(registers.DE));
                break;
            }
            case opcode::LD_L_H:
            {
                set_lower(registers.HL,get_upper(registers.HL));
                break;
            }
            case opcode::LD_L_L:
            {
                //done
                break;
            }
            case opcode::LD_L_iHL:
            {
                set_lower(registers.HL,read_from_memory(registers.HL));
                break;
            }
            case opcode::LD_L_A:
            {
                set_lower(registers.HL,get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LD_iHL_B:
            {
                write_to_memory(registers.HL,get_upper(registers.BC));
                break;
            }
            case opcode::LD_iHL_C:
            {
                write_to_memory(registers.HL,get_lower(registers.BC));
                break;
            }
            case opcode::LD_iHL_D:
            {
                write_to_memory(registers.HL,get_upper(registers.DE));
                break;
            }
            case opcode::LD_iHL_E:
            {
                write_to_memory(registers.HL, get_lower(registers.DE));
                break;
            }
            case opcode::LD_iHL_H:
            {
                write_to_memory(registers.HL, get_upper(registers.HL));
                break;
            }
            case opcode::LD_iHL_L:
            {
                write_to_memory(registers.HL, get_lower(registers.HL));
                break;
            }
            case opcode::LD_iHL_A:
            {
                write_to_memory(registers.HL, get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LD_A_B:
            {
                set_upper(registers.accumulator_and_flags,get_upper(registers.BC));
                break;
            }
            case opcode::LD_A_C:
            {
                set_upper(registers.accumulator_and_flags,get_lower(registers.BC));
                break;
            }
            case opcode::LD_A_D:
            {
                set_upper(registers.accumulator_and_flags,get_upper(registers.DE));
                break;
            }
            case opcode::LD_A_E:
            {
                set_upper(registers.accumulator_and_flags,get_lower(registers.DE));
                break;
            }
            case opcode::LD_A_H:
            {
                set_upper(registers.accumulator_and_flags,get_upper(registers.HL));
                break;
            }
            case opcode::LD_A_L:
            {
                set_upper(registers.accumulator_and_flags,get_lower(registers.HL));
                break;
            }
            case opcode::LD_A_iHL:
            {
                set_upper(registers.accumulator_and_flags,read_from_memory(registers.HL));
                break;
            }
            case opcode::LD_A_A:
            {
                //done
                break;
            }
            case opcode::LDH_ia8_A:
            {
                write_to_memory(0xFF00 + read_from_memory(registers.program_counter++), get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LD_iC_A:
            {
                write_to_memory(0xFF00 + get_lower(registers.BC), get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LD_ia16_A:
            {
                write_to_memory(read_16b_value(), get_upper(registers.accumulator_and_flags));
                break;
            }
            case opcode::LDH_A_ia8:
            {
                const auto address = read_from_memory(registers.program_counter++) + 0xFF00;
                set_upper(registers.accumulator_and_flags,read_from_memory(address));
                break;
            }
            case opcode::LD_A_iC:
            {
                const auto address = get_lower(registers.BC) + 0xFF00;
                set_upper(registers.accumulator_and_flags, read_from_memory(address));
                break;
            }
            case opcode::LD_A_ia16:
            {
                const auto address = read_16b_value();
                set_upper(registers.accumulator_and_flags, read_from_memory(address));
                break;
            }
        }
    }
    void control(opcode instruction)
    {
        switch (instruction)
        {
            case opcode::JR_r8:
            {
                std::uint8_t u_offset = read_from_memory(registers.program_counter);
                ++registers.program_counter;
                std::int8_t offset = u_offset; //if positive

                if ((u_offset & 0b10000000) > 0)
                    offset = -static_cast<int>(~u_offset + 1);

                    registers.program_counter += offset;
                
                break;
            }
            case opcode::JR_NZ_r8:
            {
                std::uint8_t u_offset =  read_from_memory(registers.program_counter); 
                ++registers.program_counter;
                std::int8_t offset = u_offset; //if positive
 
                if((u_offset&0b10000000)>0)
                    offset = -static_cast<int>(~u_offset+1);
                    
                if (!is_flag_set(Flags::zero))
                {
                    registers.program_counter += offset;
                }
                break;
            }
            case opcode::JR_Z_r8:
            {
                std::uint8_t u_offset = read_from_memory(registers.program_counter);
                ++registers.program_counter;
                std::int8_t offset = u_offset; //if positive

                if ((u_offset & 0b10000000) > 0)
                    offset = -static_cast<int>(~u_offset + 1);

                if (is_flag_set(Flags::zero))
                {
                    registers.program_counter += offset;
                }
                break;
            }
            case opcode::JR_NC_r8:
            {
                std::uint8_t u_offset = read_from_memory(registers.program_counter);
                ++registers.program_counter;
                std::int8_t offset = u_offset; //if positive

                if ((u_offset & 0b10000000) > 0)
                    offset = -static_cast<int>(~u_offset + 1);

                if (!is_flag_set(Flags::carry))
                {
                    registers.program_counter += offset;
                }
                break;
            }
            case opcode::JR_C_r8:
            {
                std::uint8_t u_offset = read_from_memory(registers.program_counter);
                ++registers.program_counter;
                std::int8_t offset = u_offset; //if positive

                if ((u_offset & 0b10000000) > 0)
                    offset = -static_cast<int>(~u_offset + 1);

                if (is_flag_set(Flags::carry))
                {
                    registers.program_counter += offset;
                }
                break;
            }
            case opcode::RET_NZ:
            {
                if(!is_flag_set(Flags::zero))
                    registers.program_counter = pop_from_stack();
                break;
            }
            case opcode::JP_NZ_a16:
            {
                const int lower = read_from_memory(registers.program_counter++);
                const int upper = read_from_memory(registers.program_counter++);

                if (!is_flag_set(Flags::zero))
                {
                    const std::uint16_t target_address = lower | (upper << 8);
                    registers.program_counter = target_address;
                }
                break;
            }
            case opcode::JP_a16:
            {
                const int lower = read_from_memory(registers.program_counter++);
                const int upper = read_from_memory(registers.program_counter++);

                const std::uint16_t target_address = lower | (upper << 8);
                registers.program_counter = target_address;
                break;
            }
            case opcode::CALL_NZ_a16:
            {
                const int lower = read_from_memory(registers.program_counter++);
                const int upper = read_from_memory(registers.program_counter++);

                if (!is_flag_set(Flags::zero))
                {
                    push_to_stack(registers.program_counter);
                    const std::uint16_t target_address = lower | (upper << 8);
                    registers.program_counter = target_address;
                }
                break;
            }
            case opcode::RST_00H:
            {
                push_to_stack(registers.program_counter);
                registers.program_counter = 0;
                break;
            }
            case opcode::RET_Z:
            {
                if (is_flag_set(Flags::zero))
                    registers.program_counter = pop_from_stack();
                break;
            }
            case opcode::RET:
            {
                registers.program_counter = pop_from_stack();
                break;
            }
            case opcode::JP_Z_a16:
            {
                if (is_flag_set(Flags::zero))
                {
                    const int lower = read_from_memory(registers.program_counter++);
                    const int upper = read_from_memory(registers.program_counter++);
                    const std::uint16_t target_address = lower | (upper << 8);
                    registers.program_counter = target_address;
                }
                break;
            }
            case opcode::CALL_Z_a16:
            {
                const int lower = read_from_memory(registers.program_counter++);
                const int upper = read_from_memory(registers.program_counter++);

                if (!is_flag_set(Flags::zero))
                {
                    push_to_stack(registers.program_counter);
                    const std::uint16_t target_address = lower | (upper << 8);
                    registers.program_counter = target_address;
                }
                break;
            }
            case opcode::CALL_a16:
            {
                const int lower = read_from_memory(registers.program_counter++);
                const int upper = read_from_memory(registers.program_counter++);

                const std::uint16_t target_address = lower | (upper << 8);
                push_to_stack(registers.program_counter);
                registers.program_counter = target_address;
                break;
            }
            case opcode::RST_08H:
            {
                push_to_stack(registers.program_counter);
                registers.program_counter = 0x08;
                break;
            }
            case opcode::RET_NC:
            {
                if (!is_flag_set(Flags::carry))
                    registers.program_counter = pop_from_stack();
                break;
            }
            case opcode::JP_NC_a16:
            {
                if (!is_flag_set(Flags::carry))
                {
                    const int lower = read_from_memory(registers.program_counter++);
                    const int upper = read_from_memory(registers.program_counter++);
                    const std::uint16_t target_address = lower | (upper << 8);
                    registers.program_counter = target_address;
                }
                break;
            }
            case opcode::CALL_NC_a16:
            {
                const int lower = read_from_memory(registers.program_counter++);
                const int upper = read_from_memory(registers.program_counter++);
                if (!is_flag_set(Flags::carry))
                {
                    const std::uint16_t target_address = lower | (upper << 8);
                    push_to_stack(registers.program_counter);
                    registers.program_counter = target_address;
                }
                break;
            }
            case opcode::RST_10H:
            {
                push_to_stack(registers.program_counter);
                registers.program_counter = 0x1 << 4;
                break;
            }
            case opcode::RET_C:
            {
                if (is_flag_set(Flags::carry))
                    registers.program_counter = pop_from_stack();
                break;
            }
            case opcode::RETI: // interrupts
            {
                
                break;
            }
            case opcode::JP_C_a16:
            {
                if (!is_flag_set(Flags::carry))
                {
                    const int lower = read_from_memory(registers.program_counter++);
                    const int upper = read_from_memory(registers.program_counter++);
                    const std::uint16_t target_address = lower | (upper << 8);
                    registers.program_counter = target_address;
                }
                break;
            }
            case opcode::CALL_C_a16:
            {
                break;
            }
            case opcode::RST_18H:
            {
                push_to_stack(registers.program_counter);
                registers.program_counter = 0x18;
                break;
            }
            case opcode::RST_20H:
            {
                push_to_stack(registers.program_counter);
                registers.program_counter = 0x20;
                break;
            }
            case opcode::JP_iHL:
            {
                registers.program_counter = memory[registers.HL];
                break;
            }
            case opcode::RST_28H:
            {
                push_to_stack(registers.program_counter);
                registers.program_counter = 0x28;
                break;
            }
            case opcode::RST_30H:
            {
                push_to_stack(registers.program_counter);
                registers.program_counter = 0x30;
                break;
            }
            case opcode::RST_38H:
            {
                push_to_stack(registers.program_counter);
                registers.program_counter = 0x38;
                break;
            }
        }
    }

    void x16_Load_Store_Move(opcode instruction)
    {
        switch (instruction)
        {
            case opcode::LD_BC_d16:
            {
                const int lower = read_from_memory(registers.program_counter++);
                const int upper = read_from_memory(registers.program_counter++);

                const std::uint16_t value = lower | (upper << 8);
                registers.BC = value;
                break;
            }
            case opcode::LD_ia16_SP:
            {
                
                break;
            }
            case opcode::LD_DE_d16:
            {
                const int lower = read_from_memory(registers.program_counter++);
                const int upper = read_from_memory(registers.program_counter++);

                const std::uint16_t value = lower | (upper<<8);
                registers.DE = value;
                break;
            }
            case opcode::LD_HL_d16:
            {
                const int lower = read_from_memory(registers.program_counter++);
                const int upper = read_from_memory(registers.program_counter++);

                const std::uint16_t value = lower | (upper << 8);
                registers.HL = value;
                break;
            }
            case opcode::LD_SP_d16:
            {
                const int lower = read_from_memory(registers.program_counter++);
                const int upper = read_from_memory(registers.program_counter++);

                const std::uint16_t value = lower | (upper << 8);
                registers.stack_pointer = value; 
                break;
            }
            case opcode::POP_BC:
            {
                registers.BC = pop_from_stack();
                break;
            }
            case opcode::PUSH_BC:
            {
                push_to_stack(registers.BC);
                break;
            }
            case opcode::POP_DE:
            {
                registers.DE = pop_from_stack();
                break;
            }
            case opcode::PUSH_DE:
            {
                push_to_stack(registers.DE);
                break;
            }
            case opcode::POP_HL:
            {
                registers.HL = pop_from_stack();
                break;
            }
            case opcode::PUSH_HL:
            {
                push_to_stack(registers.HL);
                break;
            }
            case opcode::POP_AF:
            {
                registers.accumulator_and_flags = pop_from_stack();
                break;
            }
            case opcode::PUSH_AF:
            {
                push_to_stack(registers.accumulator_and_flags);
                break;
            }
            case opcode::LD_HL_SP_Offset:
            {
                break;
            }
            case opcode::LD_SP_HL:
            {
                registers.stack_pointer = registers.HL;
                break;
            }
        }
    }
};
    