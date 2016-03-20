#ifndef STATECOUNT_H
#define STATECOUNT_H

#include <vector>

struct JobInfo;

namespace Scheduler {
namespace Core {

enum JobState
  {
  JobNoState,
  JobQueued,
  JobRunning,
  JobHeld,
  JobWaiting,
  JobTransit,
  JobExiting,
  JobSuspended,
  JobCompleted,
  JobCrossRun
  };

extern const char* JobStateString[];

struct StateCount
  {
  int running;      /* number of jobs in the running state*/
  int queued;       /* number of jobs in the queued state */
  int held;         /* number of jobs in the held state */
  int transit;      /* number of jobs in the transit state */
  int waiting;      /* number of jobs in the waiting state */
  int exiting;      /* number of jobs in the exiting state */
  int suspended;    /* number of jobs in the suspended state */
  int completed;    /* number of jobs in the completed state */
  int crossrun;     /* number of jobs in the cross-running state */
  int total;        /* total number of jobs in all states */

  StateCount();
  StateCount(const StateCount&) = default;
  StateCount& operator=(const StateCount&) = default;

  void reset();
  void count_states(const std::vector<JobInfo*>& jobs);

  StateCount& operator+=(const StateCount& sc);
  StateCount operator+(const StateCount& rhs) const;

  const char * asString(JobState state) const;
  };

}}

#endif
