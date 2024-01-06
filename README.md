# Object storage
In-memory object storage service written in C++. 

## Supported protocols
The service runs as a terminal application over TCP using two protocols - **HTTP/1.1** and **FTP**. 

Both HTTP and FTP control channels are available on the same port. The protocol used by the client is automatically infered from the request.

The service supports storing objects using one protocol and retrieving them with another.

The object storage is **in-memory**, it is **not persistent**.

## Setup
The object storage is containerized to provide seemless delivery.
`Dockefile` contains instructions to build an image with all required dependencies to build and run the application.

Build the docker image:
```
docker build -t object_storage .
```

Run the container:
```
docker run --name object_storage_container -v $(pwd):/object_storage -it object_storage
```

Start an existing container:
```
docker start -i object_storage_container
```

Remove unnecessary images and containers:
```
docker image prune -a
docker container prune
```

## Building
This project uses [Bazel](https://bazel.build/) build system.

To build the main object store service binary, run:
```
bazel build //:object_store 
```

## Testing
### Supported platforms
Object store has been tested on the following platforms:
1. Ubuntu 20.04.6 LTS -- gcc (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0
2. macOS 12.4 -- Apple clang version 13.1.6 (clang-1316.0.21.2.5)

Since object store uses platform-independent `boost::asio` library for networking, minimal changes would need to be made to the application to support Windows OS.

### Unit tests
Unit tests were implemented using the [GoogleTest](https://github.com/google/googletest) framework.

Build and execute all unit tests:
```
bazel test //...
```

Generate code coverage report using the _lcov_ and _genhtml_:
```
bazel coverage //...
genhtml bazel-out/_coverage/_coverage_report.dat -o genhtml
```

The top-level coverage report will be placed in `genhtml/index.html`

## Documentation
Generate Doxygen docummentation (will be located in `doxydoc/html/index.html`):
```
doxygen
```

## Future extensions
TODO list:
1. Implement custom logging system instead of using `std::cout` and `std::cerr`.
2. Support TLS (HTTPS and FTPS) connections on the same port.
3. Create mocks for the defined interfaces and use them in unit tests using dependency injection.
4. Create microbenchmarks for measuring performance of key components of the _Object storage_.
