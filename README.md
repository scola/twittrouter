Twittrouter
===========
Twittrouter is used to verify your wifi client by twitter friends.It runs on [openwrt](https://openwrt.org/) router.Maybe it also support on dd-wrt or tomato router,I have not tested it.

[中文说明](http://scola.github.io/update-twittrouter-about-auth-and-arp-method/)

How to build
------------
Build the source code,this project requires [liboauth](http://liboauth.sourceforge.net/) library,but I have include in the source code for static build
```bash
# At OpenWRT build root
pushd package
git clone https://github.com/scola/twittrouter.git
popd

# Enable twittrouter in network category 
make menuconfig

# Optional
make -j

# Build the package
make V=s package/twittrouter/compile
```

Usage
-----
If you trust me,you can skip the build step and [download the ipk package here](https://github.com/scola/twittrouter/tree/master/release) and install.You must config your source address correctly in /etc/opkg.conf.iptables are required

    opkg install iptables
    
and the install twittrouter ipk packages,it will automatically download and install the required library(libcurl,libpthread)

    opkg install twittrouter

Get the usage of twittrouter

    twittrouter -h 

Your own devices need not to be verified,so just add it into whitelist.00:00:00:00:00:00 is invalid mac address,so keep it in whitelist.You could append your device mac address that's split by '|'.

Edit /etc/conf/twittrouter.json
    
    "whitelist":"00:00:00:00:00:00|d8:57:ef:33:86:93"

Run this program,and I recommend you add this program when system startup.
    
    twittrouter -a   #authorize your own twitter username
    twittrouter      #run this application
    /etc/init.d/twittrouter enable  #execute twittrouter when system startup

Chinese user only
-----------------
Because of the evil GFW,chinese user must make your route cross the GFW.You can take a look of [my blog](http://scola.github.io/deploy-proxy-on-openwrt--client-need-not-to-set/).Of course, you can use other network tools,such as VPN.**It makes no sense to run this program on your router unless your wifi client can cross the GFW**.Because your wifi client need to connect to [twitter.com](https://twitter.com) without any setting.This program call twitter api and api.twitter.com is blocked too,so you must config your router to make your router cross the GFW internally.Please refer to [this topic](http://scola.github.io/add-twitter-follower-verification-over-wifi/)

so I strongly suggest you test whether you configure you network correctly to go throuth the GFW.

    twittrouter -u [username]    #test the oauth to check your network config firstly,username is one of you twitter friends.

Known issues
-------------
 * until now the web page do not support english

 * the verify web page looks ugly on computer

Thanks
------
Thanks to the developer of [goagent](https://code.google.com/p/goagent/),[shadowsocks](http://www.shadowsocks.org/),[dnsproxy](https://github.com/phuslu/dnsproxy), [bestroutetb](https://github.com/ashi009/bestroutetb),[liboauth](https://github.com/x42/liboauth) and other bloggers

License
-------
Copyright (C) 2014 Scola <shaozheng.wu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/license/>

Screenshot
----------
![verification-page.png](https://raw.github.com/scola/twittrouter-python/master/verification-page.png)
