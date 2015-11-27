document.write('<script src="/javascripts/jquery-1.11.3.js"></script>');
document.write('<script src="/javascripts/jquery.cookie.js"></script>');
document.write('<script src="/javascripts/base64.js"></script>');

var weboard = {};
weboard.getUrlParameter = function() {
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
};

weboard.getSession = function() {
	var session = $.cookie("session");
	return session ? JSON.parse(session) : null;
};

weboard.main = function(main_routine) {
	jQuery(document).ready(function($) {
		var params = weboard.getUrlParameter();
		var session = weboard.getSession();
		if (session) {
			// token 갱신
			$.ajax({
				url: "/auth",
				type: "put",
				success: function(data, status) {
					console.log("[reissue token] " + status);;
					session = weboard.getSession();
					main_routine($, params, session);
				},
				error: function(jqXHR, status, errorThrown ) {
					var res = jqXHR.responseJSON;
					console.log("[reissue token] " + status + ", " + errorThrown);
					console.log(res);
					alert(res ? res.message : errorThrown);
					main_routine($, params, null);
				}
			});
		}
		else {
			main_routine($, params, session);
		}
	});
}