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

namespace user_service 
{

class FetchUsersHandler final : public userver::server::handlers::HttpHandlerBase 
{
public:
    static constexpr std::string_view kName = "handler-fetch-users";

    FetchUsersHandler(const userver::components::ComponentConfig& config,
                      const userver::components::ComponentContext& context)
        : HttpHandlerBase(config, context),
          pg_cluster_(context
                          .FindComponent<userver::components::Postgres>("database")
                          .GetCluster()) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest&,
        userver::server::request::RequestContext&) const override 
    {
        const auto result = pg_cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                                                "SELECT username FROM users WHERE id=$1",
                                                userver::storages::postgres::Query::Name{"cm4u0jkg70000udzgu7r5ow36"} );

        if (result.IsEmpty()) return "empty";

        return result.AsSingleRow<std::string>();
    }

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  

int main(int argc, char* argv[]) 
{
    auto component_list = userver::components::MinimalServerComponentList()
                              .Append<user_service::FetchUsersHandler>()
                              .Append<userver::components::Postgres>("database")
                              .Append<userver::components::TestsuiteSupport>()
                              .Append<userver::clients::dns::Component>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
