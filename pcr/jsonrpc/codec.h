#pragma once

#include <string>
#include "pcr/jsonrpc/message.h"

namespace pcr::jsonrpc {

//
// decode_message:
//   one complete framed payload -> JSON-RPC message
//
// encode_message:
//   JSON-RPC message -> one complete payload
//
Message decode(std::string &&payload);
std::string encode(const Message &msg);

} // namespace pcr::jsonrpc
