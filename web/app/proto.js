export const Proto = {
	MessageType: undefined,
	MessageWrapper: undefined,
	Error: undefined,
	ErrorCode: undefined,
	Op: undefined,
	OpCode: undefined,
	OpMessageType: undefined,
}

// load protobuf definitions
export function load() {
	return protobuf.load("proto/server.proto")
		.then (root => {
			Proto.MessageWrapper = root.lookupType("beholder.server.MessageWrapper");
			Proto.MessageType = Proto.MessageWrapper.lookupEnum("beholder.server.MessageType");

			Proto.Error = Proto.MessageWrapper.lookupType("beholder.server.Error");
			Proto.ErrorCode = Proto.Error.lookupEnum("beholder.server.ErrorCode");

			Proto.Op = Proto.MessageWrapper.lookupType("beholder.server.Op");
			Proto.OpCode = Proto.Op.lookupEnum("beholder.server.OpCode");
			Proto.OpMessageType = Proto.Op.lookupEnum("beholder.server.OpMessageType");
		});
}

export function verify() {
	switch (true) {
		case !Proto.MessageType:
			throw new Error("MessageType enum not loaded");
		case !Proto.MessageWrapper:
			throw new Error("MessageWrapper type not loaded");
		case !Proto.Error:
			throw new Error("Error type not loaded");
		case !Proto.ErrorCode:
			throw new Error("ErrorCode enum not loaded");
		case !Proto.Op:
			throw new Error("Op type not loaded");
		case !Proto.OpMessageType:
			throw new Error("OpMessageType enum not loaded");
		case !Proto.OpCode:
			throw new Error("OpCode enum not loaded");
	}
}

