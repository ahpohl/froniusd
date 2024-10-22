FROM alpine:latest

RUN apk add git build-base autoconf automake libtool linux-headers mosquitto-dev

WORKDIR /app

ADD https://api.github.com/repos/stephane/libmodbus/git/refs/heads/master libmodbus-version.json
RUN git clone https://github.com/stephane/libmodbus.git
RUN cd /app/libmodbus && ./autogen.sh && ./configure && make install

ADD https://api.github.com/repos/ahpohl/libsunspec/git/refs/heads/master libsunspec-version.json
RUN git clone https://github.com/ahpohl/libsunspec.git 
RUN cd /app/libsunspec && sed -i 's/ln\s-sr\(.*\)/ln -s \1/g' Makefile && make PREFIX=/usr install

ADD https://api.github.com/repos/ahpohl/froniusd/git/refs/heads/master froniusd-version.json
#RUN git clone https://github.com/ahpohl/froniusd.git
COPY . /app/froniusd
RUN cd /app/froniusd && make install

ENTRYPOINT ["froniusd"]
