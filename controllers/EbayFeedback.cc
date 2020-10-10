#include "EbayFeedback.h"
#include <drogon/HttpClient.h>
#include <drogon/orm/DbClient.h>
#include <drogon/orm/Mapper.h>
#include <drogon/orm/Criteria.h>
#include "models/EbayResponse.h"

#include "base.h"
#include <lexbor/dom/dom.h>

#include <string>
#include <sstream>
#include <vector>

#include <chrono>
#include <cmath>

void EbayFeedback::get_user_feedback(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback, std::string userName) const
{
    LOG_DEBUG << "User " << userName << " get his information";

    auto dbClient = drogon::app().getDbClient();
    drogon::orm::Mapper<drogon_model::sqlite3::EbayResponse> mapper(dbClient);

    auto criteriaUsername = Criteria(drogon_model::sqlite3::EbayResponse::Cols::_USERNAME, CompareOperator::EQ, userName);

    bool found = mapper.count(criteriaUsername) == 1;
    if (found)
    {
        LOG_DEBUG << "User " << userName << " is cached!";
        auto cachedResponse = mapper.findOne(criteriaUsername);

        auto timeStamp = *cachedResponse.getTimestamp();
        auto chronoTime = std::chrono::seconds(timeStamp);
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
        auto hours = std::chrono::duration_cast<std::chrono::seconds>(now - chronoTime).count();

        LOG_DEBUG << "Age: " << hours << " hours";

        if (hours < 20)
        {
            auto body = *cachedResponse.getBody();

            const auto rawJsonLength = static_cast<int>(body.length());
            JSONCPP_STRING err;
            Json::Value root;
            Json::CharReaderBuilder builder;
            const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            if (!reader->parse(body.c_str(), body.c_str() + rawJsonLength, &root, &err))
            {
                std::cout << "error!" << std::endl;
                std::cout << err << std::endl;
            }

            auto resp = HttpResponse::newHttpJsonResponse(root);
            callback(resp);
            return;
        }
        else
        {
            LOG_DEBUG << "Refresh cache!";
        }
    }

    auto client = HttpClient::newHttpClient("https://www.ebay.com");
    auto request2 = HttpRequest::newHttpRequest();
    request2->setPath("/usr/" + userName);
    auto response = client->sendRequest(request2);
    ReqResult r = response.first;
    HttpResponsePtr resp = response.second;
    if (r != ReqResult::Ok)
    {
        Json::Value ret;
        ret["result"] = "error";
        ret["message"] = "Could not connect to ebay!";
        callback(HttpResponse::newHttpJsonResponse(ret));
        return;
    }

    auto html = std::string(resp->getBody());
    //lxb_status_t status;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    std::array<int, 4> star_ratings = {};
    std::array<int, 4> ratings_count = {};
    std::array<int, 3> sentiments_count = {};
    std::vector<std::string> comments;
    comments.reserve(5);
    std::vector<std::string> comment_times;
    comment_times.reserve(5);

    document = parse(reinterpret_cast<const unsigned char *>(html.c_str()), html.length());

    collection = lxb_dom_collection_make(&document->dom_document, 128);

    std::string star_class = "rating_star";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *)star_class.c_str(), star_class.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i += 5)
    {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);
        element = element->first_child;
        //size_t title_length;
        const lxb_char_t *title = lxb_dom_element_get_attribute(lxb_dom_interface_element(element), (const lxb_char_t *)"title", 5, nullptr);
        auto title_str = std::string((const char *)title);
        std::size_t pos = title_str.find('/');

        std::string str3 = title_str.substr(0, pos);
        float rating = std::stof(str3);
        int percent = std::round((rating/5)*100);
        star_ratings.at(i / 5) = percent;
        std::cout << std::stof(str3) << std::endl;
    }

    lxb_dom_collection_clean(collection);

    std::string rating_count_class = "dsr_count";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *)rating_count_class.c_str(), rating_count_class.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++)
    {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);

        lxb_char_t *text_content = lxb_dom_node_text_content(element, nullptr);
        auto text_str = std::string((const char *)text_content);

        ratings_count.at(i) = std::stoi(text_str);
        std::cout << std::stoi(text_str) << std::endl;
    }

    lxb_dom_collection_clean(collection);

    rating_count_class = "num";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *)rating_count_class.c_str(), rating_count_class.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++)
    {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);

        lxb_char_t *text_content = lxb_dom_node_text_content(element, nullptr);
        auto text_str = std::string((const char *)text_content);

        sentiments_count.at(i) = std::stoi(text_str);
        std::cout << std::stoi(text_str) << std::endl;
    }

    lxb_dom_collection_clean(collection);

    std::string comment_class = "fb_cmmt";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *)comment_class.c_str(), comment_class.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++)
    {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);

        lxb_char_t *text_content = lxb_dom_node_text_content(element, nullptr);
        auto text_str = std::string((const char *)text_content);

        comments.emplace_back(text_str);
        std::cout << text_str << std::endl;
    }

    lxb_dom_collection_clean(collection);

    comment_class = "itm_ttl";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *)comment_class.c_str(), comment_class.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++)
    {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);
        element = element->first_child;

        lxb_char_t *text_content = lxb_dom_node_text_content(element, nullptr);
        auto text_str = std::string((const char *)text_content);

        comment_times.emplace_back(text_str);
        std::cout << text_str << std::endl;
    }

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);

    Json::Value ret;
    ret["result"] = "ok";
    ret["user_name"] = userName;

    Json::Value jstar_ratings;
    for (auto &rating : star_ratings)
    {
        jstar_ratings.append(rating);
    }
    ret["star_ratings"] = jstar_ratings;

    Json::Value jratings_count;
    for (auto &rating_count : ratings_count)
    {
        jratings_count.append(rating_count);
    }
    ret["ratings_count"] = jratings_count;

    Json::Value jsentiments_count;
    for (auto &sentiment_count : sentiments_count)
    {
        jsentiments_count.append(sentiment_count);
    }
    ret["sentiments_count"] = jsentiments_count;

    Json::Value jcomments;
    if (comments.empty())
    {
        jcomments = Json::objectValue;
    }
    else
    {
        for (int i = 0; i < comments.size(); i++)
        {
            Json::Value comment;
            comment["text"] = comments.at(i);
            comment["date"] = comment_times.at(i);
            jcomments.append(comment);
        }
    }
    ret["comments"] = jcomments;

    auto time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    Json::StreamWriterBuilder builder;
    const std::string jsonBody = Json::writeString(builder, ret);
    if (!found)
    {

        drogon_model::sqlite3::EbayResponse ebayResponse;
        ebayResponse.setUsername(userName);
        ebayResponse.setBody(jsonBody);
        ebayResponse.setTimestamp(time);
        mapper.insert(ebayResponse);
    }
    else
    {
        auto cachedResponse = mapper.findOne(criteriaUsername);
        cachedResponse.setBody(jsonBody);
        cachedResponse.setTimestamp(time);
        mapper.update(cachedResponse);
    }
    auto jresp = HttpResponse::newHttpJsonResponse(ret);
    callback(jresp);
}
