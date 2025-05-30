cpu_sequential:
	pgc++ -o cpu -lboost_program_options -acc=host -Minfo=all -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.11/cuda/12.3/include cpu.cpp
	./cpu --size=128 --accuracy=0.000001 --max_iterations=1000000

cpu_multicore:
	pgc++ -o cpu_multicore -lboost_program_options -acc=multicore -Minfo=all -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.11/cuda/12.3/include cpu.cpp
	./cpu_multicore --size=1024 --accuracy=0.000001 --max_iterations=1000000

gpu:
	pgc++ -o gpu -lboost_program_options -acc=gpu -Minfo=all -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.11/cuda/12.3/include gpu.cpp
	./gpu --size=128 --accuracy=0.000001 --max_iterations=1000000
profile:
	nsys profile --trace=nvtx,cuda,openacc --stats=true ./gpu --size=256 --accuracy=0.0001 --max_iterations=50