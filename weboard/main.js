var app = require('./app.js');
process.on('uncaughtException', function (err) {
	console.log('Caught exception: ' + err);
});
process.on('exit', function(code) {
  console.log('About to exit with code:', code);
});
process.on('unhandledRejection', function(reason, p) {
  console.log('unhandledRejection ' + reason + ' ' + p);
});
process.on('rejectionHandled', function(p) {
  console.log('rejectionHandled ' + p);
});

console.log('start');
app.listen(12345);
