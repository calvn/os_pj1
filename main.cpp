#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

using namespace std;

#define NUM_PROCESSES 3 
#define TIME_SLICE 200
#define MAX_BURST 4000
#define MIN_BURST 500

//Global Variables
int current_time = 0;
int tcs = 17; //Time for context switch

class Process{
	public:
		Process(int p, int t, int a_t);
		int runProcess(int t);
		int getPid(){ return pid; }
		int processTime(){ return process_time; }
		int timeRemaining() { return time_remaining; }
		void getWaitTimes(int &i, int&t, int& w); 
		void resetWait();
	
	private: 
		int pid;
		int process_time;
		int time_remaining;
		int arrival_time;
		int initial_wait;
		int turnaround_time;
};

Process::Process(int p, int t, int a_t)
{
	pid = p;
	process_time = t;
	time_remaining = t;
	arrival_time = a_t;
	initial_wait = 0;
	turnaround_time = 0;
}

int Process::runProcess(int t)
{
	if (time_remaining == process_time)
		initial_wait = current_time - arrival_time;
	if (time_remaining > t)
	{
		time_remaining -= t;
		current_time += t;
		return t;
	}
	else 
	{
		t = time_remaining;
		time_remaining = 0;
		current_time += t;
		turnaround_time = current_time - arrival_time;
		int total_wait = turnaround_time - process_time;

		cout<<"[time "<<current_time<<"ms] Process "<<pid<<" completed its CPU burst (turnaround time "<<turnaround_time<<"ms, initial wait time "<<initial_wait<<"ms, total wait time "<<total_wait<<"ms)"<<endl;
		return t;	
	}
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
	void reset();
	void outputStats();

	Process* nextShortest();
	Process* highestPriority();	
	Process* next();

	bool isEmpty() {return (p.size() > 0); }

	
	private:
		vector<Process*> p;
		unsigned int current_index;
};	

Process* Process_Queue::pushProcess(Process* np)
{
	cout<<"[time "<<current_time<<"ms] Process "<<np->getPid();
	cout<<" created (requires "<<np->processTime() <<"ms CPU time)"<< endl;
	p.push_back(np);
	return np;  //Could expand for preemptive
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


void Process_Queue::outputStats()
{
	int init, turn, wait;
	double tmin = MAX_BURST*NUM_PROCESSES, tsum = 0, tmax = 0;
	double imin = MAX_BURST*NUM_PROCESSES, isum = 0, imax = 0;
	double wmin = MAX_BURST*NUM_PROCESSES, wsum = 0, wmax = 0;
	
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
	for (int i = 0; i < NUM_PROCESSES; i++)
		pq.pushProcess(p[i]);	
	int prev_process = 0;
	Process* next_p = pq.next();
	for (int i = 0; i < NUM_PROCESSES && next_p != NULL; i++){
		next_p->runProcess(4000);
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



int RR(Process** p){
	current_time = 0;
	cout<<"ROUND ROBIN"<<endl;	
	 Process_Queue pq;
	for (int i = 0; i < NUM_PROCESSES; i++)
		pq.pushProcess(p[i]);	
	int prev_process = 0;
	Process* next_p = pq.next();
	while(next_p != NULL)
	{
		next_p->runProcess(TIME_SLICE);
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



//----------------------------Main Function----------------------------------
int main()
{
	Process* p[NUM_PROCESSES];
	srand(time(NULL));

	//Create a new process
	for (int i = 0; i < NUM_PROCESSES; i++){
		int p_time = rand() % (MAX_BURST - MIN_BURST) + MIN_BURST;
		p[i] = new Process((i+1), p_time, 0);
	}

	FCFS(p);

	RR(p);

	for (int i = 0; i < NUM_PROCESSES; i++){
		free(p[i]);
	}
	return 0;
}

