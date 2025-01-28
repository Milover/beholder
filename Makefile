# Makefile

MODULE			:= $(shell go list -m)
MODULE_URL		:= $(shell go list -m | tr [:upper:] [:lower:])
TARGET			:= $(shell basename $(MODULE))
PLATFORM		:= $(shell uname)

CMAKE_PREFIX	?= /usr/local
CMAKE_PRESET	?= release
GO_FLAGS		?=

# build stuff
# -----------
.PHONY: build
build:
	@echo $(MODULE)
	scripts/build_go_project.sh $(GO_FLAGS)

.PHONY: c-api
c-api:
	scripts/build_cmake_project.sh _c-api $(CMAKE_PRESET) $(CMAKE_PREFIX)

.PHONY: third-party
third-party:
	scripts/build_cmake_project.sh _c-api/third_party release $(CMAKE_PREFIX)

# clean stuff
# -----------
.PHONY: clean
clean:
	scripts/clean_go_project.sh $(GO_FLAGS)

.PHONY: clean-c-api
clean-c-api:
	scripts/clean_cmake_project.sh _c-api $(CMAKE_PRESET)

.PHONY: clean-third-party
clean-third-party:
	scripts/clean_cmake_project.sh _c-api/third_party release

# test stuff
# ----------
.PHONY: test
test:
	go test -race $(GO_FLAGS) ./...

.PHONY: test-c-api
test-c-api:
	cd _c-api && ctest --preset=$(CMAKE_PRESET)

# Go specific stuff
# -----------------
.PHONY: lint
lint:
	$(shell go env GOPATH)/bin/golangci-lint run ./...

.PHONY: update-deps
update-deps:
	go get -u ./...
	go mod tidy

.PHONY: update-go
update-go:
	go mod edit -go=$(shell go version | awk '{print $$3}' | sed -e 's/go//g')
	go mod tidy
