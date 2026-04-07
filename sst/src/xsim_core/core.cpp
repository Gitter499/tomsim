#include <sst/core/sst_config.h>
#include <sst/core/interfaces/stdMem.h>

// #include <sst/core/simulation.h>

#include <sstream>
#include <cstdlib>
#include <fstream>
#include <cmath>

#include <xsim_core/core.hpp>

#include <json/json.h>

// helper macro for R-format instruction (rd: bits 10-8, rs: 7-5, rt: 4-2)
#define R_FORMAT_INSTRUCTION        \
	rd = (instruction >> 8) & 0x07; \
	rs = (instruction >> 5) & 0x07; \
	rt = (instruction >> 2) & 0x07;

// helper macro for I-format instruction (rd: bits 10-8 , imm8: 7-0)
#define I_FORMAT_INSTRUCTION        \
	rd = (instruction >> 8) & 0x07; \
	imm8 = instruction & 0xFF;

// helper macro for IX format instruction (imm11: 10-0)
#define IX_FORMAT_INSTRUCTION \
	imm11 = instruction & 0x7FF;

namespace XSim
{
	namespace Core
	{

		Core::Core(ComponentId_t id, Params &params) : Component(id)
		{

			// verbose is one of the configuration options for this component
			verbose = params.find<uint32_t>("verbose", verbose);

			// clock_frequency is one of the configuration options for this component
			clock_frequency = params.find<std::string>("clock_frequency", clock_frequency);
			this->registerTimeBase(clock_frequency, true);

			// set instruction latencies
			load_latencies(params);

			// load the program that is to be executed
			load_program(params);

			// Create the SST output with the required verbosity level
			output = new Output("mips_core[@t:@l]: ", verbose, 0, Output::STDOUT);

			// Create a tick function with the frequency specified
			tc = Super::registerClock(clock_frequency, new Clock::Handler2<Core, &Core::tick>(this));

			output->verbose(CALL_INFO, 1, 0, "Configuring connection to memory...\n");
			// memory_wrapper is used to make write/read requests to the SST simulated memory
			memory_wrapper = loadComponentExtension<MemoryWrapper>(params, output, tc);
			// new MemoryWrapper(*this, params, output, tc);
			output->verbose(CALL_INFO, 1, 0, "Configuration of memory interface completed.\n");

			// SST statistics
			instruction_count = registerStatistic<uint64_t>("instructions");
			output_file = params.find<std::string>("output", "stats.json");

			// tell the simulator not to end without us
			registerAsPrimaryComponent();
			primaryComponentDoNotEndSim();
		}

		void Core::init(unsigned int phase)
		{
			memory_wrapper->init(phase);
		}

		void Core::setup()
		{
			output->output("Setting up.\n");
			std::cout << "========== STARTED PROGRAM ==========" << std::endl;
		}

		void Core::finish()
		{
			std::cout << " finishing" << std::endl;
			// Output statistics to JSON via jsoncpp
			Json::Value stats;

			stats["author"] = "raa372";

			stats["Registers"] = Json::Value(Json::objectValue);
			for (int i = 0; i < 8; i++)
			{

				uint16_t val = registers[i];

				stats["Registers"]["r" + std::to_string(i)] = val;
			}

			stats["stats"] = Json::Value(Json::objectValue);
			stats["stats"]["cycles"] = (Json::Value::UInt64)total_sim_time;
			stats["stats"]["instructions"] = (Json::Value::UInt64)instruction_execution_count;

			// iterate through map
			for (const auto &[opcode, count] : opcode_count)
			{
				stats["stats"][names[opcode]] = (Json::Value::UInt64)count;
			}

			std::ofstream out_f(output_file);
			if (!out_f.is_open())
			{
				std::cerr << "Could not open output file: " << output_file << std::endl;
				exit(EXIT_FAILURE);
			}

			// stylish like me
			out_f << stats.toStyledString();

			out_f.close();
			// Nothing to cleanup
			std::cout << "========== FINISHED PROGRAM ==========" << std::endl;
		}

		/**
		 * @brief This function loads the program from the file in parameter: "program"
		 */
		void Core::load_program(Params &params)
		{
			std::string program_path = params.find<std::string>("program", "");
			std::ifstream f(program_path);
			if (!f.is_open())
			{
				std::cerr << "Invalid program file: " << program_path << std::endl;
				exit(EXIT_FAILURE);
			}
			for (std::string line; getline(f, line);)
			{
				std::size_t first_comment = line.find("#");
				std::size_t first_number = line.find_first_of("0123456789ABCDEFabcdef");
				// Ignore comments
				if (first_number < first_comment)
				{
					std::stringstream ss;
					ss << std::hex << line.substr(first_number, 4);
					uint16_t instruction;
					ss >> instruction;
					program.push_back(instruction);
				}
			}
			// Print the program for visual inspection
			std::cout << "Program:" << std::endl;
			for (uint16_t instruction : program)
			{
				std::cout << std::hex << instruction << std::dec << std::endl;
			}
		}

		/**
		 * @brief This function loads the latencies from the parameters
		 */
		void Core::load_latencies(Params &params)
		{
			latencies.insert({LIZ, params.find<uint32_t>("liz", 1)});
			names.insert({LIZ, "liz"});
			latencies.insert({LW, params.find<uint32_t>("lw", 1)});
			names.insert({LW, "lw"});
			latencies.insert({SW, params.find<uint32_t>("sw", 1)});
			names.insert({SW, "sw"});
			latencies.insert({PUT, params.find<uint32_t>("put", 1)});
			names.insert({PUT, "put"});
			latencies.insert({HALT, params.find<uint32_t>("halt", 1)});
			names.insert({HALT, "halt"});

			// logical
			latencies.insert({ADD, params.find<uint32_t>("add", 1)});
			names.insert({ADD, "add"});
			latencies.insert({SUB, params.find<uint32_t>("sub", 1)});
			names.insert({SUB, "sub"});
			latencies.insert({AND, params.find<uint32_t>("and", 1)});
			names.insert({AND, "and"});
			latencies.insert({NOR, params.find<uint32_t>("nor", 1)});
			names.insert({NOR, "nor"});

			// multiplier
			latencies.insert({DIV, params.find<uint32_t>("div", 1)});
			names.insert({DIV, "div"});
			latencies.insert({MUL, params.find<uint32_t>("mul", 1)});
			names.insert({MUL, "mul"});
			latencies.insert({MOD, params.find<uint32_t>("mod", 1)});
			names.insert({MOD, "mod"});
			latencies.insert({EXP, params.find<uint32_t>("exp", 1)});
			names.insert({EXP, "exp"});

			// immediate
			latencies.insert({LIS, params.find<uint32_t>("lis", 1)});
			names.insert({LIS, "lis"});
			latencies.insert({LUI, params.find<uint32_t>("lui", 1)});
			names.insert({LUI, "lui"});

			// jumps and branches
			latencies.insert({BP, params.find<uint32_t>("bp", 1)});
			names.insert({BP, "bp"});
			latencies.insert({BN, params.find<uint32_t>("bn", 1)});
			names.insert({BN, "bn"});
			latencies.insert({BX, params.find<uint32_t>("bx", 1)});
			names.insert({BX, "bx"});
			latencies.insert({BZ, params.find<uint32_t>("bz", 1)});
			names.insert({BZ, "bz"});
			latencies.insert({JR, params.find<uint32_t>("jr", 1)});
			names.insert({JR, "jr"});
			latencies.insert({JALR, params.find<uint32_t>("jalr", 1)});
			names.insert({JALR, "jalr"});
			latencies.insert({J, params.find<uint32_t>("j", 1)});
			names.insert({J, "j"});
		}

		bool Core::tick(Cycle_t cycle)
		{
			std::cout << "tick" << std::endl;
			total_sim_time++;
			if (!busy)
			{
				fetch_instruction();
				busy = true;
			}
			// Block waiting for memory!
			if (!waiting_memory)
			{
				if (latency_countdown == 0)
				{
					execute_instruction();
					if (instruction_count)
					{
						instruction_count->addData(1);
					}
				}
				else
				{
					latency_countdown--;
				}
			}
			return terminate;
		}

		void Core::fetch_instruction()
		{
			// Better be aligned!!
			uint16_t instruction = program[pc / 2];
			uint16_t opcode = instruction >> 11;
			std::cout << "Fetched " << names[opcode] << std::endl;
			try
			{
				latency_countdown = latencies.at(opcode) - 1; // This cycle counts as 1
			}
			catch (std::out_of_range &e)
			{
				std::cerr << "Unknown instruction: opcode " << opcode << std::endl;
				exit(EXIT_FAILURE);
			}
		}

		void Core::execute_instruction()
		{
			// Better be aligned!!
			uint16_t instruction = program[pc / 2];
			uint16_t opcode = instruction >> 11;
			uint16_t rd, rs, rt, imm8, imm11;

			std::cout << "Running " << names[opcode] << std::endl;

			opcode_count[opcode]++;
			instruction_execution_count++;

			switch (opcode)
			{
			case LW:
				rs = (instruction >> 5) & 0x07;
				rd = (instruction >> 8) & 0x07;
				waiting_memory = true;
				memory_wrapper->read(registers[rs], [this, rd](uint16_t addr, uint16_t data)
									 {
				registers[rd]=data;
				pc+=2;
				busy = false;
				waiting_memory = false; });

				break;
			case SW:
				rs = (instruction >> 5) & 0x07;
				rt = (instruction >> 2) & 0x07;
				waiting_memory = true;
				memory_wrapper->write(registers[rs], registers[rt], [this](uint16_t addr)
									  {
				pc+=2;
				busy = false;
				waiting_memory = false; });

				break;
			case LIZ:
				rd = (instruction >> 8) & 0x07;
				imm8 = instruction & 0xFF;
				registers[rd] = imm8;
				pc += 2;
				busy = false;

				break;
			case PUT:
				rs = (instruction >> 5) & 0x07;
				std::cout << "Register " << (int)rs << " = " << registers[rs] << "(unsigned) = " << (int16_t)registers[rs] << "(signed)" << std::endl;
				pc += 2;
				busy = false;

				break;
			case HALT:
				pc += 2;
				primaryComponentOKToEndSim();
				// unregisterExit();
				busy = false;

				break;
			case ADD:
				R_FORMAT_INSTRUCTION;
				registers[rd] = registers[rs] + registers[rt];
				pc += 2;
				busy = false;

				break;
			case SUB:
				R_FORMAT_INSTRUCTION;
				registers[rd] = registers[rs] - registers[rt];
				pc += 2;
				busy = false;

				break;
			case AND:
				R_FORMAT_INSTRUCTION;
				registers[rd] = registers[rs] & registers[rt];
				pc += 2;
				busy = false;

				break;
			case NOR:
				R_FORMAT_INSTRUCTION;
				registers[rd] = ~(registers[rs] | registers[rt]);
				pc += 2;
				busy = false;

				break;
			case DIV:
				R_FORMAT_INSTRUCTION;
				registers[rd] = registers[rs] / registers[rt];
				pc += 2;
				busy = false;

				break;
			case MUL:
				R_FORMAT_INSTRUCTION;
				registers[rd] = registers[rs] * registers[rt];
				pc += 2;
				busy = false;

				break;
			case MOD:
				R_FORMAT_INSTRUCTION;
				registers[rd] = registers[rs] % registers[rt];
				pc += 2;
				busy = false;

				break;
			case EXP:
				R_FORMAT_INSTRUCTION;
				registers[rd] = (uint16_t)std::pow(registers[rs], registers[rt]);
				pc += 2;
				busy = false;

				break;
			case LIS:
				I_FORMAT_INSTRUCTION;
				registers[rd] = (imm8 & 0x80) ? (0xFF00 | imm8) : imm8;
				pc += 2;
				busy = false;

				break;

			case LUI:
				I_FORMAT_INSTRUCTION;
				// $rd←imm8.$rd7..$rd0
				registers[rd] = (imm8 << 8) | (registers[rd] & 0x00FF);
				pc += 2;
				busy = false;

				break;
			case BP:
				I_FORMAT_INSTRUCTION;

				if ((int16_t)registers[rd] > 0)
				{

					pc += (uint16_t)((imm8 << 1));
				}
				else
					pc += 2;
				busy = false;

				break;
			case BN:
				I_FORMAT_INSTRUCTION;

				if ((int16_t)registers[rd] < 0)
				{
					pc += (uint16_t)((imm8 << 1));
				}
				else
					pc += 2;
				busy = false;

				break;
			case BX:
				I_FORMAT_INSTRUCTION;

				if (registers[rd] != 0)
				{
					pc += (uint16_t)((imm8 << 1));
				}
				else
					pc += 2;
				busy = false;

				break;
			case BZ:
				I_FORMAT_INSTRUCTION;

				if (registers[rd] == 0)
				{
					pc += (uint16_t)((imm8 << 1));
				}
				else
					pc += 2;
				busy = false;

				break;
			case JR:
				R_FORMAT_INSTRUCTION;
				pc = registers[rd] & 0xFFFE;
				busy = false;

				break;
			case JALR:
				R_FORMAT_INSTRUCTION;
				registers[rd] = pc + 2;
				pc = registers[rs] & 0xFFFE;
				busy = false;

				break;
			case J:
				IX_FORMAT_INSTRUCTION;
				// PC←PC15..PC12.(imm11«1)
				pc = (pc & 0xF000) | (imm11 << 1);
				busy = false;

				break;
			}
			if (opcode == HALT)
			{
				terminate = true;
			}
		}

	}
}
