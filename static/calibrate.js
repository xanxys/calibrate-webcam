HelloTutorialModule = null;  // Global application object.
statusText = 'NO-STATUS';

// Indicate load success.
function moduleDidLoad() {
  HelloTutorialModule = document.getElementById('hello_tutorial');
  updateStatus('SUCCESS');

  HelloTutorialModule.postMessage('Hello PNaCl');
}

// The 'message' event handler.  This handler is fired when the NaCl module
// posts a message to the browser by calling PPB_Messaging.PostMessage()
// (in C) or pp::Instance.PostMessage() (in C++).  This implementation
// simply displays the content of the message in an alert panel.
function handleMessage(message_event) {
	$('#result').append($('<img/>').attr('src', message_event.data.image_url));
}

// If the page loads before the Native Client module loads, then set the
// status message indicating that the module is still loading.  Otherwise,
// do not change the status message.
function pageDidLoad() {
  if (HelloTutorialModule == null) {
    updateStatus('LOADING...');
  } else {
    // It's possible that the Native Client module onload event fired
    // before the page's onload event.  In this case, the status message
    // will reflect 'SUCCESS', but won't be displayed.  This call will
    // display the current message.
    updateStatus();
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

			HelloTutorialModule.postMessage({
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

$('#ui_capture').click(function() {
	webcam.capture();
});
