/**
 *
 *  HeaderFilter.h
 *
 */

#pragma once

#include <drogon/HttpFilter.h>
using namespace drogon;


class HeaderFilter : public HttpFilter<HeaderFilter>
{
  public:
    HeaderFilter() {}
    virtual void doFilter(const HttpRequestPtr &req,
                          FilterCallback &&fcb,
                          FilterChainCallback &&fccb) override;
};

