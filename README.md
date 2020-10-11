# drogon ebayfeedback server
```
git clone https://github.com/jimzrt/ebayfeedback_server_drogon.git
cd ebayfeedback_server_drogon
git submodule update --init --recursive
```
```
mkdir build  
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make ebayfeedback
```