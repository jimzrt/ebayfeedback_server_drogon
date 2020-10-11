#include <drogon/drogon.h>

using namespace std;
int main()
{
    //Set HTTP listener address and port
    //drogon::app().addListener("0.0.0.0", 80);
    //Load config file
    drogon::app().loadConfigFile("../config.json");
    //Run HTTP framework,the method will block in the internal event loop
    std::cout << "Starting ebayfeedback server" << std::endl;
    drogon::app().run();
    return 0;
}
