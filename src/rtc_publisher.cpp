//
// Created by 宋庭聿 on 2021/11/12.
//

#include "rtc_publisher.h"

void RtcPublisher::setUp()
{

}

void RtcPublisher::publish(uint8_t *buf, int size) {

}

shared_ptr<Client> RtcPublisher::createPeerConnection(const Configuration &config, weak_ptr<WebSocket> wws, string id)
{
    auto pc = make_shared<PeerConnection>(config);
    auto client = make_shared<Client>(pc);

    pc->onStateChange([this,id](PeerConnection::State state) {
        cout << "State: " << state << endl;
        if (state == PeerConnection::State::Disconnected ||
            state == PeerConnection::State::Failed ||
            state == PeerConnection::State::Closed) {
            // remove disconnected client
            MainThread.dispatch([this,id]() {
                clients.erase(id);
            });
        }
    });

    pc->onGatheringStateChange(
            [wpc = make_weak_ptr(pc), id, wws](PeerConnection::GatheringState state) {
                cout << "Gathering State: " << state << endl;
                if (state == PeerConnection::GatheringState::Complete) {
                    if(auto pc = wpc.lock()) {
                        auto description = pc->localDescription();
                        json message = {
                                {"id", id},
                                {"type", description->typeString()},
                                {"sdp", string(description.value())}
                        };
                        // Gathering complete, send answer
                        if (auto ws = wws.lock()) {
                            ws->send(message.dump());
                        }
                    }
                }
            });

    client->video = addVideo(pc, 102, 1, "video-stream", "stream1", [this,id, wc = make_weak_ptr(client)]() {
        MainThread.dispatch([this,wc]() {
            if (auto c = wc.lock()) {
                addToStream(c, true);
            }
        });
        cout << "Video from " << id << " opened" << endl;
    });

    client->audio = addAudio(pc, 111, 2, "audio-stream", "stream1", [this,id, wc = make_weak_ptr(client)]() {
        MainThread.dispatch([this,wc]() {
            if (auto c = wc.lock()) {
                addToStream(c, false);
            }
        });
        cout << "Audio from " << id << " opened" << endl;
    });

    auto dc = pc->createDataChannel("ping-pong");
    dc->onOpen([id, wdc = make_weak_ptr(dc)]() {
        if (auto dc = wdc.lock()) {
            dc->send("Ping");
        }
    });

    dc->onMessage(nullptr, [id, wdc = make_weak_ptr(dc)](string msg) {
        cout << "Message from " << id << " received: " << msg << endl;
        if (auto dc = wdc.lock()) {
            dc->send("Ping");
        }
    });
    client->dataChannel = dc;

    pc->setLocalDescription();
    return client;
};

shared_ptr<Stream> RtcPublisher::createStream(const string h264Samples, const unsigned int fps,
                                              const string opusSamples) {
    auto video = make_shared<H264FileParser>(h264Samples, fps, true);
    // audio source
    auto audio = make_shared<OPUSFileParser>(opusSamples, true);

    auto stream = make_shared<Stream>(video, audio);
    // set callback responsible for sample sending
    stream->onSample([this,ws = make_weak_ptr(stream)](Stream::StreamSourceType type, uint64_t sampleTime, rtc::binary sample) {
        vector<ClientTrack> tracks{};
        string streamType = type == Stream::StreamSourceType::Video ? "video" : "audio";
        // get track for given type
        function<optional<shared_ptr<ClientTrackData>> (shared_ptr<Client>)> getTrackData = [type](shared_ptr<Client> client) {
            return type == Stream::StreamSourceType::Video ? client->video : client->audio;
        };
        // get all clients with Ready state
        for(auto id_client: clients) {
            auto id = id_client.first;
            auto client = id_client.second;
            auto optTrackData = getTrackData(client);
            if (client->getState() == Client::State::Ready && optTrackData.has_value()) {
                auto trackData = optTrackData.value();
                tracks.push_back(ClientTrack(id, trackData));
            }
        }
        if (!tracks.empty()) {
            for (auto clientTrack: tracks) {
                auto client = clientTrack.id;
                auto trackData = clientTrack.trackData;
                // sample time is in us, we need to convert it to seconds
                auto elapsedSeconds = double(sampleTime) / (1000 * 1000);
                auto rtpConfig = trackData->sender->rtpConfig;
                // get elapsed time in clock rate
                uint32_t elapsedTimestamp = rtpConfig->secondsToTimestamp(elapsedSeconds);

                // set new timestamp
                rtpConfig->timestamp = rtpConfig->startTimestamp + elapsedTimestamp;

                // get elapsed time in clock rate from last RTCP sender report
                auto reportElapsedTimestamp = rtpConfig->timestamp - trackData->sender->previousReportedTimestamp;
                // check if last report was at least 1 second ago
                if (rtpConfig->timestampToSeconds(reportElapsedTimestamp) > 1) {
                    trackData->sender->setNeedsToReport();
                }
                cout << "Sending " << streamType << " sample with size: " << to_string(sample.size()) << " to " << client << endl;
                bool send = false;
                try {
                    // send sample
                    send = trackData->track->send(sample);
                } catch (...) {
                    send = false;
                }
                if (!send) {
                    cerr << "Unable to send "<< streamType << " packet" << endl;
                    break;
                }
            }
        }
        MainThread.dispatch([this,ws]() {
            if (clients.empty()) {
                // we have no clients, stop the stream
                if (auto stream = ws.lock()) {
                    stream->stop();
                }
            }
        });
    });
    return stream;
}

void RtcPublisher::addToStream(shared_ptr<Client> client, bool isAddingVideo) {
    if (client->getState() == Client::State::Waiting) {
        client->setState(isAddingVideo ? Client::State::WaitingForAudio : Client::State::WaitingForVideo);
    } else if ((client->getState() == Client::State::WaitingForAudio && !isAddingVideo)
               || (client->getState() == Client::State::WaitingForVideo && isAddingVideo)) {

        // Audio and video tracks are collected now
        assert(client->video.has_value() && client->audio.has_value());

        auto video = client->video.value();
        auto audio = client->audio.value();

        auto currentTime_us = double(currentTimeInMicroSeconds());
        auto currentTime_s = currentTime_us / (1000 * 1000);

        // set start time of stream
        video->sender->rtpConfig->setStartTime(currentTime_s, RtpPacketizationConfig::EpochStart::T1970);
        audio->sender->rtpConfig->setStartTime(currentTime_s, RtpPacketizationConfig::EpochStart::T1970);

        // start stat recording of RTCP SR
        video->sender->startRecording();
        audio->sender->startRecording();

        if (avStream.has_value()) {
            sendInitialNalus(avStream.value(), video);
        }

        client->setState(Client::State::Ready);
    }
    if (client->getState() == Client::State::Ready) {
        startStream();
    }
}

void RtcPublisher::startStream() {
    shared_ptr<Stream> stream;
    if (avStream.has_value()) {
        stream = avStream.value();
        if (stream->isRunning) {
            // stream is already running
            return;
        }
    } else {
        stream = createStream(h264SamplesDirectory, 30, opusSamplesDirectory);
        avStream = stream;
    }
    stream->start();
}
void RtcPublisher::wsOnMessage(json message) {
    auto it = message.find("id");
    if (it == message.end())
        return;
    string id = it->get<string>();
    it = message.find("type");
    if (it == message.end())
        return;
    string type = it->get<string>();

    if (type == "streamRequest") {
        shared_ptr<Client> c = createPeerConnection(config, make_weak_ptr(ws), id);
        clients.emplace(id, c);
    } else if (type == "answer") {
        shared_ptr<Client> c;
        if (auto jt = clients.find(id); jt != clients.end()) {
            auto pc = clients.at(id)->peerConnection;
            auto sdp = message["sdp"].get<string>();
            auto description = Description(sdp, type);
            pc->setRemoteDescription(description);
        }
    }
}
shared_ptr<ClientTrackData> RtcPublisher::addVideo(const shared_ptr<PeerConnection> pc, const uint8_t payloadType,
                                     const uint32_t ssrc, const string cname, const string msid,
                                     const function<void (void)> onOpen)
{
    auto video = Description::Video(cname);
    video.addH264Codec(payloadType);
    video.addSSRC(ssrc, cname, msid, cname);
    auto track = pc->addTrack(video);
    // create RTP configuration
    auto rtpConfig = make_shared<RtpPacketizationConfig>(ssrc, cname, payloadType, H264RtpPacketizer::defaultClockRate);
    // create packetizer
    auto packetizer = make_shared<H264RtpPacketizer>(H264RtpPacketizer::Separator::Length, rtpConfig);
    // create H264 handler
    auto h264Handler = make_shared<H264PacketizationHandler>(packetizer);
    // add RTCP SR handler
    auto srReporter = make_shared<RtcpSrReporter>(rtpConfig);
    h264Handler->addToChain(srReporter);
    // add RTCP NACK handler
    auto nackResponder = make_shared<RtcpNackResponder>();
    h264Handler->addToChain(nackResponder);
    // set handler
    track->setMediaHandler(h264Handler);
    track->onOpen(onOpen);
    auto trackData = make_shared<ClientTrackData>(track, srReporter);
    return trackData;
}
shared_ptr<ClientTrackData> RtcPublisher::addAudio(const shared_ptr<PeerConnection> pc, const uint8_t payloadType,
                                     const uint32_t ssrc, const string cname, const string msid,
                                     const function<void (void)> onOpen)
{
    auto audio = Description::Audio(cname);
    audio.addOpusCodec(payloadType);
    audio.addSSRC(ssrc, cname, msid, cname);
    auto track = pc->addTrack(audio);
    // create RTP configuration
    auto rtpConfig = make_shared<RtpPacketizationConfig>(ssrc, cname, payloadType, OpusRtpPacketizer::defaultClockRate);
    // create packetizer
    auto packetizer = make_shared<OpusRtpPacketizer>(rtpConfig);
    // create opus handler
    auto opusHandler = make_shared<OpusPacketizationHandler>(packetizer);
    // add RTCP SR handler
    auto srReporter = make_shared<RtcpSrReporter>(rtpConfig);
    opusHandler->addToChain(srReporter);
    // add RTCP NACK handler
    auto nackResponder = make_shared<RtcpNackResponder>();
    opusHandler->addToChain(nackResponder);
    // set handler
    track->setMediaHandler(opusHandler);
    track->onOpen(onOpen);
    auto trackData = make_shared<ClientTrackData>(track, srReporter);
    return trackData;
}

void RtcPublisher::sendInitialNalus(shared_ptr<Stream> stream, shared_ptr<ClientTrackData> video) {
    auto h264 = dynamic_cast<H264FileParser *>(stream->video.get());
    auto initialNalus = h264->initialNALUS();

    // send previous NALU key frame so users don't have to wait to see stream works
    if (!initialNalus.empty()) {
        const double frameDuration_s = double(h264->sampleDuration_us) / (1000 * 1000);
        const uint32_t frameTimestampDuration = video->sender->rtpConfig->secondsToTimestamp(frameDuration_s);
        video->sender->rtpConfig->timestamp = video->sender->rtpConfig->startTimestamp - frameTimestampDuration * 2;
        video->track->send(initialNalus);
        video->sender->rtpConfig->timestamp += frameTimestampDuration;
        // Send initial NAL units again to start stream in firefox browser
        video->track->send(initialNalus);
    }
}