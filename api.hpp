#ifndef API_HPP_
#define API_HPP_

/* api.h modified for usage in C++ code */

extern "C" {
#include "pbs_cache_api.h"
}

inline char *xpbs_cache_get_local(const char *hostname, const char *name)
{ return pbs_cache_get_local(const_cast<char*>(hostname), const_cast<char*>(name)); }

inline int xcache_hash_fill_local(const char *metric, void *hash)
{ return cache_hash_fill_local(const_cast<char*>(metric), hash); }

inline char *xcache_hash_find(void *hash,const char *key)
{ return cache_hash_find(hash,const_cast<char*>(key)); }

inline int xcache_store_local(const char *hostname, const char *name, const char *value)
{ return cache_store_local(const_cast<char*>(hostname), const_cast<char*>(name), const_cast<char*>(value)); }

#endif /* API_HPP_ */
