GENERATED=_geometry.h _geometry_xdr.c _geometry_clnt.c _geometry_svc.c
BIN=geometry_server geometry_client

all: ${BIN}

strip: all
	strip ${BIN}

.c.o:
	gcc -c $<

geometry_server: ${GENERATED} shape_operator.o logger.o geometry_server.o _geometry_xdr.o _geometry_svc.o
	gcc -pthread -o $@ shape_operator.o logger.o geometry_server.o _geometry_xdr.o _geometry_svc.o

geometry_client: ${GENERATED} geometry_client.o logger.o _geometry_xdr.o _geometry_clnt.o
	gcc -pthread -o $@ geometry_client.o logger.o _geometry_xdr.o _geometry_clnt.o

${GENERATED}: _geometry.x
	rpcgen _geometry.x

clean:
	rm -f ${GENERATED} ${BIN} *.o core
