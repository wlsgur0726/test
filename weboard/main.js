var server = require('./app.js').https();
process.on('uncaughtException', function (err) {
	console.error('Caught exception: ' + err);
});
process.on('unhandledRejection', function(reason, p) {
  console.error('unhandledRejection ' + reason + ' ' + p);
});
process.on('rejectionHandled', function(p) {
  console.error('rejectionHandled ' + p);
});
process.on('exit', function(code) {
  console.log('About to exit with code:', code);
});

console.log('start');
server.listen(12345);
