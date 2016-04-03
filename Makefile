
PROjECT_ROOT = ./
SRCDIR = $(PROjECT_ROOT)/src
BINDIR = $(PROjECT_ROOT)/bin

install:
	@mkdir -p $(BINDIR)
	@g++ $(SRCDIR)/myserver.cpp -o $(BINDIR)/server
	@g++ $(SRCDIR)/user.c -o $(BINDIR)/user
	@g++ $(SRCDIR)/worker.c -o $(BINDIR)/worker -lcrypt

clean:
	@rm -rf $(BINDIR)