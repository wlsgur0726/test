var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');

var app = express();
var http = require('http');
var https = require('https');
var fs = require('fs');
var CORS = require('cors')();
var sessionManager = require("./redis").sessionManager();

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');

// uncomment after placing your favicon in /public
app.use(favicon(path.join(__dirname, 'public', 'favicon.ico')));

app.use(logger('dev'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

app.use(CORS);
app.use(sessionManager);
app.use("/", express.Router().get("/",function(req, res){
	res.redirect("/board.html");
}));
app.use("/auth", require("./routes/auth"));
app.use("/content-list", require("./routes/content-list"));
app.use("/contents", require("./routes/contents"));

// catch 404 and forward to error handler
app.use(function(req, res, next) {
  var err = new Error('Not Found');
  err.status = 404;
  next(err);
});

// error handlers

// development error handler
// will print stacktrace
if (app.get('env') === 'development') {
  app.use(function(err, req, res, next) {
	console.error(err.stack);
    res.status(err.status || 500);
    res.render('error', {
      message: err.message,
      error: err
    });
  });
}

// production error handler
// no stacktraces leaked to user
app.use(function(err, req, res, next) {
  res.status(err.status || 500);
  res.render('error', {
    message: err.message,
    error: {}
  });
});


exports.https = function() {
	var options = {
	  key: fs.readFileSync('key.pem'),
	  cert: fs.readFileSync('cert.pem')
	};
	return https.createServer(options, app);
};
exports.http = function() {
	return http.createServer(app);
}