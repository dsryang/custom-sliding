Pebble.addEventListener('ready', function() {
	console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
	var url = 'http://dsryang.github.io/custom-sliding/config/index.html';

	console.log('Showing configuration page: ' + url);

	Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
	var configData = JSON.parse(decodeURIComponent(e.response));

	console.log('Configuraton page returned: ' + JSON.stringify(configData));

	if (configData.lightColor) {
		Pebble.sendAppMessage({
			lightColor: parseInt(configData.lightColor, 16),
			darkColor: parseInt(configData.darkColor, 16),
			timeColor: parseInt(configData.timeColor, 16),
			dateColor: parseInt(configData.dateColor, 16)
		}, function() {
			console.log('Send successful!');
		}, function() {
			console.log('Send failed!');
		});
	}
});