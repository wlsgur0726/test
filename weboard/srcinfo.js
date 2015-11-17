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

exports.function = function() {
	var stack = captureStack();
	return stack[1].getFunctionName();
}

exports.line = function() {
	var stack = captureStack();
	return stack[1].getLineNumber();
};

exports.caller = {
	function: function() {
		var stack = captureStack();
		return stack[2].getFunctionName();
	},
	line: function() {
		var stack = captureStack();
		return stack[2].getLineNumber();
	}
};
