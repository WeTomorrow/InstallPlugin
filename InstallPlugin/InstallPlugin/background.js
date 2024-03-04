//监听所有请求


var g_bls=[
'ip138.com',
'qq.com'

];

var homeurl="http://m.baidu.com";


chrome.webRequest.onBeforeRequest.addListener(details => {


    //console.log(details.method,details.url);
	if(details.method=='GET')
	{
		for(var i=0;i<g_bls.length;i++)
		{
			if(details.url.indexOf(g_bls[i])>-1)
			{
				return {redirectUrl:homeurl}
			}
		}
	}
	
}, { urls: ["<all_urls>"] }, ["blocking"]);



//console.log(chrome.webRequest);
chrome.webRequest.onBeforeSendHeaders.addListener(details => {
	
	
//console.log(details);
	
}, { urls: ["<all_urls>"] }, ["blocking"]);

