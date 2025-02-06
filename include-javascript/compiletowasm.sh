emcc -O3 -s WASM=1 -s EXPORTED_RUNTIME_METHODS='["cwrap","healthcheck"]' serialize-wasm.cpp
