
#include "WebSocketServer.h"

WebSocketServer::WebSocketServer(const std::string& ip,const int& port)
            : ip(ip),
            port(port),
            inServe(false)
{
}

WebSocketServer::~WebSocketServer()
{
    quit();
}

bool WebSocketServer::execute()
{
    if(!server.init(ip.c_str(),port))
    {
        printf("[WebSocketServer::execute()]: websocket server init failed with %s\n",
                server.getLastError());
        return false;
    }

    inServe.store(true);
    std::thread([this](){
        while(inServe.load(std::memory_order_relaxed)){
            server.poll(this);
            // std::this_thread::yield();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        printf("[WebSocketServer::execute()]: server quit success with serve thread joined.\n");
    }).detach();

    printf("[WebSocketServer::execute()]: websocket init success with ws://%s:%d \n",
            ip.c_str(),
            port);
    return true;
}

void WebSocketServer::quit()
{
    inServe.store(false);
}

void WebSocketServer::send(Client& client,const char* data,const int& length)
{
    client.send(websocket::OPCODE_TEXT,(const uint8_t*)data,length);
}

void WebSocketServer::brocast(const char* data,const int& length)
{
    for(int i=0;i<clients.size();i++)
    {
        clients[i]->send(websocket::OPCODE_TEXT,(const uint8_t*)data,length);
    }
}

bool WebSocketServer::onWSConnect(Client &client, 
                                    const char *requestUrl, 
                                    const char *host, 
                                    const char *origin, 
                                    const char *protocol,
                                    const char *extensions, 
                                    char *responseProtocol, 
                                    uint32_t responseProtocolSize, 
                                    char *responseExtensions,
                                    uint32_t responseExtensionsSize)
{
    printf("[WebSocketServer::onWSConnect(...)]: new websokcet client request.\n");
    
    struct sockaddr_in address;
    client.getPeername(address);

    printf("[WebSocketServer::onWSConnect(...)]: client from %s:%d with\n\t"
            "host: %s\n\t"
            "origin: %s\n\t"
            "protocol: %s\n\t"
            "extensions: %s.\n",
            inet_ntoa(address.sin_addr),
            ntohs(address.sin_port),
            host,
            origin,
            protocol,
            extensions);

    clients.push_back(&client);

    return true;
}

void WebSocketServer::onWSClose(Client &client, 
                                uint16_t statusCode, 
                                const char *reason)
{
    printf("[WebSocketServer::onWSClose(...)]: a websocket client had closed.\n");
    
    for(ClientVectorIterator i=clients.begin();i!=clients.end();i++)
    {
        if(!(*i)->isConnected())
        {
            i = clients.erase(i);
            if(i == clients.end())
            {
                break;
            }
        }
    }
}

void WebSocketServer::onWSMsg(Client &client, 
                                uint8_t opcode, 
                                const uint8_t *payload, 
                                uint32_t payloadLength)
{
    std::string data((const char*)payload,payloadLength);
    printf("[WebSocketServer::onWSMsg(...)]: receive message\n\t"
            "payload: %s\n\t"
            "length: %lu\n",
            data.c_str(),
            data.size());
    send(client,data.data(),data.size());
}

void WebSocketServer::onWSSegment(Client &client, 
                                    uint8_t opcode, 
                                    const uint8_t *payload, 
                                    uint32_t payloadLength, 
                                    uint32_t payloadStartIndex,
                                    bool final)
{

}