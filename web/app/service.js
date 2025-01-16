// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

import { uuidv7 } from "https://unpkg.com/uuidv7@^1";
import { Proto } from "/proto.js"

const acqSwitch = document.getElementById("acq-sw-checkbox");

// A map of services keyed by opcode.
const services = [];
const servicesMap = new Map();

// Install services.
export function install(state) {
	// register services
	services.push(new StartAcquisition(state));
	services.push(new StopAcquisition(state));

	// build the map
	services.forEach((s) => {
		servicesMap.set(s.opcode, s);
	});

	// install listeners
	acqSwitch.onclick = ev => {
		if (acqSwitch.checked) {
			servicesMap.get(Proto.OpCode.values.OP_CODE_START_ACQUISITION).newRequest(ev);
		} else {
			servicesMap.get(Proto.OpCode.values.OP_CODE_STOP_ACQUISITION).newRequest(ev);
		}
	};
}

// Shutdown installed services.
export function shutdown(ev) {
	services.forEach((s) => {
		s.shutdown(ev);
	});
}

// Handle an incomming response op-message.
export function handleResponse(msg) {
	const opcode = msg.op.header.code;
	const uuid = msg.header.uuid;

	console.log(`Received op: uuid=${uuid} opcode=${opcode}`);
	// find an execute the service
	const service = servicesMap.get(opcode);
	if (!service) {
		throw new Error(`Unsupported op: opcode=${opcode}`);
	}
	service.handleResponse(msg.op);
}

// Service is an interface which implements methods needed to send service
// request messages and receive and process response messages.
class Service {
	constructor(opcode, state) {
		if (new.target === Service) {
			throw new TypeError("Cannot construct Service instances directly (interface)");
		}
		this.opcode = opcode;
		this.conn = state.conn;			// WebSocket connection
		this.timeout = state.serviceTimeout;
		this.timeoutID = undefined;		// id of the response timeout function.
	}

	// Create a new op-message request.
	// This is the actuall 'entry-point' into the service.
	newRequest(ev) {
		try {
			const obj = {
				header: {
					uuid: uuidv7(),
					type: Proto.MessageType.values.MESSAGE_TYPE_OP,
				},
				op: {
					header: {
						type: Proto.OpMessageType.values.OP_MESSAGE_TYPE_REQUEST,
						code: this.opcode,
					},
					payload: this.handle(ev),
				},
			};
			// setup the rollback
			this.timeoutID = setTimeout(() => {
				this.rollback();
				this.timeoutID = undefined;
				console.error(`Service timed out: uuid=${obj.header.uuid} opcode=${this.opcode}`);
			}, this.timeout);

			// encode and send the message
			const msg = Proto.MessageWrapper.create(obj)
			const buffer = Proto.MessageWrapper.encode(msg).finish();
			if (!this.conn || this.conn.readyState !== WebSocket.OPEN) {
				throw new Error("WebSocket not ready, cannot send data");
			}
			this.conn.send(buffer);
			console.log(`Sending request: uuid=${obj.header.uuid} opcode=${this.opcode}`);
		} catch(err) {
			this.clearRollback()
			this.rollback();
			console.error(`Error creating request: ${err.message} opcode=${this.opcode}`);
		}
	}

	// Handle an op-message response.
	handleResponse(msg) {
		try {
			// clear the request teardown.
			if (this.timeoutID) {
				clearTimeout(this.timeoutID);
				this.timeoutID = undefined;
			}
			// check the response
			if (msg.header.type !== Proto.OpMessageType.values.OP_MESSAGE_TYPE_RESPONSE) {
				throw new Error(`Unexpected op-message type: optyp=${msg.header.type}`);
			}
			if (msg.header.code !== this.opcode) {
				throw new Error(`Unexpected op: opcode=${msg.header.code}`);
			}
			// process response data and finalize
			this.finish(msg);
		} catch(err) {
			this.rollback();
			throw err; // the caller should handle/report the error
		}
	}

	// Shutdown the service, no-op by default.
	shutdown(ev) {}

	// Handle the event which started the service.
	handle(ev) {
		throw new Error("Method 'handle()' not implemented");
	}

	// Rollback the changes made when the service event was handled, that is
	// revert the effects of handle().
	rollback() {
		throw new Error("Method 'rollback()' not implemented");
	}

	// Process the response message and finalize the service.
	// The supplied message is guaranteed to be an op-message with a valid header.
	// NOTE: finish should throw if there is an error.
	finish(msg) {
		throw new Error("Method 'finish()' not implemented");
	}

	// Clear the rollback if it was set up.
	clearRollback() {
		if (this.timeoutID) {
			clearTimeout(this.timeoutID);
			this.timeoutID = undefined;
		}
	}
}

// StartAcquisition is a service which requests an image acquisition start
// from the server.
class StartAcquisition extends Service {
	constructor(state) {
		super(Proto.OpCode.values.OP_CODE_START_ACQUISITION, state);
	}

	handle(ev) {
		acqSwitch.disabled = true;	// no more clicking until we're done
		acqSwitch.checked = true;	// enforce DOM state
	}

	rollback() {
		acqSwitch.checked = false;	// enforce DOM state
		acqSwitch.disabled = false;
	}

	finish(msg) {
		if (msg.error.code !== Proto.ErrorCode.values.ERROR_CODE_SUCCESS) {
			throw new Error(`Service error: ${msg.error.description} code=${msg.error.code}`);
		}
		acqSwitch.checked = true;	// enforce DOM state
		acqSwitch.disabled = false;
		console.log(`Service success: ${msg.error.description}`);
	}
}

// StopAcquisition is a service which requests an image acquisition stop
// from the server.
class StopAcquisition extends Service {
	constructor(state) {
		super(Proto.OpCode.values.OP_CODE_STOP_ACQUISITION, state);
	}

	handle(ev) {
		acqSwitch.disabled = true;	// no more clicking until we're done
		acqSwitch.checked = false;	// enforce DOM state
	}

	rollback() {
		acqSwitch.checked = true;	// enforce DOM state
		acqSwitch.disabled = false;
	}

	finish(msg) {
		if (msg.error.code !== Proto.ErrorCode.values.ERROR_CODE_SUCCESS) {
			throw new Error(`Service error: ${msg.error.description} code=${msg.error.code}`);
		}
		acqSwitch.checked = false;	// enforce DOM state
		acqSwitch.disabled = false;
		console.log(`Service success: ${msg.error.description}`);
	}

	shutdown(ev) {
		this.newRequest(ev);
		this.clearRollback();
	}
}
