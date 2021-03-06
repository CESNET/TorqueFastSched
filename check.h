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

#ifndef CHECK_H
#define CHECK_H

#include "server_info.h"
#include "queue_info.h"
#include "job_info.h"
#include "JobLog.h"

/*
 * is_ok_to_run_in_queue - check to see if jobs can be run in queue
 */
int is_ok_to_run_queue(queue_info *qinfo);

/*
 * is_ok_to_run_job - check to see if it ok to run a job on the server
 */
int is_ok_to_run_job(server_info *sinfo, queue_info *qinfo,
                     JobInfo *jinfo, int preassign_starving);

/*
 *      check_run_job - function used by job_filter to filter out
 *                      non-running jobs.
 */
int check_run_job(JobInfo *job, void *arg);

/*
 *      check_server_max_group_run - check to see if group is within their
 *                                   server running limits
 */
int check_server_max_group_run(server_info *sinfo, const std::string& group);

/*
 *      check_queue_max_user_run - check if the user is within queue
 *                                      user run limits
 */
int check_queue_max_user_run(queue_info *qinfo, const std::string& account);

long long int count_by_group(const std::vector<JobInfo*> &jobs, const std::string &group);
long long int count_by_user(const std::vector<JobInfo*> &jobs, const std::string &account);
int count_queue_procs(const queue_info *qinfo);
int count_queue_user_procs(const queue_info *qinfo, const JobInfo *jinfo);
int count_queue_user_procs(const queue_info *qinfo, const std::string& user_name);
int count_queue_group_procs(const queue_info *qinfo, const JobInfo *jinfo);
int count_queue_group_procs(const queue_info *qinfo, const std::string& group_name);

/*
 *      check_server_max_user_run - check if the user is within server
 *                                      user run limits
 */
int check_server_max_user_run(server_info *sinfo, const std::string& account);

/*
 *      check_queue_max_group_run - check to see if the group is within their
 *                                      queue running limits
 */
int check_queue_max_group_run(queue_info *qinfo, const std::string& group);

int check_queue_proc_limits(queue_info *qinfo, JobInfo *jinfo);

/*
 *      will_cross_into_ded_time - check to see if a job would cross into
 *                                 dedicated time
 */
int will_cross_ded_time_boundry(JobInfo *jinfo);

/*
 *      check_nodes - check to see if there is suficient nodes available to
 *                    run a job.
 */
int check_nodes(int pbs_sd, JobInfo *jinfo, node_info **ninfo_arr);

/*
 *      is_node_available - determine that there is a node available to run
 *                          the job
 */
int is_node_available(JobInfo *jinfo, node_info **ninfo_arr);

/*
 *      check_ded_time_queue - check if it is the approprate time to run jobs
 *                             in a dedtime queue
 */
int check_ded_time_queue(queue_info *qinfo);

/** Check if the queue is ignored
*
* @param qinfo queue to be checked
* @returns SUCCESS if OK, QUEUE_IGNORED if ignored
*/
int check_ignored(queue_info *qinfo);

int check_dynamic_resources(server_info *sinfo, JobInfo *jinfo);

#endif

