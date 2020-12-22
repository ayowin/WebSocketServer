
#ifndef __WEBSOCKETSERVER_H__
#define __WEBSOCKETSERVER_H__

#include "websocket.h"
#include <iostream>
#include <vector>
#include <atomic>
#include <thread>

/**
 * @brief WebSocketServer
 *      class for websocket server
 *      two ways to use this class:
 *      1. (deprecated) you can modify the 
 *          onWSConnect,onWSClose,onWSMsg,onWSSegment 
 *          methods in WebSocketServer.cpp with your own business.
 *      2. you can inherit from this class and rewrite
 *          onWSConnect,onWSClose,onWSMsg,onWSSegment
 *          methods in your own class with your own business. 
 * 
 * @author ouyangwei
 * @date 2019-6-14
 * @version 1.0.0
 */ 
class WebSocketServer
{
public:
    using Server = websocket::WSServer<WebSocketServer, /* EventHandler */
                                        char, /* ConnUserData */
                                        false, /* RecvSegment */
                                        4096, /* RecvBufSize */
                                        20>; /* MaxConns */
    using Client = Server::Connection;
    using ClientVector = std::vector<Client*>;
    using ClientVectorIterator = std::vector<Client*>::iterator;

    WebSocketServer(const std::string& ip,const int& port);
    ~WebSocketServer();

    bool execute();

    void quit();

    void send(Client& client,const char* data,const int& length);

    void brocast(const char* data,const int& length);

    /**
     * @brief new websocket client connect callback
     *        optional: origin, protocol, extensions will be nullptr if not exist in the request headers
     *        optional: fill responseProtocol[responseProtocolSize] to add protocol to response headers
     *        optional: fill responseExtensions[responseExtensionsSize] to add extensions to response headers
     * @return true: accept the client
     * @return false: refuse the client
     */
    virtual bool onWSConnect(Client &client, 
                        const char *requestUrl, 
                        const char *host, 
                        const char *origin, 
                        const char *protocol,
                        const char *extensions, 
                        char *responseProtocol, 
                        uint32_t responseProtocolSize, 
                        char *responseExtensions,
                        uint32_t responseExtensionsSize);

    /**
     * @brief a websocket client close callback
     * @param client: client
     * @param statusCode: status code , 
     *        1005(no status code in the close msg) ,
     *        1006(not a clean close(tcp connection closed without a close msg)) .
     * @param reason: reason in the close header
     */
    virtual void onWSClose(Client &client, 
                    uint16_t statusCode, 
                    const char *reason);

    /**
     * @brief message receive callback with RecvSegment == false
     * @param client: client
     * @param opcode: opcode , 
     *        0: continuation frame ,
     *        1: text frame ,
     *        2: binary frame , 
     *        8: connection close frame ,  
     *        9: ping frame ,
     *        10: pong frame .
     * @param payload: payload
     * @param payloadLength: payload length
     */
    virtual void onWSMsg(Client &client, 
                    uint8_t opcode, 
                    const uint8_t *payload, 
                    uint32_t payloadLength);

    /**
     * @brief: segment receive callback with RecvSegment == true
     * @param client: client
     * @param opcode: opcode , 
     *        0: continuation frame ,
     *        1: text frame ,
     *        2: binary frame , 
     *        8: connection close frame ,  
     *        9: ping frame ,
     *        10: pong frame .
     * @param payload: payload
     * @param payloadLength: payload length
     * @param payloadStartIndex: payload start index
     * @param final: final or not , 
     *        true: is final segment , 
     *        false: is not final segment .
     */
    void onWSSegment(Client &client, 
                        uint8_t opcode, 
                        const uint8_t *payload, 
                        uint32_t payloadLength, 
                        uint32_t payloadStartIndex,
                        bool final);

private:
    std::string ip;
    int port;
    Server server;
    ClientVector clients;
    std::atomic_bool inServe;
};

#endif