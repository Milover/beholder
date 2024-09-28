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

build:
	@echo $(MODULE)
	go build -tags=$(BUILD_TAGS) -o bin/$(TARGET) main.go

run:
	go run ./...

test:
	go test ./...

testv:
	go test -v -race ./...

test-integration:
	go test -v -race -tags=integration ./...

test-full:
	go test -v -race -count=1 -tags=integration,embedtess ./...

vet:
	go vet -tags=integration,embedtess ./...

lint:
	$(shell go env GOPATH)/bin/golangci-lint run ./...

update-deps:
	go get -u ./...
	go mod tidy

update-go:
	go mod edit -go=$(shell go version | awk '{print $$3}' | sed -e 's/go//g')
	go mod tidy

clean:
	go clean
	rm -rf bin

.PHONY: run test testv test-integration vet lint clean update-deps update-go
