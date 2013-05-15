INSTALL=$(PREFIX)out
INSTALL_LIB=$(INSTALL)/libs
INSTALL_BIN=$(INSTALL)/bin
CP=cp

MXML_LIB=mxml
export INCLUDES=$(MXML_LIB)


install_libs = mxml 
install_exe = ssdp_disc/ssdp
ENV=CFLAGS="-I../$(MXML_LIB)/mxml" LDFLAGS="-lpthread -L../$(MXML_LIB) -l$(MXML_LIB)"


all: $(install_exe)

$(MXML_LIB)/lib$(MXML_LIB).a: 
	 $(MAKE) -C $(MXML_LIB)/ all


$(install_exe): $(MXML_LIB)/lib$(MXML_LIB).a
		$(ENV) $(MAKE) -C ssdp_disc/ all


clean:
	$(MAKE) -C mxml clean
	$(MAKE) -C ssdp_disc clean
	rm -rf $(INSTALL)
