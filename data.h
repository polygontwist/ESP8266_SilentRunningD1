//---------------------------------WWW------------------------------
//ESP8266

//HTML \r\n \t
const String indexHTM="<!DOCTYPE html><html><head>\r\n"
"<meta charset=\"utf-8\"/>\r\n"
"<meta content=\"width=device-width,initial-scale=0.65,maximum-scale=2,user-scalable=yes\" name=\"viewport\" />\r\n"
"<link rel=\"shortcut icon\" href=\"favicon.ico\">\r\n"
"<link rel=\"STYLESHEET\" type=\"text/css\" href=\"style.css\">\r\n"
"<script type=\"text/javascript\" src=\"system.js\"></script>\r\n"
"</head><body>\r\n"
"<h1>$h1gtag</h1>\r\n"
"<div id=\"actions\"></div>\r\n"
"<div id=\"sysinfo\"></div>\r\n"
"<div id=\"filelist\"><span id=\"filelisttab\"></span><div id=\"flcontent\">\r\n"
"$filelist"
"<form class=\"upload\" method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\">"
"<input type=\"file\" name=\"upload\" required>\r\n"
"<input type=\"submit\" value=\"Upload\" class=\"button\">\n</form>\r\n"
"</div></div>\r\n"
"</body></html>\r\n"
;


