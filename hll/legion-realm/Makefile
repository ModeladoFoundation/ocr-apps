# Copyright 2016 Rice University, Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Makefile for setting up the Legion-OCR code base
#
# Author: Sri Raj Paul <sriraj@rice.edu>

.PHONY: default
default:
	@echo ""
	@echo "Valid targets are:"
	@echo ""
	@echo "   all -- clone legion and copy the changes"
	@echo "   deploy -- clone legion and copy the changes"
	@echo "   clean -- remove the legion code base"
	@echo ""

.PHONY: all
all:deploy

.PHONY: deploy
deploy:legion
	cp -r patch/* $^

legion:
	git clone -b stable https://github.com/StanfordLegion/legion.git $@
	cd $@ && git checkout 4fc956c72698b7fe8337827c10a8471aa26ed8fc

.PHONY: clean
clean:
	rm -rf legion

