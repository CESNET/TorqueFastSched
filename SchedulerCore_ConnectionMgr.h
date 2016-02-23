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
#ifndef CONNECTIONMGR_H_
#define CONNECTIONMGR_H_

#include <map>
#include <string>

namespace Scheduler {
namespace Core {

/** \brief Manager for network connections to Torque servers
 */
class ConnectionMgr
  {
  public:
  /** \brief Try to establish a connection, mark it as master
   *
   * @param hostname FQDN of the server
   * @return connection ID
   */
  int make_master_connection(const std::string& hostname);

  /** \brief Try to establish a connection
   *
   * @param hostname FQDN of the server
   * @return connection ID
   */
  int make_remote_connection(const std::string& hostname);

  /** \brief Get the master connection ID
   *
   * @return connection ID
   */
  int get_master_connection() const;

  /** \brief Get the connection ID corresponding to this hostname
   *
   * @param hostname FQDN of the server
   * @return connection ID
   */
  int get_connection(const std::string& hostname) const;

  /** \brief Disconnect the corresponding server
   *
   * @param hostname FQDN of the server
   */
  void disconnect(const std::string& hostname);
  
  /** \brief Reset a connection
   * 
   * Resets a connection (disconnects if needed)
   * 
   * @param hostname FQDN of the server
   */
  void reset_connection(const std::string& hostname);

  /** \brief Disconnect all servers */
  void disconnect_all() noexcept;

  private:
  std::map<std::string,int> p_connections;
  std::string p_master;

  };

}}

#endif /* CONNECTIONMGR_H_ */
