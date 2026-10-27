# libpcr

**Process · Channel · RPC**

`libpcr` is a library for process management and structured communication. It virtualizes and separates process lifecycles, byte-stream channels, message framing, and RPC handling so these pieces can be used independently or composed together.

The goal is to keep application code independent of the particular transport and protocol used to exchange messages.

## Components

- **`proc`** — child-process creation and lifecycle management.
- **`stream`** — byte-stream I/O and file-descriptor ownership.
- **`framing`** — message boundaries over a stream.
- **`jsonrpc`** — requests, responses, notifications, and dispatch.
- **`ipc`** — ready-to-use combinations of the lower layers.


## Build

Unix:

```sh
./build.sh
```

Windows:

```bat
build.bat
```

## Use in CMake

```cmake
set(PCR_FETCH_DEPS ON CACHE BOOL "Fetch missing dependencies")
add_subdirectory(external/libpcr)
target_link_libraries(my_app PRIVATE pcr::ipc_stdio_jsonrpc)
```

The stdio transport is one composition of the lower-level modules. Programs needing only process control, streams, or framing can use those parts separately.


## Support

**Currently:** Subprocess communication over stdio pipes, `Content-Length` message framing, and JSON-RPC.

**Future extensions:** Unix-domain sockets, TCP (local or remote), other framing schemes, and additional RPC protocols
