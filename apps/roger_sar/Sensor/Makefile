all:
	$(MAKE) -C ./datagen
	$(MAKE) -C ./SSCP_afl/utils
	$(MAKE) -C ./SSCP_sim/utils

small:
	$(MAKE) -C ./datagen  small
	$(MAKE) -C ./SSCP_hc  small
	$(MAKE) -C ./SSCP_afl small
	$(MAKE) -C ./SSCP_sim small

large:
	$(MAKE) -C ./datagen  large
	$(MAKE) -C ./SSCP_hc  large
	$(MAKE) -C ./SSCP_afl large
#	$(MAKE) -C ./SSCP_sim large

clean:
	$(MAKE) -C ./datagen  clean
	$(MAKE) -C ./SSCP_hc  clean
	$(MAKE) -C ./SSCP_afl clean
	$(MAKE) -C ./SSCP_sim clean
