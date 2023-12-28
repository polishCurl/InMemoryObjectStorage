# Object storage
In-memory object storage service written in C++. 

## Supported protocols
The service runs as a terminal application over TCP using two protocols - HTTP/1.1 and FTP. 
Both HTTP and FTP control channels are available on the same port. The protocol used by the client is automatically infered from the request.
The service supports storing objects using one protocol and retrieving them with another.
The object storage is in-memory, it is not persistent.

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
docker container prune -y
```

## Building
This project uses [Bazel](https://bazel.build/) build system.

To build the main object store service binary, run:
```
bazel build //:object_store 
```

## Useful commands
Generate Doxygen docummentation (located in `doxydoc/html/index.html`):
```
doxygen
```


## Testing
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
