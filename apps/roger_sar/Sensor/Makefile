all:	utils radar_data
	$(MAKE) -C ./SSCP_hc  all
	$(MAKE) -C ./SSCP_afl all
	$(MAKE) -C ./SSCP_sim all

utils:
	$(MAKE) -C ./datagen
	$(MAKE) -C ./SSCP_afl/utils
	$(MAKE) -C ./SSCP_sim/utils

radar_data:
	$(MAKE) -C ./datagen  small
	$(MAKE) -C ./datagen  large

small:
	$(MAKE) -C ./SSCP_hc/src  all
	$(MAKE) -C ./SSCP_afl/small all
	$(MAKE) -C ./SSCP_sim/small all

large:
	$(MAKE) -C ./SSCP_hc/src  all
	$(MAKE) -C ./SSCP_hc/large  all
	$(MAKE) -C ./SSCP_afl/large all
#	$(MAKE) -C ./SSCP_sim/large all

clean:
	$(MAKE) -C ./datagen  clean
	$(MAKE) -C ./SSCP_hc  clean
	$(MAKE) -C ./SSCP_afl clean
	$(MAKE) -C ./SSCP_sim clean
