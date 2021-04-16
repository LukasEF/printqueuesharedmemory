#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include "LinkedList.h"

//Printjob structure to hold details
struct printJob
{
   int ID;
   int priority;
};

int main(int argc, char *argv[])
{
   //Validate command line arguments
   if(argc < 3)
   {
      printf("Not all command line arguments have been entered \n");
      return 0;
   }
   if(atoi(argv[1]) < 1 || atoi(argv[2]) < 1)
   {
      printf("Size of printer queue cannot be less than one \n");
      printf("Number of prints per cycle cannot be less than one \n");
      return 0;
   }
   if(atoi(argv[1]) < atoi(argv[2]))
   {
      printf("Size of printer queue cannot be smaller than the number of print jobs produced per cycle\n");
      return 0;
   }
   
   //Initialize variables
   int pid;
   int init;
   
   //Generate unique key for printer queue
   printf("Generating printer queue key...\n\n");
   key_t key = ftok("shmfile",65);

   //Size of memory for printer queue in bytes
   printf("Calculating size of memory...\n\n");
   size_t size = sizeof(struct printJob) * atoi(argv[1]);

   //Get the shared memory for printer queue
   printf("Retrieving shared memory...\n\n");
   int shared_memory = shmget(key,size,0666|IPC_CREAT);

   //Attach struct pointer to shared memory
   printf("Attaching pointer to shared memory...\n\n");
   struct printJob *jobs = (struct printJob*) shmat(shared_memory,(void*)0,0);
   
   //Generate unique key for mutual exclusion
   key_t checkkey = ftok("check",64);

   //Get the shared memory for mutual exclusion
   int checkshm = shmget(checkkey,sizeof(char),0666|IPC_CREAT);

   //Attach char pointer to mutual exclusion shared memory
   char *check = (char*) shmat(checkshm,(void*)0,0);

   //Get length of printer queue
   int length = atoi(argv[1]) + 1;
   //Get number of printjobs per cycle
   int cycles = atoi(argv[2]);
   
   //Initialise printer queue
   printf("Initialising printer queue...\n\n");
   for(init = 0; init < length; init++)
   {
      jobs[init].ID = 0;
      jobs[init].priority = 0;
   }
   *check = 'N';

   //Wait for consumer to be ready
   printf("Waiting for signal from consumer...\n");
   while(*check != 'A');
   
   //Initialise variables relating to the production of printjobs
   int i;
   int logcount = 0;
   int loopcount = 0;
   int forkcount = 1;
   int choice = 2;
   *check = 'P';
   
   //Runs until the user decides to exit
   while(choice != 1)
   {
      //Add print jobs to queue only if the consumer is not currently active
      if(*check == 'P')
      {
         //Adds number of printjobs to queue, specified by the user for each cycle
         for(i = 0; i < cycles; i++)
	 {
	    //Create child process
	    pid = fork();
	    
	    //Exit program if fork() error
	    if(pid < 0)
	    {
	       perror("fork");
	       exit(1);
	    }
	    
	    //Only child process executes this code
	    if(pid == 0)
	    {
	       int j = 0;
	       int r = 0;
	       struct printJob p;
	       srand(time(0));

	       //Generate random priority level 1-5
	       r = rand() % 5;
	       if(r == 0)
	       {
	          r = 1;
	       }
	       
	       //Assign child ID and priority level to struct
	       p.ID = getpid();
	       p.priority = r;
	       
	       //Attach printjob to free space in shared memory
	       //Loop through printer queue
	       for(j = 0; j < length; j++)
	       {
	          //Look for empty space
		  if(jobs[j].ID == 0)
		  {
		     //Fill empty space with new print job
		     jobs[j] = p;
		     //Alert user as to which position in the queue the new job is and its details
		     printf("====================================================\n");
		     printf("Print No. %d\n",forkcount);
		     printf("New print job added at position %d in the print queue\n",j);
		     printf("ID = %d\nPriority = %d\n",jobs[j].ID, jobs[j].priority);
		     printf("====================================================\n\n");
		     //Exit loop
		     j = length;
		  }
		  //Checks to see if all spots in the queue contain a print job
		  else if(j == length-1 && jobs[j].ID != 0)
		  {
		     printf("No space available in queue for print job %d...\n\n",forkcount);
		  }
	       }
	       //Exit child process
	       exit(0);
	    }
	    //Increment number of print jobs (a.k.a. forks)
            forkcount++;
	    //Wait for 5 seconds for child process to finish
	    sleep(5);
	 }
	 //Increment number of loops
	 loopcount++;

	 //Wait for the number of print jobs to be equal to the number the user requested before continuing
	 while(forkcount != cycles + 1);

	 //Create a new datalog if one does not exist already
	 if(logcount == 0)
	 {
	    createNewLog(logcount+1, forkcount);
	    printf("Started data log...\n");
	 }
	 //Add new datalog to the current data structure
	 else
	 {
	    addNewLog(logcount+1, forkcount);
	    printf("Added log %d\n",logcount+1);
	 }

	 //Increment number of logs
	 logcount++;
	 //Reset the forkcount variable
	 forkcount = 1;
	 //Inform user to check the consumer process
	 printf("Finished producing check consumer...\n\n");
	 //Allows the producer process to procede
	 *check = 'C';
	 
	 //Check if looped 5 times
	 if(loopcount == 5)
	 {
	    //Present user with option to quit process
	    printf("STOP PRINTING?\n");
	    printf("1 = YES\n2 = NO\n");
	    scanf("%d", &choice);
	    while(choice > 2 || choice < 1)
	    {
	       printf("Invalid option\n Try again... ");
	       scanf("%d", &choice);
	    }
	    
	    //Signal consumer to finish
	    if(choice == 1)
	    {
	       //Waits for the producer to finish printing before exiting
	       printf("Waiting for consumer to finish printing...\n\n");
	       while(*check != 'P');
	       //Signals for the producer to exit
	       *check = 'D';
	       //Print the details of all datalogs
	       printf("PRINTING ALL DATALOGS...\n");
	       displayAllLogs();
	    }
	    //Continue making new printjobs
	    else if(choice == 2)
	    {
	       //Reset loop count
	       loopcount =  0;
	    }
	 }

      }
      else
      {
         sleep(1);
      }

   }
   
   //Detach from shared memory
   shmdt(jobs);

   printf("Exiting producer...\n");
   
   return 0;
}
