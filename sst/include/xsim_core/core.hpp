#pragma once

#include <sst/core/component.h>
// #include <sst/core/elementinfo.h>
// SST datatypes
#include <sst/core/sst_types.h>
// SST output to print information
#include <sst/core/output.h>
// SST interface for memory
#include <sst/core/interfaces/stdMem.h>
// SST statistics
#include <sst/core/statapi/stataccumulator.h>

#include <xsim_core/memory_wrapper.hpp>
#include <xsim_core/opcodes.hpp>

namespace XSim
{
	namespace Core
	{

		class Core : public SST::Component
		{
		public:
			// SST registration -- See https://sst-simulator.org/sst-docs/docs/guides/dev/devtutorial
			// SST example CPU with memory -- See https://github.com/sstsimulator/sst-elements/blob/master/src/sst/elements/memHierarchy/testcpu/scratchCPU.h
			SST_ELI_REGISTER_COMPONENT(
				Core,
				"XSim",
				"Core",
				SST_ELI_ELEMENT_VERSION(1, 0, 0),
				"Simple MIPS-based simulator",
				COMPONENT_CATEGORY_PROCESSOR)

			SST_ELI_DOCUMENT_PARAMS(
				{"verbose", "(uint) Verbosity for debugging. Increased numbers for increased verbosity.", "0"},
				{"clock_frequency", "(string) Sets the clock of the core in Hz", "0"},
				{"program", "(infile) Path to program to be executed by the simulator", "REQUIRED"},
				{"liz", "(uint) Latency of the liz instruction", "1"},
				{"put", "(uint) Latency of the put instruction", "1"},
				{"sw", "(uint) Latency of the sw instruction", "1"},
				{"lw", "(uint) Latency of the lw instruction", "1"},
				{"halt", "(uint) Latency of the halt instruction", "1"},
				{"add", "(uint) Latency of the add instruction", "1"},
				{"sub", "(uint) Latency of the sub instruction", "1"},
				{"and", "(uint) Latency of the and instruction", "1"},
				{"nor", "(uint) Latency of the nor instruction", "1"},
				{"div", "(uint) Latency of the div instruction", "1"},
				{"mul", "(uint) Latency of the mul instruction", "1"},
				{"mod", "(uint) Latency of the mod instruction", "1"},
				{"exp", "(uint) Latency of the exp instruction", "1"},
				{"lis", "(uint) Latency of the lis instruction", "1"},
				{"lui", "(uint) Latency of the lui instruction", "1"},
				{"bp", "(uint) Latency of the bp instruction", "1"},
				{"bn", "(uint) Latency of the bn instruction", "1"},
				{"bx", "(uint) Latency of the bx instruction", "1"},
				{"bz", "(uint) Latency of the bz instruction", "1"},
				{"jr", "(uint) Latency of the jr instruction", "1"},
				{"jalr", "(uint) Latency of the jalr instruction", "1"},
				{"j", "(uint) Latency of the j instruction", "1"},
				{"output", "(string) Output file for statistics (JSON)", "stats.json"})

			// Statistics for our component
			SST_ELI_DOCUMENT_STATISTICS(
				{"instructions", "Number of instructions executed", "", 0})

			// This is used to connect the memory interface, thus we don't need to implement one!
			SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS({"memory", "Interface to memory (e.g., caches)", "SST::Interfaces::StandardMem"})

		private:
			using Super = SST::Component;
			template <typename Type>
			using Statistics = SST::Statistics::Statistic<Type>;
			using Cycle_t = SST::Cycle_t;
			using StandardMem = SST::Interfaces::StandardMem;
			using Output = SST::Output;
			using ComponentId_t = SST::ComponentId_t;
			using Params = SST::Params;
			using Clock = SST::Clock;
			using TimeConverter = ::SST::TimeConverter;

		public:
			/** These functions are defined by SST **/
			Core(ComponentId_t id, Params &params);
			virtual void init(unsigned int phase) override final;
			virtual void setup() override final;
			virtual void finish() override final;
			bool tick(Cycle_t cycle);

		private:
			/** For completeness **/
			Core();
			Core(const Core &params);
			Core operator=(const Core &params);
			virtual ~Core() = default;

		protected:
			// Method to load the program from a file
			void load_program(Params &params);
			// Method to load instruction latencies
			void load_latencies(Params &params);

		private:
			/** Parameters **/
			// Prints verbose information
			int verbose{0};
			// The core clock frequency
			std::string clock_frequency{"0Hz"};
			// The vector with the program instructions
			std::vector<uint16_t> program;
			// The map with instruction latencies
			std::map<uint32_t, uint32_t> latencies;
			// The map with instruction names (for printing)
			std::map<uint32_t, std::string> names;
			// Map how many times each opcode is executed (for statistics)
			std::map<uint32_t, uint64_t> opcode_count;
			// Total sim time (for statistics)
			uint64_t total_sim_time{0};
			// Total instructions executed
			uint64_t instruction_execution_count{0};
			// output file
			std::string output_file;

			/** CPU state **/
			// The program counter
			uint32_t pc{0};
			// Registers
			std::array<uint16_t, 8> registers;
			// Busy processing instruction
			bool busy{false};
			// Busy processing instruction
			int latency_countdown{0};
			// Waiting for memory return
			bool waiting_memory{false};
			// Finished simulation
			bool terminate{false};

			// SST managed output
			Output *output{nullptr};

			// Wrapper to simplify memory access with SST's memory interface
			MemoryWrapper *memory_wrapper;

			/** Functions to process events **/
			// Fetch a new instruction
			void fetch_instruction();
			// Execute a new instruction
			void execute_instruction();

			// Statistics definitions
			Statistics<uint64_t> *instruction_count;

			// TimeConverter -> memory needs this
			TimeConverter *tc{nullptr};
		};

	}
}
