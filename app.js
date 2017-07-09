Pebble.addEventListener('ready', function(e) {
  Pebble.sendAppMessage({'APP_READY': true});
});

Pebble.addEventListener('appmessage', function(dict) {
  //dict.payload['ACCESS_TOKEN']
  getPmValues();
  //sendResultToPebble({'state': 'pm10: ?'});
});

function xhrWrapper(url, type, data, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(xhr);
  };
  xhr.open(type, url);
  if(data) {
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.send(JSON.stringify(data));
  } else {
    xhr.send();
  }
}

function sendToPebble(json) {
  console.log('pm10Value = ' + Number(json.list[0].pm10Value));
  console.log('pm25Value = ' + Number(json.list[0].pm25Value));
  console.log('pm10Grade = ' + Number(json.list[0].pm10Grade));
  console.log('dataTime = ' + json.list[0].dataTime);

  Pebble.sendAppMessage({
    'PM10_VALUE': Number(json.list[0].pm10Value),
    'PM25_VALUE': Number(json.list[0].pm25Value),
    'PM10_GRADE': Number(json.list[0].pm10Grade),
    'DATA_TIME' : json.list[0].dataTime
  });
}

// new code added for air quality open api
function getPmValues() {
  var data = null;
  var url = 'http://openapi.airkorea.or.kr/openapi/services/rest/ArpltnInforInqireSvc/getMsrstnAcctoRltmMesureDnsty';
  var queryParams = '?';
  queryParams += 'ServiceKey=%%%';
  queryParams += '&' + encodeURIComponent('stationName') + '=' + encodeURIComponent('서초구');
  queryParams += '&' + encodeURIComponent('_returnType') + '=' + encodeURIComponent('json');
  queryParams += '&ver=1.3&dataTerm=month&pageNo=1&numOfRows=1';

  console.log('> Request URL: ' + url + queryParams);
  xhrWrapper(url + queryParams, 'get', data, function(req) {
    console.log(req.response);
    if(req.status == 200) {
      sendToPebble(JSON.parse(req.response));
    }
  });  

//  xhr.onreadystatechange = function () {
//    if (this.readyState == 4) {
//      alert('Status: '+this.status+' Headers: '+JSON.stringify(this.getAllResponseHeaders())+' Body: '+this.responseText);
//    }
//    if(xhr.status == 200) {
//      sendToPebble(JSON.parse(xhr.response));
//    }
//  };
}

