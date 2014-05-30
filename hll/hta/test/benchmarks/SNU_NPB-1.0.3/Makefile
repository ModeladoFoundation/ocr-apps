SUBDIRS = NPB3.3-HTA NPB3.3-OMP-C NPB3.3-SER-C 

.PHONY: clean veryclean tidy
.PHONY: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
clean: $(SUBDIRS)
	- rm -rf rec rec_pure_omp rec_swarm rec_omp rec_ocr
veryclean: $(SUBDIRS)
	- rm -rf rec rec_serial rec_pure_omp rec_swarm rec_omp
tidy: $(SUBDIRS)

