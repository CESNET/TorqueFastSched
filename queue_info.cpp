/*
*         OpenPBS (Portable Batch System) v2.3 Software License
*
* Copyright (c) 1999-2000 Veridian Information Solutions, Inc.
* All rights reserved.
*
* ---------------------------------------------------------------------------
* For a license to use or redistribute the OpenPBS software under conditions
* other than those described below, or to purchase support for this software,
* please contact Veridian Systems, PBS Products Department ("Licensor") at:
*
*    www.OpenPBS.org  +1 650 967-4675                  sales@OpenPBS.org
*                        877 902-4PBS (US toll-free)
* ---------------------------------------------------------------------------
*
* This license covers use of the OpenPBS v2.3 software (the "Software") at
* your site or location, and, for certain users, redistribution of the
* Software to other sites and locations.  Use and redistribution of
* OpenPBS v2.3 in source and binary forms, with or without modification,
* are permitted provided that all of the following conditions are met.
* After December 31, 2001, only conditions 3-6 must be met:
*
* 1. Commercial and/or non-commercial use of the Software is permitted
*    provided a current software registration is on file at www.OpenPBS.org.
*    If use of this software contributes to a publication, product, or
*    service, proper attribution must be given; see www.OpenPBS.org/credit.html
*
* 2. Redistribution in any form is only permitted for non-commercial,
*    non-profit purposes.  There can be no charge for the Software or any
*    software incorporating the Software.  Further, there can be no
*    expectation of revenue generated as a consequence of redistributing
*    the Software.
*
* 3. Any Redistribution of source code must retain the above copyright notice
*    and the acknowledgment contained in paragraph 6, this list of conditions
*    and the disclaimer contained in paragraph 7.
*
* 4. Any Redistribution in binary form must reproduce the above copyright
*    notice and the acknowledgment contained in paragraph 6, this list of
*    conditions and the disclaimer contained in paragraph 7 in the
*    documentation and/or other materials provided with the distribution.
*
* 5. Redistributions in any form must be accompanied by information on how to
*    obtain complete source code for the OpenPBS software and any
*    modifications and/or additions to the OpenPBS software.  The source code
*    must either be included in the distribution or be available for no more
*    than the cost of distribution plus a nominal fee, and all modifications
*    and additions to the Software must be freely redistributable by any party
*    (including Licensor) without restriction.
*
* 6. All advertising materials mentioning features or use of the Software must
*    display the following acknowledgment:
*
*     "This product includes software developed by NASA Ames Research Center,
*     Lawrence Livermore National Laboratory, and Veridian Information
*     Solutions, Inc.
*     Visit www.OpenPBS.org for OpenPBS software support,
*     products, and information."
*
* 7. DISCLAIMER OF WARRANTY
*
* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT
* ARE EXPRESSLY DISCLAIMED.
*
* IN NO EVENT SHALL VERIDIAN CORPORATION, ITS AFFILIATED COMPANIES, OR THE
* U.S. GOVERNMENT OR ANY OF ITS AGENCIES BE LIABLE FOR ANY DIRECT OR INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* This license will be governed by the laws of the Commonwealth of Virginia,
* without reference to its choice of law rules.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "torque.h"
#include "queue_info.h"
#include "misc.h"
#include "check.h"
#include "globals.h"

using namespace std;

using namespace Scheduler;
using namespace Core;

void  queue_info::update_on_job_run(JobInfo *j)
  {
  this -> sc.running++;
  this -> sc.queued--;

  // add this job into the running jobs array and update caches
  this->running_jobs.push_back(j);

  // number of running jobs per group
  auto grp = running_jobs_by_group.find(j->group);
  if (grp != running_jobs_by_group.end())
    {
    grp->second += 1;
    }
  else
    {
    running_jobs_by_group.insert(make_pair(j->group,count_by_group(this->running_jobs,j->group)));
    }

  // number of running jobs per user
  auto usr = running_jobs_by_user.find(j->account);
  if (usr != running_jobs_by_user.end())
    {
    usr->second += 1;
    }
  else
    {
    running_jobs_by_user.insert(make_pair(j->account,count_by_user(this->running_jobs,j->account)));
    }

  // number of used cores
  if (running_cores.first == true)
    {
    running_cores.second += j->parsed_nodespec()->total_procs;
    }
  else
    {
    running_cores.first = true;
    running_cores.second = count_queue_procs(this);
    }

  // number of used cores per group
  auto grpcore = running_cores_by_group.find(j->group);
  if (grpcore != running_cores_by_group.end())
    {
    grpcore->second += j->parsed_nodespec()->total_procs;
    }
  else
    {
    running_cores_by_group.insert(make_pair(j->group,count_queue_group_procs(this,j)));
    }

  // number of used cores per user
  auto usrcore = running_cores_by_user.find(j->account);
  if (usrcore != running_cores_by_user.end())
    {
    usrcore->second += j->parsed_nodespec()->total_procs;
    }
  else
    {
    running_cores_by_user.insert(make_pair(j->account,count_queue_user_procs(this,j)));
    }
  }


long long int queue_info::number_of_running_jobs_for_group(const string& groupname) const
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

long long int queue_info::number_of_running_jobs_for_user(const std::string& username) const
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

long long int queue_info::number_of_running_cores() const
  {
  if (!running_cores.first)
    {
    running_cores.first = true;
    running_cores.second = count_queue_procs(this);
    }

  return running_cores.second;
  }

long long int queue_info::number_of_running_cores_for_group(const std::string& group_name) const
  {
  auto i = running_cores_by_group.find(group_name);
  if (i != running_cores_by_group.end())
    {
    return i->second;
    }
  else
    {
    return running_jobs_by_group.insert(make_pair(group_name,count_queue_group_procs(this,group_name))).first->second;
    }
  }

long long int queue_info::number_of_running_cores_for_user(const std::string& user_name) const
  {
  auto i = running_cores_by_user.find(user_name);
  if (i != running_cores_by_user.end())
    {
    return i->second;
    }
  else
    {
    return running_jobs_by_user.insert(make_pair(user_name,count_queue_user_procs(this,user_name))).first->second;
    }
  }

/*
 *
 * query_queues - creates an array of queue_info structs which contain
 *   an array of jobs
 *
 *   pbs_sd - connection to the pbs_server
 *   sinfo  - server to query queues from
 *
 * returns pointer to the head of the queue structure
 *
 */
queue_info **query_queues(int pbs_sd, server_info *sinfo)
  {
  /* the linked list of queues returned from the server */

  struct batch_status *queues;

  /* the current queue in the linked list of queues */

  struct batch_status *cur_queue;

  /* array of pointers to internal scheduling structure for queues */
  queue_info **qinfo_arr;

  /* the current queue we are working on */
  queue_info *qinfo;

  /* return code */
  int ret;

  /* buffer to store comment message */
  char comment[MAX_COMMENT_SIZE];

  /* buffer to store log message */
  char log_msg[MAX_LOG_SIZE];

  int i;
  int num_queues = 0;

  /* get queue info from PBS server */

  if ((queues = pbs_statque(pbs_sd, NULL, NULL, NULL)) == NULL)
    {
    fprintf(stderr, "Statque failed: %d\n", pbs_errno);
    return NULL;
    }

  cur_queue = queues;

  while (cur_queue != NULL)
    {
    num_queues++;
    cur_queue = cur_queue -> next;
    }

  if ((qinfo_arr = static_cast<queue_info**>(calloc(num_queues+1,sizeof(queue_info *)))) == NULL)
    {
    perror("Memory Allocation error");
    pbs_statfree(queues);
    return NULL;
    }

  cur_queue = queues;

  for (i = 0; cur_queue != NULL; i++)
    {
    /* convert queue information from batch_status to queue_info */
    if ((qinfo = query_queue_info(cur_queue, sinfo)) == NULL)
      {
      pbs_statfree(queues);
      free_queues(qinfo_arr, true);
      return NULL;
      }

    /* get all the jobs which reside in the queue */
    if (!query_jobs(pbs_sd, qinfo, qinfo->jobs))
      sched_log(PBSEVENT_DEBUG2, PBS_EVENTCLASS_SERVER, qinfo->name, "Error while decoding jobs." );

    /* check if the queue is a dedicated time queue */
    if (conf.ded_prefix[0] != '\0')
      if (!strncmp(qinfo -> name, conf.ded_prefix, strlen(conf.ded_prefix)))
        qinfo -> dedtime_queue = 1;

    if (qinfo -> is_global)
      sched_log(PBSEVENT_DEBUG2, PBS_EVENTCLASS_QUEUE, qinfo->name, "Marked as global.");

    /* check if it is OK for jobs to run in the queue */
    ret = is_ok_to_run_queue(qinfo);

    if (ret == SUCCESS)
      qinfo -> is_ok_to_run = 1;
    else
      {
      qinfo -> is_ok_to_run = 0;
      translate_queue_fail_code(ret, comment, log_msg);
      sched_log(PBSEVENT_DEBUG2, PBS_EVENTCLASS_SERVER, qinfo -> server -> name, "Processing server." );
      sched_log(PBSEVENT_DEBUG2, PBS_EVENTCLASS_QUEUE, qinfo -> name, log_msg);
      update_jobs_cant_run(pbs_sd, qinfo -> jobs, NULL, comment, START_WITH_JOB);
      }

    count_states(qinfo->jobs, qinfo -> sc);

    qinfo->running_jobs.clear();
    qinfo->running_jobs.reserve(qinfo->jobs.size()/2);
    copy_if(begin(qinfo->jobs),end(qinfo->jobs),back_inserter(qinfo->running_jobs),
            [](JobInfo *j) { return j->state == JobRunning; });

    qinfo_arr[i] = qinfo;

    cur_queue = cur_queue -> next;
    }

  qinfo_arr[i] = NULL;

  pbs_statfree(queues);

  return qinfo_arr;
  }


static void push_excl_node(queue_info *qinfo, node_info *ninfo)
  {
  if (qinfo->excl_node_capacity <= qinfo->excl_node_count+1)
    {
    node_info **tmp;
  
    /* if capacity == zero, increase to 5 */
    qinfo->excl_node_capacity += 5 * (qinfo->excl_node_capacity == 0);
  
    tmp = (node_info**) realloc(qinfo->excl_nodes,sizeof(node_info*)*qinfo->excl_node_capacity*2);
    if (tmp == NULL) return; /* no better handling possible right now */
      
    qinfo->excl_node_capacity *= 2;
    qinfo->excl_nodes = tmp;
    }
    
  qinfo->excl_nodes[qinfo->excl_node_count] = ninfo;
  qinfo->excl_nodes[qinfo->excl_node_count+1] = NULL;
  qinfo->excl_node_count++;
  ninfo->set_excl_queue(qinfo);
  }   

/*
 *
 * query_queue_info - collects information from a batch_status and
 *      puts it in a queue_info struct for easier access
 *
 *   queue - batch_status struct to get queue information from
 *   sinfo - server where queue resides
 *
 * returns newly allocated and filled queue_info or NULL on error
 *
 */

queue_info *query_queue_info(struct batch_status *queue, server_info *sinfo)
  {

  struct attrl *attrp;  /* linked list of attributes from server */

  struct queue_info *qinfo; /* queue_info being created */
  char *endp;   /* used with strtol() */
  int count,i;   /* used to convert string -> integer */

  if ((qinfo = new_queue_info()) == NULL)
    return NULL;

  attrp = queue -> attribs;

  qinfo -> name = strdup(queue -> name);

  qinfo -> server = sinfo;

  while (attrp != NULL)
    {
    if (!strcmp(attrp -> name, ATTR_start))   /* started */
      {
      if (!strcmp(attrp -> value, "True"))
        qinfo -> is_started = true;
      }
    else if (!strcmp(attrp -> name, ATTR_maxrun))  /* max_running */
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\0')
        count = -1;

      qinfo -> max_run = count;
      }
    else if (!strcmp(attrp -> name, ATTR_maxuserrun))  /* max_user_run */
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\0')
        count = -1;

      qinfo -> max_user_run = count;
      }
    else if (!strcmp(attrp -> name, ATTR_maxgrprun))  /* max_group_run */
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\0')
        count = -1;

      qinfo -> max_group_run = count;
      }
    else if (!strcmp(attrp -> name, ATTR_maxproc))  /* max_group_run */
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\0')
        count = -1;

      qinfo -> max_proc = count;
      }
    else if (!strcmp(attrp -> name, ATTR_maxuserproc))  /* max_group_run */
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\0')
        count = -1;

      qinfo -> max_user_proc = count;
      }
    else if (!strcmp(attrp -> name, ATTR_maxgrpproc))  /* max_group_run */
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\0')
        count = -1;

      qinfo -> max_group_proc = count;
      }
    else if (!strcmp(attrp->name, ATTR_fairshare_tree)) /* fairshare_tree */
      {
      if (qinfo->fairshare_tree != NULL)
        free(qinfo->fairshare_tree);

      qinfo->fairshare_tree = strdup(attrp->value);
      }
    else if (!strcmp(attrp -> name, ATTR_p))    /* priority */
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\0')
        count = -1;

      qinfo -> priority = count;
      }
    else if (!strcmp(attrp -> name, ATTR_qtype))  /* queue_type */
      {
      if (!strcmp(attrp -> value, "Execution"))
        {
        qinfo -> is_exec = 1;
        qinfo -> is_route = 0;
        }
      else if (!strcmp(attrp -> value, "Route"))
        {
        qinfo -> is_route = 1;
        qinfo -> is_exec = 0;
        }
      }
    else if (!strcmp(attrp -> name, ATTR_is_transit)) /* transit queue */
      {
      if (!strcmp(attrp -> value, "True"))
        qinfo -> is_global = 1;
      }
    else if (!strcmp(attrp -> name, ATTR_starving_support)) /* starving support */
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\0')
        count = -1;

      qinfo->starving_support = count;
      }
    else if (!strcmp(attrp -> name, ATTR_fairshare_coef)) /* starving support */
      {
      qinfo->queue_cost = atof(attrp -> value);
      }
    else if (!strcmp(attrp -> name, ATTR_admin_queue))
      {
      if (!strcmp(attrp -> value, "True"))
        qinfo -> is_admin_queue = 1;
      }
    else if (!strcmp(attrp -> name, ATTR_required_property)) /* required property */
      {
      int i;
      qinfo->excl_nodes_only = 1;
      /* check all nodes for the property */
      for (i = 0; i < sinfo->num_nodes; i++)
        {
        if (sinfo->nodes[i]->has_prop(attrp->value)
            && sinfo->nodes[i]->get_dedicated_queue_name().size() == 0) /* ignore nodes with queue=... */
          push_excl_node(qinfo,sinfo->nodes[i]);
        }
      }

    attrp = attrp -> next;
    }

  /* go through all nodes, and check the queue attribute */
  for (i = 0; i < sinfo->num_nodes; i++)
    {
    if (sinfo->nodes[i]->get_dedicated_queue_name().size() != 0)
      if (sinfo->nodes[i]->get_dedicated_queue_name() == qinfo->name)
        push_excl_node(qinfo,sinfo->nodes[i]);
    }

  return qinfo;
  }

/*
 *
 * new_queue_info - allocate and initalize a new queue_info struct
 *
 * returns the newly allocated struct or NULL on error
 *
 */

queue_info *new_queue_info()
  {
  queue_info *qinfo;

  if ((qinfo = new (nothrow) queue_info) == NULL)
    return NULL;

  qinfo -> is_started  = false;

  qinfo -> is_exec  = 0;

  qinfo -> is_route  = 0;
  
  qinfo -> is_global = 0;

  qinfo -> dedtime_queue = 0;

  qinfo -> is_ok_to_run  = 0;

  qinfo -> priority  = 0;
  qinfo -> starving_support = -1;
  qinfo -> is_admin_queue = 0;

  init_state_count(&(qinfo -> sc));

  qinfo -> max_run  = RESC_INFINITY;

  qinfo -> max_user_run  = RESC_INFINITY;

  qinfo -> max_group_run = RESC_INFINITY;

  qinfo -> max_proc = RESC_INFINITY;

  qinfo -> max_user_proc = RESC_INFINITY;

  qinfo -> max_group_proc = RESC_INFINITY;

  qinfo -> name   = NULL;

  qinfo -> server  = NULL;

  qinfo -> excl_nodes_only = 0;
  qinfo -> excl_node_count = 0;
  qinfo -> excl_node_capacity = 0;
  qinfo -> excl_nodes = NULL;

  qinfo -> queue_cost = 1.0;

  qinfo -> fairshare_tree = strdup("default");

  return qinfo;
  }

/*
 *
 * free_queues - free an array of queues
 *
 *   qarr - qinfo array to delete
 *   free_jobs_too - free the jobs in the queue also
 *
 * returns nothing
 *
 */

void free_queues(queue_info **qarr, bool free_jobs_too)
  {
  int i;

  if (qarr == NULL)
    return;

  for (i = 0; qarr[i] != NULL; i++)
    {
    if (free_jobs_too)
      for (JobInfo* j : qarr[i]->jobs)
        delete j;

    free_queue_info(qarr[i]);
    }

  free(qarr);
  }

/** Update queue on job move
 *
 * Update the queue_info structure with new information, when new job is pushed
 * into the queue.
 *
 * @param qinfo Queue to be update
 * @param jinfo Job that is moved
 */
void update_queue_on_move(queue_info *qinfo, JobInfo *jinfo)
  {
  qinfo -> sc.running++; /* update the target queue */
  jinfo -> queue -> sc.queued--; /* update the local queue */
  }

/*
 *
 * free_queue_info - free space used by a queue info struct
 *
 *   qinfo - queue to free
 *
 * returns nothing
 *
 */
void free_queue_info(queue_info *qinfo)
  {
  free(qinfo -> name);
  free(qinfo -> excl_nodes);
  free(qinfo -> fairshare_tree);
  delete qinfo;
  }

/*
 *
 *   translate_queue_fail_code - translate the failure code of
 *   is_ok_to_run_in_queue into log and comment_msg messages
 *
 *   pbs_sd           - communication descriptor to the pbs server
 *   fail_code        - failure code to translate
 *   OUT: comment_msg_msg - translated comment message
 *   OUT: log_msg     - translated log message
 *
 * returns nothing
 *
 */
void translate_queue_fail_code(int fail_code,
                               char *comment_msg, char *log_msg)
  {
  switch (fail_code)
    {

    case QUEUE_NOT_STARTED:
      strcpy(comment_msg, COMMENT_QUEUE_NOT_STARTED);
      strcpy(log_msg, INFO_QUEUE_NOT_STARTED);
      break;

    case QUEUE_NOT_EXEC:
      strcpy(comment_msg, COMMENT_QUEUE_NOT_EXEC);
      strcpy(log_msg, INFO_QUEUE_NOT_EXEC);
      break;

    case DED_TIME:
      strcpy(comment_msg, COMMENT_DED_TIME);
      strcpy(log_msg, INFO_DED_TIME);
      break;
      
    case QUEUE_REMOTE_LOCAL:
      strcpy(comment_msg, COMMENT_QUEUE_REMOTE_LOCAL);
      strcpy(log_msg, INFO_QUEUE_REMOTE_LOCAL);
      break;
      
    case QUEUE_IGNORED:
      strcpy(comment_msg, COMMENT_QUEUE_IGNORED);
      strcpy(log_msg, INFO_QUEUE_IGNORED);
      break;

    default:
      comment_msg[0] = '\0';
    }
  }
