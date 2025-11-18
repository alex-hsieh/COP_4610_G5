FROM mcr.microsoft.com/devcontainers/base:ubuntu

RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    make \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspaces
