/// @file nav-gateways.js
/// @Brief
/// @author QRS
/// @version 0.0.9
/// @date 2019-02-16

var Gateways = function(ui) {
    this.ui = ui;
    this.gws = {
        "konke": {"icon": "bridge_konke", "label":"{{bridge_konke}}"},
        "hue": {"icon": "bridge_hue", "label":"{{bridge_hue}}"},
    };
    this.expandStates = [];
    this.checkStatusCount = 5;
    this.checkIntervalTimer = null;
    that_g = this;
}

Gateways.prototype.initUI = function() {
    console.log("Gateways initUI");
    that_g.ui.showLoadingIndicator("GATEWAYS LOADING...");

    that_g.list(function(resData) {
        that_g.ui.displayNav(global.NAV_GATEWAYS);
        var body = document.getElementById("body-gateways");
        var intro = document.getElementById("introduction-gateways");
        var html = "";
        if (resData.length == 0) {
            body.style.display = "none";
            intro.style.display = "";
            return;
        }
        for (var i = 0, l = resData.length; i < l; ++i) {
            var gatewayId = resData[i]["gatewayId"];
            var type = resData[i]["gatewayType"];
            var owned = resData[i]["ownedStatus"];
            var key = resData[i]["accessKey"];

            html += '<div';
            html += '    id="' +  gatewayId + '"';
            html += '    class="device-list-item">';
            html += '    <div class="device-list-item-header">';
            html += '        <button';
            html += '            id="' + gatewayId + '_iconbutton"';
            html += '            class="iconselection-button"';
            html += '            onclick="onRuleModify(' + i + ')";>';
            html += '            <img';
            html += '                id="' + gatewayId + '_icon"';
            html += '                class="device-list-item-header-icon"';
            html += '                src="' + getIconUrl(that_g.gws[type].icon) + '"';
            html += '                alt=""/>';
            html += '        </button>';
            html += '        <div class="device-list-item-header-rest">';
            html += '            <div class="device-list-item-header-title">';
            html += '                <label id="' + gatewayId + '"> <span>' + gatewayId + '</span></label>';
            html += '            </div>';
            html += '            <button';
            html += '                class="default-icon-button list-icon-button net-icon"';
            html += '                onclick="onGatewayNet(\'' + gatewayId + '\')"';
            html += '            ></button>';
            html += '            <button';
            html += '                class="default-icon-button list-icon-button trash-icon"';
            html += '                onclick="onGatewayDelete(\'' + gatewayId + '\')"';
            html += '            ></button>';
            html += '           <button';
            html += '               id="' + 'toggleExpand' + gatewayId + '"';
            html += '               class="default-icon-button list-icon-button close-icon"';
            html += '               onclick="onToggleContainer(\'' + gatewayId + '\')"';
            html += '           ></button>';
            html += '        </div>';
            html += '    </div>';
            html += '    <div id="description' + gatewayId + '"';
            html += '        style="display:' + (that_g.expandStates[gatewayId] == true ? 'block' : 'none') + ';"';
            html += '        class="device-list-item-body">';
            html += '        <div class="device-list-item-resource">';
            html += '            <div class="device-list-item-property">';
            html += '               <div class="device-list-item-property-title">{{gateway_type}}</div>';
            html += '               <div class="device-list-item-property-value">';
            html += '                  <label><span>' + type + '</span></label>';
            html += '               </div>';
            html += '            </div>';
            html += '            <div class="device-list-item-property">';
            html += '               <div class="device-list-item-property-title">{{id}}</div>';
            html += '               <div class="device-list-item-property-value">';
            html += '                  <label><span>' + gatewayId + '</span></label>';
            html += '               </div>';
            html += '            </div>';
            html += '            <div class="device-list-item-property">';
            html += '               <div class="device-list-item-property-title">{{gateway_access_key}}</div>';
            html += '               <div class="device-list-item-property-value">';
            html += '                  <label><span>' + key + '</span></label>';
            html += '               </div>';
            html += '            </div>';
            html += '        </div>';
            html += '    </div>';
            html += '</div>';
        }
        body.innerHTML = html;
        intro.style.display = "none";
        body.style.display = "";
    });
}

Gateways.prototype.toggleExpandBody = function(gid) {
    var toggleExpandId = "toggleExpand" + gid;
    var descrBodyId = "description" + gid;
    var descrElem = document.getElementById(descrBodyId);

    if (that_g.expandStates[gid] == true) {
        removeClass(descrBodyId, "device-list-item-open");
        if (descrElem)
            descrElem.style.display = "none";
        removeClass(toggleExpandId , "open-icon");
        addClass(toggleExpandId , "close-icon");
        that_g.expandStates[gid] = false;
    } else {
        addClass(descrBodyId, "device-list-item-open");
        if (descrElem)
            descrElem.style.display = "block";
        removeClass(toggleExpandId, "close-icon");
        addClass(toggleExpandId, "open-icon");
        that_g.expandStates[gid] = true;
    }
}

Gateways.prototype.onSelect = function() {
    var html = "";
    html += '<h3 class="modal-title">';
    html +=      "{{gateway_add_title}}";
    html += '</h3>';
    html += '<p class="modal-info-text">';
    html +=      "{{gateway_add_info_text}}";
    html += '</p>';

    html += '<div class="modal-scrollarea">';
    for (var type in that_g.gws) {
        var gw = that_g.gws[type];
        html += '<label class="device-selection-item">';
        html += '    <img class="device-selection-item-icon" src="' + getIconUrl(gw.icon) + '" />';
        html += '    <div class="device-selection-item-rest">';
        html += '        <span class="device-selection-item-name">';
        html +=              gw.label;
        html += '        </span>';
        html += '        <button id="' + type + '"';
        html += '              class="btn-primary"';
        html += '              onclick="onSelectGateway(\'' + type + '\')">';
        html +=             "{{gateway_add}}";
        html += '        </button>';
        html += '    </div>'
        html += '</label>';
    }
    html += '</div>';

   that_g.ui.openModal(html);
}

Gateways.prototype.onAdd = function(type) {
    var html = "";
    html += '<h3 class="modal-title">';
    html +=      "{{gateway_add_title}}";
    html += '</h3>';
    html += '<p class="modal-info-text">' + "{{gateway_add}}: " + type + '</p>';

    html += '<div id="gateway-onbind-modal" class="form-container" style="display:block">'

    switch (type) {
        case 'konke':
            html += '<div class="device-list-item-property">';
            html += '    <input id = "gateway-id"';
            html += '         onchange="onChangeGatewayInfo();"';
            html += '         type="text" placeholder="{{id}}" class="rule-input">';
            html += '</div>';
            html += '<div class="device-list-item-property">';
            html += '    <input id = "access-key"';
            html += '         onchange="onChangeGatewayInfo();"';
            html += '         type="text" placeholder="{{gateway_access_key}}" class="rule-input">';
            html += '</div>';
            break;
        case 'hue':
            html += '<div class="device-list-item-property">';
            html += '    <input id = "gateway-id"';
            html += '         onchange="onChangeGatewayInfo();"';
            html += '         type="text" placeholder="{{id}}" class="rule-input">';
            html += '</div>';
            html += '<div class="device-list-item-property">';
            html += '    <input id = "gateway-username"';
            html += '         onchange="onChangeGatewayInfo();"';
            html += '         type="text" placeholder="{{gateway_username}}" class="rule-input">';
            html += '</div>';
            html += '<div class="device-list-item-property">';
            html += '    <input id = "gateway-ip"';
            html += '         onchange="onChangeGatewayInfo();"';
            html += '         type="text" placeholder="{{gateway_ip}}" class="rule-input">';
            html += '</div>';
            break;
        default:
            ;
    }
    html += '    <div id="gateway_editmodal_errormsg" class="form-errormsg" style="">&nbsp;</div>';
    html += '    <center><button disable="" ';
    html += '        id="gateway-add-submit"';
    html += '        class="btn-primary" onclick="onGatewayAddSubmit(\'' + type + '\')" type="button">';
    html +=          "{{ok}}";
    html += '    </button></center>';
    html += '</div>';
    that_g.ui.openModal(html);
}

Gateways.prototype.addSubmit = function(type) {
    var gatewayId;
    var body = {};
    switch (type) {
        case 'konke':
            var id = document.getElementById("gateway-id");
            var key = document.getElementById("access-key");
            if (!id.value || !key.value) {
                that_g.displayErrorMessage("{{gateway_errmsg_args}}")
                return;
            }
            gatewayId = id.value;
            body = {"gatewayType": type, "gatewayId" : id.value, "accessKey": key.value};
            break;
        case 'hue':
            var id = document.getElementById("gateway-id");
            var username = document.getElementById("gateway-username");
            var ip = document.getElementById("gateway-ip");
            if (!id.value || !username.value || !ip.value) {
                that_g.displayErrorMessage("{{gateway_errmsg_args}}")
                return;
            }
            gatewayId = id.value;
            body = {
                "gatewayType": type,
                "gatewayId" : id.value,
                "accessKey" : username.value,
                "gatewayIp": ip.value};
            break;
        default:
            ;
    }
    var html = "";
    html += '<h3 class="modal-title">';
    html +=      "{{gateway_check_status}}";
    html += '</h3>';
    html += '<div class="spinning-wheel-container">';
    html += '    <div class="spinning-wheel"></div>';
    html += '    <div class="spinning-wheel-content-box">';
    html +=          "{{gateway_checking}}";
    html += '    </div>';
    html += '</div>';

    that_g.add(JSON.stringify(body));

    var c = 0;
    that_g.checkIntervalTimer = setInterval(function() {
        if (c < that_g.checkStatusCount) {
            that_g.status(gatewayId, function(resData) {
                if (resData["status"] == "2") {
                    clearInterval(that_g.checkIntervalTimer);
                    that_g.initUI();
                }
            });
        } else {
            clearInterval(that_g.checkIntervalTimer);
            that_g.ui.buildGenericErrorModal("{{gateway_add_title}}", "{{gateway_add_fail}}");
        }
        c = c + 1;
    }, 1000);
    that_g.ui.openModal(html, function() {
        clearInterval(that_g.checkIntervalTimer);
    });
}

Gateways.prototype.displayErrorMessage = function (msg) {
    var errDiv = document.getElementById("gateway_editmodal_errormsg");
    if (errDiv) {
        if (msg)
            errDiv.textContent = msg;
        else
            errDiv.innerHTML = "&nbsp;";
    }
}

Gateways.prototype.onDelete = function (bridgeId) {
    that_g.remove(bridgeId, function(resData) {
        that_g.initUI();
    });
}

Gateways.prototype.onNet = function(bridgeId) {
    console.log(bridgeId);
    var html = "";
    html += '<h3 class="modal-title">';
    html +=      "{{gateway_check_net}}";
    html += '</h3>';
    html += '<div class="spinning-wheel-container">';
    html += '    <div class="spinning-wheel"></div>';
    html += '    <div class="spinning-wheel-content-box">';
    html +=          "{{gateway_checking}}";
    html += '    </div>';
    html += '</div>';
    that_g.ui.openModal(html);
    that_g.net(bridgeId, function(resData) {
        if (resData["ret"] == "success") {
            var t = parseInt(resData["maxDuration"]);
            if (t < 0) {
                that_r.ui.buildGenericErrorModal();
                return;
            }
            setTimeout(that_g.initUI, t * 1000);
        }
    });
}

Gateways.prototype.add = function(body, finishCallback) {
    var uri = "/api/gateway/add";
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            return;
        }
        console.log("request %s :%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData["result"]);
                return;
            }
            that_r.ui.buildGenericErrorModal(
                    responseData["errors"]["code"],
                    responseData["errors"]["message"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Gateways.prototype.status = function(id, finishCallback) {
    var uri = "/api/gateway/status";
    var body = '{"gatewayId":"' + id + '"}';
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            return;
        }
        console.log("request %s :%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData["result"]);
                return;
            }
            that_r.ui.buildGenericErrorModal(
                   responseData["errors"]["code"],
                   responseData["errors"]["message"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Gateways.prototype.remove = function(id, finishCallback) {
    var uri = "/api/gateway/remove";
    var body = '{"gatewayId":"' + id + '"}';
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            return;
        }
        console.log("request %s :%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData["status"], responseData["result"]);
                return;
            }
            that_r.ui.buildGenericErrorModal(
                    responseData["errors"]["code"],
                    responseData["errors"]["message"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Gateways.prototype.list = function(finishCallback) {
    var uri = "/api/gateway/list";
    httpRequest("POST", uri, null, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            return;
        }
        console.log("request %s :%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData["result"]);
                return;
            }
            that_r.ui.buildGenericErrorModal(
                    responseData["errors"]["code"],
                    responseData["errors"]["message"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Gateways.prototype.net = function(id, finishCallback) {
    var uri = "/api/gateway/net";
    var body = '{"gatewayId":"' + id + '"}';
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            return;
        }
        console.log("request %s :%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData["result"]);
                return;
            }
            that_r.ui.buildGenericErrorModal(
                    responseData["errors"]["code"],
                    responseData["errors"]["message"]);
        } catch (err) {
            console.log(err);
        }
    });
}
