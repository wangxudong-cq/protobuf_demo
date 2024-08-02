### dependencies
```
protobuf-26.0:git@github.com:protocolbuffers/protobuf.git
```

### proto
~~~shell
cd xxx/graph_loader_debug/graph_proto
./protoc --cpp_out=./src --proto_path=./src ./src/graph.proto
~~~

### build on linux
~~~cmake
cd xxx/graph_loader_debug/graph_proto
mkdir build
cd build
cmake ..
make
~~~

### build on windows