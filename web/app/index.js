// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

import * as proto from "/proto.js";
import * as service from "/service.js";
import * as message from "/message.js";

const protoLoad = proto.load();

// global state
const state = {
	conn: undefined,
	serviceTimeout: 5000,	// service start to finalize timeout
};

// Set up the WebSocket as soon as the page loads.
window.addEventListener("load", async () => {
	state.conn = new WebSocket(`ws://${location.host}/stream`);
	state.conn.binaryType = "arraybuffer"; // because of protobuf stuff
	console.log(`WebSocket connection established: host=${location.host}`);

	// handle connection closure
	state.conn.addEventListener("close", ev => {
		console.log(`WebSocket connection closed: ${ev.code}, reason: ${ev.reason}`);
	});
	// handle connection error
	state.conn.addEventListener("error", ev => {
		console.error(`WebSocket error: ${ev}`);
	});
//	conn.addEventListener("open", ev => {
//		console.log("WebSocket connection established")
//	})
	try {
		// wait for the protobufs to finish loading
		await protoLoad;
		proto.verify();

		// start handling incomming messages
		state.conn.addEventListener("message", ev => {
			message.handle(ev.data);
		});

		// Install service event listeners
		service.install(state);
		console.log("Installed service listeners");
	} catch (err) {
		console.error(err);
	}
});

// Close the WebSocket and stop acquisition before page unload.
window.addEventListener("beforeunload", (ev) => {
	try {
		service.shutdown(ev);
	} catch (err) {
		console.error(`error: ${err.message}`);
	}
});
