#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include <chrono>
#include <vector>
#include <iostream>

enum StatType {
	ST_TOTAL = 0,		// Total time elapsed
	ST_WORKER,			// Worker time
	ST_WAIT_SERVER,		// Server wait time
	ST_WAIT,			// Worker wait time
	ST_INPUT_READ,		// Input reading time
	ST_INIT,			// Init time of polgenerator
	ST_UNIT,			// Work units processed by workers
	ST_EVAL,			// Discriminant evaluation time
	ST_SQUARE_TEST,		// Time for discriminant square test
	ST_GENERATE,		// Generation time for next polynomial
	ST_GENERATE_SERVER,	// Generation time for root
	ST_CRT,				// Time of CRT on last coefficient
	ST_SEND,			// Time sending results to root
	ST_SAVE,			// Time saving results
	ST_BOUNDS,			// Computing bounds
	ST_LAGRANGE,		// Computing Lagrange bounds
	ST_NUM_STATS
};

class Statistics {
private:
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> timers;
	long totals[ST_NUM_STATS];
	long mins[ST_NUM_STATS];
	long maxs[ST_NUM_STATS];
	long counts[ST_NUM_STATS];
	long duration_to_us(const std::chrono::high_resolution_clock::duration& d);
	void print_statistic_line(std::ostream &output, std::string name, int part);

public:
	Statistics();
	void reset();
	void start_timer(StatType t);
	void stop_timer(StatType t);
	inline void start_timer_all_stats(StatType t) {
#if STATS
		start_timer(t);
#endif
	};
	inline void stop_timer_all_stats(StatType t) {
#if STATS
		stop_timer(t);
#endif
	};
	void send(int const rank, int const tag);
	void receive(int const rank, int const tag);
	void print(std::ostream &output);
};

#endif // STATISTICS_HPP

/*extern std::chrono::time_point<std::chrono::high_resolution_clock> ST_START_TIMES[ST_SIZE];
extern long ST_DURATIONS_TOTALS[ST_SIZE];
extern long ST_DURATIONS_MIN[ST_SIZE];
extern long ST_DURATIONS_MAX[ST_SIZE];
extern long ST_COUNTS[ST_SIZE];

//if STATS = 0 on compilation, some runtime statistics will be disabled 
#if STATS
	#define st_timer_start_all_stats(part) st_timer_start(part);
	#define st_timer_stop_all_stats(part) st_timer_stop(part);
#else
	#define st_start_timer(part)
	#define st_stop_timer(part)
#endif

void st_init();
void st_add_stats(long counts[], long totals[], long min[], long max[]);
void st_get_stats(long counts[], long totals[], long min[], long max[]);
void st_timer_start(Statistic part);
void st_timer_stop(Statistic part);
long st_duration_to_us(const std::chrono::high_resolution_clock::duration &duration);

*/
