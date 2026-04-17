#include "pcr/jsonrpc/peer.h"
#include "pcr/jsonrpc/clock.h"

#include "pcr/jsonrpc/codec.h"

#include <utility>

namespace pcr::jsonrpc {


Peer::Peer(pcr::framing::AnyFramer framer, MetricsSink *metrics)
    : framer_(std::move(framer)),
      metrics_(metrics) {}


std::optional<Message> Peer::read() 
{
    auto frame = framer_.read_frame();
    if (!frame.has_value()) {
        return std::nullopt;
    }

    metric_counter(metrics_, Metric::FramesIn, 1);
    metric_counter(metrics_, Metric::BytesIn, frame->size());

    const std::uint64_t t0 = now_ns();
    Message msg = decode(std::move(*frame));
    const std::uint64_t t1 = now_ns();

    metric_timing(metrics_, Metric::DecodeNs, t1 - t0);
    return msg;
}


void Peer::write(const Message &msg) 
{
    const std::uint64_t t0 = now_ns();
    std::string payload = encode(msg);
    const std::uint64_t t1 = now_ns();

    metric_timing(metrics_, Metric::EncodeNs, t1 - t0);

    framer_.write_frame(payload);

    metric_counter(metrics_, Metric::FramesOut, 1);
    metric_counter(metrics_, Metric::BytesOut, payload.size());
}

} // namespace pcr::jsonrpc
