FROM debian:buster-slim
RUN apt-get update && apt-get install -y git gcc g++ cmake libjsoncpp-dev uuid-dev openssl libssl-dev zlib1g-dev
WORKDIR /root
RUN git clone https://github.com/jimzrt/ebayfeedback_server_drogon.git && cd ebayfeedback_server_drogon && git submodule update --init --recursive
WORKDIR /root/ebayfeedback_server_drogon
RUN mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc) ebayfeedback
WORKDIR /root/ebayfeedback_server_drogon/build
ENTRYPOINT ["/root/ebayfeedback_server_drogon/build/ebayfeedback"]