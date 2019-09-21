FROM orbiterco/eosio


COPY /docker/run_compile.sh /
RUN mkdir /src
COPY . /src

RUN /run_compile.sh

