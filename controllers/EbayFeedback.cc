#include "EbayFeedback.h"
#include <drogon/HttpClient.h>

#include <cstring>
#include <cmath>
#include <chrono>
#include <algorithm>

using namespace std::chrono_literals;

static constexpr auto CACHE_TIME = 5min;

EbayFeedback::EbayFeedback() {
    cacheMapPtr = std::make_unique<CacheMap<std::string, Json::Value>>(drogon::app().getLoop());
}

void EbayFeedback::getUserFeedback(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback, const std::string &userName) {
    LOG_INFO << "Fetch " << userName;

    if (userName.empty()) {
        fail("Username cannot be empty!", callback);
        return;
    }

    if (req->getParameter("refreshCache") == "1") {
        LOG_INFO << "Force refresh cache for " << userName;
        cacheMapPtr->erase(userName);
    }


    if (cacheMapPtr->find(userName)) {
        LOG_INFO << "User " << userName << " is cached!";
        auto ret = (*(cacheMapPtr))[userName];
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        callback(resp);
        return;
    }

    auto client = HttpClient::newHttpClient("https://www.ebay.com");
    auto request2 = HttpRequest::newHttpRequest();
    request2->setPath("/usr/" + userName);
    auto response = client->sendRequest(request2);
    ReqResult r = response.first;
    HttpResponsePtr resp = response.second;
    if (r != ReqResult::Ok) {
        fail("Could not connect to ebay!", callback);
        return;
    }

    auto html = resp->getBody();

    /* Initialization */
    lxb_html_parser_t *parser = lxb_html_parser_create();
    lxb_html_parser_init(parser);
    /* Parse */
    lxb_html_document_t *document = lxb_html_parse(parser, reinterpret_cast<const lxb_char_t *>(html.data()), html.length());
    /* Destroy parser */
    lxb_html_parser_destroy(parser);

    auto htmlTitle = lxb_html_document_title_raw(document, nullptr);
    if(std::strstr((char *) htmlTitle, "error")){
        fail("User " + userName + " not found!", callback);
        return;
    }

    lxb_dom_collection_t *collection = lxb_dom_collection_make(&document->dom_document, 64);

    const std::array<int, 4> &starRatings = getStarRatings(document, collection);
    const std::array<int, 4> &ratingCounts = getRatingCounts(document, collection);
    const std::array<std::string, 4> &ratingDescriptions = getRatingDescriptions(document, collection);
    const std::array<int, 3> &sentimentCounts = getSentimentCounts(document, collection);
    const std::vector<std::string> &comments = getComments(document, collection);
    const std::vector<std::string> &commentTimes = getCommentTimes(document, collection);
    const std::vector<std::string> &commentSentiments = getCommentSentiment(document, collection);

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);

    Json::Value ret;
    ret["result"] = "ok";
    ret["userName"] = userName;

    Json::Value jstarRatings;
    for (auto i = 0; i < 4; i++) {
        Json::Value rating;
        rating["rating"] = starRatings.at(i);
        rating["count"] = ratingCounts.at(i);
        rating["type"] = ratingDescriptions.at(i);
        jstarRatings.append(rating);
    }
    ret["ratings"] = jstarRatings;

    Json::Value jsentimentsCount;
    jsentimentsCount["positive"] = sentimentCounts.at(0);
    jsentimentsCount["neutral"] = sentimentCounts.at(1);
    jsentimentsCount["negative"] = sentimentCounts.at(2);
    ret["sentiments"] = jsentimentsCount;

    Json::Value jcomments;
    if (comments.empty()) {
        jcomments = Json::objectValue;
    } else {
        for (auto i = 0; i < comments.size(); i++) {
            Json::Value comment;
            comment["text"] = comments.at(i);
            comment["date"] = commentTimes.at(i);
            comment["sentiment"] = commentSentiments.at(i);
            jcomments.append(comment);
        }
    }
    ret["comments"] = jcomments;

    cacheMapPtr->insert(userName, ret, std::chrono::duration_cast<std::chrono::seconds>(CACHE_TIME).count(), [userName]() { LOG_INFO << "auto delete cache for " << userName; });

    auto jresp = HttpResponse::newHttpJsonResponse(ret);
    callback(jresp);
}

void EbayFeedback::fail(const std::string &errorMessage, std::function<void(const HttpResponsePtr &)> &callback) {
    LOG_WARN << errorMessage;
    Json::Value ret;
    ret["result"] = "error";
    ret["message"] = errorMessage;
    callback(HttpResponse::newHttpJsonResponse(ret));
}

std::array<int, 4> EbayFeedback::getStarRatings(const lxb_html_document_t *document, lxb_dom_collection_t *collection) {
    std::array<int, 4> starRatings = {};
    std::string starClass = "rating_star";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *) starClass.c_str(), starClass.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i += 5) {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);
        element = element->first_child;

        const lxb_char_t *title = lxb_dom_element_get_attribute(lxb_dom_interface_element(element), (const lxb_char_t *) "title", 5, nullptr);
        auto titleStr = std::string((const char *) title);
        std::size_t pos = titleStr.find('/');
        std::string str3 = titleStr.substr(0, pos);
        float rating = std::stof(str3);
        int percent = (int) std::round((rating / 5) * 100);
        starRatings.at(i / 5) = percent;
    }
    lxb_dom_collection_clean(collection);
    return starRatings;
}

std::array<int, 4> EbayFeedback::getRatingCounts(const lxb_html_document_t *document, lxb_dom_collection_t *collection) {
    std::array<int, 4> ratingCounts = {};
    std::string ratingCountClass = "dsr_count";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *) ratingCountClass.c_str(), ratingCountClass.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++) {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);

        lxb_char_t *textContent = lxb_dom_node_text_content(element, nullptr);
        auto textStr = std::string((const char *) textContent);
        textStr.erase(std::remove(textStr.begin(), textStr.end(), ','), textStr.end());
        ratingCounts.at(i) = std::stoi(textStr);
    }
    lxb_dom_collection_clean(collection);
    return ratingCounts;
}

std::array<std::string, 4> EbayFeedback::getRatingDescriptions(const lxb_html_document_t *document, lxb_dom_collection_t *collection) {
    std::array<std::string, 4> ratingDescriptions = {};
    std::string ratingCountClass = "dsr_type";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *) ratingCountClass.c_str(), ratingCountClass.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++) {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);

        lxb_char_t *textContent = lxb_dom_node_text_content(element, nullptr);
        auto textStr = std::string((const char *) textContent);

        ratingDescriptions.at(i) = textStr;
    }
    lxb_dom_collection_clean(collection);
    return ratingDescriptions;
}

std::array<int, 3> EbayFeedback::getSentimentCounts(const lxb_html_document *document, lxb_dom_collection_t *collection) {
    std::array<int, 3> sentimentCounts = {};
    std::string ratingCountClass = "num";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *) ratingCountClass.c_str(), ratingCountClass.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++) {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);

        lxb_char_t *textContent = lxb_dom_node_text_content(element, nullptr);
        auto textStr = std::string((const char *) textContent);
        textStr.erase(std::remove(textStr.begin(), textStr.end(), ','), textStr.end());
        sentimentCounts.at(i) = std::stoi(textStr);
    }
    lxb_dom_collection_clean(collection);
    return sentimentCounts;
}

std::vector<std::string> EbayFeedback::getComments(const lxb_html_document *document, lxb_dom_collection_t *collection) {
    std::vector<std::string> comments;
    std::string commentClass = "fb_cmmt";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *) commentClass.c_str(), commentClass.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++) {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);

        lxb_char_t *textContent = lxb_dom_node_text_content(element, nullptr);
        auto textStr = std::string((const char *) textContent);

        comments.emplace_back(textStr);
    }
    lxb_dom_collection_clean(collection);
    return comments;
}

std::vector<std::string> EbayFeedback::getCommentTimes(const lxb_html_document *document, lxb_dom_collection_t *collection) {
    std::vector<std::string> commentTimes;
    std::string commentClass = "itm_ttl";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *) commentClass.c_str(), commentClass.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++) {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);
        element = element->first_child;

        lxb_char_t *textContent = lxb_dom_node_text_content(element, nullptr);
        auto textStr = std::string((const char *) textContent);

        commentTimes.emplace_back(textStr);
    }
    lxb_dom_collection_clean(collection);
    return commentTimes;
}

std::vector<std::string> EbayFeedback::getCommentSentiment(const lxb_html_document *document, lxb_dom_collection_t *collection) {
    std::vector<std::string> commentTimes;
    std::string commentClass = "each_cmmt";
    lxb_dom_elements_by_class_name(lxb_dom_interface_element(document->body), collection, (const lxb_char_t *) commentClass.c_str(), commentClass.length());
    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++) {
        lxb_dom_node_t *element = lxb_dom_collection_node(collection, i);
        element = element->first_child;

        const lxb_char_t *classNames = lxb_dom_element_get_attribute(lxb_dom_interface_element(element), (const lxb_char_t *) "class", 5, nullptr);
        auto textStr = std::string((const char *) classNames);
        std::size_t pos = textStr.find("fb_");
        std::string str3 = textStr.substr(pos+3);
        commentTimes.emplace_back(str3);
    }
    lxb_dom_collection_clean(collection);
    return commentTimes;
}




