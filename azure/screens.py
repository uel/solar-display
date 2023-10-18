def Weather():
    return { "type" : "html", "html":"""<div style="transform: scale(2);right: 270;position: absolute;top: 150;" id="ww_a5ebfd2793bf0" v='1.20' loc='id' a='{"t":"horizontal","lang":"en","ids":["wl307"],"cl_bkg":"rgba(255,255,255,1)","cl_font":"#000000","cl_cloud":"#000000","cl_persp":"#000000","cl_sun":"#000000","cl_moon":"#000000","cl_thund":"#000000","sl_sot":"celsius","sl_ics":"one","font":"Arial"}'>Weather Data Source: <a href="https://wetterlabs.de/wetter_prag/" id="ww_a5ebfd2793bf0_u" target="_blank">Wetter Prag</a></div><script async src="https://srv2.weatherwidget.org/js/?id=ww_a5ebfd2793bf0"></script>""" }

def Cat():
    return { "type" : "url", "url":"https://thiscatdoesnotexist.com/" }

def Bitcoin():
    return { "type" : "html", "html":"""<div class="livecoinwatch-widget-1" lcw-coin="BTC" lcw-base="USD" lcw-secondary="BTC" lcw-period="d" lcw-color-tx="#000000" lcw-color-pr="#000000" lcw-color-bg="#ffffff" lcw-border-w="5" lcw-digits="4" ></div><script defer src="https://www.livecoinwatch.com/static/lcw-widget.js"></script>""" }

def Food():
    return { "type" : "url", "url":"https://agata.suz.cvut.cz/jidelnicky/index.php?clPodsystem=3", "element": "tbody", "style":"tbody { font-size: 14px; font-weight: bold; }" }

def Time():
    return { "type" : "url", "url":"http://free.timeanddate.com/clock/i8cg6po0/n204/szw540/szh540/hoc000/hbw2/cf100/hgr0/fas20/fdi76/mqc000/mql10/mqw4/mqd98/mhc000/mhs3/mhl10/mhw4/mhd98/mmc000/mml10/mmw1/mmd98/hwm1/hsc111", "element": "div" }


SCREENS = [Time, Weather, Cat, Food]
