FROM orbiterco/eosio


COPY /docker/run_compile.sh /
RUN mkdir /src
COPY . /src

CMD ["/run_compile.sh"]

ENTRYPOINT ["/bin/bash", "-l", "-c"]

