#!/bin/sh -x
# phone.sh
# David Rowe 4 Jan 2010
# CGI for Easy Asterisk phones GUI

# Construct the web page -------------------------------

sh check_loggedin.sh

cat <<EOF
<script src="prototype.js"></script>
<link href="astman.css" media="all" rel="Stylesheet" type="text/css" />
<script type="text/javascript" src="tooltip.js"></script>
<link rel="stylesheet" href="tooltip.css" type="text/css" />
EOF

echo "<script>"
echo 'var init_internet="'$internet'";'
cat phone.js
echo "</script>"

cat << EOF
<html>
<title>Easy Asterisk - Phones</title>
<body onload="localInit()">
<div id="t1" class="tip">Tells you if I can reach the Internet.  If 
                         not "Good" check your network settings, in 
                         particular Gateway and DNS.</div>

<table align="center" width=800>
EOF
cat banner.html
echo "    <tr>"
cat menu.html    
cat <<EOF

    <td>

    <form action="/cgi-bin/set_phone.sh" onsubmit="return validate_form(this)" method="get">
    <table align="right" width=600>
    <tr>
      <tr><td colspan="2" align="left" valign="top" ><h2>Phones</h2></td>
      <tr onMouseOver="popUp(event,'t1')" onmouseout="popUp(event,'t1')">
	  <td>Internet Connection:</td>
	  <td><div id="internet" >
	  <span style="margin-left: 4px;font-weight:bold">&nbsp;</span></div></td>
      </tr>
    </table>
    </form>

    </td>

    </tr>

</table>
</body>
</html>
EOF

