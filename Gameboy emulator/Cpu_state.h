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
    }

private:

    void x8_Rotate_and_Shift_Bits(opcode instruction)
    {
        switch (instruction)
        {
            case opcode::RLCA:
            {  

                break;
            }
            case opcode::RRCA:
            {

                break;
            }
            case opcode::RLA:
            {

                break;
            }
            case opcode::RRA:
            {
                
                break;
            }
        }
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
        return set_upper(register_,upper);
    }

    uint16_t increment_lower(uint16_t register_)
    {
        uint8_t lower = get_lower(register_);
        ++lower;
        return set_lower(register_, lower);
    }

    uint16_t decrement_upper(uint16_t register_)
    {
        uint8_t upper = get_upper(register_);
        --upper;
        return set_upper(register_, upper);
    }

    uint16_t decrement_lower(uint16_t register_)
    {
        uint8_t lower = get_lower(register_);
        --lower;
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

        if (get_upper(registers.accumulator_and_flags) & 0xf > decrement & 0xf)
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

                break;
            }
            case opcode::CP_C:
            {

                break;
            }
            case opcode::CP_D:
            {

                break;
            }
            case opcode::CP_E:
            {

                break;
            }
            case opcode::CP_H:
            {

                break;
            }
            case opcode::CP_L:
            {

                break;
            }
            case opcode::CP_iHL:
            {

                break;
            }
            case opcode::CP_A:
            {

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

                break;
            }
            case opcode::SUB_d8:
            {

                break;
            }
            case opcode::SBC_A_d8:
            {

                break;
            }
            case opcode::AND_d8:
            {

                break;
            }
            case opcode::XOR_d8:
            {

                break;
            }
            case opcode::OR_d8:
            {

                break;
            }
            case opcode::CP_d8:
            {

                break;
            }
        }
    }

    void x8_Load_Store_Move(opcode instruction)
    {
        switch (instruction)
        {
            
        }
    }


};