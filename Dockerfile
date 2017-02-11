FROM buildpack-deps:jessie

RUN apt-get update && apt-get install -y \
  autoconf \
  autotools-dev \
  build-essential \
  curl \
  gcc \
  git \
  libc6-dbg \
  libc6-dev \
  libgtest-dev \
  libgflags-dev \
  libtool \
  make \
  tesseract-ocr \
  libtesseract-dev \
  tesseract-ocr-deu \
  libleptonica-dev \
  unzip && apt-get clean

ENV GRPC_RELEASE_TAG v1.1.2 

RUN git clone https://github.com/grpc/grpc.git /var/local/git/grpc
RUN cd /var/local/git/grpc && \
  git checkout tags/${GRPC_RELEASE_TAG} && \
  git submodule update --init --recursive

RUN cd /var/local/git/grpc/third_party/protobuf && \
  ./autogen.sh && \
  ./configure --prefix=/usr && \
  make -j12 && make check && make install && make clean

RUN cd /var/local/git/grpc && make install

RUN mkdir -p /usr/src/app
WORKDIR /usr/src/app

COPY . /usr/src/app
RUN make
