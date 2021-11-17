//
// Created by 宋庭聿 on 2021/11/12.
//

#ifndef RTC_PUBLISHER_H
#define RTC_PUBLISHER_H

#include "nlohmann/json.hpp"
#include "common.h"
#include "h264fileparser.hpp"
#include "helpers.hpp"

using namespace rtc;
using namespace std;
using namespace std::chrono_literals;

using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

class RtcPublisher {
public:
    RtcPublisher(){
        ws = make_shared<WebSocket>();
        connection_setted = false;
        rtcThread = new DispatchQueue("RtcThread") ;
    }
    void setUp();
    void publish(uint8_t *buf, int size);
    bool connection_setted;
protected:
    Configuration rtc_config;
    shared_ptr<WebSocket> ws;
    std::shared_ptr<H264FileParser> video;
    optional<shared_ptr<Stream>> avStream;
    DispatchQueue* rtcThread;

    unordered_map<string, shared_ptr<Client>> clients{};

    shared_ptr<Client> createPeerConnection(const Configuration &config,
                                            weak_ptr<WebSocket> wws,
                                            string id);

    shared_ptr<Stream> createStream(const unsigned int fps);

    void addToStream(shared_ptr<Client> client, bool isAddingVideo);

    void startStream();

    void wsOnMessage(json message,Configuration config, shared_ptr<WebSocket> ws);

    shared_ptr<ClientTrackData> addVideo(const shared_ptr<PeerConnection> pc, const uint8_t payloadType,
                                         const uint32_t ssrc, const string cname, const string msid,
                                         const function<void(void)> onOpen);


    void sendInitialNalus(shared_ptr<Stream> stream, shared_ptr<ClientTrackData> video);
};

#endif //RTC_PUBLISHER_H
