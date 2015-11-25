var Utils = {
	getUrlParameter: function() {
		var ret = {};
		var urlParam = decodeURIComponent(location.search.substring(1));
		var paramList = urlParam.split("&");
		ret.toString = function() {
			return urlParam;
		};
		for (var i=0; i<paramList.length; ++i) {
			var p = paramList[i].split("=");
			Object.defineProperty(ret, p[0], {
				get: (function(val) {return function(){return val===undefined ? true : val;}})(p[1])
			});
		}
		return ret;
	},
	
	getSession : function() {
		var session = $.cookie("session");
		return session ? JSON.parse(session) : null;
	}
};