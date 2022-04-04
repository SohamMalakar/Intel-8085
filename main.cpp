#include <fstream>
#include <iostream>
#include <limits>
#include <regex>

using namespace std;

#define FLUSH cin.ignore(numeric_limits<streamsize>::max(), '\n');
#define IS_REG(x) (regex_match(x, regex("[ABCDEHLMabcdehlm]{1}")))
#define TOUPPER(x) transform(upr_token.begin(), upr_token.end(), upr_token.begin(), ::toupper);
#define DEBUG                                                                                                          \
    if (is_debug && regex_replace(lines[pc], regex(" "), "") != "")                                                    \
    {                                                                                                                  \
        cout << "Executed: " << lines[pc] << "\n";                                                                     \
        shell(f, r, mem);                                                                                              \
    }

map<uint16_t, uint8_t> modified_memory;
bool is_debug = false;
uint16_t cur_addr = 0;
string opcodes = "";

void print_memory()
{
    if (!modified_memory.empty())
    {
        cout << "Modified memory:\n";

        for (auto &m : modified_memory)
            cout << hex << m.first << ": " << hex << (int)m.second << ", ";

        cout << "\b\b  \n";
    }
}

template <typename T1, typename T2> void update_map(map<T1, T2> &m, T1 key, T2 value)
{
    auto itr = m.find(key);

    if (itr != m.end())
        itr->second = value;
    else
        m[key] = value;
}

int get_reg_index(char reg_name)
{
    reg_name = toupper(reg_name);

    if (reg_name == 'A' || reg_name == 'B' || reg_name == 'C' || reg_name == 'D' || reg_name == 'E')
        return reg_name - 'A';
    else if (reg_name == 'H')
        return 5;
    else if (reg_name == 'L')
        return 6;
    else if (reg_name == 'M')
        return 7;
    else
        return -1;
}

char get_reg_name(int reg_index)
{
    if (reg_index < 0 || reg_index >= 8)
        return '\0';
    else if (reg_index == 5)
        return 'H';
    else if (reg_index == 6)
        return 'L';
    else if (reg_index == 7)
        return 'M';
    else
        return 'A' + reg_index;
}

class memory
{
    uint8_t value[0xffff];

  public:
    memory()
    {
        reset();
    }

    void reset()
    {
        for (int i = 0; i < 0xffff; i++)
            value[i] = 0;
    }

    uint8_t get(uint16_t addr)
    {
        return value[addr];
    }

    void set(uint16_t addr, uint8_t val)
    {
        value[addr] = val;
        update_map(modified_memory, addr, val);
    }
};

class reg
{
    uint8_t value[8];

  public:
    reg()
    {
        reset();
    }

    void reset()
    {
        for (int i = 0; i < 8; i++)
            value[i] = 0;
    }

    void set(int reg_index, uint8_t val)
    {
        if (reg_index < 0 || reg_index >= 8)
            return;

        value[reg_index] = val;
    }

    void set(int reg_index, uint8_t val, memory &mem)
    {
        set(reg_index, val);

        if (reg_index == 5 || reg_index == 6)
        {
            set(7, mem.get(value[5] << 8 | value[6]));
        }
        else if (reg_index == 7)
        {
            mem.set(value[5] << 8 | value[6], val);
            hl_update(mem);
        }
    }

    uint8_t get(int reg_index)
    {
        if (reg_index < 0 || reg_index >= 8)
            return 0;

        return value[reg_index];
    }

    uint8_t get(int reg_index, memory &mem)
    {
        if (reg_index == 7)
            return mem.get(value[5] << 8 | value[6]);
        else
            return get(reg_index);
    }

    void hl_update(memory &mem)
    {
        set(5, get(5), mem);
        set(6, get(6), mem);
    }

    void print()
    {
        cout << "Registers:\n";

        for (int i = 0; i < 8; i++)
            cout << get_reg_name(i) << ": " << hex << (int)value[i] << (i == 7 ? "" : ", ");

        cout << "\n";
    }
};

class flag_reg
{
    uint8_t value;

  public:
    bool sign;
    bool zero;
    bool aux_carry;
    bool parity;
    bool carry;

    flag_reg()
    {
        reset();
    }

    void reset()
    {
        value = 0;
        sign = false;
        zero = false;
        aux_carry = false;
        parity = false;
        carry = false;
    }

    uint8_t get()
    {
        return value;
    }

    void scan(uint16_t val, bool aux_carry, bool consider_carry = true)
    {
        if (consider_carry)
            carry = val > 0xff;

        val &= 0xff;

        zero = !val;
        sign = val & 0x80;
        this->aux_carry = aux_carry;

        int parity_count = 0; // number of 1s in the binary representation of val

        for (int i = 0; i < 8; i++)
            if (val & (1 << i))
                parity_count++;

        parity = !(parity_count % 2);

        value = sign << 7 | zero << 6 | aux_carry << 4 | parity << 2 | carry;
    }

    void print()
    {
        cout << "FLAG: " << hex << (int)value << "\n";
        cout << "S: " << sign << ", Z: " << zero << ", AC: " << aux_carry << ", P: " << parity << ", CY: " << carry
             << "\n";
    }
};

// For taking input
void shell(flag_reg &f, reg &r, memory &mem)
{
    while (true)
    {
        string cmd;

        cout << "> ";
        getline(cin, cmd);

        if (cmd == "")
        {
            continue;
        }
        else if (cmd == "help")
        {
            cout << "Available commands:\n";
            cout << "  help - show this help\n";
            cout << "  setreg - set a register\n";
            cout << "  getreg - get a register\n";
            cout << "  setmem - set a memory address\n";
            cout << "  getmem - get a memory address\n";
            cout << "  next / n - set the next memory address\n";
            cout << "  prev / p - set the previous memory address\n";
            cout << "  step / s - execute one instruction\n";
            cout << "  exec - execute the program\n";
            cout << "  opcode - show the opcodes\n";
            cout << "  print - print registers and memory\n";
            cout << "  reset - reset the machine\n";
            cout << "  clear / cls - clears the console\n";
            cout << "  exit - exit the program\n";
        }
        else if (cmd == "reset")
        {
            r.reset();
            mem.reset();
            modified_memory.clear();
            cout << "Reset!\n";
        }
        else if (cmd == "opcode")
        {
            cout << "Opcodes:\n";
            cout << opcodes << "\n";
        }
        else if (cmd == "print")
        {
            f.print();
            r.print();
            print_memory();
        }
        else if (cmd == "step" || cmd == "s")
        {
            is_debug = true;
            break;
        }
        else if (cmd == "exec")
        {
            /* if (is_debug)
            {
                cout << "Already in debug mode!\n";
                continue;
            } */

            is_debug = false;
            break;
        }
        else if (cmd == "clear" || cmd == "cls")
        {
            system(cmd.c_str());
        }
        else if (cmd == "setmem")
        {
            uint16_t addr;
            int val;

            cout << "Address: ";
            cin >> hex >> addr;

            cur_addr = addr;

            cout << "Value: ";
            cin >> hex >> val;

            FLUSH

            mem.set(addr, val);
            r.hl_update(mem);
        }
        else if (cmd == "getmem")
        {
            uint16_t addr;

            cout << "Address: ";
            cin >> hex >> addr;

            FLUSH

            cout << hex << addr << ": " << hex << (int)mem.get(addr) << "\n";
        }
        else if (cmd == "next" || cmd == "n" || cmd == "prev" || cmd == "p")
        {
            int val;

            cout << "Address: " << hex << (cmd == "next" || cmd == "n" ? ++cur_addr : --cur_addr) << "\n";
            cout << "Value: ";
            cin >> hex >> val;

            FLUSH

            mem.set(cur_addr, val);
            r.hl_update(mem);
        }
        else if (cmd == "setreg")
        {
            char reg_name;
            int val;

            cout << "Register: ";
            cin >> reg_name;
            cout << "Value: ";
            cin >> hex >> val;

            FLUSH

            r.set(get_reg_index(reg_name), val, mem);
        }
        else if (cmd == "getreg")
        {
            char reg_name;

            cout << "Register: ";
            cin >> reg_name;

            FLUSH

            cout << reg_name << ": " << hex << (int)r.get(get_reg_index(reg_name), mem) << "\n";
        }
        else if (cmd == "exit")
        {
            exit(0);
        }
        else
        {
            cerr << "Unknown command: " << cmd << "\n";
        }
    }
}

void sp_update(size_t &pc, stack<size_t> &sp)
{
    if (sp.empty())
    {
        cerr << "Stack underflow\n";
        exit(1);
    }

    pc = sp.top() + 1;
    sp.pop();
}

int opcode_helper(char reg)
{
    reg = toupper(reg);

    if (reg == 'B' || reg == 'C' || reg == 'D' || reg == 'E')
        return reg - 'B';
    else if (reg == 'H')
        return 4;
    else if (reg == 'L')
        return 5;
    else if (reg == 'M')
        return 6;
    else if (reg == 'A')
        return 7;
    else
        return -1;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <file> [optional: args]\n";
        return 1;
    }

    ifstream file(argv[1]);

    if (!file.is_open())
    {
        cerr << argv[0] << ": error: " << argv[1] << ": No such file\n";
        return 1;
    }

    vector<string> lines;

    // For removing comments
    for (string line; getline(file, line);)
        lines.push_back(regex_replace(line, regex("//.*"), ""));

    file.close();

    unordered_map<string, int> labels;

    // For finding labels
    for (size_t i = 0; i < lines.size(); i++)
    {
        string line = lines[i].erase(0, lines[i].find_first_not_of(" \t")).erase(lines[i].find_last_not_of(" \t") + 1);

        if (line.find(":") != string::npos)
            labels[line.substr(0, line.find(":"))] = i;
    }

    // Actual code starts here
    memory mem;
    reg r;
    flag_reg f;
    size_t pc = 0;
    stack<size_t> sp;

    vector<string> instruction_set = {
        "MOV", "MVI",  "LDA", "LDAX", "LXI", "LHLD", "STA", "STAX", "SHLD", "XCHG", "SPHL", "XTHL", "PUSH", "POP",
        "OUT", "IN",   "ADD", "ADC",  "ADI", "ACI",  "DAD", "SUB",  "SBB",  "SUI",  "SBI",  "INR",  "INX",  "DCR",
        "DCX", "DAA",  "JMP", "JC",   "JNC", "JP",   "JM",  "JZ",   "JNZ",  "JPE",  "JPO",  "CALL", "CC",   "CNC",
        "CP",  "CM",   "CZ",  "CNZ",  "CPE", "CPO",  "RET", "RC",   "RNC",  "RP",   "RM",   "RZ",   "RNZ",  "RPE",
        "RPO", "PCHL", "RST", "CMP",  "CPI", "ANA",  "ANI", "XRA",  "XRI",  "ORA",  "ORI",  "RLC",  "RRC",  "RAL",
        "RAR", "CMA",  "CMC", "STC",  "NOP", "HLT",  "DI",  "EI",   "RIM",  "SIM"};

    string inst = "";
    int no_of_inst = instruction_set.size();

    // Regex for matching instructions
    for (int i = 0; i < no_of_inst; i++)
        inst += instruction_set[i] + (i != no_of_inst - 1 ? "|" : "");

    regex instructions_regex(inst);

    // t states
    int clock_cycles = 0;

    // opcodes in hex
    stringstream opcodes_stream;

    for (auto line : lines)
    {
        line = regex_replace(line, regex(","), " ");

        stringstream ss;
        istringstream iss(line);

        copy(istream_iterator<string>(iss), istream_iterator<string>(), ostream_iterator<string>(ss, " "));

        string token, args[3];

        int i = 0;

        while (getline(ss, token, ' '))
        {
            if (token.back() == ':')
                continue;

            if (i == 0)
            {
                string upr_token = token;
                TOUPPER(upr_token);

                if (!regex_match(upr_token, instructions_regex))
                {
                    cerr << "Unknown instruction: " << token << "\n";
                    return 1;
                }

                args[0] = upr_token;
            }
            else
            {
                if (args[0] == "MOV")
                {
                    if (IS_REG(token))
                    {
                        args[i] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    if (i == 2)
                    {
                        opcodes_stream << hex << 0x40 + 8 * opcode_helper(args[1][0]) + opcode_helper(args[2][0])
                                       << " ";
                        break;
                    }
                }
                else if (args[0] == "MVI")
                {
                    if (i == 2)
                    {
                        opcodes_stream << hex << 0x06 + 8 * opcode_helper(args[1][0]) << " " << hex
                                       << stoi(token, nullptr, 16) << " ";
                        break;
                    }

                    if (IS_REG(token))
                    {
                        args[1] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }
                }
                else if (args[0] == "LDA")
                {
                    opcodes_stream << hex << 0x3a << " " << hex << (stoi(token, nullptr, 16) & 0xff) << " " << hex
                                   << (stoi(token, nullptr, 16) >> 8 & 0xff) << " ";
                    break;
                }
                else if (args[0] == "LDAX")
                {
                    string upr_token = token;
                    TOUPPER(upr_token);

                    if (upr_token != "B" && upr_token != "D")
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0x0a + 8 * (opcode_helper(upr_token[0]) - opcode_helper('B')) << " ";
                    break;
                }
                else if (args[0] == "LXI")
                {
                    if (i == 2)
                    {
                        opcodes_stream << hex << 0x01 + 8 * (opcode_helper(args[1][0]) - opcode_helper('B')) << " "
                                       << hex << (stoi(token, nullptr, 16) & 0xff) << " " << hex
                                       << (stoi(token, nullptr, 16) >> 8 & 0xff) << " ";
                        break;
                    }

                    string upr_token = token;
                    TOUPPER(upr_token);

                    if (upr_token == "B" || upr_token == "D" || upr_token == "H")
                    {
                        args[1] = upr_token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }
                }
                else if (args[0] == "LHLD")
                {
                    opcodes_stream << hex << 0x2a << " " << hex << (stoi(token, nullptr, 16) & 0xff) << " " << hex
                                   << (stoi(token, nullptr, 16) >> 8 & 0xff) << " ";
                    break;
                }
                else if (args[0] == "STA")
                {
                    opcodes_stream << hex << 0x32 << " " << hex << (stoi(token, nullptr, 16) & 0xff) << " " << hex
                                   << (stoi(token, nullptr, 16) >> 8 & 0xff) << " ";
                    break;
                }
                else if (args[0] == "STAX")
                {
                    string upr_token = token;
                    TOUPPER(upr_token);

                    if (upr_token != "B" && upr_token != "D")
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0x02 + 8 * (opcode_helper(upr_token[0]) - opcode_helper('B')) << " ";
                    break;
                }
                else if (args[0] == "SHLD")
                {
                    opcodes_stream << hex << 0x22 << " " << hex << (stoi(token, nullptr, 16) & 0xff) << " " << hex
                                   << (stoi(token, nullptr, 16) >> 8 & 0xff) << " ";
                    break;
                }
                else if (args[0] == "ADD")
                {
                    if (IS_REG(token))
                    {
                        args[1] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0x80 + opcode_helper(args[1][0]) << " ";
                    break;
                }
                else if (args[0] == "ADC")
                {
                    if (IS_REG(token))
                    {
                        args[1] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0x88 + opcode_helper(args[1][0]) << " ";
                    break;
                }
                else if (args[0] == "ADI")
                {
                    opcodes_stream << hex << 0xc6 << " " << hex << stoi(token, nullptr, 16) << " ";
                    break;
                }
                else if (args[0] == "ACI")
                {
                    opcodes_stream << hex << 0xce << " " << hex << stoi(token, nullptr, 16) << " ";
                    break;
                }
                else if (args[0] == "DAD")
                {
                    string upr_token = token;
                    TOUPPER(upr_token);

                    if (upr_token != "B" && upr_token != "D" && upr_token != "H")
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0x09 + 8 * (opcode_helper(upr_token[0]) - opcode_helper('B')) << " ";
                    break;
                }
                else if (args[0] == "SUB")
                {
                    if (IS_REG(token))
                    {
                        args[1] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0x90 + opcode_helper(args[1][0]) << " ";
                    break;
                }
                else if (args[0] == "SBB")
                {
                    if (IS_REG(token))
                    {
                        args[1] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0x98 + opcode_helper(args[1][0]) << " ";
                    break;
                }
                else if (args[0] == "SUI")
                {
                    opcodes_stream << hex << 0xd6 << " " << hex << stoi(token, nullptr, 16) << " ";
                    break;
                }
                else if (args[0] == "SBI")
                {
                    opcodes_stream << hex << 0xde << " " << hex << stoi(token, nullptr, 16) << " ";
                    break;
                }
                else if (args[0] == "INR")
                {
                    if (IS_REG(token))
                    {
                        args[1] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0x04 + 8 * opcode_helper(args[1][0]) << " ";
                    break;
                }
                else if (args[0] == "INX")
                {
                    string upr_token = token;
                    TOUPPER(upr_token);

                    if (upr_token != "B" && upr_token != "D" && upr_token != "H")
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0x03 + 8 * (opcode_helper(upr_token[0]) - opcode_helper('B')) << " ";
                    break;
                }
                else if (args[0] == "DCR")
                {
                    if (IS_REG(token))
                    {
                        args[1] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0x05 + 8 * opcode_helper(args[1][0]) << " ";
                    break;
                }
                else if (args[0] == "DCX")
                {
                    string upr_token = token;
                    TOUPPER(upr_token);

                    if (upr_token != "B" && upr_token != "D" && upr_token != "H")
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0x0b + 8 * (opcode_helper(upr_token[0]) - opcode_helper('B')) << " ";
                    break;
                }
                else if (args[0] == "JMP")
                {
                    opcodes_stream << hex << 0xc3 << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "JC")
                {
                    opcodes_stream << hex << 0xda << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "JNC")
                {
                    opcodes_stream << hex << 0xd2 << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "JP")
                {
                    opcodes_stream << hex << 0xf2 << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "JM")
                {
                    opcodes_stream << hex << 0xfa << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "JZ")
                {
                    opcodes_stream << hex << 0xca << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "JNZ")
                {
                    opcodes_stream << hex << 0xc2 << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "JPE")
                {
                    opcodes_stream << hex << 0xea << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "JPO")
                {
                    opcodes_stream << hex << 0xe2 << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "CALL")
                {
                    opcodes_stream << hex << 0xcd << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "CC")
                {
                    opcodes_stream << hex << 0xdc << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "CNC")
                {
                    opcodes_stream << hex << 0xd4 << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "CP")
                {
                    opcodes_stream << hex << 0xf4 << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "CM")
                {
                    opcodes_stream << hex << 0xfc << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "CZ")
                {
                    opcodes_stream << hex << 0xcc << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "CNZ")
                {
                    opcodes_stream << hex << 0xc4 << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "CPE")
                {
                    opcodes_stream << hex << 0xec << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "CPO")
                {
                    opcodes_stream << hex << 0xe4 << " "
                                   << "xx"
                                   << " "
                                   << "xx"
                                   << " ";
                    break;
                }
                else if (args[0] == "CMP")
                {
                    if (IS_REG(token))
                    {
                        args[1] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0xb8 + opcode_helper(args[1][0]) << " ";
                    break;
                }
                else if (args[0] == "CPI")
                {
                    opcodes_stream << hex << 0xfe << " " << hex << stoi(token, nullptr, 16) << " ";
                    break;
                }
                else if (args[0] == "ANA")
                {
                    if (IS_REG(token))
                    {
                        args[1] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0xa0 + opcode_helper(args[1][0]) << " ";
                    break;
                }
                else if (args[0] == "ANI")
                {
                    opcodes_stream << hex << 0xe6 << " " << hex << stoi(token, nullptr, 16) << " ";
                    break;
                }
                else if (args[0] == "XRA")
                {
                    if (IS_REG(token))
                    {
                        args[1] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0xa8 + opcode_helper(args[1][0]) << " ";
                    break;
                }
                else if (args[0] == "XRI")
                {
                    opcodes_stream << hex << 0xee << " " << hex << stoi(token, nullptr, 16) << " ";
                    break;
                }
                else if (args[0] == "ORA")
                {
                    if (IS_REG(token))
                    {
                        args[1] = token;
                    }
                    else
                    {
                        cerr << "Invalid register: " << token << "\n";
                        return 1;
                    }

                    opcodes_stream << hex << 0xb0 + opcode_helper(args[1][0]) << " ";
                    break;
                }
                else if (args[0] == "ORI")
                {
                    opcodes_stream << hex << 0xf6 << " " << hex << stoi(token, nullptr, 16) << " ";
                    break;
                }
            }

            i++;
        }

        if (i != 0)
        {
            if (args[0] == "XCHG")
            {
                opcodes_stream << hex << 0xeb << " ";
            }
            else if (args[0] == "RET")
            {
                opcodes_stream << hex << 0xc9 << " ";
            }
            else if (args[0] == "RC")
            {
                opcodes_stream << hex << 0xd8 << " ";
            }
            else if (args[0] == "RNC")
            {
                opcodes_stream << hex << 0xd0 << " ";
            }
            else if (args[0] == "RP")
            {
                opcodes_stream << hex << 0xf0 << " ";
            }
            else if (args[0] == "RM")
            {
                opcodes_stream << hex << 0xf8 << " ";
            }
            else if (args[0] == "RZ")
            {
                opcodes_stream << hex << 0xc8 << " ";
            }
            else if (args[0] == "RNZ")
            {
                opcodes_stream << hex << 0xc0 << " ";
            }
            else if (args[0] == "RPE")
            {
                opcodes_stream << hex << 0xe8 << " ";
            }
            else if (args[0] == "RPO")
            {
                opcodes_stream << hex << 0xe0 << " ";
            }
            else if (args[0] == "RLC")
            {
                opcodes_stream << hex << 0x07 << " ";
            }
            else if (args[0] == "RRC")
            {
                opcodes_stream << hex << 0x0f << " ";
            }
            else if (args[0] == "RAL")
            {
                opcodes_stream << hex << 0x17 << " ";
            }
            else if (args[0] == "RAR")
            {
                opcodes_stream << hex << 0x1f << " ";
            }
            else if (args[0] == "CMA")
            {
                opcodes_stream << hex << 0x2f << " ";
            }
            else if (args[0] == "CMC")
            {
                opcodes_stream << hex << 0x3f << " ";
            }
            else if (args[0] == "STC")
            {
                opcodes_stream << hex << 0x37 << " ";
            }
            else if (args[0] == "NOP")
            {
                opcodes_stream << hex << 0x00 << " ";
            }
            else if (args[0] == "HLT")
            {
                opcodes_stream << hex << 0x76 << " ";
            }
        }
    }

    opcodes = opcodes_stream.str();

    cout << "Program loaded!\n";
    cout << "Type 'help' for help.\n";

    shell(f, r, mem); // For taking input

outer:
    // For executing instructions
    while (pc < lines.size())
    {
        string line = regex_replace(lines[pc], regex(","), " ");

        stringstream ss;
        istringstream iss(line);

        copy(istream_iterator<string>(iss), istream_iterator<string>(), ostream_iterator<string>(ss, " "));

        string token, args[3];

        int i = 0;

        while (getline(ss, token, ' '))
        {
            if (token.back() == ':')
                continue;

            if (i == 0)
            {
                string upr_token = token;
                TOUPPER(upr_token);

                args[0] = upr_token;
            }
            else
            {
                // data transfer instructions
                if (args[0] == "MOV")
                {
                    args[i] = token;

                    if (i == 2)
                    {
                        r.set(get_reg_index(args[1][0]), r.get(get_reg_index(args[2][0]), mem), mem);
                        clock_cycles += args[1][0] != 'M' || args[2][0] != 'M' ? 4 : 7;
                        break;
                    }
                }
                else if (args[0] == "MVI")
                {
                    if (i == 2)
                    {
                        try
                        {
                            r.set(get_reg_index(args[1][0]), stoi(token, nullptr, 16), mem);
                        }
                        catch (exception)
                        {
                            cerr << "Invalid value: " << token << "\n";
                            return 1;
                        }

                        clock_cycles += args[1][0] != 'M' ? 7 : 10;
                        break;
                    }

                    args[1] = token;
                }
                else if (args[0] == "LDA")
                {
                    try
                    {
                        r.set(0, mem.get(stoi(token, nullptr, 16)));
                    }
                    catch (exception)
                    {
                        cerr << "Invalid address: " << token << "\n";
                        return 1;
                    }

                    clock_cycles += 13;
                    break;
                }
                else if (args[0] == "LDAX")
                {
                    r.set(0, mem.get(r.get(get_reg_index(token[0])) << 8 | r.get(get_reg_index(token[0] + 1))));
                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "LXI")
                {
                    if (i == 2)
                    {
                        try
                        {
                            r.set(get_reg_index(args[1][0]), stoi(token, nullptr, 16) >> 8, mem);
                            r.set(get_reg_index(args[1][0] == 'H' ? 'L' : args[1][0] + 1),
                                  stoi(token, nullptr, 16) & 0xFF, mem);
                        }
                        catch (exception)
                        {
                            cerr << "Invalid value: " << token << "\n";
                            return 1;
                        }

                        clock_cycles += 10;
                        break;
                    }

                    args[1] = token;
                }
                else if (args[0] == "LHLD")
                {
                    try
                    {
                        r.set(get_reg_index('L'), mem.get(stoi(token, nullptr, 16)), mem);
                        r.set(get_reg_index('H'), mem.get(stoi(token, nullptr, 16) + 1), mem);
                    }
                    catch (exception)
                    {
                        cerr << "Invalid address: " << token << "\n";
                        return 1;
                    }

                    clock_cycles += 16;
                    break;
                }
                else if (args[0] == "STA")
                {
                    try
                    {
                        mem.set(stoi(token, nullptr, 16), r.get(0));
                        r.hl_update(mem);
                    }
                    catch (exception)
                    {
                        cerr << "Invalid address: " << token << "\n";
                        return 1;
                    }

                    clock_cycles += 13;
                    break;
                }
                else if (args[0] == "STAX")
                {
                    mem.set(r.get(get_reg_index(token[0])) << 8 | r.get(get_reg_index(token[0] + 1)), r.get(0));
                    r.hl_update(mem);
                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "SHLD")
                {
                    try
                    {
                        mem.set(stoi(token, nullptr, 16), r.get(get_reg_index('L')));
                        r.hl_update(mem);

                        mem.set(stoi(token, nullptr, 16) + 1, r.get(get_reg_index('H')));
                        r.hl_update(mem);
                    }
                    catch (exception)
                    {
                        cerr << "Invalid address: " << token << "\n";
                        return 1;
                    }

                    clock_cycles += 16;
                    break;
                }
                // arithmetic instructions
                else if (args[0] == "ADD")
                {
                    args[1] = token;

                    uint16_t result = r.get(0) + r.get(get_reg_index(args[1][0]), mem);
                    uint8_t aux = (r.get(0) & 0x0F) + (r.get(get_reg_index(args[1][0]), mem) & 0x0F);

                    r.set(0, result);
                    f.scan(result, aux & 0x10);
                    clock_cycles += args[1][0] != 'M' ? 4 : 7;
                    break;
                }
                else if (args[0] == "ADC")
                {
                    args[1] = token;

                    uint16_t result = r.get(0) + r.get(get_reg_index(args[1][0]), mem) + f.carry;
                    uint8_t aux = (r.get(0) & 0x0F) + (r.get(get_reg_index(args[1][0]), mem) & 0x0F) + f.carry;

                    r.set(0, result);
                    f.scan(result, aux & 0x10);
                    clock_cycles += args[1][0] != 'M' ? 4 : 7;
                    break;
                }
                else if (args[0] == "ADI")
                {
                    uint16_t result;
                    uint8_t aux;

                    try
                    {
                        result = r.get(0) + stoi(token, nullptr, 16);
                        aux = (r.get(0) & 0x0F) + (stoi(token, nullptr, 16) & 0x0F);
                    }
                    catch (exception)
                    {
                        cerr << "Invalid value: " << token << "\n";
                        return 1;
                    }

                    r.set(0, result);
                    f.scan(result, aux & 0x10);
                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "ACI")
                {
                    uint16_t result;
                    uint8_t aux;

                    try
                    {
                        result = r.get(0) + stoi(token, nullptr, 16) + f.carry;
                        aux = (r.get(0) & 0x0F) + (stoi(token, nullptr, 16) & 0x0F) + f.carry;
                    }
                    catch (exception)
                    {
                        cerr << "Invalid value: " << token << "\n";
                        return 1;
                    }

                    r.set(0, result);
                    f.scan(result, aux & 0x10);
                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "DAD")
                {
                    uint32_t hl_pair = r.get(get_reg_index('H')) << 8 | r.get(get_reg_index('L'));
                    uint32_t reg_pair = r.get(get_reg_index(token[0])) << 8 |
                                        r.get(get_reg_index(token[0] == 'H' ? 'L' : token[0] + 1));

                    uint32_t result = hl_pair + reg_pair;

                    r.set(get_reg_index('H'), result >> 8 & 0xFF, mem);
                    r.set(get_reg_index('L'), result & 0xFF, mem);

                    f.carry = result & 0x10000;
                    clock_cycles += 10;
                    break;
                }
                else if (args[0] == "SUB")
                {
                    args[1] = token;

                    uint16_t result = r.get(0) - r.get(get_reg_index(args[1][0]), mem);
                    uint8_t aux = (r.get(0) & 0x0F) + ((~r.get(get_reg_index(args[1][0]), mem) + 1) & 0x0F);

                    r.set(0, result);
                    f.scan(result, aux & 0x10);
                    clock_cycles += args[1][0] != 'M' ? 4 : 7;
                    break;
                }
                else if (args[0] == "SBB")
                {
                    args[1] = token;

                    uint16_t result = r.get(0) - r.get(get_reg_index(args[1][0]), mem) - f.carry;
                    uint8_t aux = (r.get(0) & 0x0F) + ((~r.get(get_reg_index(args[1][0]), mem) + 1) & 0x0F) +
                                  ((~(uint8_t)f.carry + 1) & 0x0F);

                    r.set(0, result);
                    f.scan(result, aux & 0x10);
                    clock_cycles += args[1][0] != 'M' ? 4 : 7;
                    break;
                }
                else if (args[0] == "SUI")
                {
                    uint16_t result;
                    uint8_t aux;

                    try
                    {
                        result = r.get(0) - stoi(token, nullptr, 16);
                        aux = (r.get(0) & 0x0F) + ((~stoi(token, nullptr, 16) + 1) & 0x0F);
                    }
                    catch (exception)
                    {
                        cerr << "Invalid value: " << token << "\n";
                        return 1;
                    }

                    r.set(0, result);
                    f.scan(result, aux & 0x10);
                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "SBI")
                {
                    uint16_t result;
                    uint8_t aux;

                    try
                    {
                        result = r.get(0) - stoi(token, nullptr, 16) - f.carry;
                        aux = (r.get(0) & 0x0F) + (~(stoi(token, nullptr, 16) + 1) & 0x0F) +
                              ((~(uint8_t)f.carry + 1) & 0x0F);
                    }
                    catch (exception)
                    {
                        cerr << "Invalid value: " << token << "\n";
                        return 1;
                    }

                    r.set(0, result);
                    f.scan(result, aux & 0x10);
                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "INR")
                {
                    args[1] = token;

                    // All flags, except Cy flag, are affected depending on the result thus produced.
                    uint16_t result = r.get(get_reg_index(args[1][0]), mem) + 1;
                    uint8_t aux = (r.get(get_reg_index(args[1][0]), mem) & 0x0F) + 1;

                    r.set(get_reg_index(args[1][0]), result, mem);
                    f.scan(result, aux & 0x10, false);
                    clock_cycles += args[1][0] != 'M' ? 4 : 10;
                    break;
                }
                else if (args[0] == "INX")
                {
                    uint16_t result = r.get(get_reg_index(token[0] == 'H' ? 'L' : token[0] + 1)) + 1;
                    bool carry = result & 0xFF00;
                    r.set(get_reg_index(token[0] == 'H' ? 'L' : token[0] + 1), result & 0xFF, mem);
                    result = r.get(get_reg_index(token[0])) + carry;
                    r.set(get_reg_index(token[0]), result & 0xFF, mem);
                    clock_cycles += 6;
                    break;
                }
                else if (args[0] == "DCR")
                {
                    args[1] = token;

                    // All flags, except Cy flag, are affected depending on the result thus produced.
                    uint16_t result = r.get(get_reg_index(args[1][0]), mem) - 1;
                    uint8_t aux = (r.get(get_reg_index(args[1][0]), mem) & 0x0F) + ((~1 + 1) & 0x0F);

                    r.set(get_reg_index(args[1][0]), result, mem);
                    f.scan(result, aux & 0x10, false);
                    clock_cycles += args[1][0] != 'M' ? 4 : 10;
                    break;
                }
                else if (args[0] == "DCX")
                {
                    uint16_t result = r.get(get_reg_index(token[0] == 'H' ? 'L' : token[0] + 1)) - 1;
                    bool borrow = result & 0xFF00;
                    r.set(get_reg_index(token[0] == 'H' ? 'L' : token[0] + 1), result & 0xFF, mem);
                    result = r.get(get_reg_index(token[0])) - borrow;
                    r.set(get_reg_index(token[0]), result & 0xFF, mem);
                    clock_cycles += 6;
                    break;
                }
                // branching instructions
                else if (args[0] == "JMP")
                {
                    DEBUG
                    pc = labels[token];
                    clock_cycles += 10;
                    goto outer;
                }
                else if (args[0] == "JC")
                {
                    if (f.carry)
                    {
                        DEBUG
                        pc = labels[token];
                        clock_cycles += 10;
                        goto outer;
                    }

                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "JNC")
                {
                    if (!f.carry)
                    {
                        DEBUG
                        pc = labels[token];
                        clock_cycles += 10;
                        goto outer;
                    }

                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "JP")
                {
                    if (!f.sign)
                    {
                        DEBUG
                        pc = labels[token];
                        clock_cycles += 10;
                        goto outer;
                    }

                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "JM")
                {
                    if (f.sign)
                    {
                        DEBUG
                        pc = labels[token];
                        clock_cycles += 10;
                        goto outer;
                    }

                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "JZ")
                {
                    if (f.zero)
                    {
                        DEBUG
                        pc = labels[token];
                        clock_cycles += 10;
                        goto outer;
                    }

                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "JNZ")
                {
                    if (!f.zero)
                    {
                        DEBUG
                        pc = labels[token];
                        clock_cycles += 10;
                        goto outer;
                    }

                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "JPE")
                {
                    if (f.parity)
                    {
                        DEBUG
                        pc = labels[token];
                        clock_cycles += 10;
                        goto outer;
                    }

                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "JPO")
                {
                    if (!f.parity)
                    {
                        DEBUG
                        pc = labels[token];
                        clock_cycles += 10;
                        goto outer;
                    }

                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "CALL")
                {
                    DEBUG
                    sp.push(pc);
                    pc = labels[token];
                    clock_cycles += 18;
                    goto outer;
                }
                else if (args[0] == "CC")
                {
                    if (f.carry)
                    {
                        DEBUG
                        sp.push(pc);
                        pc = labels[token];
                        clock_cycles += 18;
                        goto outer;
                    }

                    clock_cycles += 9;
                    break;
                }
                else if (args[0] == "CNC")
                {
                    if (!f.carry)
                    {
                        DEBUG
                        sp.push(pc);
                        pc = labels[token];
                        clock_cycles += 18;
                        goto outer;
                    }

                    clock_cycles += 9;
                    break;
                }
                else if (args[0] == "CP")
                {
                    if (!f.sign)
                    {
                        DEBUG
                        sp.push(pc);
                        pc = labels[token];
                        clock_cycles += 18;
                        goto outer;
                    }

                    clock_cycles += 9;
                    break;
                }
                else if (args[0] == "CM")
                {
                    if (f.sign)
                    {
                        DEBUG
                        sp.push(pc);
                        pc = labels[token];
                        clock_cycles += 18;
                        goto outer;
                    }

                    clock_cycles += 9;
                    break;
                }
                else if (args[0] == "CZ")
                {
                    if (f.zero)
                    {
                        DEBUG
                        sp.push(pc);
                        pc = labels[token];
                        clock_cycles += 18;
                        goto outer;
                    }

                    clock_cycles += 9;
                    break;
                }
                else if (args[0] == "CNZ")
                {
                    if (!f.zero)
                    {
                        DEBUG
                        sp.push(pc);
                        pc = labels[token];
                        clock_cycles += 18;
                        goto outer;
                    }

                    clock_cycles += 9;
                    break;
                }
                else if (args[0] == "CPE")
                {
                    if (f.parity)
                    {
                        DEBUG
                        sp.push(pc);
                        pc = labels[token];
                        clock_cycles += 18;
                        goto outer;
                    }

                    clock_cycles += 9;
                    break;
                }
                else if (args[0] == "CPO")
                {
                    if (!f.parity)
                    {
                        DEBUG
                        sp.push(pc);
                        pc = labels[token];
                        clock_cycles += 18;
                        goto outer;
                    }

                    clock_cycles += 9;
                    break;
                }
                // logical instructions
                else if (args[0] == "CMP")
                {
                    args[1] = token;

                    uint8_t accumulator = r.get(0);
                    uint8_t operand = r.get(get_reg_index(args[1][0]), mem);

                    f.carry = accumulator < operand;
                    f.zero = accumulator == operand;
                    clock_cycles += args[1][0] != 'M' ? 4 : 7;
                    break;
                }
                else if (args[0] == "CPI")
                {
                    uint8_t accumulator = r.get(0);
                    uint8_t operand;

                    try
                    {
                        operand = stoi(token, nullptr, 16);
                    }
                    catch (exception)
                    {
                        cerr << "Invalid value: " << token << "\n";
                        return 1;
                    }

                    f.carry = accumulator < operand;
                    f.zero = accumulator == operand;
                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "ANA")
                {
                    args[1] = token;

                    uint8_t accumulator = r.get(0);
                    uint8_t operand = r.get(get_reg_index(args[1][0]), mem);
                    uint8_t result = accumulator & operand;

                    r.set(0, result);
                    f.scan(result, true);
                    clock_cycles += args[1][0] != 'M' ? 4 : 7;
                    break;
                }
                else if (args[0] == "ANI")
                {
                    uint8_t accumulator = r.get(0);
                    uint8_t operand;

                    try
                    {
                        operand = stoi(token, nullptr, 16);
                    }
                    catch (exception)
                    {
                        cerr << "Invalid value: " << token << "\n";
                        return 1;
                    }

                    uint8_t result = accumulator & operand;

                    r.set(0, result);
                    f.scan(result, true);
                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "XRA")
                {
                    args[1] = token;

                    uint8_t accumulator = r.get(0);
                    uint8_t operand = r.get(get_reg_index(args[1][0]), mem);
                    uint8_t result = accumulator ^ operand;

                    r.set(0, result);
                    f.scan(result, false);
                    clock_cycles += args[1][0] != 'M' ? 4 : 7;
                    break;
                }
                else if (args[0] == "XRI")
                {
                    uint8_t accumulator = r.get(0);
                    uint8_t operand;

                    try
                    {
                        operand = stoi(token, nullptr, 16);
                    }
                    catch (exception)
                    {
                        cerr << "Invalid value: " << token << "\n";
                        return 1;
                    }

                    uint8_t result = accumulator ^ operand;

                    r.set(0, result);
                    f.scan(result, false);
                    clock_cycles += 7;
                    break;
                }
                else if (args[0] == "ORA")
                {
                    args[1] = token;

                    uint8_t accumulator = r.get(0);
                    uint8_t operand = r.get(get_reg_index(args[1][0]), mem);
                    uint8_t result = accumulator | operand;

                    r.set(0, result);
                    f.scan(result, false);
                    clock_cycles += args[1][0] != 'M' ? 4 : 7;
                    break;
                }
                else if (args[0] == "ORI")
                {
                    uint8_t accumulator = r.get(0);
                    uint8_t operand;

                    try
                    {
                        operand = stoi(token, nullptr, 16);
                    }
                    catch (exception)
                    {
                        cerr << "Invalid value: " << token << "\n";
                        return 1;
                    }

                    uint8_t result = accumulator | operand;

                    r.set(0, result);
                    f.scan(result, false);
                    clock_cycles += 7;
                    break;
                }
            }

            i++;
        }

        // zero address instructions
        if (i != 0)
        {
            // data transfer instructions
            if (args[0] == "XCHG")
            {
                uint8_t temp;

                temp = r.get(get_reg_index('D'));
                r.set(get_reg_index('D'), r.get(get_reg_index('H')));
                r.set(get_reg_index('H'), temp, mem);

                temp = r.get(get_reg_index('E'));
                r.set(get_reg_index('E'), r.get(get_reg_index('L')));
                r.set(get_reg_index('L'), temp, mem);

                clock_cycles += 4;
            }
            // branching instructions
            else if (args[0] == "RET")
            {
                DEBUG
                sp_update(pc, sp);
                clock_cycles += 10;
                goto outer;
            }
            else if (args[0] == "RC")
            {
                if (f.carry)
                {
                    DEBUG
                    sp_update(pc, sp);
                    clock_cycles += 12;
                    goto outer;
                }

                clock_cycles += 6;
            }
            else if (args[0] == "RNC")
            {
                if (!f.carry)
                {
                    DEBUG
                    sp_update(pc, sp);
                    clock_cycles += 12;
                    goto outer;
                }

                clock_cycles += 6;
            }
            else if (args[0] == "RP")
            {
                if (!f.sign)
                {
                    DEBUG
                    sp_update(pc, sp);
                    clock_cycles += 12;
                    goto outer;
                }

                clock_cycles += 6;
            }
            else if (args[0] == "RM")
            {
                if (f.sign)
                {
                    DEBUG
                    sp_update(pc, sp);
                    clock_cycles += 12;
                    goto outer;
                }

                clock_cycles += 6;
            }
            else if (args[0] == "RZ")
            {
                if (f.zero)
                {
                    DEBUG
                    sp_update(pc, sp);
                    clock_cycles += 12;
                    goto outer;
                }

                clock_cycles += 6;
            }
            else if (args[0] == "RNZ")
            {
                if (!f.zero)
                {
                    DEBUG
                    sp_update(pc, sp);
                    clock_cycles += 12;
                    goto outer;
                }

                clock_cycles += 6;
            }
            else if (args[0] == "RPE")
            {
                if (f.parity)
                {
                    DEBUG
                    sp_update(pc, sp);
                    clock_cycles += 12;
                    goto outer;
                }

                clock_cycles += 6;
            }
            else if (args[0] == "RPO")
            {
                if (!f.parity)
                {
                    DEBUG
                    sp_update(pc, sp);
                    clock_cycles += 12;
                    goto outer;
                }

                clock_cycles += 6;
            }
            // logical instructions
            else if (args[0] == "RLC")
            {
                uint8_t accumulator = r.get(0);
                r.set(0, (accumulator << 1) | (accumulator >> 7));
                f.carry = accumulator >> 7;
                clock_cycles += 4;
            }
            else if (args[0] == "RRC")
            {
                uint8_t accumulator = r.get(0);
                r.set(0, (accumulator >> 1) | (accumulator << 7));
                f.carry = accumulator & 1;
                clock_cycles += 4;
            }
            else if (args[0] == "RAL")
            {
                uint8_t accumulator = r.get(0);
                r.set(0, (accumulator << 1) | f.carry);
                f.carry = accumulator >> 7;
                clock_cycles += 4;
            }
            else if (args[0] == "RAR")
            {
                uint8_t accumulator = r.get(0);
                r.set(0, (accumulator >> 1) | (f.carry << 7));
                f.carry = accumulator & 1;
                clock_cycles += 4;
            }
            else if (args[0] == "CMA")
            {
                r.set(0, ~r.get(0));
                clock_cycles += 4;
            }
            else if (args[0] == "CMC")
            {
                f.carry = !f.carry;
                clock_cycles += 4;
            }
            else if (args[0] == "STC")
            {
                f.carry = true;
                clock_cycles += 4;
            }
            // control instructions
            else if (args[0] == "NOP")
            {
                // nothing to do
                clock_cycles += 4;
            }
            else if (args[0] == "HLT")
            {
                if (is_debug)
                    cout << "Executed: HLT\n";

                clock_cycles += 5;
                break;
            }
        }

        DEBUG

        pc++;
    }

    f.print();
    r.print();
    print_memory();

    cout << "Clock cycles: " << dec << clock_cycles << "\n";

    return 0;
}
