
#include <iostream>

#include "WebSocketServer.h"

int main(int argc,char** argv)
{
    WebSocketServer server("0.0.0.0",12345);
    if(server.execute())
    {
        printf("serving...\n");

        std::thread([&](){
            int i = 0;
            while(true)
            {
                i++;
                std::string data = "brocast test ... ";
                data += std::to_string(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                server.brocast(data.data(),data.size());
            }
        }).detach();
    }

    getchar();
    return 0;
}
