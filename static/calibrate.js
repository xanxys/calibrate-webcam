CalibrationModule = null;  // Global application object.
statusText = 'NO-STATUS';

// Indicate load success.
function moduleDidLoad() {
	CalibrationModule = document.getElementById('hello_tutorial');
	updateStatus('SUCCESS');
}

function handleMessage(event) {
	var type = String(event.data.type);
	if(type === 'debug') {
		console.log('PNaCl module:', String(event.data.message));
	} else if(type === 'calibration') {
		console.log('Calibration result', event.data);
	} else if(type === 'image_result') {
		$('#result').append($('<img/>').attr('src', event.data.image_url));
	} else {
		console.log('Unknown message from PNaCl module:', event);
	}
}



// Set the global status message.  If the element with id 'statusField'
// exists, then set its HTML to the status message as well.
// opt_message The message test.  If this is null or undefined, then
// attempt to set the element with id 'statusField' to the value of
// |statusText|.
function updateStatus(opt_message) {
  if (opt_message)
    statusText = opt_message;
  var statusField = document.getElementById('statusField');
  if (statusField) {
    statusField.innerHTML = statusText;
  }
}

var listener = document.getElementById('listener');
listener.addEventListener('load', moduleDidLoad, true);
listener.addEventListener('message', handleMessage, true);

var pos = 0;

var canvas = document.createElement("canvas");
canvas.setAttribute('width', 320);
canvas.setAttribute('height', 240);

var img = canvas.getContext("2d").getImageData(0, 0, 320, 240);

$("#video").webcam({
	width: 320,
	height: 240,
	mode: "callback",
	swffile: "jscam_canvas_only.swf",
	onTick: function() {},
	onSave: function(data) {
		var col = data.split(";");

		for(var i = 0; i < 320; i++) {
			var tmp = parseInt(col[i]);
			img.data[pos + 0] = (tmp >> 16) & 0xff;
			img.data[pos + 1] = (tmp >> 8) & 0xff;
			img.data[pos + 2] = tmp & 0xff;
			img.data[pos + 3] = 0xff;
			pos += 4;
		}

		if(pos >= 4 * 320 * 240) {
			console.log('Image size(data URL): ', canvas.toDataURL().length);
			canvas.getContext('2d').putImageData(img, 0, 0);

			CalibrationModule.postMessage({
				"type": "image",
				"data": canvas.toDataURL()
			});

			pos = 0;
		}
	},
	onCapture: function() {
		webcam.save();
	},
	debug: function() {},
	onLoad: function() {}
});

$(document).ready(function() {
	if (CalibrationModule == null) {
		updateStatus('LOADING...');
	} else {
		updateStatus();
	}
});


$('#ui_capture').click(function() {
	webcam.capture();
});

$('#ui_calibrate').click(function() {
	CalibrationModule.postMessage({
		"type": "calibrate"
	});
});
