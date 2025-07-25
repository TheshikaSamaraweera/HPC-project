# serial vertion
## gcc main_serial.c gemini_chatbot.c metrics.c -o gemini_main -lcurl -lcjson
## run     - ./gemini_main



# OMP vertion
## compil  -   gcc main.c OMP_parallel.c gemini_chatbot.c metrics.c -o chatbot_omp -lcurl -lcjson -fopenmp
## run     - ./chatbot_omp prompt.txt 


# MPI vertion
## compil  -   mpicc -o chatbot_mpi main_mpi.c MPI_chatbot.c gemini_chatbot.c metrics.c -lcurl -lcjson
## run     - mpirun -np 5 ./chatbot_mpi prompt.txt