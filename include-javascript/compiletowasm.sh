emcc -O0 -s WASM=1 -s EXPORTED_RUNTIME_METHODS='["cwrap","versiont"]' -s MALLOC="emmalloc-verbose" -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=true serialize-wasm.cpp
