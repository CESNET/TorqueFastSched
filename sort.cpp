#include "sort.h"

#include "job_info.h"
#include "globals.h"
#include "fairshare.h"



/*
 *
 * sorting_info[] - holds information about all the different ways you
 *    can sort the jobs
 *
 * Format: { sort_type, config_name, cmp_func_ptr }
 *
 *   sort_type    : an element from the enum sort_type
 *   config_name  : the name which appears in the scheduling policy config
 *           file (sched_config)
 *   cmp_func_ptr : function pointer the qsort compare function
 *    (located in sort.c)
 *
 */
const struct sort_info sorting_info[] =
  {
    { NO_SORT, "no_sort", NULL },
    { SHORTEST_JOB_FIRST, "shortest_job_first", cmp_job_cput_asc},
    { LONGEST_JOB_FIRST, "longest_job_first", cmp_job_cput_dsc},
    { SMALLEST_MEM_FIRST, "smallest_memory_first", cmp_job_mem_asc},
    { LARGEST_MEM_FIRST, "largest_memory_first", cmp_job_mem_dsc},
    { HIGH_PRIORITY_FIRST, "high_priority_first", cmp_job_prio_dsc},
    { LOW_PRIORITY_FIRST, "low_priority_first", cmp_job_prio_asc},
    { LARGE_WALLTIME_FIRST, "large_walltime_first", cmp_job_walltime_dsc},
    { SHORT_WALLTIME_FIRST, "short_walltime_first", cmp_job_walltime_asc},
    { FAIR_SHARE, "fair_share", cmp_fair_share},
    { MULTI_SORT, "multi_sort", multi_sort}
  };

/* number of indicies in the sorting_info array */
const int num_sorts = sizeof(sorting_info) / sizeof(struct sort_info);


bool cmp_generic_asc(resource_req *req1, resource_req *req2)
  {
  if (req1 == NULL && req2 != NULL)
    return false;
  if (req1 != NULL && req2 == NULL)
    return true;

  if (req1 != NULL && req2 != NULL)
    return req1 -> amount < req2 -> amount;

  return false;
  }

bool cmp_generic_desc(resource_req *req1, resource_req *req2)
  {
  if (req1 == NULL && req2 != NULL)
    return true;
  if (req1 != NULL && req2 == NULL)
    return false;

  if (req1 != NULL && req2 != NULL)
    return req1 -> amount > req2 -> amount;

  return false;
  }

bool cmp_job_walltime_asc(const JobInfo *j1, const JobInfo *j2)
  {
  resource_req *req1, *req2;

  req1 = find_resource_req(j1->resreq, "walltime");
  req2 = find_resource_req(j2->resreq, "walltime");

  return cmp_generic_asc(req1,req2);
  }

bool cmp_job_walltime_dsc(const JobInfo *j1, const JobInfo *j2)
  {
  resource_req *req1, *req2;

  req1 = find_resource_req(j1->resreq, "walltime");
  req2 = find_resource_req(j2->resreq, "walltime");

  return cmp_generic_desc(req1,req2);
  }

bool cmp_job_cput_asc(const JobInfo *j1, const JobInfo *j2)
  {
  resource_req *req1, *req2;

  req1 = find_resource_req(j1->resreq, "cput");
  req2 = find_resource_req(j2->resreq, "cput");

  return cmp_generic_asc(req1,req2);
  }

bool cmp_job_cput_dsc(const JobInfo *j1, const JobInfo *j2)
  {
  resource_req *req1, *req2;

  req1 = find_resource_req(j1->resreq, "cput");
  req2 = find_resource_req(j2->resreq, "cput");

  return cmp_generic_desc(req1,req2);
  }

bool cmp_job_mem_asc(const JobInfo *j1, const JobInfo *j2)
  {
  resource_req *req1, *req2;

  req1 = find_resource_req(j1->resreq, "mem");
  req2 = find_resource_req(j2->resreq, "mem");

  return cmp_generic_asc(req1,req2);
  }

bool cmp_job_mem_dsc(const JobInfo *j1, const JobInfo *j2)
  {
  resource_req *req1, *req2;

  req1 = find_resource_req(j1->resreq, "mem");
  req2 = find_resource_req(j2->resreq, "mem");

  return cmp_generic_desc(req1,req2);
  }

bool cmp_job_prio_asc(const JobInfo *j1, const JobInfo *j2)
  {
  return j1->priority < j2->priority;
  }

bool cmp_job_prio_dsc(const JobInfo *j1, const JobInfo *j2)
  {
  return j1->priority > j2->priority;
  }

bool cmp_fair_share(const JobInfo *j1, const JobInfo *j2)
  {
  return j1->ginfo->percentage > j2->ginfo->percentage;
  }

bool multi_sort(const JobInfo *j1, const JobInfo *j2)
  {
  int i = 1;

  while (i <= num_sorts && cstat.sort_by[i].sort != NO_SORT &&
    !cstat.sort_by[i].cmp_func(j1,j2) &&
    !cstat.sort_by[i].cmp_func(j2,j1))
    i++;

  if (i <= num_sorts && cstat.sort_by[i].sort != NO_SORT)
    return cstat.sort_by[i].cmp_func(j1,j2);
  else
    return false;
  }

bool cmp_sort(const JobInfo *v1, const JobInfo *v2)
  {
  return cstat.sort_by->cmp_func(v1, v2);
  }
