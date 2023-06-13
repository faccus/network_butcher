mkdir -p ../../profiling_results/FINAL/TBB

./main_kfinder
./main_synthetic_graph
./main_Constrained_Builder
./main_IO_Manager
./main_path_reconstruction

mv report_* ../../profiling_results/FINAL/TBB