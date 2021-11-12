//
// Created by 宋庭聿 on 2021/11/12.
//

#ifndef RTC_PUBLISHER_H
#define RTC_PUBLISHER_H

#include "nlohmann/json.hpp"

#include "h264fileparser.hpp"
#include "opusfileparser.hpp"
#include "helpers.hpp"

using namespace rtc;
using namespace std;
using namespace std::chrono_literals;

using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

class RtcPublisher {
public:
    //TODO: current buf is raw, no pps and sps data
    RtcPublisher() {

    }

    void setUp();

    void publish(uint8_t *buf, int size);

protected:
    optional<shared_ptr<Stream>> avStream;
    DispatchQueue MainThread = DispatchQueue("Main");
    Configuration config;
    shared_ptr<WebSocket> ws;
    unordered_map<string, shared_ptr<Client>> clients{};

    shared_ptr<Client> createPeerConnection(const Configuration &config,
                                            weak_ptr<WebSocket> wws,
                                            string id);

    shared_ptr<Stream> createStream(const string h264Samples, const unsigned fps, const string opusSamples);

    void addToStream(shared_ptr<Client> client, bool isAddingVideo);

    void startStream();

    void wsOnMessage(json message);

    shared_ptr<ClientTrackData> addVideo(const shared_ptr<PeerConnection> pc, const uint8_t payloadType,
                                         const uint32_t ssrc, const string cname, const string msid,
                                         const function<void(void)> onOpen);

    shared_ptr<ClientTrackData> addAudio(const shared_ptr<PeerConnection> pc, const uint8_t payloadType,
                                         const uint32_t ssrc, const string cname, const string msid,
                                         const function<void(void)> onOpen);

    void sendInitialNalus(shared_ptr<Stream> stream, shared_ptr<ClientTrackData> video);
};

#endif //RTC_PUBLISHER_H
