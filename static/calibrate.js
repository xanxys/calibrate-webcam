function isPNaClSupported() {
	return navigator.mimeTypes['application/x-pnacl'] !== undefined;
}


var WebcamCalibrator = function() {
	// model
	this.num_good = 0;

	// view
	$.toast.config.width = 400;
	$.toast.config.align = 'left';

	this._initCapture(320, 240);
	this._bind();
};

WebcamCalibrator.prototype._initCapture = function(width, height) {
	var _this = this;

	var pos = 0;

	var canvas = document.createElement("canvas");
	canvas.setAttribute('width', width);
	canvas.setAttribute('height', height);

	var img = canvas.getContext("2d").getImageData(0, 0, width, height);

	$("#video").webcam({
		width: width,
		height: height,
		mode: "callback",
		swffile: "jscam_canvas_only.swf",
		onTick: function() {},
		onSave: function(data) {
			var col = data.split(";");

			for(var i = 0; i < width; i++) {
				var tmp = parseInt(col[i]);
				img.data[pos + 0] = (tmp >> 16) & 0xff;
				img.data[pos + 1] = (tmp >> 8) & 0xff;
				img.data[pos + 2] = tmp & 0xff;
				img.data[pos + 3] = 0xff;
				pos += 4;
			}

			if(pos >= 4 * width * height) {
				console.log('Image size(data URL): ', canvas.toDataURL().length);
				canvas.getContext('2d').putImageData(img, 0, 0);

				_this._sendCommand('process_image', {
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
};

WebcamCalibrator.prototype._bind = function() {
	var _this = this;

	$(document).ready(function() {
		if(!isPNaClSupported()) {
			$('#ui_status').empty();
			$('#ui_status').append('PNaCl is not supported on your browser. ');
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
		$('#ui_status').text('PNaCl module loaded successfully');
		$('#ui_status').hide();
		$('#ui_target').show('slide');

		$('#ui_capture').removeClass('disabled');
	}, true);

	$('#listener')[0].addEventListener('message', function(event) {
		var type = String(event.data.type);
		if(type === 'debug') {
			console.log('PNaCl module:', String(event.data.message));
		} else if(type === 'calibration') {
			_this._onCalibration(event.data);
		} else if(type === 'image_result') {
			_this._onImageResult(event.data);
		} else {
			console.log('Unknown message from PNaCl module:', event);
		}
	}, true);

	$('#ui_capture').click(function() {
		webcam.capture();
	});

	$('#ui_calibrate').click(function() {
		$('#ui_capture').hide();
		$('#ui_calibrate').hide();
		_this._sendCommand('calibrate', {});
	});

	$('#ui_more').click(function() {
		$('#ui_capture').show();
		$('#ui_calibrate').show();

		$('#ui_result').hide();
		$('#ui_target').show('slide');
	});
};

WebcamCalibrator.prototype._sendCommand = function(command, args) {
	var message = _.clone(args);
	message['type'] = command;

	$('#calibration_module')[0].postMessage(message);
};

WebcamCalibrator.prototype._onCalibration = function(data) {
	console.log('Calibration result', data);
	$('#ui_target').hide();

	$('#ui_reproj_error').text(data.error);
	$('#ui_result_json').text(JSON.stringify(data.intrinsic, null, 2));
	$('#ui_result').show('slide');
};

WebcamCalibrator.prototype._onImageResult = function(data) {
	if(data.success) {
		$.toast('Captured!', {
			type: 'success',
			duration: 1500,
		});
		this.num_good += 1;
		$('#ui_num_good').text(this.num_good);
	} else {
		$.toast("Couldn't detect the chessboard.", {
			type: 'danger',
			duration: 5000,
		});
	}

	$('#result').append(
		$('<img/>')
			.attr('src', data.image_url)
			.attr('width', '80px'));
};

var wc = new WebcamCalibrator();
