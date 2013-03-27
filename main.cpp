#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <algorithm>

using namespace std;

#define NUM_PROCESSES 3 
#define TIME_SLICE 1000
#define MAX_BURST 4000
#define MIN_BURST 500
#define MAX_PRIORITY 4

//Global Variables
int current_time = 0;
int tcs = 17; //Time for context switch

/*
Class to represent a process that enters the system
 - Maintains its own process time, priority, remaining time, and wait times
 - Used in the process queue class
*/
class Process{
	public:
		Process(int p, int t, int a_t, int pri);
		int runProcess();
		int getPid(){ return pid; }
		int getArrival(){ return arrival_time; }
		int processTime(){ return process_time; }
		int timeRemaining() { return time_remaining; }
		int getPriority() {return priority; }
		void getWaitTimes(int &i, int&t, int& w); 
		void resetWait();
	
	private: 
		int pid;
		int process_time;
		int time_remaining;
		int arrival_time;
		int initial_wait;
		int turnaround_time;
		int priority;
};

// Sets the initial values for the process
Process::Process(int p, int t, int a_t, int pri)
{
	pid = p;
	process_time = t;
	time_remaining = t;
	arrival_time = a_t;
	initial_wait = 0;
	turnaround_time = 0;
	priority = pri;
}

/*
* Simulates running the process
* Returns 1 if the process has burst time remaining, 0 if finished
*/
int Process::runProcess()
{
	// Set the initial wait if it had not been run before
	if (time_remaining == process_time)
		initial_wait = current_time - arrival_time;
	//Decrement the time remaining if it is not finished
	if (time_remaining > 1)
	{
		time_remaining--;
		current_time ++;
		return 1;
	}
	//Save and print the final wait time if the program has finished running
	else if (time_remaining == 1)
	{
		time_remaining--;
		current_time++;
		turnaround_time = current_time - arrival_time;
		int total_wait = turnaround_time - process_time;
		cout<<"[time "<<current_time<<"ms] Process "<<pid<<" completed its CPU burst (turnaround time "<<turnaround_time<<"ms, initial wait time "<<initial_wait<<"ms, total wait time "<<total_wait<<"ms)"<<endl;
		return 1;	
	}
	return 0;
}

//returns the wait times for the process
void Process::getWaitTimes(int & i, int& t, int& w)
{
	i = initial_wait;
	t = turnaround_time;
	w = turnaround_time - process_time;
}

/*Resets the wait time for a process
* Used for using the same processes for multiple algorithms
*/
void Process::resetWait(){
	initial_wait = 0;
	turnaround_time = 0;
	time_remaining = process_time;
}

//-------------------------PROCESS QUEUE-----------------------------------
/*
* Process Queue class handles the organization of the queue of processes
* Has functions to push processes and select processes based upon a certain
* algorithm.
*/
class Process_Queue{
	public:
		Process_Queue(){current_index = 0;};
		
		int pushProcess(Process* np);
		int pushProcessPri(Process* np);
		Process* pushSJFProcess(Process* np);
		Process* pushRRProcess(Process* np);
	
		Process* nextHighPriority();
		Process* next();
		Process* nextSJF();
		
		void reset();
		void outputStats();

	private:
		vector<Process*> p;
		unsigned int current_index;
};

//Pushes a process to the back of the queue 
//Used for FCFS
int Process_Queue::pushProcess(Process* np)
{
	cout<<"[time "<<current_time<<"ms] Process "<<np->getPid();
	cout<<" created (requires "<<np->processTime() <<"ms CPU time)"<< endl;
	p.push_back(np);
	return 1;  
}

//Pushes a process to the back of the queue
//Returns if the new process is a higher priority than the current running process
int Process_Queue::pushProcessPri(Process* np)
{
	cout<<"[time "<<current_time<<"ms] Process "<<np->getPid();
	cout<<" created (requires "<<np->processTime() <<"ms CPU time, Priority "<<np->getPriority()<<")"<< endl;
	p.push_back(np);
	if (current_index == 0)
	{
		return (p[p.size()-1]->getPriority() > np->getPriority());	
	}
	else 
	{
		return (p[current_index -1]->getPriority() > np->getPriority());	
	}
}

//Inserts a process at the end of the queue
//The head of the queue is the current index
Process* Process_Queue::pushRRProcess(Process* np)
{
	cout<<"[time "<<current_time<<"ms] Process "<<np->getPid();
        cout<<" created (requires "<<np->processTime() <<"ms CPU time)"<< endl;
	
	//If the queue is empty
	if (p.size() < 1)
	{
		p.push_back(np);
		current_index = 1;
	}
	//Insert into queue directly "behind" the head
	else
	{
		int queue_end = current_index;
		if (queue_end < 1 )
			queue_end = p.size();
		//Insert directly before the current position
		p.insert(p.begin() +  queue_end - 1, np);
		current_index++;
	}
	return np;
}

//Pushes process for the SJF queue
//The head of the queue is the current index
Process* Process_Queue::pushSJFProcess(Process* np)
{
	cout<<"[time "<<current_time<<"ms] Process "<<np->getPid();
	cout<<" created (requires "<<np->processTime() <<"ms CPU time)"<< endl;
	//If the queue is empty push to the end
	if(p.size() < 1)
	{
		p.push_back(np);
	}
	else
	{
		for(unsigned int i = current_index; i < p.size(); i++)
		{
			//Insert the process before a process if the arrival time is greater or equal it
			//and remaining time is less than the process in the queue
			if (p[i]->getArrival() <= np->getArrival() && p[i]->timeRemaining() > np->timeRemaining())
			{
				p.insert(p.begin() + i, np);
				return np;
			}
		}
		//Else push it to the back of the queue
		p.push_back(np);

	}


	return np;  
}

//Returns the first process after the current index that has remaining burst time
Process* Process_Queue::next()
{
	if (current_index == p.size())
		current_index = 0;
	//Return the current_index if there is time remaining
	if(p[current_index]->timeRemaining() > 0)
		return p[current_index++];
	else 
	{
		unsigned int temp = current_index + 1;
		//Loop to begining if end of vector is reached
		if (temp == p.size())
			temp = 0;
		//Loop through all of the indexes once
		while(temp != current_index){
			//Return if there is time remaining
			if (p[temp]->timeRemaining() > 0)
			{
				current_index = temp;
				return p[current_index++];
			}
			temp++;
			if (temp == p.size())
				temp = 0;
		}
	}	
	//Return null if no processes with time remaining was found
	return NULL;	
}

//Returns the next process with the highest priority
Process* Process_Queue::nextHighPriority()
{
	//Loop if end of vector was reached
	if (current_index == p.size())
		current_index = 0;
	
	int highest_priority = MAX_PRIORITY + 1;

	//Determine the highest priority in the queue
	for (unsigned int i = 0; i < p.size(); i++)
	{
		if (highest_priority > p[i]->getPriority() && p[i]->timeRemaining()>0)
			highest_priority = p[i]->getPriority();
	}
	unsigned int loc = current_index;
	unsigned int n = 0;
	//Loop through the queue to find the first with the highest priority
	while(n < p.size())
	{	
		if(p[loc]->timeRemaining() > 0)
		{
			//Return the first with the highest priority
			if (p[loc]->getPriority() == highest_priority)
			{
				current_index = loc;
				return p[current_index++];
			}
		}
		n++;
		if (++loc == p.size())
			loc = 0;
	}	
	//If none were found, return null
	return NULL;	
	
}

//Returns the next process with the shortest non-zero remaining time
Process* Process_Queue::nextSJF()
{
    current_index = 0;
    for( current_index = 0; current_index < p.size(); current_index++)
    {
        if(p[current_index]->timeRemaining() > 0)
            return p[current_index];
    }
	return NULL;
}

//Outputs the statistics of the queue
void Process_Queue::outputStats()
{
	int init, turn, wait;
	int tmin = MAX_BURST*NUM_PROCESSES, tsum = 0, tmax = 0;
	int imin = MAX_BURST*NUM_PROCESSES, isum = 0, imax = 0;
	int wmin = MAX_BURST*NUM_PROCESSES, wsum = 0, wmax = 0;
	
	//Loop through all processes and calculate the wait times for each
	for (unsigned int i = 0; i < p.size(); i++){
		p[i]->getWaitTimes(init, turn, wait);
		if (turn < tmin) tmin = turn;
		if (turn > tmax) tmax = turn;
		tsum += turn;

		if (init < imin) imin = init;
		if (init > imax) imax = init;
		isum += init;
			
		if (wait < wmin) wmin = wait;
		if (wait > wmax) wmax = wait;
		wsum += wait;
	}
	
	cout<<setprecision(3)<<fixed;

	cout<<"Turnaround time: min "<<tmin<<"ms; avg "<<tsum/p.size()<<"ms; max "<<tmax<<"ms"<<endl;
	cout<<"Initial wait time: min "<<imin<<"ms; avg "<<isum/p.size()<<"ms; max "<<imax<<"ms"<<endl;
	cout<<"Total wait time: min "<<wmin<<"ms; avg "<<wsum/p.size()<<"ms; max "<<wmax<<"ms"<<endl;
}	

//resets all of the processes to be reused for a different queue
void Process_Queue::reset()
{
	for (unsigned int i = 0; i < p.size(); i++)
	{
		p[i]->resetWait();
	}
}


//-------------------------------ALGORITHMS-----------------------------
/*
*Function that performs the First Come First Served algorithm
*/
int FCFS(Process** p){
	current_time = 0;
	cout<<"FCFS"<<endl;
	//Initialize the queue
	Process_Queue pq;
	int p_index;
	//Push processes that start at time 0
	for (p_index = 0; p_index < NUM_PROCESSES; p_index++)
	{
		if (p[p_index]->getArrival() == 0)
		{	
			pq.pushProcess(p[p_index]);
		}
		else 
			break;	
	}
	
	int prev_process = 0;
	//Get the next process in the queue
	Process* current_p = pq.next();
	//While there is a next process
	while(current_p != NULL)
	{
		//Run for all processes while there is a process in the ready queue
		for (int i = 0; i < NUM_PROCESSES && current_p != NULL; i++)
		{
			int ran = 1;
			//Run the process until the process has finished
			for (int j = 0; j < MAX_BURST && ran==1; j++)
			{
				ran = current_p->runProcess();
				//If a process arrives, add it to the queue
				if (p_index < NUM_PROCESSES && current_time == p[p_index]->getArrival())
					pq.pushProcess(p[p_index++]);
			}
			//Get the next process
			prev_process = current_p->getPid();
			current_p = pq.next();
			//If it is valid and different, context switch
			if (current_p != NULL && current_p->getPid() != prev_process)
			{
				cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<<prev_process<<" for process "<<current_p->getPid()<<")"<<endl;
				int temp = current_time + tcs;
				//Check that no processes have arrived
				for (; p_index < NUM_PROCESSES && current_time < temp; current_time++)
				{
					if (p[p_index]->getArrival()== current_time)
						pq.pushProcess(p[p_index++]);
				}
				//ensure the current time is accurate
				current_time = temp;
			}
		}
		//If the CPU is idle and there are still processes, push the next process to the queue
		if (p_index < NUM_PROCESSES)
		{
			int next_arrival = p[p_index]->getArrival();
			current_time = next_arrival;
			pq.pushProcess(p[p_index++]);
			current_p = pq.next();
		}
	}
	//Output the statistics
	pq.outputStats();
	pq.reset();
	return 0;
}

/*
* Executes the Round Robin Algorithm on a set of processes
*/
int RR(Process** p){
	current_time = 0;
	cout<<"ROUND ROBIN"<<endl;
	//Initialize the queue	
	Process_Queue pq;
	int p_index;
	int next_arrival = 0;
	
	//Push all processes that start at time 0
	for (p_index = 0; p_index < NUM_PROCESSES; p_index++)
	{
		if (p[p_index]->getArrival() == 0)
		{	
			pq.pushRRProcess(p[p_index]);
		}
		//If it does not start at time 0, save the arrival time for reference and continue
		else 
		{
			next_arrival = p[p_index]->getArrival();
			break;
		}	
	}
	int prev_process = 0;
	//Get the first process in the RR queue
	Process* current_p = pq.next();

	//While there is a process in the ready queue
	while(current_p != NULL)
	{
		int ran = 1;
		//Run the process for the time slice or until completion
		for (int i = 0 ; i < TIME_SLICE && ran == 1; i++)
		{
			ran = current_p->runProcess();
			//If a process arrives, push it to the queue
			if (current_time == next_arrival)
			{
				pq.pushRRProcess(p[p_index++]);
				if (p_index != NUM_PROCESSES)
					next_arrival = p[p_index]->getArrival();
			}
		}
		//Get the next process
		prev_process = current_p->getPid();
		current_p = pq.next();
		//If it is different and not null, perform a context switch
		if (current_p != NULL && current_p->getPid() != prev_process){
			cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<<prev_process<<" for process "<<current_p->getPid()<<")"<<endl;
			int temp = current_time + tcs;
			for(;p_index < NUM_PROCESSES && current_time < temp; current_time++)
			{
				//If a process arrives during context switch, push it to the queue
				if (current_time == next_arrival)
				{
					pq.pushRRProcess(p[p_index++]);
					if (p_index < NUM_PROCESSES)
						next_arrival = p[p_index]->getArrival();
				}
			}	
			current_time = temp;
		}
		//If the CPU is idle (no current process) and there are still processes to run
		//	jump to next arrival time
		if (current_p == NULL && p_index < NUM_PROCESSES){
			current_time = next_arrival;
			pq.pushRRProcess(p[p_index++]);
			current_p = pq.next();
			if (p_index < NUM_PROCESSES) 
				next_arrival = p[p_index]->getArrival();
		}
	}
	//Output the statistics and reset the variables of each process
	pq.outputStats();
	pq.reset();
	return 0;
}

/* 
 * Function that performs non-preemptive shortest job first algorithm
 */
int SJF(Process** p){
	current_time = 0;
	int p_index;
	cout<<"SJF Non Preemptive"<<endl;
	Process_Queue pq;

	//Return 0 if no processes exists
	if (NUM_PROCESSES == 0)
	{
		return 0;
	}

	//Push all processes that start at time 0
    for (p_index = 0; p_index < NUM_PROCESSES; p_index++)
	{
		if (p[p_index]->getArrival() == 0)
		{
			pq.pushSJFProcess(p[p_index]);
		}
		else
			break;
	}

    int prev_p_id = 0;
    //Get the first process in the queue
	Process* current_p = pq.nextSJF();
	//While there is a process in the ready queue
	while( current_p != NULL )
	{
	    int ran = 1;
	    //Run process until complemetion
	    for( int i = 0; i < MAX_BURST && ran == 1; i++ )
	    {
	        ran = current_p->runProcess();
	        //If a process arrives, push it to the queue
            if (p_index < NUM_PROCESSES && p[p_index]->getArrival() <= current_time)
            {
                pq.pushSJFProcess(p[p_index]);
                p_index++;
            }
	    }

	    //Get the next process
	    prev_p_id = current_p->getPid();
	    current_p = pq.nextSJF();
	    //If it is different and not null, perform a context switch
	    if (current_p != NULL && current_p->getPid() != prev_p_id)
	    {
	        cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<< prev_p_id <<" for process "<<current_p->getPid()<<")"<<endl;
			int temp = current_time + tcs;
			for(; p_index < NUM_PROCESSES && current_time < temp; current_time++)
			{
				//If a process arrives during context switch, push it to the queue
			    if(p[p_index]->getArrival() == current_time)
			    {
			        pq.pushSJFProcess(p[p_index]);
			    }
			}
			current_time = temp;
	    }
	    //If the CPU is idle (no current process) and there are still processes to run
		//	jump to next arrival time
	    if (p_index < NUM_PROCESSES && current_p == NULL){
		current_time = p[p_index]->getArrival();
		pq.pushSJFProcess(p[p_index++]);
		current_p = pq.next();
		}
	}

	//Output the statistics and reset the variables of each process
    pq.outputStats();
	pq.reset();

	return 0;
}

/*
* Function that performs non-preemptive shortest job first algorithm
*/
int PreemptiveSJF(Process** p){
	current_time = 0;
	int p_index;
	int next_arrival = 0;
	cout<<"SJF Preemptive"<<endl;
	Process_Queue pq;

	//Return 0 if no processes exists
	if (NUM_PROCESSES == 0)
	{
		return 0;
	}

	//Push all processes that start at time 0
    for (p_index = 0; p_index < NUM_PROCESSES; p_index++)
	{
		if (p[p_index]->getArrival() == 0)
		{
			pq.pushSJFProcess(p[p_index]);
		}
		//If it does not start at time 0, save the arrival time for reference and continue
		else
		{
            next_arrival = p[p_index]->getArrival();
            break;
		}
	}

    int prev_p_id = 0;
    //Get the first process in the queue
	Process* current_p = pq.nextSJF();
	int preempt = 0;
	//While there is a process in the ready queue
	while( current_p != NULL )
	{
	    int ran = 1;
	    //Run the process until it completion or preemption
	    for( int i = 0; i < MAX_BURST && ran == 1 && !preempt; i++ )
	    {
	        ran = current_p->runProcess();
	        //If a process arrives, push it to the queue
            if (p_index < NUM_PROCESSES && p[p_index]->getArrival() <= current_time)
            {
                pq.pushSJFProcess(p[p_index]);
                //If current the time remaining for the arriving process is less that current process
                //Perform a context switch and raise preemption flag
                if( current_p->timeRemaining() > p[p_index]->timeRemaining())
                {
                    preempt = 1;
                }
                p_index++;
                //Get next arrival time
                if (p_index < NUM_PROCESSES)
                {
                    next_arrival = p[p_index]->getArrival();
                }
            }
	    }

	    prev_p_id = current_p->getPid();
	    //Gets the next process
	    current_p = pq.nextSJF();
	    preempt = 0;
	    //If it is different and not null, perform a context switch
	    if (current_p != NULL && current_p->getPid() != prev_p_id)
	    {
	        cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<< prev_p_id <<" for process "<<current_p->getPid()<<")"<<endl;
			int temp = current_time + tcs;
			for(; p_index < NUM_PROCESSES && current_time < temp; current_time++)
			{
				//If a process arrives during context switch, push it to the queue
			    if(p[p_index]->getArrival() == current_time)
			    {
			    	//Perform context switch if arriving process has less remaining time that current process
			        if( current_p->timeRemaining() > p[p_index]->timeRemaining())
			        {
			            preempt = 1;
			            pq.pushSJFProcess(p[p_index]);
			            p_index++;
			        }
			        //Get next arrival time
			        if( p_index < NUM_PROCESSES )
			        {
			            next_arrival = p[p_index]->getArrival();
			        }
			    }
			}
			current_time = temp;
	    }
	    //If the CPU is idle (no current process) and there are still processes to run
		//	jump to next arrival time
	    if (p_index < NUM_PROCESSES && current_p == NULL){
			current_time = next_arrival;
			pq.pushSJFProcess(p[p_index++]);
			current_p = pq.next();
			if (p_index < NUM_PROCESSES) next_arrival = p[p_index]->getArrival();
		}
	}

	//Output the statistics and reset the variables of each process
    pq.outputStats();
	pq.reset();

	return 0;
}

/*
* Runs the preemptive priority algorithm on a set of processes
*/
void PreemptivePriority(Process** p)
{
	current_time = 0;
	cout<<"Preemptive Priority"<<endl;
	//Initializes the queue
	Process_Queue pq;
	int p_index;
	int next_arrival;
	//Add all processes that start at time 0
	for (p_index = 0; p_index < NUM_PROCESSES; p_index++)
	{
		if (p[p_index]->getArrival() == 0)
		{	
			pq.pushProcessPri(p[p_index]);
		}
		else 
		{
			next_arrival = p[p_index]->getArrival();
			break;	
		}
	}
	int prev_process = 0;
	// Get the first process
	Process* current_p = pq.nextHighPriority();
	int preempt = 0;
	//While there are processes to run
	while(current_p != NULL)
        {
		int ran = 1;
		//Run for the time slice, unless the system has been preempted
		for (int i = 0 ; i < TIME_SLICE && ran == 1 && !preempt; i++)
		{
			ran = current_p->runProcess();
			//Check if another process has arrived
			if (current_time == next_arrival)
			{
				//If the process is a higher priority than the others, preempt
				preempt = pq.pushProcessPri(p[p_index++]);
				if (p_index != NUM_PROCESSES)
					next_arrival = p[p_index]->getArrival();
			}
		}
		//Get the next process
		prev_process = current_p->getPid();
		current_p = pq.nextHighPriority();
		//Reset the preeempt flag
		preempt = 0;
		//If the next process is different, context switch
		if (current_p != NULL && current_p->getPid() != prev_process){
			cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<<prev_process<<" for process "<<current_p->getPid()<<")"<<endl;
			int temp = current_time + tcs;
			/*
			* If a process has come in, add it to the queue, save if it should be preempted
			* The process that preeempted the other will be switched in immediately after the context switch
			*/
			for(;p_index < NUM_PROCESSES && current_time < temp; current_time++)
			{
				if (current_time == next_arrival)
				{
					preempt = pq.pushProcessPri(p[p_index++]);
					if (p_index < NUM_PROCESSES)
						next_arrival = p[p_index]->getArrival();
				}
			}	
			current_time = temp;
		}
		//Queue is empty but processes still remain, jump to next time
		if (current_p == NULL && p_index < NUM_PROCESSES){
			current_time = next_arrival;
			pq.pushProcessPri(p[p_index++]);
			current_p = pq.nextHighPriority();
			next_arrival = p[p_index]->getArrival();
		}
	}
	//Output the wait min,avg,max wait times for the algorithm
	pq.outputStats();
	pq.reset();
}

//----------------------------Random arrival---------------------------------
int randomArrival(int processCount){

	int val = 0;

	//20 percent of the process will have arrival time of 0ms
	if (processCount < NUM_PROCESSES * 0.2)
		return val;

	//Perform exponential distribution with an average time of 1000ms using 0.001 lambda value
	//Ignores values greater than 8000ms
	while(1)
	{
		double lambda = 0.001;
		//srand(time(NULL));
		double r = ((double)rand() / (RAND_MAX + 1LL));
		val = -log(r) / lambda;
		if (val <= 8000){
			break;
		}
	}
	return val;
}

//Operator to sort array by increasing arrival time
bool sortbyArrival(Process* i, Process* j){
	return i->getArrival() < j->getArrival();
}

//----------------------------Main Function----------------------------------
int main()
{
	Process* p[NUM_PROCESSES];
	srand(time(NULL));

	//Create a new process
	for (int i = 0; i < NUM_PROCESSES; i++){
		int p_time = rand() % (MAX_BURST - MIN_BURST) + MIN_BURST;
		int priority = rand() % (MAX_PRIORITY+1);
		int arrival = 0;
		#ifdef PART2
		arrival = randomArrival(i);
		#endif
		p[i] = new Process((i+1), p_time, arrival, priority);
	}

	sort(p, p+NUM_PROCESSES, sortbyArrival);


	FCFS(p);
	cout<<endl;

	RR(p);
	cout<<endl;

	SJF(p);
	cout<<endl;

	PreemptiveSJF(p);
	cout<<endl;

	PreemptivePriority(p);

	for (int i = 0; i < NUM_PROCESSES; i++){
		free(p[i]);
	}
	return 0;
}

