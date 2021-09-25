local core = require("apisix.core")

local http = require("resty.http")

local plugin_name = "my-auth"

local authUrl = "http://192.168.31.130:8009/common/v1/user_info"


local schema = {
    type = "object",
    properties = {
        urls = { type = "array" },
        param = { type = "string" },
        limit = { type = "integer" },
    },
    required = { "urls", "param", "limit" }
}

local _M = {
    version = 0.1,
    priority = 11010,
    name = plugin_name,
    schema = schema,
}

function _M.check_schema(conf)
    return core.schema.check(schema, conf)
end

local function newClient()
    local httpC = http.new()
    httpC:set_timeout(timeout)
    return httpC
end

local function httpGet(url, sid)
    local httpc = newClient()
    local res, err = httpc:request_uri(url, {
        method = "GET",
        headers = {
            ["Cookie"] = "wps_sid=" .. sid,
        }
    })
    if err or (not res) then
        return nil
    end
    if res.status == ngx.HTTP_OK then
        return res
    end
    return nil
end

local function getUserInfo(sid)
    local res = httpGet(authUrl, sid)
    if res == nil then
        return nil
    end
    return res
end

local function get_request_table()
    local method = ngx.req.get_method()
    local content_type = ngx.req.get_headers()["Content-Type"] or ""
    if string.find(content_type, "application/json", 1, true) and
            (method == "POST" or method == "PUT" or method == "PATCH")
    then
        local req_body, _ = core.request.get_body()
        if req_body then
            local data, _ = json.decode(req_body)
            if data then
                return data
            end
        end
    end
    if method == "POST" then
        return ngx.req.get_post_args()
    end
    return ngx.req.get_uri_args()
end

function _M.rewrite(conf, ctx)
    core.log1.error("hello, conf: ", core.json.delay_encode(conf))
    local key = ctx.var.method .. ":" .. ctx.var.uri
    for i = 1, #conf.urls do
        while true do
            local start_i, end_j, substr = string.find(conf.urls[i], key)
            -- url 匹配
            if start_i == 1 then
                -- 1.判断param是否存在，且超过限制
                local request_args_tab = get_request_table()
                local value = request_args_tab[conf.param]
                if value == nil then
                    return 403, { message = "invalid parameter" }
                elseif tonumber(value) <= conf.limit then
                    break
                end
                -- 2.获取wps_sid
                local wps_sid = ngx.var.cookie_wps_sid
                if wps_sid == nil then
                    return 403, { message = "wps_sid empty" }
                end
                -- 3.发起http请求
                local res = getUserInfo(wps_sid)
                -- 4.解析响应数据
                if res == nil then
                    return 403, { message = "internal server error" }
                end
                local body = core.json.decode(res.body)
                if body.result == "ok" and tonumber(body.data) > 0 then
                    break
                end
                return 403, { message = "required login" }
            end
            break
        end
    end
end

return _M
