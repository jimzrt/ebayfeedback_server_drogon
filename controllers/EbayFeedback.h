#pragma once
#include <drogon/HttpController.h>
using namespace drogon;
class EbayFeedback : public drogon::HttpController<EbayFeedback>
{
public:
  METHOD_LIST_BEGIN
  //use METHOD_ADD to add your custom processing function here;
  //METHOD_ADD(EbayFeedback::get,"/{2}/{1}",Get);//path is /EbayFeedback/{arg2}/{arg1}
  METHOD_ADD(EbayFeedback::get_user_feedback, "/?userName={1}", Get); //path is /EbayFeedback/{arg1}/{arg2}/list
  //ADD_METHOD_TO(EbayFeedback::your_method_name,"/absolute/path/{1}/{2}/list",Get);//path is /absolute/path/{arg1}/{arg2}/list

  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  // void get(const HttpRequestPtr& req,std::function<void (const HttpResponsePtr &)> &&callback,int p1,std::string p2);
  void get_user_feedback(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback, std::string userName) const;

};
