/*
The MIT License (MIT)
Copyright (c) 2016 Simon Toth (simon@cesnet.cz), CESNET a.l.e.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SchedulerCore_ConnectionMgr.h"
#include "utility.h"

/* Helper functions */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <cstring>
#include <stdexcept>
using namespace std;

extern "C" {
#include "legacy/server_limits.h"
#include "legacy/libpbs.h"

extern struct connect_handle connection[];
pbs_net_t get_hostaddr(char *);
int  client_to_svr(pbs_net_t, unsigned int, int, char *);
}

namespace {

/* sock refers to an opened socket */
int socket_to_conn(int sock) noexcept
  {
  int     i;

  for (i = 0; i < PBS_NET_MAX_CONNECTIONS; i++)
    {
    if (connection[i].ch_inuse == 0)
      {
      connection[i].ch_inuse = 1;
      connection[i].ch_errno = 0;
      connection[i].ch_socket = sock;
      connection[i].ch_errtxt = NULL;
      return (i);
      }
    }

  pbs_errno = PBSE_NOCONNECTS;

  return (-1);
  }

int privileged_connect(const char* server) noexcept
  {
  pbs_net_t hostaddr;
  if ((hostaddr = get_hostaddr((char*)server)) == (pbs_net_t)0)
    {
    return -1;
    }

  unsigned int port = 15051;
  int sock = client_to_svr(hostaddr, port, 1, NULL);

  if (sock < 0)
    {
    return(-1);
    }

  return socket_to_conn(sock);
  }

}


namespace Scheduler {
namespace Core {

/** \brief Verify that the given string represents a valid FQDN
 *
 * @param fqdn Tested FQDN
 * @return \c TRUE if given FQDN is valid, \c FALSE if not
 */
bool verify_fqdn(const string& fqdn)
  {
  struct addrinfo addr;
  memset(&addr,0,sizeof(addr));

  addr.ai_family = AF_INET;
  addr.ai_socktype = SOCK_STREAM;
  addr.ai_flags = AI_CANONNAME;

  struct addrinfo *info = NULL;
  if (getaddrinfo(fqdn.c_str(),"15051",&addr,&info) != 0)
    {
    if (errno == EAI_MEMORY)
      global_oom_handle();
    else if (errno == EAI_AGAIN)
      throw runtime_error(string("Temporary Network Error: Couldn't resolve hostname in \"verify_fqdn()\"."));
    else if (errno == EAI_FAIL)
      throw runtime_error(string("Permanent Network Error: Couldn't resolve hostname in \"verify_fqdn()\"."));

    return false;
    }

  bool result = (fqdn == info->ai_canonname);
  freeaddrinfo(info);

  return result;
  }

/** \brief Deduce a FQDN from a given hostname
 *
 * @param host Input hostname
 * @return Deduced FQDN
 */
std::string get_fqdn(const std::string& host)
  {
  struct addrinfo addr;
  memset(&addr,0,sizeof(addr));

  addr.ai_family = AF_INET;
  addr.ai_socktype = SOCK_STREAM;
  addr.ai_flags = AI_CANONNAME;

  struct addrinfo *info = NULL;
  if (getaddrinfo(host.c_str(),"15051",&addr,&info) != 0)
    {
    if (errno == EAI_MEMORY)
      global_oom_handle();
    else if (errno == EAI_AGAIN)
      throw runtime_error(string("Temporary Network Error: Couldn't resolve hostname in \"get_fqdn()\"."));
    else
      throw runtime_error(string("Permanent Network Error: Couldn't resolve hostname in \"get_fqdn()\"."));
    }

  string result = info->ai_canonname;
  freeaddrinfo(info);

  return result;
  }

/** \brief Get the FQDN for the local system
 *
 * @return FQDN of local machine
 */
std::string get_local_fqdn()
  {
  char hostname[1024+1] = {0};

  if (gethostname(hostname,1024) != 0)
    throw runtime_error(string("Permanent Network Error: Couldn't get local hostname in \"get_local_fqdn()\" (") + get_errno_string() + string(")."));

  return get_fqdn(string(hostname));
  }

int ConnectionMgr::make_master_connection(const string& hostname)
  {
  /* pass-through errors */
  p_master = get_fqdn(hostname);

  map<string,int>::const_iterator i = p_connections.find(p_master);
  if (i != p_connections.end())
    {
    return i->second;
    }

  int connector = privileged_connect(p_master.c_str());
  if (connector < 0)
    throw runtime_error(string("Connection to master server (\"" + p_master + "\") failed."));

  p_connections.insert(make_pair(p_master,connector));

  return connector;
  }

int ConnectionMgr::make_remote_connection(const string& hostname)
  {
  /* pass-through errors */
  string connect = get_fqdn(hostname);

  map<string,int>::const_iterator i = p_connections.find(connect);
  if (i != p_connections.end())
    {
    return i->second;
    }

  int connector = privileged_connect(connect.c_str());
  if (connector < 0)
    throw runtime_error(string("Connection to remote server (\"" + connect + "\") failed."));

  p_connections.insert(make_pair(connect,connector));

  return connector;
  }

void ConnectionMgr::disconnect(const string& hostname)
  {
  map<string,int>::iterator i = p_connections.find(hostname);
  if (i != p_connections.end())
    {
    pbs_disconnect(i->second);
    p_connections.erase(i);
    }
  }

void ConnectionMgr::disconnect_all() noexcept
  {
  map<string,int>::iterator i;
  for (i = p_connections.begin(); i != p_connections.end(); ++i)
    {
    pbs_disconnect(i->second);
    }
  p_connections.clear();
  }

int ConnectionMgr::get_master_connection() const
  {
  return get_connection(p_master);
  }

int ConnectionMgr::get_connection(const string& hostname) const
  {
  map<string,int>::const_iterator i = p_connections.find(hostname);
  if (i == p_connections.end())
    throw runtime_error(string("Unexpected hostname (\"") + hostname + string("\") in ConnectionMgr::get_connection()."));

  return i->second;
  }

void ConnectionMgr::reset_connection(const std::string& hostname)
  {
  map<string,int>::iterator i = p_connections.find(hostname);
  if (i == p_connections.end())
    return; // no need to reset

  pbs_disconnect(i->second);
  p_connections.erase(i);
  }

}}
