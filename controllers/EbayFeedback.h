#pragma once

#include <drogon/HttpController.h>
#include <lexbor/html/html.h>

using namespace drogon;

class EbayFeedback : public drogon::HttpController<EbayFeedback> {
private:

    std::unique_ptr<CacheMap<std::string, Json::Value>> cacheMapPtr;

    static void fail(const std::string& errorMessage, std::function<void(const HttpResponsePtr &)> &callback);

    void getUserFeedback(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback, const std::string& userName);

    static std::array<int, 4> getStarRatings(const lxb_html_document_t *document, lxb_dom_collection_t *collection);

    static std::array<int, 4> getRatingCounts(const lxb_html_document_t *document, lxb_dom_collection_t *collection);

    static std::array<int, 3> getSentimentCounts(const lxb_html_document *document, lxb_dom_collection_t *collection);

    static std::vector<std::string> getComments(const lxb_html_document *document, lxb_dom_collection_t *collection);

    static std::vector<std::string> getCommentTimes(const lxb_html_document *document, lxb_dom_collection_t *collection);
public:
    EbayFeedback();
    METHOD_LIST_BEGIN
        //use METHOD_ADD to add your custom processing function here;
        //METHOD_ADD(EbayFeedback::get,"/{2}/{1}",Get);//path is /EbayFeedback/{arg2}/{arg1}
        METHOD_ADD(EbayFeedback::getUserFeedback, "/?userName={1}", Get, "HeaderFilter"); //path is /EbayFeedback/{arg1}/{arg2}/list
        //ADD_METHOD_TO(EbayFeedback::your_method_name,"/absolute/path/{1}/{2}/list",Get);//path is /absolute/path/{arg1}/{arg2}/list
    METHOD_LIST_END

    // your declaration of processing function maybe like this:
    // void get(const HttpRequestPtr& req,std::function<void (const HttpResponsePtr &)> &&callback,int p1,std::string p2);


    static std::array<std::string, 4> getRatingDescriptions(const lxb_html_document_t *document, lxb_dom_collection_t *collection);
};
