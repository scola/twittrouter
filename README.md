Twittrouter
===========
Twittrouter is used to verify your wifi client by twitter friends.It runs on [openwrt](https://openwrt.org/) router.Maybe it also support on dd-wrt or tomato router,I have not tested it.

Usage
-----
First, make sure you install python on your router.net-tools-arp and iptables is required

    opkg install python
    $ python --version
    Python 2.7.3
    opkg install iptables
    opkg install net-tools-arp

and pip is required for install requests and requests_oauthlib  

    pip install requests
    pip install requests_oauthlib

you must create your own [twitter app](https://dev.twitter.com/apps) and add the Consumer key,secret Access token,secret in the config.json

Your own devices need not to be verified,so just add it into whitelist.00:00:00:00:00:00 is invalid mac address,so keep it in whitelist.You could append your device mac address that's split by '|'.You can add your own twitter id in config.json optionally

Edit config.json

    "TwitterID":"your-twitter-id",
    "CONSUMER_KEY":"",
    "CONSUMER_SECRET":"",
    "OAUTH_TOKEN":"",
    "OAUTH_TOKEN_SECRET":"",
    "whitelist":"00:00:00:00:00:00|d8:57:ef:33:86:93|00:04:23:97:20:26|04:46:65:53:00:0b"

Run this program

    python twittrouter.py #you must add your twitter id in the config.json
or:

    python twittrouter.py your-twiiter-id # it will overlap the twitter id in config.json

Chinese user only
-----------------
Because of the evil GFW,chinese user must make your route cross the GFW.You can take a look of [my blog](http://scola.github.io/deploy-proxy-on-openwrt--client-need-not-to-set/).Of course, you can use other network tools,such as VPN.It make no sense to run this program on your router unless your wifi client can cross the GFW.Because your wifi client need to connect to [twitter.com](https://twitter.com) without any setting.This program call twitter api and api.twitter.com is blocked too,so you must config your router to make your router cross the GFW internally.Please refer to [this topic](http://scola.github.io/add-twitter-follower-verification-over-wifi/)

Known issues
-------------
 * until now the web page do not support english

 * mobile device which connect to the wifi connect to mobile.twitter.com/your-twitter-id automatically.and it can check your followers and those you following without logging in.So it can not bring the user to twitter absolutely.He can look up one of your follower and fill in to finish verification.Maybe he just sign up or log in and follow you as the indication on the verification page.In fact,you can ask your wifi user say something to you on twitter such as '@your-twitter-id hi',and then you use twitter api to check the mention message.so the user has to log in his own twitter.But I do not want to make it complicated.If someone use this program and think it's necessary to enhance the verification method.I will think about improving it

Thanks
------
Thanks to the developer of [goagent](https://code.google.com/p/goagent/),[shadowsocks](http://www.shadowsocks.org/),[dnsproxy](https://github.com/phuslu/dnsproxy), [bestroutetb](https://github.com/ashi009/bestroutetb) and other bloggers

![verification-page.png](https://raw.github.com/scola/twittrouter/master/verification-page.png)
