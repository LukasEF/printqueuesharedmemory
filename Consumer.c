#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include "LinkedList.h"

//Printjob structure
struct printJob
{
   int ID;
   int priority;
};

int main(int argc, char *argv[])
{
   //Validating command line arguments
   if(argc < 3)
   {
      printf("Not all command line arguments have been entered \n");
      return 0;
   }

   //Get length of printer queue from the producer
   int length = atoi(argv[1]);
   //Store the number of print cycles in a variable
   int numloops = atoi(argv[2]);

   if(numloops > length)
   {
      printf("Number of print cycles cannot be larger than the length of the printer queue\n");
      return 0;
   }
   if(length < 1 || numloops < 1)
   {
      printf("Size of printer queue or the number of print jobs printed per cycle cannot be less than one\n");
   }

   //Generate unique key for printer queue
   printf("Generating unique key...\n\n");
   key_t key = ftok("shmfile",65);

   //Size of memory for printer queue
   printf("Calculating size of memory...\n\n");
   size_t size = sizeof(struct printJob) * length;

   //Get the shared memory for printer queue
   printf("Retrieving shared memory...\n\n");
   int shared_memory = shmget(key,size,0666|IPC_CREAT);

   //Attach struct pointer to shared memory
   printf("Attaching pointer to shared memory\n\n");
   struct printJob *jobs = (struct printJob*) shmat(shared_memory,(void*)0,0);

   //Generate unique key for mutual exclusion
   key_t checkkey = ftok("check",64);

   //Get the shared memory for mutual exclusion
   int checkshm = shmget(checkkey,sizeof(char),0666|IPC_CREAT);

   //Attach char pointer to mutual exclusion shared memory
   char *check = (char*) shmat(checkshm,(void*)0,0);

   //Send signal to producer that consumer is ready
   *check = 'A';
   
   //Initialise variables for reading from memory
   int i;
   int pri;
   int printcount;
   int logcount = 0;

   printf("Waiting for producer to add to the print queue...\n");

   //Process will run until it recieves a stop signal from producer
   while(*check != 'D')
   {
      //Reset priority and printcount variables
      pri = 0;
      printcount = 0;

      //Wait for producer to finish and have sent its signal
      if(*check == 'C')
      {
         printf("NEW PRINT CYCLE\n");
	 printf("=================================\n");
         //Prints only the number of jobs requested by the user for each cycle
         while(printcount < numloops)
	 {
	    pri++;

	    //Start looping through printer queue
	    for(i = 1; i < length; i++)
	    {
	       //Check if the current print job has the current priority level
	       if(jobs[i].priority == pri)
	       {
	          printf("Printing...\n");
		  sleep(0.5);
		  printf("Position in queue %d\n",i);
		  printf("Printjob ID: %d\n",jobs[i].ID);
		  printf("PRIORITY: %d\n\n",jobs[i].priority);
		  //Increment number of prints for this cycle
		  printcount++;
		  jobs[i].ID = 0;
		  jobs[i].priority = 0;
		  sleep(1);
	       }

	       //Check if number of prints is equal to the limit the user requested
	       if(printcount == numloops)
	       {
	          //Stop looping through the printer queue
	          i = length;
	       }
	    }
	 }


	 //Creates new linked list data structure to hold the datalogs if it does not already exist
	 if(logcount == 0)
	 {
	    createNewLog(logcount + 1, printcount);
	    printf("Started data log...\n");
	 }
	 //Adds a new datalog to the linked list
	 else
	 {
	    addNewLog(logcount + 1, printcount);
	    printf("Added log %d\n",logcount+1);
	 }
	 //Increment the number of datalogs
	 logcount++;

	 printf("=================================\n\n");
	 printf("Finished printing check producer...\n\n");
	 //Return signal to producer to alert it of print cycle completion
	 *check = 'P';
      }
      else
      {
         sleep(1);
      }
      
   }
   
   printf("Detaching from shared memory...\n");
   //Detach from shared memory
   shmdt(jobs);
   
   printf("Destroying shared memory...\n\n");
   //Destroy shared memory
   shmctl(shared_memory,IPC_RMID,NULL);

   printf("PRINTING ALL DATALOGS...\n");
   //Display all the datalogs to the user before exiting
   displayAllLogs();

   printf("Exiting consumer...\n");
   
   return 0;
}

