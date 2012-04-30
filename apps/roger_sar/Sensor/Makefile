all:
	$(MAKE) -C ./datagen
	$(MAKE) -C ./SSCP_hc
	$(MAKE) -C ./SSCP_afl
	$(MAKE) -C ./SSCP_sim

run:
	$(MAKE) -C ./datagen  run
	$(MAKE) -C ./SSCP_hc  run
	$(MAKE) -C ./SSCP_afl run
	$(MAKE) -C ./SSCP_sim run

clean:
	$(MAKE) -C ./datagen  clean
	$(MAKE) -C ./SSCP_hc  clean
	$(MAKE) -C ./SSCP_afl clean
	$(MAKE) -C ./SSCP_sim clean
