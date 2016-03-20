//
// Created by simon on 3/18/16.
//

#include "SchedulerCore_StateCount.h"
#include "JobInfo.h"
#include "misc.h"

using namespace Scheduler;
using namespace Core;

const char* ::Scheduler::Core::JobStateString[] = { "none", "queued", "running", "held", "waiting", "transit", "exiting", "suspended", "completed", nullptr };

StateCount::StateCount() : running(0), queued(0), held(0), transit(0), waiting(0), exiting(0),  suspended(0), completed(0), crossrun(0), total(0) {}

void StateCount::reset()
  {
  running = 0;
  queued = 0;
  transit = 0;
  exiting = 0;
  held = 0;
  waiting = 0;
  suspended = 0;
  completed = 0;
  crossrun = 0;
  total = 0;
  }

void StateCount::count_states(const std::vector<JobInfo*>& jobs)
  {
  for (auto j : jobs)
    {
    switch (j->state)
      {
      case JobQueued:     queued++;     break;
      case JobRunning:    running++;    break;
      case JobTransit:    transit++;    break;
      case JobExiting:    exiting++;    break;
      case JobHeld:       held++;       break;
      case JobWaiting:    waiting++;    break;
      case JobSuspended:  suspended++;  break;
      case JobCompleted:  completed++;  break;
      case JobCrossRun:   crossrun++;   break;
      default:
        sched_log(PBSEVENT_JOB, PBS_EVENTCLASS_JOB, j->job_id.c_str(), "Job in unknown state");
      }
    }

  total = queued + running + transit + exiting + held + waiting + suspended + completed + crossrun;
  }

StateCount& StateCount::operator+=(const StateCount& sc)
  {
  queued += sc.queued;
  running += sc.running;
  transit += sc.transit;
  exiting += sc.exiting;
  held += sc.held;
  waiting += sc.waiting;
  suspended += sc.suspended;
  completed += sc.completed;
  crossrun += sc.crossrun;

  return *this;
  }

StateCount StateCount::operator+(const StateCount& rhs) const
  {
  StateCount sc(*this);

  sc.queued += rhs.queued;
  sc.running += rhs.running;
  sc.transit += rhs.transit;
  sc.exiting += rhs.exiting;
  sc.held += rhs.held;
  sc.waiting += rhs.waiting;
  sc.suspended += rhs.suspended;
  sc.completed += rhs.completed;
  sc.crossrun += rhs.crossrun;

  return sc;
  }

const char* StateCount::asString(JobState state) const
  { return JobStateString[state]; }