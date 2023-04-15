# docker build -t network_butcher .
# docker run --name nn --rm -v $(pwd):/network_butcher -it network_butcher

FROM ubuntu:22.04

ENV MY_DIR=/network_butcher
ENV TZ=Europe/Rome


# Install build requirments
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && apt-get install wget build-essential cmake git libtbb-dev --no-install-recommends -y
RUN apt-get install python3-dev python3-pip --no-install-recommends -y

# Install python libraries
WORKDIR ${MY_DIR}
COPY dep/requirements.txt .
RUN python3 -m pip install -r requirements.txt


# Prepare for the test_run
COPY . .

CMD bash