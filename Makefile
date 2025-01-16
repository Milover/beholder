# Makefile

MODULE		:= $(shell go list -m)
MODULE_URL	:= $(shell go list -m | tr [:upper:] [:lower:])
TARGET		:= $(shell basename $(MODULE))
PLATFORM	:= $(shell uname)

BUILD_TAGS ?=

ifeq ($(PLATFORM), Darwin)
 # TODO: remove when no longer necessary, for more info see:
 # https://stackoverflow.com/questions/77164140/ld-warning-ignoring-duplicate-libraries-lgcc-after-the-recent-update-of-xc
 export CGO_LDFLAGS="-Wl,-no_warn_duplicate_libraries"
endif

.PHONY: build
build:
	@echo $(MODULE)
	go generate ./...
	go build -tags=$(BUILD_TAGS) -o bin/$(TARGET) main.go

.PHONY: generate
generate:
	go generate ./...

.PHONY: run
run:
	go run ./...

.PHONY: test
test:
	go test ./...

.PHONY: testv
testv:
	go test -v -race ./...

.PHONY: test-integration
test-integration:
	go test -v -race -tags=integration ./...

.PHONY: test-full
test-full:
	go test -v -race -count=1 -tags=integration,embedtess ./...

.PHONY: vet
vet:
	go vet -tags=integration,embedtess ./...

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

.PHONY: clean
clean:
	# TODO: should clean generated files here
	go clean
	rm -rf bin
