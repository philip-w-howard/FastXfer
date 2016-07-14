OPTS= -c -Wall

OBJS= EdisonFastXfer.o \
      sendfile.o \

all: EdisonFastXfer

clean:
	rm -f $(OBJS)
	rm -f EdisonFastXfer

.c.o:
	gcc $(OPTS) $? -o $@

EdisonFastXfer: $(OBJS)
	gcc -Wall $(OBJS) -o EdisonFastXfer

