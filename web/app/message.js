// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

import { Proto } from "/proto.js";
import { handleResponse as handleServiceResponse } from "/service.js";

const localTime = new Intl.DateTimeFormat(navigator.language, {
	timeStyle: "long",
	dateStyle: "short",
	hour12: false,
});

// Extract timestamp from a UUIDv7.
// Taken from: https://park.is/blog_posts/20240803_extracting_timestamp_from_uuid_v7/
function uuidv7Timestamp(uuid) {
	const parts = uuid.split("-");
	const highBitsHex = parts[0] + parts[1].slice(0, 4);

	const timeMs = parseInt(highBitsHex, 16);
	return new Date(timeMs);
}

// handle decodes an incomming protobuf message and processes it.
export function handle(binaryData) {
	let uuid;
	let type;
	try {
		const msg = Proto.MessageWrapper.decode(new Uint8Array(binaryData))
		if (!msg.header.type || !msg.header.uuid) {
			throw new Error(`Bad message`);
		}
		uuid = msg.header.uuid;
		type = msg.header.type;
		//console.log(`Received message: uuid=${uuid} type=${type}`);

		// TODO: handle different message types
		switch (msg.header.type) {
			case Proto.MessageType.values.MESSAGE_TYPE_ERROR:
				handleError(msg);
				break;
			case Proto.MessageType.values.MESSAGE_TYPE_IMAGE:
				handleImage(msg);
				break;
			case Proto.MessageType.values.MESSAGE_TYPE_OP:
				handleServiceResponse(msg);
				break;
			default:
				throw new Error(`Unsupported message`)
		}
	} catch(err) {
		console.error(`Error handling message: uuid=${uuid} type=${type} error=${err.message}`);
	}
}

// Handle an incomming error message.
function handleError(msg) {
	const err = msg.error;
	const uuid = msg.header.uuid;

	console.error(`Received error: uuid=${uuid} code=${err.code} description=${err.description}`);
}

// Handle an incomming image message.
function handleImage(msg) {
	const blob = new Blob([msg.image.raw], { type: `image/${msg.image.mime}` });
	const imageURL = URL.createObjectURL(blob);
	const uuid = msg.header.uuid; // TODO: change to msg.image.uuid when implemented
	const time = localTime.format(uuidv7Timestamp(uuid));
	const source = msg.image.source;

	console.log(`Received image: uuid=${uuid} time=${time} source=${source}`);
	// set the image source
	document.getElementById("acq-img").src = imageURL;
	// display image overlay text
	document.getElementById("acq-img-timestamp").textContent = `time: ${time}`;
	document.getElementById("acq-img-uuid").textContent = `uuid: ${uuid}`;
	document.getElementById("acq-img-source").textContent = `src: ${source}`;
	// show the container
	document.getElementById("acq-img-container").style.display = "block";
}
