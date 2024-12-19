#include <userver/clients/dns/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
 
#include <userver/components/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/utils/daemon_run.hpp>
 
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>

#include <string>
#include <bits/basic_string.h>

#include "post_service.hpp"


int main(int argc, char* argv[]) 
{
    auto component_list = userver::components::MinimalServerComponentList()
                              
                              .Append<post_service::PostHandler>()
                              .Append<userver::components::Postgres>("database")
                              .Append<userver::components::TestsuiteSupport>()
                              .Append<userver::clients::dns::Component>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
