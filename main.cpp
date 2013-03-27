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

//
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

int Process::runProcess()
{
	if (time_remaining == process_time)
		initial_wait = current_time - arrival_time;
	if (time_remaining > 1)
	{
		time_remaining--;
		current_time ++;
		return 1;
	}
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

void Process::getWaitTimes(int & i, int& t, int& w)
{
	i = initial_wait;
	t = turnaround_time;
	w = turnaround_time - process_time;
}

void Process::resetWait(){
	initial_wait = 0;
	turnaround_time = 0;
	time_remaining = process_time;
}

//-------------------------PROCESS QUEUE-----------------------------------
class Process_Queue{
	public:
	Process_Queue(){current_index = 0;};
	int pushProcess(Process* np);
	int pushProcessPri(Process* np);
	Process* pushSJFProcess(Process* np);
	Process* pushRRProcess(Process* np);
	void reset();
	void outputStats();

	Process* nextHighPriority();
	Process* next();
	Process* nextSJF();

	bool isEmpty() {return (p.size() > 0); }

	
	private:
		vector<Process*> p;
		unsigned int current_index;
};

int Process_Queue::pushProcess(Process* np)
{
	cout<<"[time "<<current_time<<"ms] Process "<<np->getPid();
	cout<<" created (requires "<<np->processTime() <<"ms CPU time)"<< endl;
	p.push_back(np);
	return 1;  
}

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

Process* Process_Queue::pushRRProcess(Process* np)
{
	cout<<"[time "<<current_time<<"ms] Process "<<np->getPid();
        cout<<" created (requires "<<np->processTime() <<"ms CPU time)"<< endl;
	
	if (p.size() < 1)
	{
		p.push_back(np);
		current_index = 1;
	}
	else
	{
		int queue_end = current_index;
		if (queue_end < 1 )
			queue_end = p.size();
		p.insert(p.begin() +  queue_end - 1, np);
		current_index++;
	}
	return np;
}


Process* Process_Queue::pushSJFProcess(Process* np)
{
	cout<<"[time "<<current_time<<"ms] Process "<<np->getPid();
	cout<<" created (requires "<<np->processTime() <<"ms CPU time)"<< endl;
	if(p.size() < 1)
	{
		p.push_back(np);
	}
	else
	{
		for(unsigned int i = current_index; i < p.size(); i++)
		{
			if (p[i]->getArrival() <= np->getArrival() && p[i]->timeRemaining() > np->timeRemaining())
			{
				p.insert(p.begin() + i, np);
				return np;
			}
		}
		p.push_back(np);

	}


	return np;  
}

Process* Process_Queue::next()
{
	if (current_index == p.size())
		current_index = 0;
	if(p[current_index]->timeRemaining() > 0)
		return p[current_index++];
	else 
	{
		unsigned int temp = current_index + 1;
		if (temp == p.size())
			temp = 0;
		while(temp != current_index){
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
	return NULL;	
}

Process* Process_Queue::nextHighPriority()
{
	if (current_index == p.size())
		current_index = 0;
	
	int highest_priority = MAX_PRIORITY + 1;

	for (unsigned int i = 0; i < p.size(); i++)
	{
		if (highest_priority > p[i]->getPriority() && p[i]->timeRemaining()>0)
			highest_priority = p[i]->getPriority();
	}
	unsigned int loc = current_index;
	unsigned int n = 0;
	while(n < p.size())
	{	
		if(p[loc]->timeRemaining() > 0)
		{
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
	return NULL;	
	
}

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

void Process_Queue::outputStats()
{
	int init, turn, wait;
	int tmin = MAX_BURST*NUM_PROCESSES, tsum = 0, tmax = 0;
	int imin = MAX_BURST*NUM_PROCESSES, isum = 0, imax = 0;
	int wmin = MAX_BURST*NUM_PROCESSES, wsum = 0, wmax = 0;
	
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


void Process_Queue::reset()
{
	for (unsigned int i = 0; i < p.size(); i++)
	{
		p[i]->resetWait();
	}
}


//-------------------------------ALGORITHMS-----------------------------
int FCFS(Process** p){
	current_time = 0;
	cout<<"FCFS"<<endl;
	Process_Queue pq;
	int p_index;
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
	Process* current_p = pq.next();
	while(current_p != NULL)
	{
		for (int i = 0; i < NUM_PROCESSES && current_p != NULL; i++)
		{
			int ran = 1;
			for (int j = 0; j < 4000 && ran==1; j++)
			{
				ran = current_p->runProcess();
				if (p_index < NUM_PROCESSES && current_time == p[p_index]->getArrival())
					pq.pushProcess(p[p_index++]);
			}
			prev_process = current_p->getPid();
			current_p = pq.next();
			if (current_p != NULL && current_p->getPid() != prev_process)
			{
				cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<<prev_process<<" for process "<<current_p->getPid()<<")"<<endl;
				int temp = current_time + tcs;
				for (; p_index < NUM_PROCESSES && current_time < temp; current_time++)
				{
					if (p[p_index]->getArrival()== current_time)
						pq.pushProcess(p[p_index++]);
				}
				current_time = temp;
			}
		}
		if (p_index < NUM_PROCESSES)
		{
			int next_arrival = p[p_index]->getArrival();
			current_time = next_arrival;
			pq.pushProcess(p[p_index++]);
			current_p = pq.next();
		}
	}
	pq.outputStats();
	pq.reset();
	return 0;
}



int RR(Process** p){
	current_time = 0;
	cout<<"ROUND ROBIN"<<endl;	
	 Process_Queue pq;
	int p_index;
	int next_arrival = 0;
	for (p_index = 0; p_index < NUM_PROCESSES; p_index++)
	{
		if (p[p_index]->getArrival() == 0)
		{	
			pq.pushRRProcess(p[p_index]);
		}
		else 
		{
			next_arrival = p[p_index]->getArrival();
			break;
		}	
	}
	int prev_process = 0;
	Process* current_p = pq.next();
	while(current_p != NULL)
	{
		int ran = 1;
		for (int i = 0 ; i < TIME_SLICE && ran == 1; i++)
		{
			ran = current_p->runProcess();
			if (current_time == next_arrival)
			{
				pq.pushRRProcess(p[p_index++]);
				if (p_index != NUM_PROCESSES)
					next_arrival = p[p_index]->getArrival();
			}
		}
		prev_process = current_p->getPid();
		current_p = pq.next();
		if (current_p != NULL && current_p->getPid() != prev_process){
			cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<<prev_process<<" for process "<<current_p->getPid()<<")"<<endl;
			int temp = current_time + tcs;
			for(;p_index < NUM_PROCESSES && current_time < temp; current_time++)
			{
				if (current_time == next_arrival)
				{
					pq.pushRRProcess(p[p_index++]);
					if (p_index < NUM_PROCESSES)
						next_arrival = p[p_index]->getArrival();
				}
			}	
			current_time = temp;
		}
		if (current_p == NULL && p_index < NUM_PROCESSES){
			current_time = next_arrival;
			pq.pushRRProcess(p[p_index++]);
			current_p = pq.next();
			if (p_index < NUM_PROCESSES) 
				next_arrival = p[p_index]->getArrival();
		}
	}
	pq.outputStats();
	pq.reset();
	return 0;
}

int SJF(Process** p){
	current_time = 0;
	int p_index;
	cout<<"SJF Non Preemptive"<<endl;
	Process_Queue pq;

	if (NUM_PROCESSES == 0)
	{
		return 0;
	}

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
	Process* current_p = pq.nextSJF();
	while( current_p != NULL )
	{
	    int ran = 1;
	    for( int i = 0; i < 4000 && ran == 1; i++ )
	    {
	        ran = current_p->runProcess();
            if (p_index < NUM_PROCESSES && p[p_index]->getArrival() <= current_time)
            {
                pq.pushSJFProcess(p[p_index]);
                p_index++;
            }
	    }

	    prev_p_id = current_p->getPid();
	    current_p = pq.nextSJF();
	    if (current_p != NULL && current_p->getPid() != prev_p_id)
	    {
	        cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<< prev_p_id <<" for process "<<current_p->getPid()<<")"<<endl;
			int temp = current_time + tcs;
			for(; p_index < NUM_PROCESSES && current_time < temp; current_time++)
			{
			    if(p[p_index]->getArrival() == current_time)
			    {
			        pq.pushSJFProcess(p[p_index]);
			    }
			}
			current_time = temp;
	    }
	    if (current_p == NULL && p_index < NUM_PROCESSES){
			int next_arrival = p[p_index]->getArrival();
			current_time = next_arrival;
			pq.pushProcess(p[p_index++]);
			current_p = pq.next();
		}
	}

    pq.outputStats();
	pq.reset();

	return 0;
}

int PreemptiveSJF(Process** p){
	current_time = 0;
	int p_index;
	int next_arrival = 0;
	cout<<"SJF Preemptive"<<endl;
	Process_Queue pq;

	if (NUM_PROCESSES == 0)
	{
		return 0;
	}

    for (p_index = 0; p_index < NUM_PROCESSES; p_index++)
	{
		if (p[p_index]->getArrival() == 0)
		{
			pq.pushSJFProcess(p[p_index]);
		}
		else
		{
            next_arrival = p[p_index]->getArrival();
            break;
		}
	}

    int prev_p_id = 0;
	Process* current_p = pq.nextSJF();
	int preempt = 0;
	while( current_p != NULL )
	{
	    int ran = 1;
	    for( int i = 0; i < 4000 && ran == 1 && !preempt; i++ )
	    {
	        ran = current_p->runProcess();
            if (p_index < NUM_PROCESSES && p[p_index]->getArrival() <= current_time)
            {
                pq.pushSJFProcess(p[p_index]);
                if( current_p->timeRemaining() > p[p_index]->timeRemaining())
                {
                    preempt = 1;
                }
                p_index++;
                if (p_index < NUM_PROCESSES)
                {
                    next_arrival = p[p_index]->getArrival();
                }
            }
	    }

	    prev_p_id = current_p->getPid();
	    current_p = pq.nextSJF();
	    preempt = 0;
	    if (current_p != NULL && current_p->getPid() != prev_p_id)
	    {
	        cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<< prev_p_id <<" for process "<<current_p->getPid()<<")"<<endl;
			int temp = current_time + tcs;
			for(; p_index < NUM_PROCESSES && current_time < temp; current_time++)
			{
			    if(p[p_index]->getArrival() == current_time)
			    {
			        if( current_p->timeRemaining() > p[p_index]->timeRemaining())
			        {
			            preempt = 1;
			            pq.pushSJFProcess(p[p_index]);
			            p_index++;
			        }
			        if( p_index < NUM_PROCESSES )
			        {
			            next_arrival = p[p_index]->getArrival();
			        }
			    }
			}
			current_time = temp;
	    }
	    if (p_index < NUM_PROCESSES && current_p == NULL){
			current_time = next_arrival;
			pq.pushProcess(p[p_index++]);
			current_p = pq.next();
			if (p_index < NUM_PROCESSES) next_arrival = p[p_index]->getArrival();
	}
	}

    pq.outputStats();
	pq.reset();

	return 0;
}

void PreemptivePriority(Process** p)
{
	current_time = 0;
	cout<<"Preemptive Priority"<<endl;
	Process_Queue pq;
	int p_index;
	int next_arrival;
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
	Process* current_p = pq.nextHighPriority();
	int preempt = 0;
	while(current_p != NULL)
        {
		int ran = 1;
		for (int i = 0 ; i < TIME_SLICE && ran == 1 && !preempt; i++)
		{
			ran = current_p->runProcess();
			if (current_time == next_arrival)
			{
				preempt = pq.pushProcessPri(p[p_index++]);
				if (p_index != NUM_PROCESSES)
					next_arrival = p[p_index]->getArrival();
			}
		}
		prev_process = current_p->getPid();
		current_p = pq.nextHighPriority();
		preempt = 0;
		if (current_p != NULL && current_p->getPid() != prev_process){
			cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<<prev_process<<" for process "<<current_p->getPid()<<")"<<endl;
			int temp = current_time + tcs;
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

	pq.outputStats();
	pq.reset();
}

//----------------------------Random arrival---------------------------------
int randomArrival(int processCount){

	int val = 0;

	if (processCount < NUM_PROCESSES * 0.2)
		return val;

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
		int arrival = randomArrival(i);
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

