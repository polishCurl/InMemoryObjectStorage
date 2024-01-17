FROM ubuntu:20.04

RUN apt-get update
RUN apt-get install -y graphviz
RUN apt-get install -y build-essential
RUN apt-get install -y git
RUN apt-get install -y doxygen
RUN apt-get install -y lcov
RUN apt-get install -y git-lfs
RUN apt-get install -y gdb
RUN apt-get install -y clang

# Install Bazel
RUN apt-get install -y apt-transport-https curl gnupg -y
RUN curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor >bazel-archive-keyring.gpg
RUN mv bazel-archive-keyring.gpg /usr/share/keyrings
RUN echo "deb [arch=amd64 signed-by=/usr/share/keyrings/bazel-archive-keyring.gpg] https://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list
RUN apt-get update
RUN apt-get install -y bazel
RUN echo "source /etc/bash_completion.d/bazel" >> ~/.bashrc

WORKDIR /object_storage