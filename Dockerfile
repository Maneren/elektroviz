FROM ubuntu:24.10

RUN apt-get update && apt-get install -y --no-install-recommends \
  libasound2-dev libx11-dev libxrandr-dev libxi-dev \
  libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev \
  libwayland-dev libxkbcommon-dev build-essential git clang ca-certificates

RUN mkdir /workspace

WORKDIR /workspace

RUN git config --global --add safe.directory /workspace

CMD [ "make", "setup", "build" ]
