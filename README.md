# HTTP/FTP in-memory file server
In-memory file storage service written in C++. 


## Description
_Object Storage_ runs as a terminal application over TCP using two protocols - HTTP/1.1 and FTP. 

Both HTTP and FTP control channels are available on the same port. The protocol used by the client is initially inferred from the client's port number. Then it is detected from packet headers.

The port number range dedicated to FTP protocol is configurable. All other port numbers are assumed HTTP.

The service supports storing files using one protocol and retrieving them with another.

The object storage is **in-memory**, it is **not persistent**.

### Features
_Object Storage_ server:
- Multithreaded operation (configurable number of threads)
- Asynchronous IO
- Configurable logging level

FTP:
- List all stored files: `LIST`
- Download files: `RETR /{key}`
- Upload files: `STOR /{key}`
- Remove files: `DELE /{key}`
- FTP login (optional): `USER <username>` and `PASS <password>`
- Support for passive mode (only): `PASV`
- Change working directory: `CWD <directory>`
- Close connection: `QUIT`

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

Example application is located in `example_app.cpp`.

The Object Storage can be built with the following commands.

Build with gcc:
```
bazel build //:object_storage  --config=gcc
```

Build with clang:
```
bazel build //:object_storage  --config=clang
```

### External dependencies
The only external build dependencies are:
- `boost::asio`
- `boost::log`

These dependencies are managed by Bazel.


## Running
To learn about _object_storage_ usage, run:
```
./bazel-bin/object_storage 
```

For example, to launch the Object Storage server with:
- IPv4 address 127.0.0.1
- port number 1670
- 8 worker threads
- HTTP basic authentication and FTP login enabled
- With client port numbers 30000-40000 to use for FTP

Run:
```
./bazel-bin/object_storage 127.0.0.1 1670 8 auth 30000-40000
```

To stop Object Storage, simply press `<Enter>`.

In the provided example, the server is by default configured with one user: `Nord:VPN`.

### Sending FTP/HTTP requests using _curl_
Upload files:
```
curl http://127.0.0.1:1670/test/data/example.json -T test/data/example.json  --local-port 20000-30000  --user Nord:VPN
curl ftp://localhost:1670/the_office/ringtone.mp3 -T test/data/the_office_theme.mp3  --local-port 30000-40000 --user "Nord:VPN"
```

List all files stored in Object Storage:
```
curl http://localhost:1670/ --local-port 20000-30000 --user "Nord:VPN"
curl ftp://localhost:1670  --local-port 30000-40000 --user "Nord:VPN"
```

Download files:
```
curl http://localhost:1670/the_office/ringtone.mp3 -o /tmp/music_to_my_years.mp3 --local-port 20000-30000 --user "Nord:VPN"
curl ftp://localhost:1670/test/data/example.json  -o /tmp/ftp_download_example.json --local-port 30000-40000 --user "Nord:VPN" 
```

Delete files:
```
curl http://localhost:1670/the_office/ringtone.mp3 -X DELETE --local-port 20000-30000 --user "Nord:VPN"
curl ftp://localhost:1670  -Q "DELE /test/data/example.json" --local-port 30000-40000 --user "Nord:VPN"
```


## Testing
Both unit and intergration tests were implemented using the [GoogleTest](https://github.com/google/googletest) framework.

Integration tests use the _curl_ command line utility and they are located in `test` folder.


Build and execute all tests (unit and integration) using the default compiler:
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

Note: `bazel coverage` is not supported on MacOS.


## Supported platforms
Object store has been tested on the following platforms (OS -- compiler):
1. Ubuntu 20.04.6 LTS -- gcc 9.4.0
2. Ubuntu 20.04.6 LTS -- clang 10.0.0
3. macOS 12.4 -- Apple clang version 13.1.6

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
4. Create mocks for the defined interfaces and use them in unit tests using dependency injection and improve code coverage.
5. Create microbenchmarks for measuring performance of key components of the _Object storage_. See my [example microbenchmark](https://github.com/polishCurl/IPv4_geo_lookup/blob/main/csv/csv_reader/bench/csv_reader_bench.cpp).
6. Enable CI/CD.
