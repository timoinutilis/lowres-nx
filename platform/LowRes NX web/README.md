These lines are interesting, but still not a real guide for compiling:

./emsdk activate latest
source ./emsdk_env.sh
emmake make

Always clean before build, otherwise changed files will be ignored!
emmake make clean