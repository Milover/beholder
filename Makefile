# Makefile

MODULE		:= $(shell go list -m)
MODULE_URL	:= $(shell go list -m | tr [:upper:] [:lower:])
TARGET		:= $(shell basename $(MODULE))

BUILD_TAGS ?=

build:
	@echo $(MODULE)
	go build -tags=$(BUILD_TAGS) -o bin/$(TARGET) main.go

run:
	go run ./...

test:
	go test ./...

testv:
	go test -v ./...

test-integration:
	go test -v -tags=integration ./...

vet:
	go vet ./...

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
