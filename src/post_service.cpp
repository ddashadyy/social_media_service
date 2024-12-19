#include "post_service.hpp"

#include <iomanip>
#include <userver/utils/datetime/date.hpp>

namespace post_service 
{

PostHandler::PostHandler(const userver::components::ComponentConfig& config, const userver::components::ComponentContext& context) :
    HttpHandlerBase(config, context),
      pg_cluster_(context.FindComponent<userver::components::Postgres>("database").GetCluster()) {}

std::string PostHandler::HandleRequest(
    userver::server::http::HttpRequest& request, 
    userver::server::request::RequestContext&) const 
{
    const auto& key = request.GetArg("key");
    
    request.GetHttpResponse().SetContentType(userver::http::content_type::kTextPlain);

    switch (request.GetMethod()) 
    {
        case userver::server::http::HttpMethod::kGet:
            return GetValue(key, request);
        case userver::server::http::HttpMethod::kPost:
            return PostValue(request);
        case userver::server::http::HttpMethod::kDelete:
            return DeleteValue(key);
        default:
            throw userver::server::handlers::ClientError(
                userver::server::handlers::ExternalBody{
                    fmt::format("Unsupported method {}", request.GetMethod())
                }
            );
    }
}

const userver::storages::postgres::Query kSelectValues{
    "SELECT id, author_id, content, created_at, updated_at FROM posts WHERE id = $1",
};


std::string PostHandler::GetValue(
    std::string_view key, 
    const userver::server::http::HttpRequest& request) const
{
    try 
    {
        auto result = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            kSelectValues,
            key
        );

        if (result.IsEmpty()) 
        {
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            return "{\"error\":\"Post not found\"}";
        }

        auto row = result.AsSingleRow<std::tuple<std::string, std::string, std::string, userver::storages::postgres::TimePointWithoutTz, userver::storages::postgres::TimePointWithoutTz>>(userver::storages::postgres::kRowTag);

        userver::formats::json::ValueBuilder response_json;
        response_json["id"] = std::get<0>(row);
        response_json["author_id"] = std::get<1>(row);
        response_json["content"] = std::get<2>(row);
        response_json["created_at"] = std::get<3>(row);
        response_json["updated_at"] = std::get<4>(row);

return userver::formats::json::ToString(response_json.ExtractValue());
    } 
    catch (const std::exception& ex) 
    {
        LOG_ERROR() << "Database query failed: " << ex.what();
        throw;  
    }

}

const userver::storages::postgres::Query kInsertValues{
    "INSERT INTO posts (id, author_id, content, created_at, updated_at) "
    "VALUES (gen_random_uuid(), $1, $2, NOW(), NOW()) "
    "ON CONFLICT DO NOTHING",
};


std::string PostHandler::PostValue( 
    const userver::server::http::HttpRequest& request) const
{
    const auto& kAuthorId = request.GetArg("author_id");
    const auto& kContent = request.GetArg("content");

    userver::storages::postgres::Transaction transaction =
        pg_cluster_->Begin(
            "insert_transaction", 
            userver::storages::postgres::ClusterHostType::kMaster, {});

    auto res = transaction.Execute(kInsertValues, kAuthorId, kContent);
    
    if (res.RowsAffected() > 0) 
    {
        transaction.Commit();
        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
        return std::string{kAuthorId};
    }
    else
    {
        transaction.Rollback();
        request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
        return "{\"error\":\"Failed to insert post\"}";
    }
}


std::string PostHandler::DeleteValue(std::string_view key) const 
{
    auto res = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster, 
        "DELETE FROM posts WHERE id=$1", 
        key
    );

    return std::to_string(res.RowsAffected());
}

} // namespace post_service