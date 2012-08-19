DESTDIR := /usr/local
TARGET := luminescence
CFLAGS := -g -Wall
LIBDL := $(shell [ -e /lib/libdl.so ] && echo '-ldl' )

all: luminescence

$(TARGET): $(TARGET).c 
	gcc -o $(TARGET) $< $(CFLAGS) $(LIBDL) `pkg-config --cflags --libs gtk+-3.0 webkitgtk-3.0 gthread-2.0`

clean:
	rm -f $(TARGET)

install: luminescence
	mkdir -p $(DESTDIR)/{bin,include,src}
	mkdir -p $(DESTDIR)/src/$(TARGET)/plugins
	install -m 755 $(TARGET) $(TARGET)-install $(DESTDIR)/bin
	install -m 644 $(TARGET).h $(DESTDIR)/include
	install -m 644 plugins/*.c $(DESTDIR)/src/$(TARGET)/plugins

uninstall:
	rm -rf $(DESTDIR)/src/$(TARGET)
	rm -f $(DESTDIR)/bin/$(TARGET)
	rm -f $(DESTDIR)/bin/$(TARGET)-install
	rm -f $(DESTDIR)/include/$(TARGET).h
