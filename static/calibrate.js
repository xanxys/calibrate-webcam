var CalibrationModule = null;

function isPNaClSupported() {
	return navigator.mimeTypes['application/x-pnacl'] !== undefined;
}

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
	if(!isPNaClSupported()) {
		$('#ui_status').empty();
		$('#ui_status').append('PNaCl is not supported on your browser.');
		$('#ui_status').append('<a href="https://google.com/chrome">Get the latest Chrome.</a>');
		return;
	}
});

// jQuery .bind doesn't work.
$('#listener')[0].addEventListener('progress', function(event) {
	// Calculate [0, 1] progress.
	var progress =
		(event.lengthComputable && event.total > 0) ?
		(event.loaded / event.total) :
		0.1;

	$('.progress-bar').css('width', progress * 100 + '%');
}, true);

$('#listener')[0].addEventListener('load', function() {
	console.log('AAA');
	$('#ui_status').text('PNaCl module loaded successfully');
	$('#ui_status').hide();
	$('#ui_target').show();
}, true);

$('#listener')[0].addEventListener('message', function(event) {
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
}, true);

$('#ui_capture').click(function() {
	webcam.capture();
});

$('#ui_calibrate').click(function() {
	CalibrationModule.postMessage({
		"type": "calibrate"
	});
});
