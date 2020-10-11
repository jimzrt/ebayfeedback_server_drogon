/**
 *
 *  HeaderFilter.cc
 *
 */

#include "HeaderFilter.h"

using namespace drogon;

void HeaderFilter::doFilter(const HttpRequestPtr &req,
                            FilterCallback &&fcb,
                            FilterChainCallback &&fccb)
{
    const auto &headers = req->getHeaders();
    const auto &params = req->getParameters();
    if(headers.find("presta") != headers.end() || params.find("debug") != params.end())
    {
        //Passed
        fccb();
        return;
    }
    //Check failed
    auto res = drogon::HttpResponse::newHttpResponse();
    res->setStatusCode(k500InternalServerError);
    fcb(res);
}
