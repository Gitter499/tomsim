#include <xsim_core/memory_wrapper.hpp>

#define print_request(req_to_print) (std::cout<<(req_to_print)->getString()<<std::endl)


namespace XSim
{
namespace Core
{

MemoryWrapper::MemoryWrapper(ComponentId_t id, Params &params, Output *output, TimeConverter *tc):
	ComponentExtension(id)
{
	//
	// The following code creates the SST interface that will connect to the memory hierarchy
	//
	StandardMem::Handler2<MemoryWrapper, &MemoryWrapper::callback> *data_handler=
		new StandardMem::Handler2<MemoryWrapper, &MemoryWrapper::callback>(this);

	const std::string port_name = "memory";
	data_memory_link = loadUserSubComponent<SST::Interfaces::StandardMem>(port_name, ComponentInfo::SHARE_NONE, tc, data_handler);
	
	if(!data_memory_link)
	{
		output->fatal(CALL_INFO, -1, "Error loading memory interface module.\n");
	}
	else
	{
		output->verbose(CALL_INFO, 1, 0, "Loaded memory interface successfully.\n");
	}
}

void MemoryWrapper::init(unsigned int phase) {
	data_memory_link->init(phase);
}

void MemoryWrapper::callback(StandardMem::Request *req)
{
	std::cout<< "Got here"<<std::endl;
	try
	{
		StandardMem::WriteResp *write_response = dynamic_cast<StandardMem::WriteResp*>(req);
		StandardMem::ReadResp *read_response = dynamic_cast<StandardMem::ReadResp*>(req);
		if(write_response)
		{
			w_callbacks.at(write_response->getID())(write_response->pAddr);
			w_callbacks.erase(write_response->getID());
		}
		else if(read_response)
		{
			uint16_t data = (read_response->data[0]<<8)|read_response->data[0+1]; // BE
			r_callbacks.at(read_response->getID())(read_response->pAddr, data);
			r_callbacks.erase(read_response->getID());
		}
	}
	catch(std::exception &e)
	{
		std::cout<<"No such callback"<<std::endl;
		exit(EXIT_FAILURE);
	}
	if (req) delete req;
}

void MemoryWrapper::read(uint16_t address, std::function<void(uint16_t, uint16_t)> cb)
{
	std::cout<< "Reading memory wrapper "<< address<<std::endl;
	StandardMem::Request *req=new StandardMem::Read(
				//uint64 physical addr
				address,
				//uint64 size
				sizeof(uint16_t)
	);
	r_callbacks.insert({req->getID(), cb});
	data_memory_link->send(req);
}

void MemoryWrapper::write( uint16_t address, uint16_t data, std::function<void(uint16_t)> cb)
{
	std::cout<< "Writting memory wrapper "<< address <<" "<<data<<std::endl;
	std::vector<uint8_t> data_vec;
	data_vec.push_back((data>>8)&0x00FF);
	data_vec.push_back(data&0x00FF);
	StandardMem::Request *req=new StandardMem::Write(
				//uint64 addr
				address,
				//uint64 size
				sizeof(uint16_t),
				//data
				data_vec
	);
	w_callbacks.insert({req->getID(), cb});
	data_memory_link->send(req);
}

}
}
