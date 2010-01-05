#!/bin/sh -x
# dashboard.sh
# David Rowe 4 Jan 2010
# CGI for Easy Asterisk dashboard GUI

# See if we have Internet connectivity, first check dns as time outs can be very slow

dns=`cat /etc/resolv.conf | awk '/^nameserver/ {print $2}'`
dns_packet_loss=`ping $dns -c 1 -q | sed -n 's/.*received, \(.*\)% packet loss/\1/p'`
internet="no";
if [ $dns_packet_loss == "0" ]; then
  packet_loss=`ping google.com -c 1 -q | sed -n 's/.*received, \(.*\)% packet loss/\1/p'`
  if [ $packet_loss == "0" ]; then
    internet="yes";
  fi
fi

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
cat dashboard.js
echo "</script>"

cat << EOF
<html>
<title>Easy Asterisk - Dashboard</title>
<body onload="localInit()">
EOF

cat tooltips.html
echo '<table align="center" width=800 border=0>'
cat banner.html
echo "    <tr>"
cat menu.html    
cat <<EOF

    <td valign="top">

    <form action="/cgi-bin/set_network.sh" onsubmit="return validate_form(this)" method="get">
    <table align="right" width=600 border=0>
      <tr><td colspan="3" align="left" valign="top" ><h2>Dashboard</h2></td></tr>
      <tr onMouseOver="popUp(event,'network_internet')" onmouseout="popUp(event,'network_internet')">
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

