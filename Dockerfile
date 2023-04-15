# sudo docker build -t network_butcher .
# sudo docker run --name nn --rm -v $(pwd):/network_butcher -it network_butcher

FROM ubuntu:20.04

ENV MY_DIR=/network_butcher
ENV TZ=Europe/Rome

# Install build requirments
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && apt-get install build-essential git libtbb-dev --no-install-recommends -y
RUN apt-get install python3.8 python3.8-dev python3-pip --no-install-recommends -y

# Install python libraries
WORKDIR ${MY_DIR}
COPY dep/requirements.txt .
RUN python3 -m pip install -r requirements.txt


# Install lastest version of CMake (https://askubuntu.com/a/865294)
RUN apt-get install wget dirmngr gpg-agent software-properties-common lsb-release --no-install-recommends -y
RUN apt-get clean all
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 42D5A192B819C5DA
RUN apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
RUN apt-get update && apt-get install cmake --no-install-recommends -y


COPY . .

CMD bash