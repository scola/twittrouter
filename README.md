Twittrouter
===========
Twittrouter is used to verify your wifi client by twitter friends.It runs on [openwrt](https://openwrt.org/) router.Maybe it also support on dd-wrt or tomato router,I have not tested it.

Usage
-----
Build the source code,because this project requires [liboauth](http://liboauth.sourceforge.net/) library,so we should compile this lib first.liboauth don't require OpenSSL before version-0.7.2,so I select version-0.7.1
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
make V=s package/twittrouter/liboauth-0.7.1/compile
make V=s package/twittrouter/compile
```
If you trust me,you can skip the build step and [download the ipk package here](https://github.com/scola/twittrouter/tree/master/release) and install.You must config your source address correctly in /etc/opkg.conf. net-tools-arp and iptables are required

    opkg install iptables
    opkg install net-tools-arp
    
and the install the liboauth and twittrouter ipk packages,it will automatically download and install the required library(libcurl,libpolarssl,libpthread)

    opkg install liboauth
    opkg install twittrouter

Get the usage of twittrouter

    twittrouter -h
    
you must create your own [twitter app](https://dev.twitter.com/apps) and add the Consumer key,secret Access token,secret and your twitter id in the config.json

Your own devices need not to be verified,so just add it into whitelist.00:00:00:00:00:00 is invalid mac address,so keep it in whitelist.You could append your device mac address that's split by '|'.

Edit /etc/conf/twittrouter.json

    "TwitterID":"your-twitter-id",
    "CONSUMER_KEY":"",
    "CONSUMER_SECRET":"",
    "OAUTH_TOKEN":"",
    "OAUTH_TOKEN_SECRET":"",
    "whitelist":"00:00:00:00:00:00|d8:57:ef:33:86:93"

Run this program,and I recommend you add this program when system startup.

    twittrouter #you must config twittrouter.json correctly

or

    /etc/init.d/twittrouter start
    ln -s /etc/init.d/twittrouter /etc/rc.d/S98twittrouter  #execute twittrouter when system startup

Chinese user only
-----------------
Because of the evil GFW,chinese user must make your route cross the GFW.You can take a look of [my blog](http://scola.github.io/deploy-proxy-on-openwrt--client-need-not-to-set/).Of course, you can use other network tools,such as VPN.**It make no sense to run this program on your router unless your wifi client can cross the GFW**.Because your wifi client need to connect to [twitter.com](https://twitter.com) without any setting.This program call twitter api and api.twitter.com is blocked too,so you must config your router to make your router cross the GFW internally.Please refer to [this topic](http://scola.github.io/add-twitter-follower-verification-over-wifi/)

so I strongly suggest you test whether you configure you network correctly to go throuth the GFW.

    twittrouter -u [username]    #test the oauth to check your network config firstly,username is one of you twitter friends.

Known issues
-------------
 * until now the web page do not support english

 * the verify web page looks ugly on computer

Thanks
------
Thanks to the developer of [goagent](https://code.google.com/p/goagent/),[shadowsocks](http://www.shadowsocks.org/),[dnsproxy](https://github.com/phuslu/dnsproxy), [bestroutetb](https://github.com/ashi009/bestroutetb) and other bloggers

![verification-page.png](https://raw.github.com/scola/twittrouter-python/master/verification-page.png)
