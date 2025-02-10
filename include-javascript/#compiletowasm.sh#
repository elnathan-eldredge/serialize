source ~/emsdk/emsdk_env.sh
emcc -O2 \
     serialize-wasm.cpp \
     -s WASM=1 \
     -s EXPORTED_RUNTIME_METHODS='["cwrap"]' \
     -s ASSERTIONS=1 -s \
     -s ALLOW_MEMORY_GROWTH=true \
     -s STACK_OVERFLOW_CHECK=2 \
     -s VERBOSE=0 \
     -s MALLOC="dlmalloc" \
     -s ABORTING_MALLOC=0 \
     -s ALLOW_MEMORY_GROWTH=1 
