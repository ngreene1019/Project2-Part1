#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include  <unistd.h>
#include  <sys/wait.h>
#include  <semaphore.h> // for sem_t
#include <fcntl.h>
#include  <time.h>

void  ClientProcess(void*,sem_t*);
void  ParentProcess(void*,sem_t*);

// these two integers will be shared between processes
// only one process can modify these values at one time
int turn = 0;
int bankAccount = 0;


int  main(int  argc, char *argv[])
{
	int    ShmID;
    //void* because we don't know if sem_t is long long or something
	void   *ShmPTR; // this will point to array that holds the values that are passed in via argv[]
	pid_t  pid;
	int    status;
    int    err   ;
    //shared memory variables
    int*   bankAccount   ;
    int*   counter       ;
    sem_t* mutex         ; //initialize with sem_init
    sem_t* mutex_        ; //initialize with sem_open
  
	int i = 0;

	srand (time(NULL));	// seeds the rand num generator

	//taking commandline args
	if (argc != 5) {
		printf("Use: %s #1 #2 #3 #4\n", argv[0]); // four integers
		exit(1);
	}
  
    //this line applies to using sem_init in combo with shmget
    if (sizeof(sem_t) < sizeof(int)) { printf ("this program assumes sem_t is as large or larger than int\n") ; exit (1) ; }
    
    //not assuming sem_t is the same size as int although it very well may be
	ShmID = shmget(IPC_PRIVATE, sizeof(sem_t) + 2 * sizeof(int), IPC_CREAT | 0666);
	if (ShmID < 0) {
		printf("*** shmget error (server) ***\n");
		exit(1);
	}
	printf("Server has received a shared memory of four integers...\n");

  //sem_open knows to make sure semaphore is in shared memory
  mutex_ = sem_open ("ddad-pstud-mex",O_CREAT,0644,1) ;
    
	ShmPTR = shmat(ShmID, NULL, 0);
	if (ShmPTR == (void*)-1) {
		printf("*** shmat error (server) ***\n");
		exit(1);
	}
	printf("Server has attached the shared memory...\n");

    //look up how to init semaphores
    
	  // storing the args into your array
    // making aliases to different parts of shared memory so we don't have to memorize where the variables are stored
    mutex         = (sem_t*)ShmPTR             ;
    bankAccount   = (int*) (mutex         + 1) ;
    //sem_wait (mutex_counter) , sem_post (mutex_counter)
    
    //initialize semaphore to 1 so the first wait() does not block, other variables to corresponding start values
    //documentation doesn't say if the non-zero value has to be 1, 2 or some constant so just going to use 1 for process-wise semaphore
    err = sem_init (mutex,1,1) ;
    //not parallel access yet, safe to assign the values from here
    * bankAccount   = 2000 ; // let's start off the student with 2000 bucks in bank
      

	//printf("Server has filled %d %d in shared memory...\n", ShmPTR[0], ShmPTR[1]);

	printf("Server is about to fork a child process...\n");
	pid = fork();  // this forkes the first process
    //now in a new process
	if (pid < 0) {
		printf("*** fork error (server) ***\n");
		exit(1);
	}
	else if (pid == 0) {
        //CHILD PROCESS
		for (i = 0; i < 25; i++){
			ClientProcess(ShmPTR,mutex_);  // this when the child gets control
		}
		
		exit(0);
	}
    else {
        //PARENT PROCESS
		for (i = 0; i < 25; i++){
  			ParentProcess(ShmPTR,mutex_);  // parent gets control
		}
		wait(&status); // parent waits for child
    err = sem_destroy (mutex) ;
    err = sem_close   (mutex_) ;
		printf("Server has detected the completion of its child...\n");
		shmdt((void *) ShmPTR);
		printf("Server has detached its shared memory...\n");
		shmctl(ShmID, IPC_RMID, NULL);
		printf("Server has removed its shared memory...\n");
	
	
	
		printf("Server exits...\n");
		exit(0);
    }
}


//FILE 
void  ClientProcess(void*  ShmPTR,sem_t* mutex_)
{
    //immediately map shared memory to aliases so we get that out of the way
    sem_t* mutex         = (sem_t*)ShmPTR             ;
    int*   bankAccount   = (int*) (mutex         + 1) ;
    int*   counter       =        (bankAccount   + 1) ;
  
	int  account = 0;  // this is a copy of accountBalance (the shared value)
	int  need    = 0; //  this is the random amount the child wants, an expense of living ie I wanna buy a new shirt.
	int  asleep  = 0; // this is the random amount of sleep time
    
  
	asleep = rand() % 5;
  
	// sleep 0 -5 s
	sleep(asleep);

  
	// upon waking
    //while(ShmPTR[0] != 1);
    //while we are reading from ShmPTR another process (parent) can be changing it at the same time
    
    sem_wait (mutex_) ;
    //printf ("--> CHILD critical section\n") ;
	  account = * bankAccount ;
    //can proceed to do stuff with account because we're not messing with shared memory for a bit

	// randomly general int between 0 - 50
	need = rand() % 50;
    printf("Poor Student needs $%d\n", need);


	if (need <= account){
		account = account - need ;
        printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, account);
	}
	else
		printf("Poor Student: Not Enough Cash ($%d)\n", account);

    //lock for the whole time so that other process can't change the value while until we know we're not going to change it anymore
    
    * bankAccount = account ;
    //printf ("<-- CHILD critical section\n") ;
    sem_post (mutex_) ;
}

void  ParentProcess(void*  ShmPTR,sem_t* mutex_)
{
    //immediately map shared memory to aliases so we get that out of the way
    sem_t* mutex         = (sem_t*)ShmPTR             ;
    int*   bankAccount   = (int*) (mutex         + 1) ;
    int*   counter       =        (bankAccount   + 1) ;
  
	int account           = 0;  // this is a copy of accountBalance (the shared value)
	int give              = 0; //  this is the random amount the parent wants
    int do_i_want_to_give = 0 ;//coin toss, 1 for true, 0 for false
	int asleep            = 0; // this is the random amount of sleep time
  
	asleep = rand() % 5;

	// sleep 0 -5 s
	sleep(asleep);
  
 
    //while(ShmPTR[0] != 0); relic of last project
  
    //check student's bank account
    sem_wait (mutex_) ;
    //printf ("--> PARENT critical section\n") ;
    account = * bankAccount ;
  
    do_i_want_to_give = rand() % 2 == 0 ;
    if (do_i_want_to_give) {
     //account is low
     if (account < 100) {
       //give is the amount of money to transfer
       give = rand() % 100;
          // deposit 
          //only give when the generated cash quantity is even
		  if (give % 2 == 0) {
			  account += give ;
			  printf("Dear old Dad: Deposits $%d / Balance = $%d\n", give, account);
		   }  else{
			  printf("Dear old Dad: Doesn't have any money to give\n");
		   } 
           
          * bankAccount = account ;
          
       }
     }
  	 else{
	     	printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
     }
      
    //printf ("<-- PARENT critical section\n") ;
     sem_post (mutex_) ;
}