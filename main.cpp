#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

using namespace std;

#define NUM_PROCESSES 3 
#define TIME_SLICE 2000
#define MAX_BURST 4000
#define MIN_BURST 500
#define MAX_PRIORITY 2

//Global Variables
int current_time = 0;
int tcs = 17; //Time for context switch

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
	Process* pushProcess(Process* np);
	Process* pushRRProcess(Process* np);
	void reset();
	void outputStats();

	Process* nextShortest();
	Process* nextHighPriority();	
	int numPriority(int num);
	Process* next();

	bool isEmpty() {return (p.size() > 0); }

	
	private:
		vector<Process*> p;
		unsigned int current_index;
};	

Process* Process_Queue::pushProcess(Process* np)
{
	cout<<np->getPriority();
	cout<<"[time "<<current_time<<"ms] Process "<<np->getPid();
	cout<<" created (requires "<<np->processTime() <<"ms CPU time)"<< endl;

	p.push_back(np);
	return np;  //Could expand for preemptive
}

Process* Process_Queue::pushRRProcess(Process* np)
{
	cout<<np->getPriority();
	cout<<"[time "<<current_time<<"ms] Process "<<np->getPid();
        cout<<" created (requires "<<np->processTime() <<"ms CPU time)"<< endl;
	if (p.size() < 1)
	{
		p.push_back(np);
	}
	else
	{
		int queue_end = current_index;
		if (queue_end < 0 )
			queue_end = p.size() - 1;
		cout<<p.size()<< " "<<queue_end<<endl;
		p.insert(p.begin() + queue_end, np);
		current_index++;
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

int Process_Queue::numPriority(int num)
{
	int r = 0;
	for (unsigned int i = 0; i < p.size(); i++)
	{
		if (p[i]->getPriority() == num && p[i]->timeRemaining())
			r++;
	}
	return r;
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
			cout<<p[p_index]->getPid();
			pq.pushProcess(p[p_index]);
		}
		else 
			break;	
	}
	int prev_process = 0;
	Process* next_p = pq.next();
	while(next_p != NULL){
		for (int i = 0; i < NUM_PROCESSES && next_p != NULL; i++){
			int ran = 1;
			for (int j = 0; j < 4000 && ran==1; j++){
				ran = next_p->runProcess();
				if (p_index < NUM_PROCESSES && current_time == p[p_index]->getArrival())
					pq.pushProcess(p[p_index++]);
			}
			prev_process = next_p->getPid();
			next_p = pq.next();
			if (next_p != NULL){
				int changed = 0;
				while (p_index < NUM_PROCESSES && p[p_index]->getArrival() < current_time+ tcs)
				{
					int diff = p[p_index]->getArrival() - current_time;
					current_time += diff;
					pq.pushProcess(p[p_index++]);
					changed = 1;
				}
				cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<<prev_process<<" for process "<<next_p->getPid()<<")"<<endl;
				if (!changed) current_time += tcs;
			}
		}
		if (p_index < NUM_PROCESSES){
			int next_arrival = p[p_index]->getArrival();
			current_time = next_arrival;
			pq.pushProcess(p[p_index++]);
			next_p = pq.next();
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
	for (int i = 0; i < NUM_PROCESSES; i++)
		pq.pushRRProcess(p[i]);	
	int prev_process = 0;
	Process* next_p = pq.next();
	while(next_p != NULL)
	{
		int ran = 1;
		for (int i = 0 ; i < TIME_SLICE && ran == 1; i++)
		{
			ran = next_p->runProcess();
		}
		prev_process = next_p->getPid();
		next_p = pq.next();
		if (next_p != NULL){
			cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<<prev_process<<" for process "<<next_p->getPid()<<")"<<endl;
			current_time += tcs;
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
	for (int i = 0; i < NUM_PROCESSES; i++)
		pq.pushProcess(p[i]);	
	int prev_process = 0;
	int numHighest = 0;
	Process* next_p = pq.nextHighPriority();
	while(next_p != NULL)
        {
		numHighest = pq.numPriority(next_p->getPriority());
		//If multiple have same priority
		if (numHighest > 1)
		{	
			int ran = 1;
			for (int i = 0 ; i < TIME_SLICE && ran == 1; i++)
			{
				ran = next_p->runProcess();
			}
		}
		//If it is the only with that priority
		else 
		{
			int ran = 1;
			while(ran == 1)
			{
				ran = next_p->runProcess();
			}
		}
		prev_process = next_p->getPid();
		next_p = pq.nextHighPriority();
		if (next_p != NULL){
			cout<<"[time "<<current_time<<"ms] Context switch (swapping out process "<<prev_process<<" for process "<<next_p->getPid()<<")"<<endl;
			current_time += tcs;
		}
	}

	pq.outputStats();
	pq.reset();
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
		p[i] = new Process((i+1), p_time, i*2000, priority);
	}
	

	FCFS(p);

	RR(p);
	
	PreemptivePriority(p);

	for (int i = 0; i < NUM_PROCESSES; i++){
		free(p[i]);
	}
	return 0;
}

