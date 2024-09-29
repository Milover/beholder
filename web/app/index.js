let conn;				// global WebSocket resource
let acquiring = false;	// global acquisition state

let MessageType;		// global protobuf message type
let MessageWrapper;		// global protobuf message wrapper

const localTime = new Intl.DateTimeFormat(navigator.language, {
	timeStyle: "long",
	dateStyle: "short",
	hour12: false,
})

// Extract timestamp from a UUIDv7.
// Taken from: https://park.is/blog_posts/20240803_extracting_timestamp_from_uuid_v7/
function uuidv7Timestamp(uuid) {
	const parts = uuid.split("-")
	const highBitsHex = parts[0] + parts[1].slice(0, 4)

	const timeMs = parseInt(highBitsHex, 16)
	return new Date(timeMs)
}

const acqImgContainer = document.getElementById("acq-img-container")
const acqStartButton = document.getElementById("acq-start-button")
const acqStopButton = document.getElementById("acq-stop-button")

// Toggle the disabled state of acquisition start/stop buttons.
function toggleAcqButtons(acquiring) {
	if (acquiring) {
		acqStartButton.disabled = true
		acqStopButton.disabled = false
	} else {
		acqStartButton.disabled = false
		acqStopButton.disabled = true
	}
}

// Start image acquisition via POST request if the WebSocket is ready to
// receive data.
async function startAcquisition() {
	if (conn && conn.readyState === WebSocket.OPEN) {
		// if we're already acquiring, there's nothing to do
		if (acquiring) {
			return
		}
		const resp = await fetch("/acquisition-start", {
			method: "POST",
		})
		if (resp.status !== 200) {
			throw new Error(`Unexpected HTTP status: ${resp.status} ${resp.statusText}`)
		}
		// everything OK, disable/enable start/stop buttons
		acquiring = true
		toggleAcqButtons(acquiring)

		console.log("Image acquisition started")
		// send messages to the server
		// ws.send("startImageAcquisition")
	} else {
		throw new Error("WebSocket not ready, cannot start image acquisition")
	}
}

// Stop image acquisition via POST request.
async function stopAcquisition() {
	// if we're not acquiring, there's nothing to do
	if (!acquiring) {
		return
	}
	const resp = await fetch("/acquisition-stop", {
		method: "POST",
	})
	if (resp.status !== 200) {
		throw new Error(`Unexpected HTTP status: ${resp.status} ${resp.statusText}`)
	}
	// everything OK, disable/enable start/stop buttons
	acquiring = false
	toggleAcqButtons(acquiring)

	console.log("Image acquisition stopped")
}

// handleMessage decodes a protobuf-encoded message and does stuff with the data.
function handleMessage(binaryData) {
	switch (true) {
		case !MessageType:
			console.error("MessageType type not loaded")
			return
		case !MessageWrapper:
			console.error("MessageWrapper type not loaded")
			return
	}
	const msg = MessageWrapper.decode(new Uint8Array(binaryData))
	// TODO: handle different message types
	if (msg.header.type !== MessageType.values.MESSAGE_TYPE_IMAGE) {
		console.error(`Unsupported message type: ${msg.header.type}`)
		return
	}
	// TODO: detect image MIME type
	const blob = new Blob([msg.image.raw], { type: `image/${msg.image.mime}` });
	const imageURL = URL.createObjectURL(blob)
	const time = localTime.format(uuidv7Timestamp(msg.header.uuid))

	// set the image source
	document.getElementById("acq-img").src = imageURL
	// display image overlay text
	document.getElementById("acq-img-timestamp").textContent = `time: ${time}`
	document.getElementById("acq-img-uuid").textContent = `uuid: ${msg.header.uuid}`
	document.getElementById("acq-img-source").textContent = `src: ${msg.image.source}`
	// show the container
	acqImgContainer.style.display = "block"
}

// Set up the WebSocket as soon as the page loads.
window.addEventListener("load", () => {
	conn = new WebSocket(`ws://${location.host}/stream`)
	conn.binaryType = "arraybuffer" // because of protobuf stuff

	// load protobuf definitions and start handling messages
	try {
		protobuf.load("proto/server.proto", (err, root) => {
			if (err) {
				throw err
			}
			// cache the protobuf types
			MessageType = root.lookupEnum("beholder.server.MessageType")
			MessageWrapper = root.lookup("beholder.server.MessageWrapper")

			// handle incomming messages
			conn.addEventListener("message", ev => {
				console.log("got mesage event")
				handleMessage(ev.data)
			})
		})
	} catch(err) {
		console.error("Failed to load .proto file:", err)
	}
	// handle connection closure
	conn.addEventListener("close", ev => {
		console.log(`WebSocket connection closed: ${ev.code}, reason: ${ev.reason}`)
	})
	// handle connection error
	conn.addEventListener("error", ev => {
		console.error(`WebSocket error: ${ev}`)
	})
//	conn.addEventListener("open", ev => {
//		console.log("WebSocket connection established")
//	})
	// everything OK, enable/disable start/stop buttons
	acquiring = false
	toggleAcqButtons(acquiring)

	console.log("WebSocket connection established")
})

// Close the WebSocket and stop acquisition before page unload.
window.addEventListener("beforeunload", () => {
	try {
		stopAcquisition()
		if (conn) {
			conn.close()
			console.log("WebSocket connection closed due to page unload")
		}
	} catch(err) {
		console.error(`Failed to stop acquisition: ${err.message}`)
	}
})

// Start image acquisition when acqStartButton is clicked.
acqStartButton.onclick = async ev => {
	ev.preventDefault()
	try {
		startAcquisition()
	} catch(err) {
		console.error(`Failed to start acquisition: ${err.message}`)
	}
}

// Stop image acquisition when acqStopButton is clicked.
acqStopButton.onclick = async ev => {
	ev.preventDefault()
	try {
		stopAcquisition()
	} catch(err) {
		console.error(`Failed to stop acquisition: ${err.message}`)
	}
}
