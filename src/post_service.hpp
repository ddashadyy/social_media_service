#pragma once

#include <userver/clients/dns/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
 
#include <userver/components/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/utils/daemon_run.hpp>
 
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>


namespace post_service 
{
class PostHandler final : public userver::server::handlers::HttpHandlerBase 
{
public:
    static constexpr std::string_view kName = "handler-posts";

    PostHandler(const userver::components::ComponentConfig& config, const userver::components::ComponentContext& context);
    std::string HandleRequest(userver::server::http::HttpRequest& request, userver::server::request::RequestContext&) const override;

private:
    std::string GetValue(std::string_view key, const userver::server::http::HttpRequest& request) const;
    std::string PostValue(const userver::server::http::HttpRequest& request) const;
    std::string DeleteValue(std::string_view key) const;

    userver::storages::postgres::ClusterPtr pg_cluster_;
};

} // namespace post_service
