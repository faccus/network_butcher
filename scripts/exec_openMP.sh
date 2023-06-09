for i in 2 4 8 16
do
   mkdir -p ../results/FINAL/Original/OpenMP/$i
   export OMP_NUM_THREADS=$i
   echo $OMP_NUM_THREADS


   ./main_kfinder
   ./main_synthetic_graph
   ./main_Constrained_Builder
   ./main_IO_Manager
   ./main_path_reconstruction
   mv report_* ../results/FINAL/Original/OpenMP/$i
   
done