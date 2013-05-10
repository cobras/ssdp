INSTALL=$(PREFIX)out
INSTALL_LIB=$(INSTALL)/libs
INSTALL_BIN=$(INSTALL)/bin
CP=cp
.PHONY: $(INSTALL) install all .FORCE

MXML_LIB=mxml
export INCLUDES=$(MXML_LIB)

install_libs = mxml 
install_exe = ssdp_disc

$(MXML_LIB)/lib$(MXML_LIB).a: 
	 $(MAKE) -C $(MXML_LIB)/ all

$(INSTALL_BIN)/ssdp: $(MXML_LIB)/lib$(MXML_LIB).a .FORCE
	CFLAGS="-I../$(MXML_LIB)/mxml" LDFLAGS="-lpthread -L../$(MXML_LIB) -l$(MXML_LIB)" $(MAKE) -C ssdp_disc/ all

# all: $(install_libs) $(install_exe)
# 	$(foreach dir,$^, ;$(MAKE) -C $(dir) all;)

all:$(INSTALL_BIN)/ssdp
	echo "$^ built."
clean:
	$(MAKE) -C mxml clean
	$(MAKE) -C ssdp_disc clean
	rm -rf $(INSTALL)