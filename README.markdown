
# Skeleton project

Use this code as a basis for your project. The code implements a simple SST CPU with memory access.

# Software dependencies

> SST 15.1 (may work with other versions, but untested)
> CMake 3.5 (may work with other versions, but untested)
JSONcpp

# Build

sh build.sh

# Run
sh run.sh

# Files
	- `sst` directory the core implementation
		- Includes a memory interface so you don't have to worry about it
	- build and run scripts - use them to check the steps to build and run the simulation
	- latencies and programs - example of the configuration and program that our CPU should run.
	- config.py - the file that sst uses to setup and run the simulation
