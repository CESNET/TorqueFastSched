#ifndef RESOURCESET_H_
#define RESOURCESET_H_

#include <map>
#include <string>
#include "base/Resource.h"
#include "boost/shared_ptr.hpp"

namespace Scheduler {
namespace Core {

class ResourceSet
  {
  public:
    boost::shared_ptr<Resource> get_resource(const std::string& name) const;
    boost::shared_ptr<Resource> get_alloc_resource(const std::string& name);

    ResourceSet();
    ResourceSet(const ResourceSet& src);

    ResourceSet& operator = (const ResourceSet& src);

    void join_resources(const ResourceSet& right);

  private:
    std::map<std::string, boost::shared_ptr<Resource> > p_resc;
  };

}}

#endif /* RESOURCESET_H_ */
