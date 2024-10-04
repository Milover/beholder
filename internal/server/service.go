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

	// ErrPermissions signals that the request issuer does not have
	// permission to use the service.
	ErrPermissions = errors.New("insufficient permissions")
)

// defaultServiceMessage creates a new default service response payload.
func defaultServiceMessage() *pb.Error {
	return &pb.Error{
		Code:        pb.ErrorCode_ERROR_CODE_FAIL,
		Description: ErrMsg.Error(),
	}
}

// TODO: docs
type Service interface {
	// HasPermissions returns true if a connection with the given role can
	// use the service.
	HasPermissions(ConnectionRole) bool
	// Execute runs the service with the provided op-message payload and
	// returns an op-message response payload.
	//
	// The returned message must always be a valid op-message payload,
	// even if there is an error.
	//
	// TODO: would be nicer if Execute took and returned an op-message
	// payload directly.
	Execute(proto.Message, ConnectionRole, *AcquisitionServer) (proto.Message, error)
}

// A ServiceGenerator creates a new Service ready for use.
//
// [ServiceGenerator.New] should return a Service which must be safe
// for concurrent use.
type ServiceGenerator interface {
	New() Service
}

// A ServiceGeneratorFunc type is an adapter to allow the use of ordinary
// functions as service generators. If f is a function with the appropriate
// signature, ServiceGeneratorFunc(f) is a [ServiceGenerator] that calls f.
type ServiceGeneratorFunc func() Service

// New calls f().
func (f ServiceGeneratorFunc) New() Service {
	return f()
}

// services is a global map of service generators for services which
// the [AcquisitionServer] provides.
var services = map[pb.OpCode]ServiceGenerator{
	pb.OpCode_OP_CODE_START_ACQUISITION: ServiceGeneratorFunc(newStartAcquisitionService),
	pb.OpCode_OP_CODE_STOP_ACQUISITION:  ServiceGeneratorFunc(newStopAcquisitionService),
}

// A startAcquisitionService is a [Service] which starts acquisition by calling
// [AcquisitionServer.startAcquisition], if the request is valid, and
// generates a response message.
type startAcquisitionService struct{}

// newStartAcquisitionService is a [ServiceGeneratorFunc] which
// creates a new startAcquisitionService.
func newStartAcquisitionService() Service {
	return &startAcquisitionService{}
}

func (srv *startAcquisitionService) HasPermissions(r ConnectionRole) bool {
	return r == RoleController
}

func (srv *startAcquisitionService) Execute(
	m proto.Message,
	role ConnectionRole,
	s *AcquisitionServer,
) (proto.Message, error) {
	msg := defaultServiceMessage()

	op, ok := m.(*pb.Op)
	if !ok {
		return msg, fmt.Errorf("server.startAcquisitionService: %w: bad type assertion", errSanity)
	}
	if op.Payload != nil {
		return msg, fmt.Errorf("server.startAcquisitionService: %w", errOpPayload)
	}
	// permission check
	if !srv.HasPermissions(role) {
		msg.Description = ErrPermissions.Error()
		return msg, ErrNoDistribute
	}
	// execute
	if err := s.startAcquisition(); err != nil {
		msg.Description = err.Error()
		return msg, ErrNoDistribute
	}
	msg.Code = pb.ErrorCode_ERROR_CODE_SUCCESS
	msg.Description = "acquisition started"
	return msg, nil

}

// A stopAcquisitionService is a [Service] which stops acquisition by calling
// [AcquisitionServer.stopAcquisition], if the request is valid, and
// generates a response message.
type stopAcquisitionService struct{}

// newStopAcquisitionService is a [ServiceGeneratorFunc] which
// creates a new startAcquisitionService.
func newStopAcquisitionService() Service {
	return &stopAcquisitionService{}
}

func (srv *stopAcquisitionService) HasPermissions(r ConnectionRole) bool {
	return r == RoleController
}

func (srv *stopAcquisitionService) Execute(
	m proto.Message,
	role ConnectionRole,
	s *AcquisitionServer,
) (proto.Message, error) {
	msg := defaultServiceMessage()

	op, ok := m.(*pb.Op)
	if !ok {
		return msg, fmt.Errorf("server.stopAcquisitionService: %w: bad type assertion", errSanity)
	}
	if op.Payload != nil {
		return msg, fmt.Errorf("server.stopAcquisitionService: %w", errOpPayload)
	}
	// permission check
	if !srv.HasPermissions(role) {
		msg.Description = ErrPermissions.Error()
		return msg, ErrNoDistribute
	}
	// execute
	if err := s.stopAcquisition(); err != nil {
		msg.Description = err.Error()
		return msg, ErrNoDistribute
	}
	msg.Code = pb.ErrorCode_ERROR_CODE_SUCCESS
	msg.Description = "acquisition stopped"
	return msg, nil
}
