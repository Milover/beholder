# Makefile

MODULE		:= $(shell go list -m)
MODULE_URL	:= $(shell go list -m | tr [:upper:] [:lower:])
TARGET		:= $(shell basename $(MODULE))

build:
	echo $(MODULE)
	go build -o bin/$(TARGET) main.go

# FIXME: this probably doesn't work
publish:
	GOARCH=amd64 GOOS=linux   go build -o bin/$(TARGET)-linux   main.go
	GOARCH=arm64 GOOS=darwin  go build -o bin/$(TARGET)-darwin  main.go

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
