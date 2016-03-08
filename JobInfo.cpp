#include "JobInfo.h"
#include "misc.h"
#include "data_types.h"
#include "logic/NodeFilters.h"
#include "NodeSort.h"
#include "SchedulerCore_RescInfoDb.h"
#include "globals.h"

#include "base/MiscHelpers.h"
using namespace Scheduler;
using namespace Core;

#include "include/gsl.h"

#include <algorithm>
#include <limits>
using namespace std;

JobInfo::~JobInfo()
  {
  free_resource_req_list(this->resreq);
  free_resource_req_list(this->resused);
  free_parsed_nodespec(this->p_parsed_nodespec);
  }

JobInfo::JobInfo(struct batch_status *job, queue_info *queue) : job_id(),
  custom_name(), comment(), nodespec(), queue(nullptr), qtime(0), stime(0),
  priority(0), account(), group(), state(JobNoState), can_never_run(false),
  can_not_run(false), calculated_fairshare(0), sched_nodespec(),
  p_planned_nodes(), p_planned_start(0), p_waiting_for(), ginfo(nullptr),
  schedule(), cluster_name(), cluster_mode(ClusterNone), placement(),
  resreq(nullptr), resused(nullptr), p_parsed_nodespec(nullptr)
  {
  struct attrl *attrp;  /* list of attributes returned from server */
  int count;   /* int used in string -> int conversion */
  char *endp;   /* used for strtol() */
  resource_req *resreq;  /* resource_req list for resources requested  */

  this->job_id = job->name;
  this->queue = queue;

  attrp = job -> attribs;

  while (attrp != NULL)
    {
    if (!strcmp(attrp -> name, ATTR_name))
      {
      this->custom_name = attrp->value;
      }
    else if (!strcmp(attrp -> name, ATTR_comment))
      {
      this->comment = attrp->value;
      }
    else if (!strcmp(attrp -> name, ATTR_qtime))
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\n')
        this -> qtime = count;
      else
        this -> qtime = -1;
      }
    else if (!strcmp(attrp -> name, ATTR_start_time))
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\n')
        this -> stime = count;
      else
        this -> stime = -1;
      }
    else if (!strcmp(attrp -> name, ATTR_p))
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\n')
        this -> priority = count;
      else
        this -> priority = -1;
      }
    else if (!strcmp(attrp -> name, ATTR_euser))
      this->account = attrp -> value;
    else if (!strcmp(attrp -> name, ATTR_egroup))
      this->group = attrp -> value;
    else if (!strcmp(attrp -> name, ATTR_state))
      set_state(attrp -> value, this);
    else if (!strcmp(attrp -> name, ATTR_fairshare_cost))
      this -> calculated_fairshare = atof(attrp->value);
    else if (!strcmp(attrp -> name, ATTR_schedspec))
      this -> sched_nodespec = attrp -> value;
    else if (!strcmp(attrp -> name, ATTR_planned_nodes))
      this -> p_planned_nodes = attrp -> value;
    else if (!strcmp(attrp -> name, ATTR_waiting_for))
      this -> p_waiting_for = attrp -> value;
    else if (!strcmp(attrp -> name, ATTR_planned_start))
      {
      count = strtol(attrp -> value, &endp, 10);

      if (*endp != '\n')
        this -> p_planned_start = count;
      else
        this -> p_planned_start = -1;
      }
    else if (!strcmp(attrp -> name, ATTR_l))    /* resources requested*/
      {
      /* special handling for cluster */
      if (strcmp(attrp->resource,"cluster") == 0)
        {
        if (strcmp(attrp->value,"create") == 0)
          {
          this->cluster_mode = ClusterCreate;
          }
        else
          {
          this->cluster_mode = ClusterUse;
          this->cluster_name = attrp->value;
          }
        }
      else if (strcmp(attrp->resource,"processed_nodes") == 0)
        {
        this->nodespec = attrp->value;
        }
      else if (strcmp(attrp->resource,"place") == 0)
        {
        this->placement = attrp->value;
        }
      }
    else if (!strcmp(attrp -> name, ATTR_total_resources))
      {
      resreq = find_alloc_resource_req(attrp -> resource, this -> resreq);

      if (resreq != NULL)
        {
        resreq -> res_str = strdup(attrp -> value);
        resreq -> amount = res_to_num(attrp -> value);
        }

      if (this -> resreq == NULL)
        this -> resreq = resreq;
      }
    else if (!strcmp(attrp -> name, ATTR_used))    /* resources used */
      {
      resreq = find_alloc_resource_req(attrp -> resource, this -> resused);

      if (resreq != NULL)
        resreq -> amount = res_to_num(attrp -> value);

      if (this -> resused == NULL)
        this -> resused = resreq;
      }

    attrp = attrp -> next;
    }
  }

void JobInfo::unplan_from_node(node_info* ninfo, pars_spec_node* spec)
  {
  Resource *res;

  if (this->is_exclusive())
    ninfo->freeup_proc(ninfo->get_cores_total());
  else
    ninfo->freeup_proc(spec->procs);

  /* mem */
  if ((res = ninfo->get_resource("mem")) != NULL)
    res->freeup_resource(spec->mem);

  if (ninfo->get_scratch_assign() == ScratchLocal)
    {
    if ((res = ninfo->get_resource("scratch_local")) != NULL)
      res->freeup_resource(spec->scratch);
    }

  if (ninfo->get_scratch_assign() == ScratchSSD)
    {
    if ((res = ninfo->get_resource("scratch_ssd")) != NULL)
      res->freeup_resource(spec->scratch);
    }

  if (ninfo->get_scratch_assign() == ScratchShared)
    {
    if ((res = ninfo->get_resource("scratch_pool")) != NULL)
      {
      string pool = res->get_str_val();
      map<string,DynamicResource>::iterator i;
      i = ninfo->get_parent_server()->dynamic_resources.find(pool);
      if (i != ninfo->get_parent_server()->dynamic_resources.end())
        {
        i->second.remove_scheduled(spec->scratch);
        }
      }
    }

  /* the rest */
  pars_prop *iter = spec->properties;
  while (iter != NULL)
    {
    if (iter->value == NULL)
      { iter = iter->next; continue; }

    if (res_check_type(iter->name) != ResCheckNone)
      {
      if ((res = ninfo->get_resource(iter->name)) != NULL)
        res->freeup_resource(iter->value);
      }

    iter = iter->next;
    }
  }

void JobInfo::plan_on_node(node_info* ninfo, pars_spec_node* spec)
  {
  Resource *res;

  if (this->cluster_mode == ClusterCreate)
    {
    for (size_t i = 0; i < ninfo->get_host()->get_hosted().size(); i++)
      {
      ninfo->get_host()->get_hosted()[i]->set_notusable();
      sched_log(PBSEVENT_SCHED, PBS_EVENTCLASS_NODE, ninfo->get_name(),
                "Node marked as incapable of running and booting jobs, because it, or it's sister is servicing a cluster job.");
      }
    }

  if (this->is_exclusive())
    ninfo->deplete_proc(ninfo->get_cores_total());
  else
    ninfo->deplete_proc(spec->procs);

  /* mem */
  if ((res = ninfo->get_resource("mem")) != NULL)
    res->consume_resource(spec->mem);

  if (ninfo->get_scratch_assign() == ScratchLocal)
    {
    if ((res = ninfo->get_resource("scratch_local")) != NULL)
      res->consume_resource(spec->scratch);
    }

  if (ninfo->get_scratch_assign() == ScratchSSD)
    {
    if ((res = ninfo->get_resource("scratch_ssd")) != NULL)
      res->consume_resource(spec->scratch);
    }

  if (ninfo->get_scratch_assign() == ScratchShared)
    {
    if ((res = ninfo->get_resource("scratch_pool")) != NULL)
      {
      string pool = res->get_str_val();
      map<string,DynamicResource>::iterator i;
      i = ninfo->get_parent_server()->dynamic_resources.find(pool);
      if (i != ninfo->get_parent_server()->dynamic_resources.end())
        {
        i->second.add_scheduled(spec->scratch);
        }
      }
    }

  /* the rest */
  pars_prop *iter = spec->properties;
  while (iter != NULL)
    {
    if (iter->value == NULL)
      { iter = iter->next; continue; }

    if (res_check_type(iter->name) != ResCheckNone)
      {
      if ((res = ninfo->get_resource(iter->name)) != NULL)
        res->consume_resource(iter->value);
      }

    iter = iter->next;
    }
  }

void JobInfo::plan_on_queue(queue_info* qinfo)
  {
  }

void JobInfo::plan_on_server(server_info* sinfo)
  {
  /* count dynamic resources */
  resource_req *req = resreq;
  while (req != NULL)
    {
    map<string,DynamicResource>::iterator it = sinfo->dynamic_resources.find(string(req->name));
    if (it != sinfo->dynamic_resources.end())
      {
      it->second.add_assigned(req->amount);
      }

    req = req->next;
    }
  }

double JobInfo::calculate_fairshare_cost(const vector<node_info*>& nodes) const
  {
  double fairshare_cost = 0;

  pars_spec_node *iter = this->parsed_nodespec()->nodes;
  while (iter != NULL)
    {
    vector<node_info*> fairshare_nodes; // construct possible nodes
    NodeSuitableForSpec::filter_fairshare(nodes,fairshare_nodes,this,iter);
    sort(fairshare_nodes.begin(),fairshare_nodes.end(),NodeCostSort(iter->procs,iter->mem,this->is_exclusive()));

    unsigned i = 0;
    for (unsigned count = 0; count < iter->node_count; count++)
      {
      while (fairshare_nodes[i]->has_fairshare_flag())
        i++;

      unsigned long long node_procs = fairshare_nodes[i]->get_cores_total();
      unsigned long long node_mem   = fairshare_nodes[i]->get_mem_total();

      if (this->is_exclusive())
        fairshare_cost += node_procs*fairshare_nodes[i]->get_node_cost();
      else
        fairshare_cost += max(static_cast<double>(iter->mem)/node_mem,static_cast<double>(iter->procs)/node_procs)*node_procs*fairshare_nodes[i]->get_node_cost();

      fairshare_nodes[i]->set_fairshare_flag();
      }
    iter = iter->next;
    }

  return fairshare_cost;
  }

long JobInfo::get_walltime() const
  {
  resource_req *resc = find_resource_req(this->resreq,"walltime");
  if (resc == NULL)
    return 0;

  return resc->amount;
  }

time_t JobInfo::completion_time()
  {
  resource_req *req = find_resource_req(this->resreq,"walltime");
  if (req == NULL)
    return numeric_limits<time_t>::max();

  return stime + req->amount;
  }

bool JobInfo::is_starving() const noexcept
  {
  Expects(this->queue != NULL);

  return (this->state == JobQueued && this->queue->starving_support >= 0 && this->qtime + this->queue->starving_support < cstat.current_time);
  }

bool JobInfo::is_multinode() const noexcept
  {
  return this->parsed_nodespec()->total_nodes > 1;
  }

bool JobInfo::is_exclusive() const noexcept
  {
  return this->parsed_nodespec()->is_exclusive;
  }

pars_spec *JobInfo::parsed_nodespec() const
  {
  if (this->p_parsed_nodespec == NULL)
    {
    if (this->nodespec.size() == 0)
      this->nodespec = "1:ppn=1";

    this->p_parsed_nodespec = parse_nodespec(this->nodespec.c_str());
    }

  return this->p_parsed_nodespec;
  }