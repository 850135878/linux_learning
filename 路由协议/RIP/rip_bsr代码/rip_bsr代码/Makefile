ifeq ($(SWITCH_NAME), )
	ifeq ($(ROUTER_NAME), )
		include makefile.psos
	else
		ifeq ($(findstring $(ROUTER_NAME),$(CAVIUM_CPU_TYPE)), $(ROUTER_NAME))
			include makefile.vxworks3
		else
			include makefile.vxworks2
		endif
	endif
else
	include makefile.vxworks
endif