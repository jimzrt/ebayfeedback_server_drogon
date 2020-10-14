FROM debian:buster-slim
RUN apt-get update && \
    apt-get install -y git gcc g++ cmake libjsoncpp-dev uuid-dev openssl libssl-dev zlib1g-dev
WORKDIR /root

# invalidate cache when new commit(s) is available
ADD https://api.github.com/repos/jimzrt/ebayfeedback_server_drogon/git/refs/heads/master version.json

RUN git clone https://github.com/jimzrt/ebayfeedback_server_drogon.git && \
    cd ebayfeedback_server_drogon && \
    git submodule update --init --recursive && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) ebayfeedback

WORKDIR /root/ebayfeedback_server_drogon/build
ENTRYPOINT ["/root/ebayfeedback_server_drogon/build/ebayfeedback"]