#include <time.h>

//Datalog structure
struct datalog
{
   int logid;
   struct tm * timeinfo;
   int printcount;
   struct datalog* nextlog;
}*stlog, *tmp;

//Function headers
void createNewLog(int id, int count);
void addNewLog(int id, int count);
void displayAllLogs();
struct tm * getCurrentTime();

//Function for getting current system time
struct tm * getCurrentTime()
{
   	 time_t now;
	 struct tm * currenttime;
	 time(&now);
	 currenttime = localtime(&now);

     return currenttime;
}

//Function for creating a new linked list of datalogs
void createNewLog(int id, int count)
{
   stlog = (struct datalog *)malloc(sizeof(struct datalog));

   if(stlog == NULL)
   {
      printf("Failed to create new datalog...\n");
   }
   else
   {
      stlog->logid = id;
      stlog->timeinfo = getCurrentTime();
      stlog->printcount = count;
      stlog->nextlog = NULL;
   }
   tmp = stlog;
}

//Function for adding a new datalog to the already existing data structure
void addNewLog(int id, int count)
{
   struct datalog *fnlog;
   fnlog = (struct datalog *)malloc(sizeof(struct datalog));
   if(fnlog == NULL)
   {
      printf("Failed to create new datalog...\n");
   }
   else
   {
      fnlog->logid = id;
      fnlog->timeinfo = getCurrentTime();
      fnlog->printcount = count;
      tmp->nextlog = fnlog;
      tmp = tmp->nextlog;
   }
}

//Function that prints the details of each datalog in the data structure
void displayAllLogs()
{
   struct datalog *view;
   if(stlog == NULL)
   {
      printf("No logs recorded...\n");
   }
   else
   {
      view = stlog;
      while(view != NULL)
      {
	 printf("===========================================\n");
         printf("LOG ID: %d\n",view->logid);
	 printf("Time of production cycle completion: %s\n", asctime(view->timeinfo));
	 printf("Number of print jobs in cycle: %d\n",view->printcount);
	 printf("===========================================\n\n");
	 view = view->nextlog;
      }
   }
}
