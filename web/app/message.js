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
	try {
		const msg = Proto.MessageWrapper.decode(new Uint8Array(binaryData))
		if (!msg.header.type || !msg.header.uuid) {
			throw new Error(`Bad message header`);
		}
		console.log(`Received message: type=${msg.header.type} uuid=${msg.header.uuid}`);

		// TODO: handle different message types
		switch (msg.header.type) {
			case Proto.MessageType.values.MESSAGE_TYPE_ERROR:
				handleError(msg)
				break;
			case Proto.MessageType.values.MESSAGE_TYPE_IMAGE:
				handleImage(msg)
				break;
			case Proto.MessageType.values.MESSAGE_TYPE_OP:
				handleServiceResponse(msg.op.header.code, msg.op)
				break;
			default:
				console.error(`Unsupported message type: ${msg.header.type} uuid=${msg.header.uuid}`)
		}
	} catch(err) {
		console.error(`Error handling message: ${err.message}`)
	}
}

// Handle an incomming error message.
function handleError(msg) {
	const err = msg.error
	console.error(`Received error message: description=${err.description} code=${err.code}`)
}

// Handle an incomming image message.
function handleImage(msg) {
	const blob = new Blob([msg.image.raw], { type: `image/${msg.image.mime}` });
	const imageURL = URL.createObjectURL(blob);
	const time = localTime.format(uuidv7Timestamp(msg.header.uuid));

	// set the image source
	document.getElementById("acq-img").src = imageURL;
	// display image overlay text
	document.getElementById("acq-img-timestamp").textContent = `time: ${time}`;
	document.getElementById("acq-img-uuid").textContent = `uuid: ${msg.header.uuid}`;
	document.getElementById("acq-img-source").textContent = `src: ${msg.image.source}`;
	// show the container
	document.getElementById("acq-img-container").style.display = "block";
}
