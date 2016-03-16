#ifndef SERVERINFO_H_
#define SERVERINFO_H_

#include <map>
#include <unordered_map>
#include <vector>
#include <deque>
#include <set>
#include <string>

#include "data_types.h"
#include "base/DynamicResource.h"

#define DEFAULT_JOB_START_TIMEOUT 60

struct server_info
  {
public:
  void update_on_jobrun(JobInfo *j);
  long long int number_of_running_jobs_for_group(const std::string& groupname) const;
  long long int number_of_running_jobs_for_user(const std::string& username) const;

  char *name;   /* name of server */

  std::map<std::string, Scheduler::Core::DynamicResource> dynamic_resources; /* list of dynamic resources */
  char *default_queue;  /* the default queue atribute of the server */
  int max_run;   /* max jobs that can be run at one time */
  int max_user_run;  /* max jobs a user can run at one time */
  int max_group_run;  /* max jobs a group can run at one time */
  int num_queues;  /* number of queues that reside on the server */
  int num_nodes;  /* number of nodes associated with the server */
  state_count sc;  /* number of jobs in each state */
  queue_info **queues;  /* array of queues */
  JobInfo **jobs;  /* array of jobs on the server */
  std::vector<JobInfo*> running_jobs; /* array of jobs in the running state */

  node_info **nodes;  /* array of nodes associated with the server */
  node_info **non_dedicated_nodes; /* array of nodes, not exclusively assigned anywhere */
  int non_dedicated_node_count;

  token **tokens;               /* array of tokens */

  int max_installing_nodes;
  int installing_node_count;

  void recount_installing_nodes();
  bool installing_nodes_overlimit();

  int job_start_timeout;

  std::map<std::string,int> exec_count; /* executed jobs this cycle per-user */

  std::unordered_map<std::string,std::deque<JobInfo *>> jobs_by_owner;
  void regenerate_jobs_by_owner();

private:
  mutable std::unordered_map<std::string,long long int> running_jobs_by_group; // cache
  mutable std::unordered_map<std::string,long long int> running_jobs_by_user; // cache
  };

#endif /* SERVERINFO_H_ */
