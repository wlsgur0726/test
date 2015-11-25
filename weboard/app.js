var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');
var uuid = require('node-uuid');

var routes = require('./routes/index');
var users = require('./routes/users');

var app = express();
var http = require('http');
var https = require('https');
var fs = require('fs');
var CORS = require('cors')();
var sessionManager = require("./redis").sessionManager();
var _this = require("./srcinfo");

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
app.use('/', routes);
app.use('/users', users);
app.use('/auth', require("./routes/auth"));
app.use('/content-list', require("./routes/content-list"));
app.use('/contents', require("./routes/contents"));

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
    res.status(err.status || 500);
    res.render('error', {
      message: err.message,
      error: err
    });
	console.error("STATUS : " + err.status);
	console.error(JSON.stringfy(err));
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

var options = {
  key: fs.readFileSync('key.pem'),
  cert: fs.readFileSync('cert.pem')
};

//module.exports = https.createServer(options, app);
module.exports = http.createServer(app);