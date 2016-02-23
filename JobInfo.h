#ifndef JOBINFO_H_
#define JOBINFO_H_

#include <map>
#include <vector>
#include <string>

#include "data_types.h"
#include "legacy/pbs_ifl.h"

struct JobInfo
  {
  JobInfo();
  JobInfo(struct batch_status *job, queue_info *queue);
  ~JobInfo();
  
  /* Job ID */
  std::string job_id;
  /* User selected job ID, also used for cluster name */
  std::string custom_name;
  /* Comment field of job */
  std::string comment;
  /** Processed nodespec from the server processed nodespec from the server */
  std::string nodespec;
  /* Parent queue where the job resides */
  struct queue_info *queue;
  /* Time the job was last en-queued */
  time_t qtime;
  /* Time the job was last started */
  time_t stime;
  /* User priority for job */ 
  int priority;
  /* Username of the owner of the job*/
  std::string account;
  /* Groupname of the owner of the job*/
  std::string group;
  /* Job state */
  JobState state;
  /* Job can never run */
  bool can_never_run;
  /* Job can not run now */
  bool can_not_run;
  /* Fairshare calculated when job is run */
  double calculated_fairshare;
  /* Scheduler nodespec, filled when job is run */
  std::string sched_nodespec;
  /* Planned nodes, for this job */
  std::string p_planned_nodes;
  /* Planned start, for this job */
  time_t p_planned_start;
  /* Jobs that are before this job */
  std::string p_waiting_for;
  /* the fair share node for the owner */
  group_info *ginfo;
  
  pars_spec *parsed_nodespec;
  /* currently considered schedule */
  std::vector<node_info*> schedule; 
  std::string cluster_name; /**< cluster name passed from -l cluster=...*/
  enum ClusterMode cluster_mode;
  std::string placement;

  resource_req *resreq;  /* list of resources requested */
  resource_req *resused; /* a list of resources used */

  void plan_on_node(node_info* ninfo, pars_spec_node* spec);
  void unplan_from_node(node_info* ninfo, pars_spec_node* spec);
  void plan_on_queue(queue_info* qinfo);
  void plan_on_server(server_info* sinfo);
  long get_walltime() const;
  int preprocess();
  double calculate_fairshare_cost(const std::vector<node_info *>& nodes) const;
  time_t completion_time();
  const char* state_string() { return JobStateString[state]; }
  
  // REFACTORED
  // the following code has already been refactored
  
  /** Determine whether the job can ever run */
  bool can_possibly_run() { return !can_never_run; }
  
  /** Determine whether the job can be run in this cycle */
  bool suitable_for_run() { return !can_not_run; }
  
  /** Determine whether job is currently starving
   * 
   * @return TRUE if job is starving, FALSE otherwise
   */
  bool is_starving() const noexcept;
  
  /** Determine whether job is a multi-node job
   * 
   * @return TRUE if job is multi-node, FALSE otherwise
   */  
  bool is_multinode() const noexcept;
  
  /** Determine whether job is a requesting machines exclusively
   * 
   * @return TRUE if job is requesting machines exclusively, FALSE otherwise
   */  
  bool is_exclusive() const noexcept;  
  
  };

#endif /* JOBINFO_H_ */
