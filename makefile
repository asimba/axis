SRCDIR = ./src
BINDIR = ../bin

all:
	@make -s -C $(SRCDIR) all
	@rm -f ./sha1sums
	@rm -f ./axissums
	@make -s -C $(SRCDIR) clean-obj
	@echo "Calculating checksums..."
	@$(SRCDIR)/hashsums.sh

clean:
	@make -s -C $(SRCDIR) clean

clean-obj:
	@make -s -C $(SRCDIR) clean-obj

.NOEXPORT:
