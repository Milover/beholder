let conn;				// global WebSocket resource
let acquiring = false;	// global acquisition state

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

// Set up the WebSocket as soon as the page loads.
window.addEventListener("load", () => {
	conn = new WebSocket(`ws://${location.host}/stream`)

	// handle incomming messages
	conn.addEventListener("message", ev => {
		console.log("got mesage event")
		// create image binary blob
		const blob = new Blob([ev.data], {type: "image/jpeg"})
		// create an URL for the blob and set it as the image source
		const imgElement = document.getElementById("acq-img")
		imgElement.src = URL.createObjectURL(blob)
		// show the image
		acqImgContainer.style.display = "block"
	})
	conn.addEventListener("close", ev => {
		console.log(`WebSocket connection closed: ${ev.code}, reason: ${ev.reason}`)
	})
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
