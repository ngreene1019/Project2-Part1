shm_proc: shm_processes.c
	gcc shm_processes.c -D_SVID_SOURCE -pthread -std=c99 -lpthread  -o shm_proc -g
example: example.c
	gcc example.c -pthread -std=c99 -lpthread  -o example