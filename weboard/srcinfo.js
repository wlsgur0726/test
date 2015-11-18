function captureStack() {
	var orig = Error.prepareStackTrace;
	Error.prepareStackTrace = function(_, stack) {
		return stack;
	};
	var err = new Error;
	Error.captureStackTrace(err, arguments.callee);
	var stack = err.stack;
	Error.prepareStackTrace = orig;
	return stack;
};

function StackPointer(index) {
	this.index = index==null ? 1 : index;
}
StackPointer.prototype.function = function() {
	var stack = captureStack();
	return stack[this.index].getFunctionName();
};
StackPointer.prototype.line = function() {
	var stack = captureStack();
	return stack[this.index].getLineNumber();
};
StackPointer.prototype.caller = function() {
	return new StackPointer(this.index + 1);
};

exports.function = function() {
	var s = new StackPointer(2);
	return s.function();
};
exports.line = function() {
	var s = new StackPointer(2);
	return s.line();
};
exports.caller = function() {
	return new StackPointer(2);
}