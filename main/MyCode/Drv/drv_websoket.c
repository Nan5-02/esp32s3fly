#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_http_server.h"
#include "esp_log.h"

#include "drv_websoket.h"

#if 0  // ===== WebSocket / 网页方案已停用（改用 UDP），以下代码保留备用 =====
static const char *TAG = "DRONE_WS";

#define WS_MAX_CLIENTS 8
#define CONTROL_TIMEOUT_MS 1500
#define STATE_BROADCAST_MS 200

typedef struct {
    int roll;
    int pitch;
    int yaw;
    int throttle;
    int armed;
} joystick_data_t;

static joystick_data_t g_joystick = {0, 0, 0, 0, 0};
static uint32_t g_last_control_ms = 0;
static bool g_failsafe_active = true;

static const char index_html[] =
"<!doctype html><html><head><meta charset='utf-8'>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<title>ESP32 Drone Console</title>"
"<style>"
":root{--bg:#0f172a;--card:#111827;--line:#334155;--txt:#e2e8f0;--ok:#22c55e;--warn:#f59e0b;--accent:#38bdf8;}"
"body{margin:0;font-family:ui-monospace,Menlo,Consolas,monospace;background:radial-gradient(circle at 20% 10%,#1e293b,#0f172a 45%);color:var(--txt);}"
".wrap{max-width:980px;margin:20px auto;padding:12px;}"
".card{background:rgba(17,24,39,.86);border:1px solid var(--line);border-radius:12px;padding:14px;margin-bottom:12px;}"
".row{display:flex;gap:10px;flex-wrap:wrap;align-items:center;}"
"button{border:0;border-radius:10px;padding:10px 14px;background:#2563eb;color:#fff;font-weight:700;cursor:pointer;}"
"button.alt{background:#059669;}"
".badge{display:inline-block;padding:4px 8px;border-radius:999px;background:#334155;}"
".ctrl{display:grid;grid-template-columns:1fr 1fr;gap:18px;margin-top:12px;}"
".joystick-wrap{display:flex;flex-direction:column;align-items:center;gap:8px;}"
".stick{width:260px;height:260px;border-radius:50%;position:relative;background:radial-gradient(circle at 50% 50%,#1f2937,#0b1220);border:1px solid #475569;touch-action:none;user-select:none;}"
".stick::before{content:'';position:absolute;left:50%;top:0;bottom:0;width:1px;background:#334155;transform:translateX(-50%);}"
".stick::after{content:'';position:absolute;top:50%;left:0;right:0;height:1px;background:#334155;transform:translateY(-50%);}"
".knob{position:absolute;left:50%;top:50%;width:82px;height:82px;border-radius:50%;transform:translate(-50%,-50%);background:radial-gradient(circle at 30% 30%,#7dd3fc,#0ea5e9);box-shadow:0 10px 30px rgba(14,165,233,.35);}"
".knob.right{background:radial-gradient(circle at 30% 30%,#fde68a,#f59e0b);box-shadow:0 10px 30px rgba(245,158,11,.35);}"
".meters{display:grid;grid-template-columns:repeat(2,minmax(140px,1fr));gap:10px;margin-top:8px;}"
".meter{display:flex;justify-content:space-between;border:1px solid var(--line);border-radius:8px;padding:8px 10px;background:#020617;}"
"input[type=range]{width:100%;}"
".small{color:#94a3b8;font-size:12px;}"
"#log{height:200px;overflow:auto;white-space:pre-wrap;border:1px solid var(--line);border-radius:10px;padding:8px;background:#020617;}"
"@media (max-width:820px){.ctrl{grid-template-columns:1fr;}.stick{width:220px;height:220px;}}"
"</style></head><body><div class='wrap'>"
"<div class='card'><div class='row'><button id='btnConnect'>Connect WS</button><button id='btnArm' class='alt'>ARM</button><span class='badge' id='status'>DISCONNECTED</span></div>"
"<div class='ctrl'>"
"<div class='joystick-wrap'><div id='stickL' class='stick'><div id='knobL' class='knob'></div></div><div class='small'>左摇杆: X=Roll, Y=Pitch</div></div>"
"<div class='joystick-wrap'><div id='stickR' class='stick'><div id='knobR' class='knob right'></div></div><div class='small'>右摇杆: X=Yaw, Y=Throttle</div></div>"
"</div><div class='meters'>"
"<div class='meter'><span>Roll</span><strong id='vRoll'>0</strong></div>"
"<div class='meter'><span>Pitch</span><strong id='vPitch'>0</strong></div>"
"<div class='meter'><span>Yaw</span><strong id='vYaw'>0</strong></div>"
"<div class='meter'><span>Throttle</span><strong id='vThrottle'>0</strong></div>"
"</div></div>"
"<div class='card'><div id='log'></div></div></div>"
"<script>"
"let ws=null,armed=0,seq=0;"
"let reconnectTimer=null,reconnectDelay=600;"
"const $=id=>document.getElementById(id);"
"const stickL=$('stickL'),knobL=$('knobL');"
"const stickR=$('stickR'),knobR=$('knobR');"
"const ctl={roll:0,pitch:0,yaw:0,throttle:0};"
"const joy={L:{drag:false,r:0,max:0},R:{drag:false,r:0,max:0}};"
"function log(s){const el=$('log');el.textContent+='\\n'+s;const lines=el.textContent.split('\\n');if(lines.length>140)el.textContent=lines.slice(lines.length-140).join('\\n');el.scrollTop=el.scrollHeight;}"
"function clamp(v,min,max){return Math.max(min,Math.min(max,v));}"
"function updateVals(){$('vRoll').textContent=ctl.roll;$('vPitch').textContent=ctl.pitch;$('vYaw').textContent=ctl.yaw;$('vThrottle').textContent=ctl.throttle;}"
"function sendControl(){if(!ws||ws.readyState!==1)return;seq++;ws.send(JSON.stringify({type:'control',seq,roll:ctl.roll,pitch:ctl.pitch,yaw:ctl.yaw,throttle:ctl.throttle,armed}));}"
"function scheduleReconnect(){if(reconnectTimer)clearTimeout(reconnectTimer);reconnectTimer=setTimeout(()=>{connect();reconnectDelay=Math.min(reconnectDelay+300,2500);},reconnectDelay);}"
"function connect(){if(ws&&(ws.readyState===0||ws.readyState===1))return;ws=new WebSocket(`ws://${location.host}/ws`);"
"ws.onopen=()=>{reconnectDelay=600;$('status').textContent='CONNECTED';$('status').style.background='#166534';ws.send(JSON.stringify({type:'hello',ver:1,client:'web'}));log('[TX] hello');};"
"ws.onclose=()=>{$('status').textContent='DISCONNECTED';$('status').style.background='#7f1d1d';log('[SYS] closed, reconnecting...');scheduleReconnect();};"
"ws.onerror=()=>log('[SYS] ws error');"
"ws.onmessage=e=>{try{const m=JSON.parse(e.data);if(m.type==='state'){ctl.roll=m.roll|0;ctl.pitch=m.pitch|0;ctl.yaw=m.yaw|0;ctl.throttle=m.throttle|0;draw();return;}if(m.type!=='pong'&&m.type!=='ack'){log('[RX] '+e.data);}}catch(_){log('[RX] '+e.data);}};}"
"function setKnob(node,max,x,y){node.style.transform=`translate(calc(-50% + ${x*max}px),calc(-50% + ${y*max}px))`;}"
"function draw(){setKnob(knobL,joy.L.max,ctl.roll/100,-ctl.pitch/100);setKnob(knobR,joy.R.max,ctl.yaw/100,-((ctl.throttle-50)/50));updateVals();}"
"function applyLeft(nx,ny){ctl.roll=Math.round(clamp(nx,-1,1)*100);ctl.pitch=Math.round(clamp(-ny,-1,1)*100);draw();sendControl();}"
"function applyRight(nx,ny){ctl.yaw=Math.round(clamp(nx,-1,1)*100);const t=((clamp(-ny,-1,1)+1)/2)*100;ctl.throttle=Math.round(clamp(t,0,100));draw();sendControl();}"
"function pointToNorm(stick,limit,clientX,clientY){const r=stick.getBoundingClientRect();const cx=r.left+r.width/2;const cy=r.top+r.height/2;let dx=clientX-cx;let dy=clientY-cy;const d=Math.hypot(dx,dy);if(d>limit){dx=dx*limit/d;dy=dy*limit/d;}return {x:dx/limit,y:dy/limit};}"
"function down(side,e){joy[side].drag=true;const p=e.touches?e.touches[0]:e;const n=pointToNorm(side==='L'?stickL:stickR,joy[side].max,p.clientX,p.clientY);if(side==='L')applyLeft(n.x,n.y);else applyRight(n.x,n.y);if(e.cancelable)e.preventDefault();}"
"function move(e){let dirty=false;const p=e.touches?e.touches[0]:e;if(joy.L.drag){const n=pointToNorm(stickL,joy.L.max,p.clientX,p.clientY);applyLeft(n.x,n.y);dirty=true;}if(joy.R.drag){const n=pointToNorm(stickR,joy.R.max,p.clientX,p.clientY);applyRight(n.x,n.y);dirty=true;}if(dirty&&e.cancelable)e.preventDefault();}"
"function up(){if(joy.L.drag){joy.L.drag=false;ctl.roll=0;ctl.pitch=0;}if(joy.R.drag){joy.R.drag=false;ctl.yaw=0;ctl.throttle=0;}draw();sendControl();}"
"$('btnConnect').onclick=connect;"
"$('btnArm').onclick=()=>{armed=armed?0:1;$('btnArm').textContent=armed?'DISARM':'ARM';sendControl();};"
"stickL.addEventListener('mousedown',e=>down('L',e));stickR.addEventListener('mousedown',e=>down('R',e));"
"window.addEventListener('mousemove',move);window.addEventListener('mouseup',up);"
"stickL.addEventListener('touchstart',e=>down('L',e),{passive:false});stickR.addEventListener('touchstart',e=>down('R',e),{passive:false});"
"window.addEventListener('touchmove',move,{passive:false});window.addEventListener('touchend',up);"
"function initSticks(){joy.L.r=stickL.clientWidth/2;joy.R.r=stickR.clientWidth/2;joy.L.max=joy.L.r-44;joy.R.max=joy.R.r-44;draw();}"
"window.addEventListener('resize',initSticks);initSticks();"
"setInterval(()=>{if(ws&&ws.readyState===1)sendControl();},120);"
"setInterval(()=>{if(ws&&ws.readyState===1)ws.send(JSON.stringify({type:'ping',ts:Date.now()}));},2000);"
"window.addEventListener('online',connect);document.addEventListener('visibilitychange',()=>{if(!document.hidden)connect();});"
"</script></body></html>";

static bool json_get_int(const char *json, const char *key, int *out)
{
    char token[32];
    snprintf(token, sizeof(token), "\"%s\":", key);
    const char *p = strstr(json, token);
    if (p == NULL) {
        return false;
    }
    p += strlen(token);
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    char *end = NULL;
    long v = strtol(p, &end, 10);
    if (end == p) {
        return false;
    }
    *out = (int)v;
    return true;
}

static bool json_get_string(const char *json, const char *key, char *out, size_t out_size)
{
    char token[32];
    snprintf(token, sizeof(token), "\"%s\":", key);
    const char *p = strstr(json, token);
    if (p == NULL) {
        return false;
    }
    p += strlen(token);
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    if (*p != '\"') {
        return false;
    }
    p++;
    const char *q = strchr(p, '\"');
    if (q == NULL) {
        return false;
    }
    size_t len = (size_t)(q - p);
    if (len + 1 > out_size) {
        return false;
    }
    memcpy(out, p, len);
    out[len] = '\0';
    return true;
}

static esp_err_t ws_send_text_to_fd(httpd_handle_t server, int fd, const char *text)
{
    httpd_ws_frame_t frame = {
        .final = true,
        .fragmented = false,
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t *)text,
        .len = strlen(text),
    };
    return httpd_ws_send_frame_async(server, fd, &frame);
}

/**
 * @brief 向所有已连接的 WebSocket 客户端广播一条文本消息
 * @param server HTTP 服务器句柄
 * @param text 要广播的文本消息
 */
static void ws_broadcast_text(httpd_handle_t server, const char *text)
{
    if (server == NULL || text == NULL) {
        return;
    }

    int fds[WS_MAX_CLIENTS] = {0};
    size_t count = WS_MAX_CLIENTS;
    if (httpd_get_client_list(server, &count, fds) != ESP_OK) {
        return;
    }

    for (size_t i = 0; i < count; i++) {
        if (httpd_ws_get_fd_info(server, fds[i]) == HTTPD_WS_CLIENT_WEBSOCKET) {
            esp_err_t ret = ws_send_text_to_fd(server, fds[i], text);
            if (ret != ESP_OK) {
                ESP_LOGW(TAG, "WS send failed fd=%d, closing socket: %s", fds[i], esp_err_to_name(ret));
                httpd_sess_trigger_close(server, fds[i]);
            }
        }
    }
}

static esp_err_t index_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t telemetry_get_handler(httpd_req_t *req)
{
    char json[128];
    snprintf(json, sizeof(json),
             "{\"roll\":%d,\"pitch\":%d,\"yaw\":%d,\"throttle\":%d,\"armed\":%d}",
             g_joystick.roll, g_joystick.pitch,
             g_joystick.yaw, g_joystick.throttle, g_joystick.armed);

    return httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "WS handshake completed");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt = {0};
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame(len) failed: %s", esp_err_to_name(ret));
        return ret;
    }

    if (ws_pkt.len == 0) {
        return ESP_OK;
    }

    uint8_t *buf = calloc(1, ws_pkt.len + 1);
    if (buf == NULL) {
        return ESP_ERR_NO_MEM;
    }

    ws_pkt.payload = buf;
    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame(payload) failed: %s", esp_err_to_name(ret));
        free(buf);
        return ret;
    }

    char *msg = (char *)ws_pkt.payload;
    int fd = httpd_req_to_sockfd(req);
    char type[20] = {0};

    if (!json_get_string(msg, "type", type, sizeof(type))) {
        ws_send_text_to_fd(req->handle, fd, "{\"type\":\"error\",\"code\":\"bad_format\",\"msg\":\"missing type\"}");
        free(buf);
        return ESP_OK;
    }

    if (strcmp(type, "hello") == 0) {
        ws_send_text_to_fd(req->handle, fd, "{\"type\":\"hello_ack\",\"ok\":1,\"proto\":1,\"server\":\"esp32s3\"}");
    } else if (strcmp(type, "ping") == 0) {
        int ts = 0;
        json_get_int(msg, "ts", &ts);
        char rsp[96];
        snprintf(rsp, sizeof(rsp), "{\"type\":\"pong\",\"ts\":%d}", ts);
        ws_send_text_to_fd(req->handle, fd, rsp);
    } else if (strcmp(type, "control") == 0) {
        int seq = 0;
        int roll = 0;
        int pitch = 0;
        int yaw = 0;
        int throttle = 0;
        int armed = 0;

        bool ok = json_get_int(msg, "seq", &seq)
                  && json_get_int(msg, "roll", &roll)
                  && json_get_int(msg, "pitch", &pitch)
                  && json_get_int(msg, "yaw", &yaw)
                  && json_get_int(msg, "throttle", &throttle)
                  && json_get_int(msg, "armed", &armed);

        if (!ok) {
            ws_send_text_to_fd(req->handle, fd, "{\"type\":\"error\",\"code\":\"invalid_control\",\"msg\":\"missing fields\"}");
            free(buf);
            return ESP_OK;
        }

        if (roll < -100) roll = -100;
        if (roll > 100) roll = 100;
        if (pitch < -100) pitch = -100;
        if (pitch > 100) pitch = 100;
        if (yaw < -100) yaw = -100;
        if (yaw > 100) yaw = 100;
        if (throttle < 0) throttle = 0;
        if (throttle > 100) throttle = 100;
        armed = armed ? 1 : 0;

        g_joystick.roll = roll;
        g_joystick.pitch = pitch;
        g_joystick.yaw = yaw;
        g_joystick.throttle = throttle;
        g_joystick.armed = armed;
        g_last_control_ms = pdTICKS_TO_MS(xTaskGetTickCount());
        g_failsafe_active = false;

        static uint32_t last_rx_log_ms = 0;
        uint32_t now_ms = pdTICKS_TO_MS(xTaskGetTickCount());
        if (now_ms - last_rx_log_ms >= 1000) {
            last_rx_log_ms = now_ms;
            ESP_LOGI(TAG, "RX JOY seq=%d roll=%d pitch=%d yaw=%d throttle=%d armed=%d",
                     seq, g_joystick.roll, g_joystick.pitch,
                     g_joystick.yaw, g_joystick.throttle, g_joystick.armed);
        }

        if (seq % 5 == 0) {
            char ack[96];
            snprintf(ack, sizeof(ack), "{\"type\":\"ack\",\"seq\":%d,\"ok\":1}", seq);
            ws_send_text_to_fd(req->handle, fd, ack);
        }
    } else {
        ws_send_text_to_fd(req->handle, fd, "{\"type\":\"error\",\"code\":\"unknown_type\"}");
    }

    free(buf);
    return ESP_OK;
}

static const httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t telemetry_uri = {
    .uri = "/telemetry",
    .method = HTTP_GET,
    .handler = telemetry_get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t ws_uri = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = ws_handler,
    .user_ctx = NULL,
    .is_websocket = true,
    .handle_ws_control_frames = false,
    .supported_subprotocol = "drone.v1",
};



/**
 * @brief 注册 WebSocket 和 HTTP 端点处理程序
 * @param server HTTP 服务器句柄
 */
void Drv_Websocket_Register_Handlers(httpd_handle_t server)
{
    httpd_register_uri_handler(server, &index_uri);
    httpd_register_uri_handler(server, &telemetry_uri);
    httpd_register_uri_handler(server, &ws_uri);

    g_last_control_ms = pdTICKS_TO_MS(xTaskGetTickCount());
    g_failsafe_active = true;

    ESP_LOGI(TAG, "Web endpoints registered: /, /telemetry, WS /ws");
}

/**
 * @brief 广播遥测数据给所有已连接的 WebSocket 客户端
 * @param server HTTP 服务器句柄
 */
void Drv_Websocket_Broadcast_Telemetry(httpd_handle_t server)
{
    if (server == NULL) {
        return;
    }

    static uint32_t last_broadcast_ms = 0;
    uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());

    if ((now - g_last_control_ms) > CONTROL_TIMEOUT_MS && !g_failsafe_active) {
        g_joystick.roll = 0;
        g_joystick.pitch = 0;
        g_joystick.yaw = 0;
        g_joystick.throttle = 0;
        g_joystick.armed = 0;
        g_failsafe_active = true;
        ESP_LOGW(TAG, "FAILSAFE triggered: control timeout, motors disarmed");
    }

    if (now - last_broadcast_ms >= STATE_BROADCAST_MS) {
        last_broadcast_ms = now;
        char state[128];
        snprintf(state, sizeof(state),
                 "{\"type\":\"state\",\"roll\":%d,\"pitch\":%d,\"yaw\":%d,\"throttle\":%d,\"armed\":%d}",
                 g_joystick.roll, g_joystick.pitch, g_joystick.yaw,
                 g_joystick.throttle, g_joystick.armed);
        ws_broadcast_text(server, state);
    }
}
#endif  // ===== WebSocket / 网页方案已停用 =====

