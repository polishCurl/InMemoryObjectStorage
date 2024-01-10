# Object storage
In-memory file storage service written in C++. 


## Description
_Object Storage_ runs as a terminal application over TCP using two protocols - HTTP/1.1 and FTP. 

Both HTTP and FTP control channels are available on the same port. The protocol used by the client is automatically inferred from the client's port number.

The port number range dedicated to FTP protocol is configurable. All other port numbers are assumed HTTP.

The service supports storing files using one protocol and retrieving them with another.

The object storage is **in-memory**, it is **not persistent**.

### Features
- Multithreaded operation (configurable number of threads)
- Asynchronous IO
- Configurable logging level

FTP:
- List all stored files: `LIST`
- Download files: `RETR /{key}`
- Upload files: `STOR /{key}`
- Remove files: `DELETE /{key}`
- Support for passive mode (only)
- FTP login (optional): `USER <username>` and `PASS <password>`

HTTP:
- List all stored files: `GET /`
- Download files: `GET /{key}`
- Upload files: `PUT /{key}`
- Remove files: `DELETE /{key}`
- Basic Authentication (optional)

**Note**: Object storage does not support encryption.


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

Remove _object_storage_container_:
```
docker rm object_storage_container
```


## Building
This project uses [Bazel](https://bazel.build/) build system.

Example Object Storage application is located in `example_app.cpp`.

The application can be built with the following command:
```
bazel build //:object_storage
```

## Running
The application can be built and executed with the following command:
```
bazel run //:object_storage
```


### External dependencies
The only external build dependencies are:
- `boost::asio`
- `boost::log`

These dependencies are managed by Bazel.




## Testing
Both unit and intergration tests were implemented using the [GoogleTest](https://github.com/google/googletest) framework.

Integration tests use the _curl_ command line utility and they are located in `test` folder.


Build and execute all tests (unit and integration):
```
bazel test //...
```

Build and execute integration tests:
```
bazel test //test:integration_tests
```

### Code coverage
Generate code coverage report using the _lcov_ and _genhtml_:
```
bazel coverage //...
genhtml bazel-out/_coverage/_coverage_report.dat -o genhtml
```

The top-level coverage report will be placed in `genhtml/index.html`


## Supported platforms
Object store has been tested on the following platforms (OS -- compiler):
1. Ubuntu 20.04.6 LTS -- gcc (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0
2. macOS 12.4 -- Apple clang version 13.1.6 (clang-1316.0.21.2.5)

Since object store uses platform-independent `boost::asio` library for networking, minimal changes would need to be made to the application to support Windows OS.


## Documentation
Generate Doxygen docummentation: 
```
doxygen
```
Documentation will be located in `doxydoc/html/index.html`.


## Future extensions
TODO list:
1. Support TLS (HTTPS and FTPS) connections on the same port.
2. Add support for more FTP commands.
3. Add support for more HTTP methods.
4. Create mocks for the defined interfaces and use them in unit tests using dependency injection.
5. Create microbenchmarks for measuring performance of key components of the _Object storage_.
6. Enable CI/CD.
