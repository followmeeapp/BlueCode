FROM ubuntu:15.10
COPY discoveryd /usr/bin/
RUN mkdir /db
EXPOSE 3003
CMD /usr/bin/discoveryd -port=3003 -db_path=/db -db_size=4096
