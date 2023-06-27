// Decode an uplink message from a buffer
// payload - array of bytes
// metadata - key/value object

/** Decoder **/

var rawDataStr = decodeToString(payload);

// decode payload to JSON
var data = decodeToJson(payload);
var decPayload = data.uplink_message.decoded_payload;

var deviceName = data.end_device_ids.device_id;
var deviceType = data.end_device_ids.application_ids.application_id;

// use assetName and assetType instead of deviceName and deviceType
// to automatically create assets instead of devices.
// var assetName = 'Asset A';
// var assetType = 'building';

// Result object with device/asset attributes/telemetry data
var result = {
// Use deviceName and deviceType or assetName and assetType, but not both.
   deviceName: deviceName,
   deviceType: deviceType,
// assetName: assetName,
// assetType: assetType,
// customerName: customerName,
   telemetry: {
       owTemp: decPayload['owTemp'],
       moisture: decPayload['moist'],
       bmeTemp: decPayload['bmeTemp'],
       bmePressure: decPayload['bmePress'],
       bmeHumidity: decPayload['bmeHumid'],
       humidity: 80,
       rawData: rawDataStr
   }
};

/** Helper functions **/

function decodeToString(payload) {
   return String.fromCharCode.apply(String, payload);
}

function decodeToJson(payload) {
   // covert payload to string.
   var str = decodeToString(payload);

   // parse string to JSON
   var data = JSON.parse(str);
   return data;
}

return result;
