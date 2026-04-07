#pragma once
#include <sst/core/interfaces/stdMem.h>
#include <sst/core/componentExtension.h>
#include <functional>

namespace XSim
{
namespace Core
{


class MemoryWrapper: public SST::ComponentExtension
{
	private:
		// A simple SST memory interface: http://sst-simulator.org/sst-docs/docs/core/iface/StandardMem/class
		using StandardMem = ::SST::Interfaces::StandardMem;
		using ComponentId_t = ::SST::ComponentId_t;
		using ComponentInfo = ::SST::ComponentInfo;
		using Params = ::SST::Params;
		using Output = ::SST::Output;
		using TimeConverter = ::SST::TimeConverter;

	public:
		// Pass in a reference to the core, and configurations/output of simulation
		MemoryWrapper(ComponentId_t id, Params& params, Output* output, TimeConverter *tc);

		// Disable copies of this instance
		MemoryWrapper(const MemoryWrapper &other) = delete;
		MemoryWrapper(MemoryWrapper &&other) = delete;
		MemoryWrapper &operator=(const MemoryWrapper &other) = delete;
		MemoryWrapper &operator=(MemoryWrapper &&other) = delete;

		// Destructor (called when instance is deleted)
		virtual ~MemoryWrapper() noexcept = default;

		void init(unsigned int phase);

		void read(uint16_t address, std::function<void(uint16_t, uint16_t)> cb);
		void write(uint16_t address, uint16_t data, std::function<void(uint16_t)> cb);

	protected:
		std::map<uint64_t, std::function<void(uint16_t)> > w_callbacks;
		std::map<uint64_t, std::function<void(uint16_t, uint16_t)> > r_callbacks;
	private:
		// Link to SST StandardMem
		StandardMem *data_memory_link{nullptr};

		void callback(StandardMem::Request *req);


};

}
}
