PREFIX=/usr

MSG_ORG_PATH=./etc/kanon-snow
MSG_PATH=/etc/kanon-snow
MSG=message

INCLUDE=`zx11-config -I`
LIB=`zx11-config -L`
LINK=`zx11-config -l`

CC=g++
CFLAGS=-Wall -O2 -Wno-deprecated $(LIB) $(INCLUDE) -DKS_DEFAULT_MSG_PATH=\"$(MSG_PATH)\" -DKS_DEFAULT_MSG=\"$(MSG)\"

TARGET=kanon-snow

all: $(TARGET)
$(TARGET): $(TARGET).cpp $(TARGET).h
	$(CC) $(CFLAGS) -o $@ $< $(LINK)
install:
	install -m 755 $(TARGET) $(PREFIX)/bin;
	if [ ! -d $(MSG_PATH) ]; then \
		mkdir $(MSG_PATH); \
	fi
	install -m 644 $(MSG_ORG_PATH)/message* $(MSG_PATH);
clean:
	rm -f *.o *~ core $(TARGET)
