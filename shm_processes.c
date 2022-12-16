#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

//Modified code of Lab 3 part 2 


void ClientProcess(int []);
void ParentProcess(int [], sem_t *);
void ChildProcess(int [], sem_t *, int);
void MomProcess(int [], sem_t *);

//define ShmID; ShmPTR; pid; status; mutex
int  main(int  argc, char *argv[])
{
     int    ShmID, i;
     int    *ShmPTR;
     pid_t  pid;
     int    status;
     sem_t *mutex;

     srand (time(NULL));	// seeds the rand num generator

// ShmID sets IPC_PRIVATE
// if statement whether its greater than 0

     ShmID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }


     printf("Server has received a shared memory of four integers...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }


    if ((mutex = sem_open("examplesemaphore5", O_CREAT, 0644, 1)) == SEM_FAILED) {
          perror("semaphore initilization");
         exit(1);
     }
     printf("Server has attached the shared memory...\n");

     printf("Server is about to fork a child process...\n");
     pid = fork();
     if (pid < 0) {
          printf("*** fork error (server) ***\n");
          exit(1);
     }
     else if (pid == 0) {
       while (1){
         ChildProcess(ShmPTR, mutex, getpid());
       }
       exit(0);
     }
     while (1){
       ParentProcess(ShmPTR, mutex);
     }

     wait(&status);
     printf("Server has detected the completion of its child...\n");
     shmdt((void *) ShmPTR);
     printf("Server has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Server has removed its shared memory...\n");
     printf("Server exits...\n");
     exit(0);
}


void ChildProcess(int sharedMem[], sem_t* mutex, int id){
  int localBalance = 0;
  int asleep = rand() % 5;
  int try = 0;
  int need = 0;
  //sharedMem[0] is equal to the BankAccount

  sleep(asleep);
  printf("Poor Student: Attempting to Check Balance\n");
  try = rand() % 100;
  sem_wait(mutex);

  if (try % 2 == 0){
    localBalance = sharedMem[0];
    need = rand() % 50;
    printf("Poor Student needs $%d\n", need);
    if (need <= localBalance){
      localBalance -= need;
      printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
    }else{
      printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
    }
    sharedMem[0] = localBalance;
  }else{
    printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
  }
  sem_post(mutex);

}


void ParentProcess(int sharedMem[], sem_t* mutex) {
  int localBalance = 0;
  int try = 0;
  int amount = 0;
  int asleep = rand() % 5; 
  //sharedMem[0] is equal to the BankAccount
  
  sleep(asleep);
  printf("Dear Old Dad: Attempting to Check Balance\n");
  sem_wait(mutex);

  try = rand() % 100;
  if (try % 2 == 0){
    if (localBalance < 100){
      localBalance = sharedMem[0];
      amount = rand() % 100;
      if (amount % 2 == 0){
        localBalance += amount;
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amount, localBalance);
        sharedMem[0] = localBalance;        
      }else{
        printf("Dear old Dad: Doesn't have any money to give\n");
      }
    }else{
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
    }
  }else{
    printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
  }

  sem_post(mutex);
  
}
