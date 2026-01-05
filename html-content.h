const char TXT_CONTENT_TYPE_JSON[] PROGMEM = "application/json";
const char TXT_CONTENT_TYPE_INFLUXDB[] PROGMEM =
    "application/x-www-form-urlencoded";
const char TXT_CONTENT_TYPE_TEXT_HTML[] PROGMEM = "text/html; charset=utf-8";
const char TXT_CONTENT_TYPE_TEXT_CSS[] PROGMEM = "text/css";
const char TXT_CONTENT_TYPE_TEXT_PLAIN[] PROGMEM = "text/plain";
// ICI
const char TXT_CONTENT_TYPE_IMAGE_PNG[] PROGMEM = "image/png";
const char TXT_CONTENT_TYPE_IMAGE_SVG[] PROGMEM = "image/svg";

const char DBG_TXT_TEMPERATURE[] PROGMEM = "Temperature (°C): ";
const char DBG_TXT_HUMIDITY[] PROGMEM = "Humidity (%): ";
const char DBG_TXT_PRESSURE[] PROGMEM = "Pressure (Pa): ";
const char DBG_TXT_CO2PPM[] PROGMEM = "CO₂ (ppm): ";
const char DBG_TXT_VOCPPB[] PROGMEM = "VOC (ppb): ";
const char DBG_TXT_START_READING[] PROGMEM = "R/ ";
const char DBG_TXT_END_READING[] PROGMEM = "/R ";
const char DBG_TXT_CHECKSUM_IS[] PROGMEM = "Checksum is: ";
const char DBG_TXT_CHECKSUM_SHOULD[] PROGMEM = "Checksum should: ";
const char DBG_TXT_DATA_READ_FAILED[] PROGMEM = "Data read failed";
const char DBG_TXT_UPDATE[] PROGMEM = "[update] ";
const char DBG_TXT_UPDATE_FAILED[] PROGMEM = "Update failed.";
const char DBG_TXT_UPDATE_NO_UPDATE[] PROGMEM = "No update.";
const char DBG_TXT_SENDING_TO[] PROGMEM = "## Sending to ";
const char DBG_TXT_SDS011_VERSION_DATE[] PROGMEM = "SDS011 version date";
const char DBG_TXT_NPM_VERSION_DATE[] PROGMEM = "Next PM version date";
const char DBG_TXT_CONNECTING_TO[] PROGMEM = "Connecting to ";
const char DBG_TXT_FOUND[] PROGMEM = " ... found";
const char DBG_TXT_NOT_FOUND[] PROGMEM = " ... not found";
const char DBG_TXT_SEP[] PROGMEM = "----";

const char SENSORS_SDS011[] PROGMEM = "SDS011";
const char SENSORS_NPM[] PROGMEM = "Tera Sensor Next PM";
const char SENSORS_BME280[] PROGMEM = "BME280";
const char SENSORS_BMP280[] PROGMEM = "BMP280";
const char SENSORS_CCS811[] PROGMEM = "CJMCU-811";
const char SENSORS_MHZ16[] PROGMEM = "MH-Z16";
const char SENSORS_S88[] PROGMEM = "SensAir S88";

const char WEB_PAGE_HEADER[] PROGMEM =
    "<!DOCTYPE html><html lang='" INTL_LANG "'>\
<head>\
<meta charset='utf-8'/>\
<title>{t}</title>";

const char WEB_PAGE_STATIC_CSS[] PROGMEM = "\
*{box-sizing:border-box;margin:0;padding:0}\
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Oxygen,Ubuntu,Cantarell,sans-serif;margin:0;background:#f8f9fa;color:#2c3e50;line-height:1.6}\
.content{max-width:1200px;margin:0 auto;padding:24px 20px}\
.footer{background:#fff;border-top:1px solid #e1e8ed;padding:24px;text-align:center;margin-top:60px;box-shadow:0 -2px 10px rgba(0,0,0,0.05)}\
.footer a{color:#438DBF;text-decoration:none;transition:color 0.3s;font-weight:500}\
.footer a:hover{color:#2d5f8d}\
.r{text-align:right}\
td{vertical-align:middle;padding:8px 10px;font-size:13px}\
td:first-child{font-weight:600;color:#2c3e50;width:35%;padding-right:12px}\
table{width:100%;border-collapse:collapse;margin:8px 0}\
.v{background:#fff;border-radius:10px;overflow:hidden;box-shadow:0 2px 8px rgba(0,0,0,0.08);margin-bottom:16px}\
.v>tbody>tr{border-bottom:1px solid #f0f0f0;transition:background 0.2s}\
.v>tbody>tr:last-child{border-bottom:none}\
.v>tbody>tr:hover{background:#f8f9fa}\
.v>tbody>tr:nth-child(odd){background:#fafbfc}\
.v>tbody>tr:nth-child(odd):hover{background:#f1f3f5}\
.panel table{background:#fff;border-radius:8px;border:1px solid #e1e8ed;overflow:hidden}\
.panel table tr{border-bottom:1px solid #f0f0f0}\
.panel table tr:last-child{border-bottom:none}\
.b{text-decoration:none;padding:14px 24px;background:linear-gradient(135deg,#438DBF 0%,#5ba3d0 100%);color:#fff;display:block;width:auto;border-radius:10px;margin-bottom:12px;box-shadow:0 4px 12px rgba(67,141,191,0.25);transition:all 0.3s;font-weight:500;text-align:center;border:none}\
.b:hover{transform:translateY(-2px);box-shadow:0 6px 20px rgba(67,141,191,0.35)}\
.b:active{transform:translateY(0);box-shadow:0 2px 8px rgba(67,141,191,0.25)}\
.wifi{background:#fff;color:#2c3e50;padding:12px 16px;display:flex;align-items:center;justify-content:space-between;width:100%;border:2px solid #e1e8ed;border-radius:8px;cursor:pointer;transition:all 0.3s;font-weight:600;margin:6px 0;box-shadow:0 2px 6px rgba(0,0,0,0.08);font-size:14px;text-decoration:none}\
.wifi:hover{background:linear-gradient(135deg,#438DBF 0%,#5ba3d0 100%);color:#fff;border-color:#438DBF;box-shadow:0 4px 12px rgba(67,141,191,0.3);transform:translateX(4px)}\
.wifi:active{transform:translateX(2px);box-shadow:0 2px 6px rgba(67,141,191,0.2)}\
.wifi-signal{font-weight:700;font-size:13px;opacity:0.9;margin-left:auto;padding-left:12px;white-space:nowrap}\
input[type=text],input[type=password],input[type=number]{width:100%;padding:8px 12px;border:1px solid #e1e8ed;border-radius:6px;font-size:13px;transition:all 0.3s;background:#fff}\
input[type=text]:focus,input[type=password]:focus,input[type=number]:focus{outline:none;border-color:#438DBF;box-shadow:0 0 0 3px rgba(67,141,191,0.1);background:#fafbfc}\
input[type=submit]{color:#fff;text-align:center;cursor:pointer;border-radius:10px;font-size:16px;background:linear-gradient(135deg,#e74c3c 0%,#c0392b 100%);padding:14px 24px;width:100%;border:none;font-weight:600;transition:all 0.3s;box-shadow:0 4px 12px rgba(231,76,60,0.25)}\
input[type=submit]:hover{transform:translateY(-2px);box-shadow:0 6px 20px rgba(231,76,60,0.35)}\
input[type=submit]:active{transform:translateY(0)}\
.s_green{padding:14px 24px;width:100%;border:none;background:linear-gradient(135deg,#438DBF 0%,#5ba3d0 100%);color:#fff;text-align:center;border-radius:10px;cursor:pointer;font-weight:600;font-size:16px;transition:all 0.3s;box-shadow:0 4px 12px rgba(67,141,191,0.25)}\
.s_green:hover{transform:translateY(-2px);box-shadow:0 6px 20px rgba(67,141,191,0.35)}\
.s_red{padding:14px 24px;width:100%;border:none;background:linear-gradient(135deg,#e74c3c 0%,#c0392b 100%);color:#fff;text-align:center;border-radius:10px;cursor:pointer;font-weight:600;font-size:16px;transition:all 0.3s;box-shadow:0 4px 12px rgba(231,76,60,0.25)}\
.s_red:hover{transform:translateY(-2px);box-shadow:0 6px 20px rgba(231,76,60,0.35)}\
.tabs{display:flex;flex-wrap:wrap;overflow-x:auto;overflow-y:hidden;background:#f8f9fa;border-radius:12px;box-shadow:0 2px 8px rgba(0,0,0,0.08);margin-top:20px;margin-bottom:8px;padding:12px;gap:8px;scrollbar-width:thin;scrollbar-color:#438DBF #f0f0f0;border:2px solid #e1e8ed}\
.tabs::-webkit-scrollbar{height:6px}\
.tabs::-webkit-scrollbar-track{background:#f0f0f0;border-radius:3px}\
.tabs::-webkit-scrollbar-thumb{background:#438DBF;border-radius:3px}\
.tab{flex:0 0 auto;padding:12px 20px;display:inline-block;color:#5a6c7d;cursor:pointer;transition:all 0.3s;border:2px solid #e1e8ed;border-radius:10px;font-weight:700;font-size:14px;white-space:nowrap;background:#fff;box-shadow:0 1px 3px rgba(0,0,0,0.08)}\
.tab:hover{background:#fff;color:#438DBF;border-color:#438DBF;transform:translateY(-2px);box-shadow:0 3px 8px rgba(67,141,191,0.2)}\
.panels{min-height:200px;overflow:hidden;padding:24px;background:#fff;border-radius:12px;margin-bottom:20px;margin-top:0;box-shadow:0 2px 8px rgba(0,0,0,0.08);border:2px solid #e1e8ed}\
.panel h3{color:#2c3e50;margin-bottom:20px;margin-top:24px;font-size:17px;font-weight:700;padding:12px 16px;background:linear-gradient(135deg,#f8f9fa 0%,#fff 100%);border-left:5px solid #438DBF;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.05)}\
.panel h3:first-child{margin-top:0}\
.panel>b{display:block;color:#2c3e50;font-size:15px;font-weight:700;margin:24px 0 12px;padding:10px 14px;background:linear-gradient(135deg,#fafbfc 0%,#fff 100%);border-left:4px solid #5ba3d0;border-radius:6px;box-shadow:0 1px 3px rgba(0,0,0,0.05)}\
.panel label{display:flex;align-items:center;margin:10px 0;color:#4a5568;font-weight:500;cursor:pointer;padding:10px 14px;border-radius:8px;transition:all 0.2s;font-size:14px;background:#fff;border:2px solid #e1e8ed}\
.panel label:hover{background:#f8f9fa;border-color:#438DBF;transform:translateX(4px);box-shadow:0 2px 6px rgba(67,141,191,0.15)}\
.panel input[type=checkbox]{margin-right:12px;width:20px;height:20px;cursor:pointer;flex-shrink:0;accent-color:#438DBF}\
.radio{display:none}\
.panel{display:none}\
.canvas{min-height:60px;background:linear-gradient(135deg,#438DBF 0%,#5ba3d0 100%);margin-bottom:0;padding:8px 16px;box-shadow:0 4px 12px rgba(0,0,0,0.1);position:relative;display:flex;align-items:center;gap:12px}\
.canvas::after{content:'';position:absolute;bottom:0;left:0;right:0;height:3px;background:linear-gradient(90deg,#2d5f8d,#438DBF,#5ba3d0,#438DBF,#2d5f8d);opacity:0.6}\
h3{color:#2c3e50;margin-bottom:16px;font-weight:600;font-size:16px;padding:10px 14px;background:#f8f9fa;border-left:4px solid #438DBF;border-radius:6px}\
h4{color:#2c3e50;margin-bottom:16px;font-weight:600;font-size:18px;margin-top:8px}\
hr{border:none;border-top:2px solid #e1e8ed;margin:20px 0}\
#r6:checked~.panels>#panel6,#r5:checked~.panels>#panel5,#r4:checked~.panels>#panel4,#r1:checked~.panels>#panel1,#r3:checked~.panels>#panel3,#r2:checked~.panels>#panel2{display:block;animation:fadeIn 0.4s}\
#r6:checked~.tabs>#tab6,#r5:checked~.tabs>#tab5,#r4:checked~.tabs>#tab4,#r1:checked~.tabs>#tab1,#r3:checked~.tabs>#tab3,#r2:checked~.tabs>#tab2{background:linear-gradient(135deg,#438DBF 0%,#5ba3d0 100%);color:#fff;border-color:#438DBF;box-shadow:0 4px 14px rgba(67,141,191,0.4);transform:translateY(-2px);font-weight:800}\
#r6:checked~.tabs>#tab6:hover,#r5:checked~.tabs>#tab5:hover,#r4:checked~.tabs>#tab4:hover,#r1:checked~.tabs>#tab1:hover,#r3:checked~.tabs>#tab3:hover,#r2:checked~.tabs>#tab2:hover{transform:translateY(-2px)}\
@keyframes fadeIn{from{opacity:0;transform:translateY(10px)}to{opacity:1;transform:translateY(0)}}\
.nav-tabs{display:flex;overflow-x:auto;overflow-y:hidden;white-space:nowrap;gap:8px;padding:16px;background:#fff;border-radius:12px;box-shadow:0 2px 8px rgba(0,0,0,0.08);margin-bottom:20px;scrollbar-width:thin;scrollbar-color:#438DBF #f0f0f0}\
.nav-tabs::-webkit-scrollbar{height:6px}\
.nav-tabs::-webkit-scrollbar-track{background:#f0f0f0;border-radius:3px}\
.nav-tabs::-webkit-scrollbar-thumb{background:#438DBF;border-radius:3px}\
.nav-tab{flex:0 0 auto;padding:10px 20px;background:#fff;color:#438DBF;border:2px solid #438DBF;border-radius:8px;text-decoration:none;font-weight:600;font-size:14px;transition:all 0.3s;text-align:center;min-width:120px}\
.nav-tab:hover{background:#438DBF;color:#fff;transform:translateY(-2px);box-shadow:0 4px 12px rgba(67,141,191,0.25)}\
nav a:hover{background:#438DBF!important;color:#fff!important;transform:translateY(-2px);box-shadow:0 4px 12px rgba(67,141,191,0.25)}\
nav>div::-webkit-scrollbar{height:4px}\
nav>div::-webkit-scrollbar-track{background:#f0f0f0;border-radius:2px}\
nav>div::-webkit-scrollbar-thumb{background:#438DBF;border-radius:2px}\
.section-card{background:#fff;padding:24px;border-radius:12px;box-shadow:0 2px 8px rgba(0,0,0,0.08);margin-bottom:16px}\
.section-card a[href^='/']:not(.b){transition:all 0.3s}\
.section-card a[href^='/']:not(.b):hover{transform:translateY(-4px);box-shadow:0 6px 20px rgba(67,141,191,0.2);border-color:#438DBF!important}\
.section-card a[href^='https://openairmap']:hover{transform:translateY(-2px);box-shadow:0 6px 20px rgba(67,141,191,0.4)}\
.section-title{color:#2c3e50;font-size:18px;font-weight:600;margin-bottom:16px;padding-bottom:12px;border-bottom:2px solid #e1e8ed}\
#wifilist{background:#f8f9fa;padding:12px;border-radius:8px;margin:12px 0;border-left:3px solid #438DBF;font-weight:600;color:#2c3e50}\
#wifilist table{background:transparent;border:none;box-shadow:none;margin:8px 0 0 0}\
#wifilist tr{background:none;border:none}\
#wifilist td{padding:0}\
@media(max-width:768px){.content{padding:20px 15px}.tabs{padding:6px;gap:4px}.tab{padding:6px 12px;font-size:12px}.panels{padding:16px}.b{padding:12px 20px;font-size:15px}.nav-tab{min-width:100px;font-size:13px}}";

#define STATIC_PREFIX "/" INTL_LANG "_s1"

const char WEB_PAGE_HEADER_HEAD[] PROGMEM =
    "<meta name='viewport' content='width=device-width'/>\
<meta name='theme-color' content='#438DBF'>\
<link rel='stylesheet' href='" STATIC_PREFIX "?r=css'>\
</style>\
</head><body>\
<div class='canvas'>\
<a class='b' href='/' style='background:none;display:inline'>\
<img src='" STATIC_PREFIX "?r=logo' alt='" INTL_BACK_TO_HOME
    "' style='margin:0' width='50' height='44'/></a>";

const char WEB_PAGE_HEADER_BODY[] PROGMEM = "<div style='flex:1;padding:0'>\
<h2 style='margin:0;color:#fff;font-size:16px;font-weight:700;text-shadow:0 2px 4px rgba(0,0,0,0.1)'>" INTL_PM_SENSOR
                                            "</h2>\
<div style='margin-top:4px;color:rgba(255,255,255,0.9);font-size:11px;line-height:1.4'>\
<span style='margin-right:8px'><strong>ID:</strong> {id}</span>\
<span><strong>" INTL_FIRMWARE ":</strong> " SOFTWARE_VERSION_STR "</span>\
</div></div>\
</div>\
<nav style='background:#fff;border-bottom:2px solid #e1e8ed;box-shadow:0 2px 8px rgba(0,0,0,0.08);position:sticky;top:0;z-index:100'>\
<div style='max-width:1200px;margin:0 auto;display:flex;overflow-x:auto;overflow-y:hidden;gap:4px;padding:12px 20px;scrollbar-width:thin;scrollbar-color:#438DBF #f0f0f0'>\
<a href='/' style='flex:0 0 auto;padding:8px 16px;background:#fff;color:#438DBF;border:2px solid #438DBF;border-radius:8px;text-decoration:none;font-weight:600;font-size:13px;transition:all 0.3s;text-align:center;white-space:nowrap;display:flex;align-items:center;gap:6px'><svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><path d=\"M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z\"/><polyline points=\"9 22 9 12 15 12 15 22\"/></svg>Accueil</a>\
<a href='/values' style='flex:0 0 auto;padding:8px 16px;background:#fff;color:#438DBF;border:2px solid #438DBF;border-radius:8px;text-decoration:none;font-weight:600;font-size:13px;transition:all 0.3s;text-align:center;white-space:nowrap;display:flex;align-items:center;gap:6px'><svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><line x1=\"12\" y1=\"20\" x2=\"12\" y2=\"10\"/><line x1=\"18\" y1=\"20\" x2=\"18\" y2=\"4\"/><line x1=\"6\" y1=\"20\" x2=\"6\" y2=\"16\"/></svg>Données</a>\
<a href='/status' style='flex:0 0 auto;padding:8px 16px;background:#fff;color:#438DBF;border:2px solid #438DBF;border-radius:8px;text-decoration:none;font-weight:600;font-size:13px;transition:all 0.3s;text-align:center;white-space:nowrap;display:flex;align-items:center;gap:6px'><svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><polyline points=\"22 12 18 12 15 21 9 3 6 12 2 12\"/></svg>État</a>\
<a href='/config' style='flex:0 0 auto;padding:8px 16px;background:#fff;color:#438DBF;border:2px solid #438DBF;border-radius:8px;text-decoration:none;font-weight:600;font-size:13px;transition:all 0.3s;text-align:center;white-space:nowrap;display:flex;align-items:center;gap:6px'><svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><circle cx=\"12\" cy=\"12\" r=\"3\"/><path d=\"M12 1v6m0 6v6m5.2-13.2l-4.2 4.2m-2 2l-4.2 4.2M23 12h-6m-6 0H1m18.2 5.2l-4.2-4.2m-2-2l-4.2-4.2\"/></svg>Config</a>\
<a href='/debug' style='flex:0 0 auto;padding:8px 16px;background:#fff;color:#438DBF;border:2px solid #438DBF;border-radius:8px;text-decoration:none;font-weight:600;font-size:13px;transition:all 0.3s;text-align:center;white-space:nowrap;display:flex;align-items:center;gap:6px'><svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><rect x=\"3\" y=\"3\" width=\"18\" height=\"18\" rx=\"2\"/><line x1=\"9\" y1=\"9\" x2=\"15\" y2=\"9\"/><line x1=\"9\" y1=\"15\" x2=\"15\" y2=\"15\"/></svg>Debug</a>\
<a href='/update' style='flex:0 0 auto;padding:8px 16px;background:#fff;color:#438DBF;border:2px solid #438DBF;border-radius:8px;text-decoration:none;font-weight:600;font-size:13px;transition:all 0.3s;text-align:center;white-space:nowrap;display:flex;align-items:center;gap:6px'><svg width=\"16\" height=\"16\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><path d=\"M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4\"/><polyline points=\"7 10 12 15 17 10\"/><line x1=\"12\" y1=\"15\" x2=\"12\" y2=\"3\"/></svg>" INTL_UPDATE
                                            "</a>\
</div>\
</nav>\
<script>document.querySelectorAll('nav a').forEach(a=>{if(a.getAttribute('href')===window.location.pathname||(window.location.pathname==='/'&&a.getAttribute('href')==='/')){a.style.background='#438DBF';a.style.color='#fff';a.style.boxShadow='0 2px 8px rgba(67,141,191,0.3)'}});</script>\
<div class='content'><h4 style='font-size:18px;color:#2c3e50;margin-bottom:16px'>" INTL_HOME
                                            " {n} {t}</h4>";

const char BR_TAG[] PROGMEM = "<br/>";
const char WEB_DIV_PANEL[] PROGMEM = "</div><div class='panel' id='panel{v}'>";
const char TABLE_TAG_OPEN[] PROGMEM = "<table>";
const char TABLE_TAG_CLOSE_BR[] PROGMEM = "</table>";
const char EMPTY_ROW[] PROGMEM = "<tr><td colspan='3'>&nbsp;</td></tr>";

const char WEB_PAGE_FOOTER[] PROGMEM =
    "<div style='text-align:center;margin:40px 0 20px'>"
    "<a class='b' href='/' "
    "style='display:inline-block;max-width:300px'>" INTL_BACK_TO_HOME
    "</a></div>"
    "</div><footer class='footer'><div style='padding:20px;font-size:14px'>"
    "<div style='margin-bottom:8px'><a href='https://www.aircarto.fr/' "
    "target='_blank' rel='noreferrer' "
    "style='color:#438DBF;font-size:16px;font-weight:600'>"
    "&copy; 2025 AirCarto</a></div>"
    "<div><a href='https://github.com/aircarto/ModuleAir_V2/labels/bug' "
    "target='_blank' rel='noreferrer' style='color:#5a6c7d'>" INTL_REPORT_ISSUE
    "</a></div>"
    "</div></footer></body></html>\r\n";

const char WEB_ROOT_PAGE_CONTENT[] PROGMEM = "\
<div class='section-card'>\
<div class='section-title' style='display:flex;align-items:center;gap:8px'><svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><path d=\"M2 12h20\"/><path d=\"M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z\"/></svg>Carte interactive</div>\
<a href='https://openairmap.fr/' target='_blank' rel='noreferrer' style='text-decoration:none;background:linear-gradient(135deg,#438DBF 0%,#5ba3d0 100%);border-radius:12px;padding:24px;display:flex;align-items:center;gap:16px;color:#fff;transition:all 0.3s'>\
<svg width=\"48\" height=\"48\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><path d=\"M2 12h20\"/><path d=\"M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z\"/></svg>\
<div style='flex:1'>\
<div style='font-size:16px;font-weight:600;margin-bottom:4px'>" INTL_ACTIVE_SENSORS_MAP
                                             "</div>\
<div style='font-size:13px;opacity:0.9'>Consulter la carte des capteurs actifs</div>\
</div>\
<svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><line x1=\"5\" y1=\"12\" x2=\"19\" y2=\"12\"/><polyline points=\"12 5 19 12 12 19\"/></svg>\
</a>\
</div>\
<div class='section-card'>\
<div class='section-title' style='display:flex;align-items:center;gap:8px'><svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><line x1=\"12\" y1=\"8\" x2=\"12\" y2=\"12\"/><line x1=\"12\" y1=\"16\" x2=\"12.01\" y2=\"16\"/></svg>Actions avanc\u00e9es</div>\
<div style='background:#fff5e6;border:2px solid #f39c12;border-radius:10px;padding:16px;margin-bottom:16px'>\
<div style='display:flex;align-items:center;gap:12px'>\
<svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#f39c12\" stroke-width=\"2\"><path d=\"M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z\"/><line x1=\"12\" y1=\"9\" x2=\"12\" y2=\"13\"/><line x1=\"12\" y1=\"17\" x2=\"12.01\" y2=\"17\"/></svg>\
<div style='font-size:14px;color:#5a6c7d;flex:1'>Ces actions peuvent affecter le fonctionnement du capteur. Utilisez-les avec pr\u00e9caution.</div>\
</div>\
</div>\
<div style='display:grid;grid-template-columns:repeat(auto-fit,minmax(240px,1fr));gap:12px'>\
<a class='b' href='/reset' style='padding:20px;background:linear-gradient(135deg,#438DBF 0%,#5ba3d0 100%);box-shadow:0 4px 12px rgba(67,141,191,0.25);margin:0;display:flex;flex-direction:column;align-items:center;gap:8px'>\
<svg width=\"40\" height=\"40\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><polyline points=\"23 4 23 10 17 10\"/><polyline points=\"1 20 1 14 7 14\"/><path d=\"M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15\"/></svg>\
<div style='text-align:center'>\
<div style='font-size:15px;font-weight:600;margin-bottom:4px'>{restart}</div>\
<div style='font-size:12px;opacity:0.9'>Red\u00e9marre le capteur (conserve les param\u00e8tres)</div>\
</div>\
</a>\
<a class='b' href='/removeConfig' style='padding:20px;background:linear-gradient(135deg,#e74c3c 0%,#c0392b 100%);box-shadow:0 4px 12px rgba(231,76,60,0.25);margin:0;display:flex;flex-direction:column;align-items:center;gap:8px'>\
<svg width=\"40\" height=\"40\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><polyline points=\"3 6 5 6 21 6\"/><path d=\"M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2\"/><line x1=\"10\" y1=\"11\" x2=\"10\" y2=\"17\"/><line x1=\"14\" y1=\"11\" x2=\"14\" y2=\"17\"/></svg>\
<div style='text-align:center'>\
<div style='font-size:15px;font-weight:600;margin-bottom:4px'>" INTL_CONFIGURATION_DELETE
                                             "</div>\
<div style='font-size:12px;opacity:0.9'>Efface toute la configuration du capteur</div>\
</div>\
</a>\
</div></div>";

const char WEB_CONFIG_SCRIPT[] PROGMEM = "<script>\
function togglePwdVisibility(){const e=document.getElementById('wlanpwd');if('saved'===e.getAttribute('data-state'))return;const t=document.getElementById('pwd-toggle'),o='<svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z\"></path><circle cx=\"12\" cy=\"12\" r=\"3\"></circle></svg>',s='<svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24\"></path><line x1=\"1\" y1=\"1\" x2=\"23\" y2=\"23\"></line></svg>';'password'===e.type?(e.type='text',t.innerHTML=s):(e.type='password',t.innerHTML=o)}function setSSID(e){document.getElementById('wlanssid').value=e.getAttribute('data-ssid');const t=document.getElementById('wlanpwd');t.value='',t.focus(),t.setAttribute('data-state','new');const o=document.getElementById('pwd-toggle');o&&(o.style.opacity='1',o.style.cursor='pointer')}function load_wifi_list(){const e=document.getElementById('wlanpwd');if(e&&!document.getElementById('pwd-toggle')){e.type='password',e.setAttribute('data-state','saved');const t='<svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z\"></path><circle cx=\"12\" cy=\"12\" r=\"3\"></circle></svg>',o=document.createElement('span');o.id='pwd-toggle',o.innerHTML=t,o.style.position='absolute',o.style.right='10px',o.style.top='50%',o.style.transform='translateY(-50%)',o.style.cursor='not-allowed',o.style.opacity='0.5',o.onclick=togglePwdVisibility;const s=e.parentNode;if('relative'!==s.style.position){const w=document.createElement('div');w.style.position='relative',s.replaceChild(w,e),w.appendChild(e),w.appendChild(o)}else s.appendChild(o)}const t=document.getElementById('has_wifi');if(t){t.checked=!0}var xhr=new XMLHttpRequest();xhr.open('GET','/wifi');xhr.onload=function(){200===xhr.status&&(document.getElementById('wifilist').innerHTML=xhr.responseText)};xhr.send()}\
</script>";

const char WEB_REMOVE_CONFIG_CONTENT[] PROGMEM = "\
<div style='background:#fff;padding:40px;border-radius:12px;box-shadow:0 4px 20px rgba(0,0,0,0.1);max-width:600px;margin:40px auto;text-align:center'>\
<div style='margin-bottom:20px;display:flex;justify-content:center'><svg width=\"64\" height=\"64\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#e74c3c\" stroke-width=\"2\"><path d=\"M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z\"/><line x1=\"12\" y1=\"9\" x2=\"12\" y2=\"13\"/><line x1=\"12\" y1=\"17\" x2=\"12.01\" y2=\"17\"/></svg></div>\
<h3 style='color:#e74c3c;margin-bottom:20px;font-size:22px'>" INTL_CONFIGURATION_REALLY_DELETE
                                                 "</h3>\
<p style='color:#5a6c7d;margin-bottom:30px;line-height:1.6'>Cette action supprimera toute la configuration du capteur (WiFi, capteurs, APIs). Vous devrez le reconfigurer complètement.</p>\
<div style='display:grid;grid-template-columns:1fr 1fr;gap:12px'>\
<form method='POST' action='/removeConfig' style='margin:0'>\
<input type='submit' class='s_red' name='submit' value='" INTL_DELETE
                                                 "'/></form>\
<a class='b' href='/' style='background:linear-gradient(135deg,#95a5a6 0%,#7f8c8d 100%);box-shadow:0 4px 12px rgba(149,165,166,0.25);margin:0'>" INTL_CANCEL
                                                 "</a>\
</div></div>";

const char WEB_RESET_CONTENT[] PROGMEM = "\
<div style='background:#fff;padding:40px;border-radius:12px;box-shadow:0 4px 20px rgba(0,0,0,0.1);max-width:600px;margin:40px auto;text-align:center'>\
<div style='margin-bottom:20px;display:flex;justify-content:center'><svg width=\"64\" height=\"64\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#438DBF\" stroke-width=\"2\"><polyline points=\"23 4 23 10 17 10\"/><polyline points=\"1 20 1 14 7 14\"/><path d=\"M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15\"/></svg></div>\
<h3 style='color:#e74c3c;margin-bottom:20px;font-size:22px'>" INTL_REALLY_RESTART_SENSOR
                                         "</h3>\
<p style='color:#5a6c7d;margin-bottom:30px;line-height:1.6'>Le capteur va redémarrer. Cela prendra environ 30 secondes. Vos paramètres seront conservés.</p>\
<div style='display:grid;grid-template-columns:1fr 1fr;gap:12px'>\
<form method='POST' action='/reset' style='margin:0'>\
<input type='submit' class='s_red' name='submit' value='" INTL_RESTART
                                         "'/></form>\
<a class='b' href='/' style='background:linear-gradient(135deg,#95a5a6 0%,#7f8c8d 100%);box-shadow:0 4px 12px rgba(149,165,166,0.25);margin:0'>" INTL_CANCEL
                                         "</a>\
</div></div>";

const char WEB_IOS_REDIRECT[] PROGMEM = "<html><body>Redirecting...\
<script type=\"text/javascript\">\
window.location = \"http://192.168.4.1/config\";\
</script>\
</body></html>";

const char WEB_B_BR_BR[] PROGMEM = "</b><br/><br/>";
const char WEB_BRACE_BR[] PROGMEM = ")<br/>";
const char WEB_B_BR[] PROGMEM = "</b><br/>";
const char WEB_BR_BR[] PROGMEM = "<br/><br/>";
const char WEB_BR_FORM[] PROGMEM = "<br/></form>";
const char WEB_BR_LF_B[] PROGMEM = "<br/>\n<b>";
const char WEB_LF_B[] PROGMEM = "\n<b>";
const char WEB_CSV[] PROGMEM = "CSV";
const char WEB_MADAVI[] PROGMEM =
    "<a target='_blank' "
    "href='https://www.madavi.de/ok-lab-stuttgart/'>Madavi.de</a>";
const char WEB_SENSORCOMMUNITY[] PROGMEM =
    "<a target='_blank' "
    "href='https://archive.sensor.community/00disclamer.md'>Sensor.Community</"
    "a>";
const char WEB_HTTPS[] PROGMEM = "HTTPS";
const char WEB_NBSP_NBSP_BRACE[] PROGMEM = "&nbsp;&nbsp;(";
const char WEB_REPLN_REPLV[] PROGMEM = "\"{n}\":\"{v}\",";
const char WEB_PM1[] PROGMEM = "PM1";
const char WEB_PM25[] PROGMEM = "PM2.5";
const char WEB_PM10[] PROGMEM = "PM10";
const char WEB_PM4[] PROGMEM = "PM4";

// OTA Update page content
const char WEB_UPDATE_PAGE[] PROGMEM = "\
<div class='section-card'>\
<div class='section-title' style='display:flex;align-items:center;gap:8px'>\
<svg width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><path d=\"M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4\"/><polyline points=\"7 10 12 15 17 10\"/><line x1=\"12\" y1=\"15\" x2=\"12\" y2=\"3\"/></svg>\
" INTL_UPDATE_FIRMWARE "</div>\
<div id='update-status'></div>\
</div>\
<script>\
function checkUpdate(){\
document.getElementById('update-status').innerHTML='<div style=\"text-align:center;padding:20px\"><div style=\"border:3px solid #e1e8ed;border-top-color:#438DBF;border-radius:50%;width:40px;height:40px;animation:spin 1s linear infinite;margin:0 auto 16px\"></div><p>" INTL_UPDATE_CHECKING "</p></div><style>@keyframes spin{to{transform:rotate(360deg)}}</style>';\
fetch('/update?action=check').then(r=>r.json()).then(d=>{renderStatus(d)}).catch(e=>{document.getElementById('update-status').innerHTML='<div style=\"color:#e74c3c;padding:20px;text-align:center\">" INTL_UPDATE_ERROR ": '+e+'</div>'});\
}\
function startUpdate(){\
if(!confirm('" INTL_UPDATE_WARNING "'))return;\
document.getElementById('update-status').innerHTML='<div style=\"text-align:center;padding:30px\"><div style=\"background:#fff5e6;border:2px solid #f39c12;border-radius:10px;padding:16px;margin-bottom:20px\"><svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#f39c12\" stroke-width=\"2\" style=\"vertical-align:middle;margin-right:8px\"><path d=\"M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z\"/><line x1=\"12\" y1=\"9\" x2=\"12\" y2=\"13\"/><line x1=\"12\" y1=\"17\" x2=\"12.01\" y2=\"17\"/></svg><strong>" INTL_UPDATE_WARNING "</strong></div><div style=\"margin-bottom:16px\"><div style=\"border:3px solid #e1e8ed;border-top-color:#438DBF;border-radius:50%;width:50px;height:50px;animation:spin 1s linear infinite;margin:0 auto\"></div></div><p style=\"font-size:18px;font-weight:600\">" INTL_UPDATE_IN_PROGRESS "</p><div id=\"progress-bar\" style=\"background:#e1e8ed;border-radius:10px;height:20px;margin:20px 0;overflow:hidden\"><div id=\"progress-fill\" style=\"background:linear-gradient(135deg,#438DBF 0%,#5ba3d0 100%);height:100%;width:0%;transition:width 0.3s\"></div></div><p id=\"progress-text\">0%</p></div><style>@keyframes spin{to{transform:rotate(360deg)}}</style>';\
fetch('/update?action=start').then(r=>r.json()).then(d=>{if(d.success){pollProgress();}else{document.getElementById('update-status').innerHTML='<div style=\"color:#e74c3c;padding:20px;text-align:center\">" INTL_UPDATE_FAILED ": '+d.error+'</div>';}}).catch(e=>{document.getElementById('update-status').innerHTML='<div style=\"color:#e74c3c;padding:20px;text-align:center\">" INTL_UPDATE_ERROR ": '+e+'</div>'});\
}\
var pollErrors=0;\
function pollProgress(){\
fetch('/update?action=progress').then(r=>r.json()).then(d=>{\
pollErrors=0;\
document.getElementById('progress-fill').style.width=d.progress+'%';\
document.getElementById('progress-text').textContent=d.progress+'%';\
if(d.inProgress){setTimeout(pollProgress,500);}\
else if(d.success){showUpdateSuccess();}\
else if(d.failed){document.getElementById('update-status').innerHTML='<div style=\"text-align:center;padding:30px\"><svg width=\"64\" height=\"64\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#e74c3c\" stroke-width=\"2\"><circle cx=\"12\" cy=\"12\" r=\"10\"/><line x1=\"15\" y1=\"9\" x2=\"9\" y2=\"15\"/><line x1=\"9\" y1=\"9\" x2=\"15\" y2=\"15\"/></svg><p style=\"font-size:18px;font-weight:600;color:#e74c3c;margin-top:16px\">" INTL_UPDATE_FAILED "</p><p style=\"margin-top:8px\">'+d.error+'</p><button onclick=\"checkUpdate()\" class=\"b\" style=\"margin-top:20px;display:inline-block;max-width:200px\">" INTL_UPDATE_CHECK "</button></div>';}\
}).catch(e=>{\
pollErrors++;\
if(pollErrors>=3){showUpdateSuccess();}else{setTimeout(pollProgress,1000);}\
});\
}\
function showUpdateSuccess(){\
document.getElementById('update-status').innerHTML='<div style=\"text-align:center;padding:30px\"><svg width=\"64\" height=\"64\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#27ae60\" stroke-width=\"2\"><path d=\"M22 11.08V12a10 10 0 1 1-5.93-9.14\"/><polyline points=\"22 4 12 14.01 9 11.01\"/></svg><p style=\"font-size:18px;font-weight:600;color:#27ae60;margin-top:16px\">" INTL_UPDATE_SUCCESS "</p><p style=\"margin-top:8px\">" INTL_UPDATE_REBOOT
                                       "</p><p id=\"reconnect-msg\" style=\"margin-top:16px;color:#666\">Reconnexion dans <span id=\"countdown\">30</span>s...</p></div>';\
let sec=30;const cd=setInterval(()=>{sec--;document.getElementById('countdown').textContent=sec;if(sec<=0){clearInterval(cd);tryReconnect();}},1000);\
}\
function tryReconnect(){\
document.getElementById('reconnect-msg').innerHTML='Tentative de reconnexion...';\
fetch('/').then(()=>{window.location.href='/update';}).catch(()=>{setTimeout(tryReconnect,3000);});\
}\
function renderStatus(d){\
let html='<table class=\"v\"><tr><td>" INTL_UPDATE_CURRENT_VERSION
                                       "</td><td><strong>'+d.currentVersion+'</strong></td></tr>';\
if(d.updateAvailable){html+='<tr><td>" INTL_UPDATE_NEW_VERSION
                                       "</td><td><strong style=\"color:#27ae60\">'+d.availableVersion+'</strong></td></tr>';}\
html+='<tr><td>" INTL_UPDATE_LAST_CHECK "</"
                                        "td><td>'+(d.lastCheck||"
                                        "'" INTL_UPDATE_NEVER
                                       "')+'</td></tr></table>';\
if(d.updateAvailable){\
html+='<div style=\"background:#e8f5e9;border:2px solid #27ae60;border-radius:10px;padding:16px;margin:16px 0;text-align:center\"><svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#27ae60\" stroke-width=\"2\" style=\"vertical-align:middle;margin-right:8px\"><path d=\"M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4\"/><polyline points=\"7 10 12 15 17 10\"/><line x1=\"12\" y1=\"15\" x2=\"12\" y2=\"3\"/></svg><strong style=\"color:#27ae60\">" INTL_UPDATE_AVAILABLE
                                       "!</strong></div>';\
html+='<div style=\"display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-top:16px\"><button onclick=\"startUpdate()\" class=\"b\" style=\"margin:0;background:linear-gradient(135deg,#27ae60 0%,#2ecc71 100%);box-shadow:0 4px 12px rgba(39,174,96,0.25)\">" INTL_UPDATE_START
                                       "</button><button "
                                       "onclick=\"checkUpdate()\" class=\"b\" "
                                       "style=\"margin:0\">" INTL_UPDATE_CHECK
                                       "</button></div>';\
}else{\
html+='<div style=\"background:#f8f9fa;border:2px solid #e1e8ed;border-radius:10px;padding:16px;margin:16px 0;text-align:center\"><svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"#27ae60\" stroke-width=\"2\" style=\"vertical-align:middle;margin-right:8px\"><path d=\"M22 11.08V12a10 10 0 1 1-5.93-9.14\"/><polyline points=\"22 4 12 14.01 9 11.01\"/></svg><strong>" INTL_UPDATE_NOT_AVAILABLE
                                       "</strong></div>';\
html+='<button onclick=\"checkUpdate()\" class=\"b\" style=\"margin-top:16px\">" INTL_UPDATE_CHECK
                                       "</button>';\
}\
document.getElementById('update-status').innerHTML=html;\
}\
checkUpdate();\
</script>";

// OTA Update banner for homepage (shown when update is available)
const char WEB_UPDATE_BANNER[] PROGMEM = "\
<div style='background:linear-gradient(135deg,#27ae60 0%,#2ecc71 100%);border-radius:12px;padding:16px;margin-bottom:16px;box-shadow:0 4px 12px rgba(39,174,96,0.25)'>\
<a href='/update' style='display:flex;align-items:center;gap:12px;color:#fff;text-decoration:none'>\
<svg width=\"32\" height=\"32\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><path d=\"M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4\"/><polyline points=\"7 10 12 15 17 10\"/><line x1=\"12\" y1=\"15\" x2=\"12\" y2=\"3\"/></svg>\
<div style='flex:1'>\
<div style='font-size:16px;font-weight:600'>" INTL_UPDATE_AVAILABLE "</div>\
<div style='font-size:13px;opacity:0.9'>{v}</div>\
</div>\
<svg width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\"><line x1=\"5\" y1=\"12\" x2=\"19\" y2=\"12\"/><polyline points=\"12 5 19 12 12 19\"/></svg>\
</a>\
</div>";
const char WEB_NC0k5[] PROGMEM = "NC0.5";
const char WEB_NC1k0[] PROGMEM = "NC1.0";
const char WEB_NC2k5[] PROGMEM = "NC2.5";
const char WEB_NC4k0[] PROGMEM = "NC4.0";
const char WEB_NC10[] PROGMEM = "NC10";
const char WEB_TPS[] PROGMEM = "TPS";
const char WEB_GPS[] PROGMEM = "GPS";
