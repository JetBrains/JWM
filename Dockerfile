FROM ubuntu:20.04

WORKDIR /root

RUN apt-get update -y && apt-get install -y software-properties-common
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN add-apt-repository -y ppa:openjdk-r/ppa
RUN add-apt-repository -y ppa:git-core/ppa
RUN apt-get update -y && apt-get install -y wget zip python git build-essential g++-9 cmake ninja-build libxcomposite-dev libxrandr-dev libgl1-mesa-dev libxi-dev libxcursor-dev openjdk-11-jdk-headless libegl1-mesa libegl1-mesa-dev extra-cmake-modules wayland-protocols wayland-utils libwayland-dev
RUN wget --no-verbose https://downloads.apache.org/maven/maven-3/3.6.3/binaries/apache-maven-3.6.3-bin.tar.gz --output-document - | tar -xz
RUN echo 'export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64' > /etc/profile.d/02-jdk.sh
RUN echo 'export PATH=$JAVA_HOME/bin:/root/apache-maven-3.6.3/bin:$PATH' >> /etc/profile.d/02-jdk.sh
