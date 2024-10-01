package server

import (
	"fmt"

	pb "github.com/Milover/beholder/internal/server/proto"
	"google.golang.org/protobuf/proto"
)

// NOTE: should take an Op.Payload and return a new Op.Payload as a response
type Service func(proto.Message, *AcquisitionServer) (proto.Message, error)

// services is a global map of services [AcquisitionServer] provides.
var services = map[pb.OpCode]Service{
	pb.OpCode_OP_CODE_START_ACQUISITION: startAcquisitionService,
	pb.OpCode_OP_CODE_STOP_ACQUISITION:  stopAcquisitionService,
}

func startAcquisitionService(m proto.Message, s *AcquisitionServer) (proto.Message, error) {
	op, ok := m.(*pb.Op)
	if !ok {
		return nil, fmt.Errorf("server.startAcquisitionService: %w: bad type assertion", errSanity)
	}
	if op.Payload != nil {
		return nil, fmt.Errorf("server.startAcquisitionService: %w", errOpPayload)
	}
	s.startAcquisition()

	return &pb.Error{
		Code:        pb.ErrorCode_ERROR_CODE_SUCCESS,
		Description: "acquisition started",
	}, nil
}

func stopAcquisitionService(m proto.Message, s *AcquisitionServer) (proto.Message, error) {
	op, ok := m.(*pb.Op)
	if !ok {
		return nil, fmt.Errorf("server.stopAcquisitionService: %w: bad type assertion", errSanity)
	}
	if op.Payload != nil {
		return nil, fmt.Errorf("server.stopAcquisitionService: %w", errOpPayload)
	}
	s.stopAcquisition()

	return &pb.Error{
		Code:        pb.ErrorCode_ERROR_CODE_SUCCESS,
		Description: "acquisition stopped",
	}, nil
}
