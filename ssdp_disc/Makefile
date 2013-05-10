MODULE_NAME=ssdp
MODULE_SRCS=ssdp.c xml2json.c json2soap.c
MODULE_OBJS=$(MODULE_SRCS:.c=.o)
$(info cible:$(MAKECMDGOALS))

ifeq ($(MAKECMDGOALS),json2soap)
$(info -$(MAKECMDGOALS))
CFLAGS+=-DTEST_SOAP -I ../mxml/mxml/
LDFLAHS+= -lmxml
endif


CFLAGS +=-g -Wall
$(MODULE_NAME):$(MODULE_OBJS)
	$(CC) $^ -o $@ -ljansson $(LDFLAGS)

#CFLAGS+= ../mxml
all: $(MODULE_NAME)

json2soap: json2soap.o xml2json.o
	$(CC) $^ -o $@ -ljansson -L ../mxml -lmxml -lpthread $(LDFLAGS)
clean: 
	rm -rf $(MODULE_NAME) $(MODULE_OBJS)
