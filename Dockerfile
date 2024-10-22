FROM alpine:latest

RUN apk add git build-base

WORKDIR /app

ADD https://api.github.com/repos/ahpohl/libsunspec/git/refs/heads/master libsunspec-version.json
RUN git clone https://github.com/ahpohl/libsunspec.git 
ADD https://api.github.com/repos/ahpohl/froniusd/git/refs/heads/master froniusd-version.json
RUN git clone https://github.com/ahpohl/froniusd.git

CMD ["/bin/sh"]
