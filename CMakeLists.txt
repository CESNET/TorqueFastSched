cmake_minimum_required(VERSION 3.0)
project(torque-fast-sched VERSION 1.0.0)

find_file(HAVE_PBS_CACHE_API_H pbs_cache_api.h /usr/include)
if(NOT HAVE_PBS_CACHE_API_H)
	message(FATAL_ERROR "Can't find pbs cache header file: pbs_cache_api.h")
endif()

set(BOOST_USE_STATIC_LIBS OFF)
set(BOOST_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED)
#include_directories( ${Boost_INCLUDE_DIRS} )

include_directories(legacy)
include_directories(.)

set(CMAKE_CXX_COMPILER $ENV{CXX})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -D_POSIX_C_SOURCE=200809L -ggdb3 -Wall -pedantic -pedantic-errors -Wextra -Wwrite-strings")

# -Waggregate-return -Wcast-align -Wcast-qual  -Wchar-subscripts  -Wcomment -Wconversion -Wdisabled-optimization -Werror -Wfloat-equal  -Wformat  -Wformat=2 -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wimport  -Winit-self  -Winline -Winvalid-pch -Wunsafe-loop-optimizations -Wmissing-braces -Wmissing-field-initializers -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wparentheses  -Wpointer-arith -Wreturn-type -Wsequence-point  -Wshadow -Wsign-compare  -Wstack-protector -Wstrict-aliasing -Wstrict-aliasing=2 -Wswitch  -Wswitch-default -Wswitch-enum -Wtrigraphs  -Wuninitialized -Wunknown-pragmas  -Wunreachable-code -Wunused -Wunused-function  -Wunused-label  -Wunused-parameter -Wunused-value  -Wunused-variable  -Wvariadic-macros -Wvolatile-register-var")

#-Wpadded -Weffc++ -Wredundant-decls

set(SOURCE_FILES pbs_sched.cpp server_info.cpp ServerInfo.cpp check.cpp dedtime.cpp fairshare.cpp fifo.cpp globals.cpp job_info.cpp JobInfo.cpp JobLog.cpp misc.cpp node_info.cpp NodeInfo.cpp NodeSort.cpp nodespec.cpp parse.cpp prev_job_info.cpp prime.cpp queue_info.cpp RescInfoDb.cpp SchedulerRealm.cpp sort.cpp state_count.cpp utility.cpp logic/FairshareTree.cpp logic/JobAssign.cpp logic/NodeFilters.cpp logic/NodeLogic.cpp base/Resource.cpp base/ResourceSet.cpp base/PropRegistry.cpp base/NodeState.cpp base/NodeData.cpp base/MiscHelpers.cpp base/DynamicResource.cpp base/ConnectionMgr.cpp site_pbs_cache.cpp)
add_executable(torque-fast-sched ${SOURCE_FILES} legacy/site_pbs_cache_common.c)

TARGET_LINK_LIBRARIES(torque-fast-sched torque pbscache)

install(TARGETS torque-fast-sched DESTINATION sbin)
install(DIRECTORY dist/sched_priv DESTINATION /var/spool/torque)
install(DIRECTORY DESTINATION /var/spool/torque/sched_logs)


if (IS_DIRECTORY /usr/lib/systemd)
	install(FILES dist/torque-fast-sched.service DESTINATION /usr/lib/systemd/system)
else()
	install(FILES dist/torque-fast-sched DESTINATION /etc/init.d)
endif()

if (IS_DIRECTORY /etc/cron.daily)
	install(FILES dist/torque-fast-sched.cron DESTINATION /etc/cron.daily)
	install(FILES dist/torque-fast-sched-update-fairshare.cron DESTINATION /etc/cron.daily)
else()
	message(FATAL_ERROR "Can't find cron daily directory.")
endif()


##############################################################################
# PACKAGE GENERATION SECTION                                                 #
##############################################################################

# cmake -DPKG_VERSION=deb7 can be used to add specific version postfix
option(PKG_VERSION "Postfix for the package version.")

if (PKG_VERSION)
	SET(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}+${PKG_VERSION}")
else()
	SET(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
endif()

SET(CPACK_PACKAGE_NAME "torque-meta-fastsched")
SET(CPACK_PACKAGE_CONTACT "Simon Toth (simon@cesnet.cz)")

# DEBIAN PACKAGE GENERATION
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "torque-meta-common, libpbs-cache, pbs-cache-server")
SET(CPACK_DEBIAN_PACKAGE_CONFLICTS "torque-scheduler, torque-meta-scheduler")
SET(DISTFILEDIR "${CMAKE_CURRENT_SOURCE_DIR}/dist")
SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${DISTFILEDIR}/postinst;${DISTFILEDIR}/conffiles;${DISTFILEDIR}/changelog")
INCLUDE(CPack)
