package server

import (
	"errors"
	"fmt"

	pb "github.com/Milover/beholder/internal/server/proto"
	"google.golang.org/protobuf/proto"
)

var (
	// ErrNoDistribute signals that the service response should only be sent to
	// the request issuer and not distributed to all active connections.
	ErrNoDistribute = errors.New("no distribute")
)

// defaultServiceMessage creates a new default service response payload.
func defaultServiceMessage() *pb.Error {
	return &pb.Error{
		Code:        pb.ErrorCode_ERROR_CODE_FAIL,
		Description: ErrMsg.Error(),
	}
}

// TODO: docs
// A Service must always return a valid response message (payload).
//
// NOTE: should take an Op.Payload and return a new Op.Payload as a response
type Service func(proto.Message, *AcquisitionServer) (proto.Message, error)

// services is a global map of services [AcquisitionServer] provides.
var services = map[pb.OpCode]Service{
	pb.OpCode_OP_CODE_START_ACQUISITION: startAcquisitionService,
	pb.OpCode_OP_CODE_STOP_ACQUISITION:  stopAcquisitionService,
}

func startAcquisitionService(m proto.Message, s *AcquisitionServer) (proto.Message, error) {
	msg := defaultServiceMessage()

	op, ok := m.(*pb.Op)
	if !ok {
		return msg, fmt.Errorf("server.startAcquisitionService: %w: bad type assertion", errSanity)
	}
	if op.Payload != nil {
		return msg, fmt.Errorf("server.startAcquisitionService: %w", errOpPayload)
	}

	if err := s.startAcquisition(); err != nil {
		msg.Description = err.Error()
		return msg, ErrNoDistribute
	}
	msg.Code = pb.ErrorCode_ERROR_CODE_SUCCESS
	msg.Description = "acquisition started"
	return msg, nil
}

func stopAcquisitionService(m proto.Message, s *AcquisitionServer) (proto.Message, error) {
	msg := defaultServiceMessage()

	op, ok := m.(*pb.Op)
	if !ok {
		return msg, fmt.Errorf("server.stopAcquisitionService: %w: bad type assertion", errSanity)
	}
	if op.Payload != nil {
		return msg, fmt.Errorf("server.stopAcquisitionService: %w", errOpPayload)
	}

	if err := s.stopAcquisition(); err != nil {
		msg.Description = err.Error()
		return msg, ErrNoDistribute
	}
	msg.Code = pb.ErrorCode_ERROR_CODE_SUCCESS
	msg.Description = "acquisition stopped"
	return msg, nil
}
