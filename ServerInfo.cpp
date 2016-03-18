#include <unordered_map>

#include "ServerInfo.h"
#include "check.h"

using namespace std;

void server_info::recount_installing_nodes()
  {
  this->installing_node_count = 0;

  for (int i=0; i<this->num_nodes; i++)
    {
    if (this->nodes[i]->is_building_cluster())
      ++(this->installing_node_count);
    }
  }

bool server_info::installing_nodes_overlimit()
  {
  if (this->max_installing_nodes == RESC_INFINITY)
    return false;

  return this->installing_node_count >= this->max_installing_nodes;
  }

void server_info::regenerate_jobs_by_owner()
  {
  jobs_by_owner.clear();

  // sort jobs into deques based on owner
  for (auto j : this->jobs)
    {
    auto el = jobs_by_owner.insert(make_pair(j->account,deque<JobInfo*>{j}));
    if (el.second == false) // if the users record already exists
      el.first->second.push_back(j);
    }

  // sort the jobs for each user
  // 1) queue priority (descending)
  // 2) job priority (descending)
  // 3) job ID (ascending)
  for (auto &i : this->jobs_by_owner)
    {
    sort(i.second.begin(),i.second.end(),
         [](JobInfo* l, JobInfo* r)
           {
           if (l->queue->priority > r->queue->priority)
             return true;
           else if (l->queue->priority < r->queue->priority)
             return false;


           if (l->priority > r->priority)
             return true;
           else if (l->priority < r->priority)
             return false;

           return l->job_id < r->job_id;
           });
    }
  }

void server_info::update_on_job_run(JobInfo *j)
  {
  this -> sc.running++;
  this -> sc.queued--;

  j->plan_on_server(this);

  for (int i = 0; i < this->num_nodes; i++)
    if (this->nodes[i]->has_assignment())
      j->plan_on_node(this->nodes[i],this->nodes[i]->get_assignment());

  // add this job into the running jobs array and update caches
  this->running_jobs.push_back(j);

  auto grp = running_jobs_by_group.find(j->group);
  if (grp != running_jobs_by_group.end())
    {
    grp->second += 1;
    }
  else
    {
    running_jobs_by_group.insert(make_pair(j->group,count_by_group(this->running_jobs,j->group)));
    }

  auto usr = running_jobs_by_user.find(j->account);
  if (usr != running_jobs_by_user.end())
    {
    usr->second += 1;
    }
  else
    {
    running_jobs_by_user.insert(make_pair(j->account,count_by_user(this->running_jobs,j->account)));
    }
  }

long long int server_info::number_of_running_jobs_for_group(const string& groupname) const
  {
  auto i = running_jobs_by_group.find(groupname);
  if (i != running_jobs_by_group.end())
    {
    return i->second;
    }
  else
    {
    return running_jobs_by_group.insert(make_pair(groupname,count_by_group(this->running_jobs,groupname))).first->second;
    }
  }

long long int server_info::number_of_running_jobs_for_user(const std::string& username) const
  {
  auto i = running_jobs_by_user.find(username);
  if (i != running_jobs_by_user.end())
    {
    return i->second;
    }
  else
    {
    return running_jobs_by_user.insert(make_pair(username,count_by_user(this->running_jobs,username))).first->second;
    }
  }
