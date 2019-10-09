/// @file nav-devices.js
/// @Brief
/// @author QRS
/// @version 0.0.9
/// @date 2019-01-29

var Devices = function(ui) {
    this.ui = ui;
    this.devs = [];
    this.prfs = [];
    this.expandStates = [];
    this.updateInterval = 1000;
    this.updateIntervalTimer = null;
    this.updateValueSlot = [];
    this.uglyRetry = false;
    that_d = this;
}

Devices.prototype.initUI = function() {
    // console.log("Devices initUI");
    that_d.devs = [];
    that_d.ui.showLoadingIndicator("DEVICES LOADING...");
    that_d.list(function(devs) {
        var unCachePrfs = [];
        for(var i = 0, l = devs.length; i < l; ++i) {
            if (devs[i].ownedStatus !== "HB_binded")
                continue;
            var typeId = devs[i].typeId;
            if (!that_d.prfs[typeId])
                unCachePrfs.push(typeId);
            that_d.devs[devs[i].deviceId] = devs[i];
        }
        (function finishCallback() {
            if (unCachePrfs.length > 0)
                that_d.profile(unCachePrfs.pop(), finishCallback);
            else {
                that_d.ui.displayNav(global.NAV_DEVICES, function() {
                    clearInterval(that_d.updateIntervalTimer);
                    that_d.updateIntervalTimer = null;
                    var body = document.getElementById("body-devices");
                    body.innerHTML = "";
                });
                that_d.buildDevices(that_d.devs);
            }
        })();
    });

    if (that_d.updateIntervalTimer)
        clearInterval(that_d.updateIntervalTimer);
    that_d.updateIntervalTimer = setInterval(function() {
        if (that_d.uglyRetry) {
            that_d.uglyRetry = false;
            return;
        }
        var needUpdateIds = [];
        for (var did in that_d.updateValueSlot)
            needUpdateIds.push(did);
        (function foreachDeviceId() {
            if (needUpdateIds.length == 0)
                return;
            var did = needUpdateIds.pop();
            if (that_d.expandStates[did] === true) {
                that_d.status(did, function(objs) {
                    for (var prop in objs)
                        that_d.updateValue(did, prop, objs[prop]);
                    foreachDeviceId();
                });
            } else
                foreachDeviceId();
        })();
    }, that_d.updateInterval);
}

Devices.prototype.updateValue = function(did, prop, value) {
    if (!that_d.updateValueSlot[did])
        return;
    if (that_d.updateValueSlot[did][prop])
        that_d.updateValueSlot[did][prop](did+'_'+prop, value);
}

Devices.prototype.buildDevices = function(devs) {
    // console.log("buildDevices");
    var body = document.getElementById("body-devices");
    var intro = document.getElementById("introduction-devices");
    var html = "";
    for(let did in devs) {
        var tid = devs[did].typeId;
        var iconSrc = "ic_default_device";
        if (that_d.prfs[tid])
            iconSrc = that_d.prfs[tid]['iconid'];
        html += '<div';
        html += '    id="' +  did + '"';
        html += '    class="device-list-item">';
        html += '    <div class="device-list-item-header">';
        html += '        <button';
        html += '            id="' + did + '_iconbutton"';
        html += '            class="iconselection-button"';
        html += '            onclick="ui.notImplAlert(\'' + did + '\')"';
        html += '        >';
        html += '            <img';
        html += '                id="' + did + '_icon"';
        html += '                class="device-list-item-header-icon"';
        html += '                src="' + getIconUrl(iconSrc) + '"';
        html += '                alt=""';
        html += '            />';
        html += '        </button>';
        html += '        <div class="device-list-item-header-rest">';
        html +=             that_d.buildDeviceHeader(did, tid, devs[did].label, devs[did].alias);
        html += '        </div>';
        html += '    </div>';
        html += '    <div';
        html += '        id="resourceBody' + did + '"';
        html += '        style="display:' + (that_d.expandStates[did] == true ? 'block' : 'none') + ';"';
        html += '        class="device-list-item-body">';
        html +=          that_d.buildDeviceBody(did, tid, null, null);
        html += '    </div>';
        html += '</div>';
    }
    if (html) {
        body.innerHTML = html;
        intro.style.display = "none";
        body.style.display = "";
    } else {
        intro.style.display = "";
        body.style.display = "none";
    }
}

Devices.prototype.buildDeviceHeader = function(did, tid, label, alias) {
    var iconClass = that_d.expandStates[did] == true ? "close-icon" : "open-icon";
    var html = '';
	html += '<div class="device-list-item-header-title">';
    if (alias != 'null')
        html += buildInlineTextEdit(did, alias, "onChangeName('" + did + "', value)");
    else
        html += buildInlineTextEdit(did, did + ' ' + label, "onChangeName('" + did + "', value)");
    html += '</div>';
    if (that_d.devs[did].isonline != "true") {
        html += ' <button';
        html += '     class="default-icon-button list-icon-button offline-icon"';
        html += '     disabled=true';
        html += ' />';
    }
    html += ' <button';
    html += '     class="default-icon-button list-icon-button reset-icon"';
    html += '     onclick="onDeleteDevice(\'' + did + '\')"';
    html += ' ></button>';
    html += ' <button';
    html += '     class="default-icon-button list-icon-button trash-icon"';
    html += '     onclick="onUnbindDevice(\'' + did + '\')"';
    html += ' ></button>';
    html += '<button';
    html += '    id="' + 'toggleExpand' + did + '"';
    html += '    class="default-icon-button list-icon-button ' + iconClass + '"';
    html += '    onclick="onToggleContainer(\'' + did + '\')"';
    html += '></button>';
    return html;
}

Devices.prototype.buildDeviceBody = function(did, tid, leftHtmlCallback, rightHtmlCallback) {
    // console.log("Devices build body");
    var noleft = leftHtmlCallback == null;
    that_d.updateValueSlot[did] = new Array();
    var profile = that_d.prfs[tid]["profile"];
    var html = "";
    html += '<div class="device-list-item-resource">';
    for (let propName in profile) {
        if (propName == "OnlineState")
            continue;
        let key = did + "_" + propName;
        html += '<div class="device-list-item-property">';
        html +=      noleft ? "" : leftHtmlCallback(did, propName, profile[propName]);
        html += '    <div class="device-list-item-property-title">';
        html +=         profile[propName]['tag'];
        html += '    </div>';
        html += '    <div class="device-list-item-property-value">';
        switch (profile[propName]['rt']) {
            case "oic.r.switch.binary":
                html += '<button';
                html += '    id="' + key + '"';
                html += '    class="property-control default-icon-button switch-off-icon"';
                html += '    onclick="onToggleDeviceSwitch(\'' + did + '\', \'' + propName + '\', \'' + key + '\')"';
                html += '    button=true';
                html += '></button>';
                if (!that_d.updateValueSlot[did][propName]) {
                    that_d.updateValueSlot[did][propName] = function(key, value) {
                        if (value == "1" || value == "true") {
                            if (!hasClass(key, "switch-on-icon")) {
                                addClass(key, "switch-on-icon");
                                removeClass(key, "switch-off-icon");
                            }
                        } else {
                            if (!hasClass(key, "switch-off-icon")) {
                                addClass(key, "switch-off-icon");
                                removeClass(key, "switch-on-icon");
                            }
                        }
                    };
                }
                break;
            case "oic.r.light.dimming":
            case "oic.r.light.brightness":
                var range = parseRange(profile[propName]['range']);
                html += '<input id="' + key + '"';
                html += '    type="range"';
                html += '    class="property-control property-control-slider slider-brightness"';
                html += '    onchange="onChangeProperty(\'' + did + '\', \'' + propName + '\', this.value)"';
                html += '    min="' + range[0] + '" max="' + range[1] + '" step="1">';
                if (!that_d.updateValueSlot[did][propName])
                    that_d.updateValueSlot[did][propName] = updateElementValue;
                break;
            case "oic.r.colour.hs":
                // TODO urgly diff profiles with OCF
                var range = parseRange(profile[propName]['range']);
                if (propName == "hue") {
                    html += '<input id="' + key + '"';
                    html += '    type="range"';
                    html += '    class="property-control property-control-slider slider-hue"';
                    html += '    onchange="onChangeProperty(\'' + did + '\', \'' + propName + '\', this.value)"';
                    html += '    oninput="updateSaturationSliderBackground(\'' + did + '\', this.value, ' + range[1] + ')"';
                    html += '    min="' + range[0] + '" max="' + range[1] + '" step="1">';
                    if (!that_d.updateValueSlot[did][propName]) {
                        that_d.updateValueSlot[did][propName] = function(key, value) {
                            var inputElement = document.getElementById(key);
                            if (inputElement) {
                                inputElement.value = value;
                                updateSaturationSliderBackground(key, inputElement.max);
                            }
                        };
                    }
                } else if (propName === "saturation") {
                    html += '<input id="' + key + '"';
                    html += '    type="range"';
                    html += '    class="property-control property-control-slider slider-saturation"';
                    html += '    onchange="onChangeProperty(\'' + did + '\', \'' + propName + '\', this.value)"';
                    html += '    min="' + range[0] + '" max="' + range[1] + '" step="1">';
                    if (!that_d.updateValueSlot[did][propName])
                        that_d.updateValueSlot[did][propName] = updateElementValue;
                }
                break;
            case "oic.r.colour.colourtemperature":
                var range = parseRange(profile[propName]['range']);
                html += '<input id="' + key + '"';
                html += '    type="range"';
                html += '    class="property-control property-control-slider slider-ct"';
                html += '    onchange="onChangeProperty(\'' + did + '\', \'' + propName + '\', this.value)"';
                html += '    min="' + range[0] + '" max="' + range[1] + '" step="1">';
                if (!that_d.updateValueSlot[did][propName])
                    that_d.updateValueSlot[did][propName] = updateElementValue;
                break;
            default:
                html += that_d.buildDefaultPropertyView(did, propName, profile[propName], noleft);
                break;
        }
        html += '    </div>';
        html +=      rightHtmlCallback == null ? "" : rightHtmlCallback(did, propName, profile[propName]);
        html += '</div>';
    }
    html += '</div>';
    return html;
}

Devices.prototype.buildDefaultPropertyView = function(did, propName, resObj, noleft) {
    // console.log("Devices buildDevicePropertyView %s", did);
    var key = did + "_" + propName;
    var html = "";
    switch(resObj['type']) {
        case 'enum':
            html += '        <select id="' + key + '"';
            html += '            class="property-control property-control-selection"';
            if (!noleft || resObj['write'] === 'T')
                html += '        onchange="onChangeProperty(\'' + did + '\', \'' + propName + '\', this.value)"';
            html += '        >';
            for (var it in resObj['range'])
                html += '        <option value="' + it + '">' + resObj['range'][it] + '</option>';
            html += '        </select>';
            if (!that_d.updateValueSlot[did][propName])
                that_d.updateValueSlot[did][propName] = updateElementValue;
            break;
        case 'boolean':
            if (noleft && resObj['write'] === 'F') {
                html += '        <span id="' + key + '"></span>';
                if (!that_d.updateValueSlot[did][propName])
                    that_d.updateValueSlot[did][propName] = updateElementText;
            } else {
                html += '        <input id="' + key + '"';
                html += '            type="checkbox"';
                html += '            class="property-control property-control-checkbox"';
                html += '            onchange="onChangeProperty(\'' + did + '\', \'' + propName + '\', this.checked)"';
                html += '        />';
                if (!that_d.updateValueSlot[did][propName]) {
                    that_d.updateValueSlot[did][propName] = function(key, value) {
                        var inputElement = document.getElementById(key);
                        if (inputElement) {
                            if (value == '0')
                                inputElement.checked = false;
                            else
                                inputElement.checked = true;
                        }
                    };
                }
            }
            break;
        case 'int':
            if (noleft && resObj['write'] === 'F') {
                html += '        <span id="' + key + '"></span>';
                if (!that_d.updateValueSlot[did][propName])
                    that_d.updateValueSlot[did][propName] = updateElementText;
            } else {
                var intReg = /v\s*>[=]\s*(-?\d+)\s*and\s*v\s*<[=]\s*(-?\d+)/g;
                var result = intReg.exec(resObj['range']);
                html += '        <input type="range" id="' + key + '"';
                html += '            class="property-control property-control-slider"';
                html += '            onchange="onChangeProperty(\'' + did + '\', \'' + propName + '\', this.value)"';
                html += '            min="' + result[1] + '" max="' + result[2] + '" step="1"></input>';
                if (!that_d.updateValueSlot[did][propName])
                    that_d.updateValueSlot[did][propName] = updateElementValue;
            }
            break;
        default:
            break;
    }
    return html;
}

Devices.prototype.toggleExpandBody = function(did) {
    var toggleExpandId = "toggleExpand" + did;
    var resourceBodyId = "resourceBody" + did;
    var resourceElem = document.getElementById(resourceBodyId);

    if (that_d.expandStates[did] == true) {
        removeClass(resourceBodyId, "device-list-item-open");
        if (resourceElem)
            resourceElem.style.display = "none";
        removeClass(toggleExpandId , "close-icon");
        addClass(toggleExpandId , "open-icon");
        that_d.expandStates[did] = false;
    } else {
        addClass(resourceBodyId, "device-list-item-open");
        if (resourceElem)
            resourceElem.style.display = "block";
        removeClass(toggleExpandId, "open-icon");
        addClass(toggleExpandId, "close-icon");
        that_d.expandStates[did] = true;
    }
}

Devices.prototype.toggleSwitchBinary = function(did, propName, key) {
    if (hasClass(key, "switch-off-icon")) {
        addClass(key, "switch-on-icon");
        removeClass(key, "switch-off-icon");
        onChangeProperty(did, propName, true);
    } else {
        addClass(key, "switch-off-icon");
        removeClass(key, "switch-on-icon");
        onChangeProperty(did, propName, false);
    }
}

Devices.prototype.updateSaturationSliderBG = function (did, hueValue, hueMax) {
    var inputElement = document.getElementById(did + "_saturation");

    var degrees = 360 * hueValue / hueMax;
    var color = "hsl(" + (degrees|0) + ",100%,50%)";
    var gradient = "linear-gradient(to right, white , " + color + ")";
    if (inputElement)
        inputElement.style.background = gradient;
}

Devices.prototype.changeName = function(did, value) {
    // console.log(value);
    that_d.rename(did, value, function(result) {
        sendMessage(events.SHOW_DEVICES);
    });
}

Devices.prototype.discovery = function() {
    var html = "";
    html += '<h3 class="modal-title">';
    html +=      "{{onboarding_title}}";
    html += '</h3>';
    html += '<div class="spinning-wheel-container">';
    html += '    <div class="spinning-wheel"></div>';
    html += '    <div class="spinning-wheel-content-box">';
    html +=          "{{searching_new_devices}}";
    html += '    </div>';
    html += '</div>';
    that_d.ui.openModal(html);

    var uri = "/api/familydevice/discovery";
    httpRequest("POST", uri, null, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            that_d.ui.buildGenericErrorModal();
            return;
        }
        // console.log("request %s:%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] != global.RES_STATUS_OK) {
                throw new Error();
            }
            maxDuration = parseInt(responseData["result"]["maxDuration"]) / 5 + 1;
            setTimeout(function(){
                that_d.list(function(devs){
                    var html = "";
                    html += '<h3 class="modal-title">';
                    html +=      "{{onboarding_title}}";
                    html += '</h3>';
                    if (devs.length === 0) {
                        html += '<p class="modal-info-text">';
                        html +=      "{{no_new_devices_error_message}}";
                        html += '</p>';
                        that_d.ui.openModal(html, null);
                    } else {
                        html += '<p class="modal-info-text">';
                        html +=      "{{onboarding_devices_info_text}}";
                        html += '</p>';
                        html += '<div class="modal-scrollarea">';
                        for (var i = 0;  i< devs.length; i++) {
                            if (devs[i].ownedStatus == "HB_binded")
                                continue;
                            html += '<label class="device-selection-item">';
                            html += '    <img class="device-selection-item-icon" src="' + getIconUrl(devs[i].iconid) + '" />';
                            html += '    <div class="device-selection-item-rest">';
                            html += '        <span class="device-selection-item-name">';
                            html +=              devs[i].deviceId + "     " + devs[i].label;
                            html += '        </span>';
                            html += '        <button id="' + devs[i].deviceId + '"';
                            html += '              class="btn-primary"';
                            html += '              onclick="onBindDevice(\'' + devs[i].typeId + '\', \'' + devs[i].deviceId + '\')">';
                            html +=             "{{add_device}}";
                            html += '        </button>';
                            html += '    </div>'
                            html += '</label>';
                        }
                        html += '</div>';
                        that_d.ui.openModal(html, function() {
                            that_d.initUI();
                        });
                    }
                });
            }, maxDuration*1000);
        } catch (err) {
            console.log(err);
            that_d.ui.buildGenericErrorModal();
        }
    });
}

Devices.prototype.list = function(finishCallback) {
    var uri = "/api/familydevice/list";
    httpRequest("POST", uri, null, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            that_d.ui.buildGenericErrorModal();
            return;
        }
        // console.log("request %s:%s", uri, responseText);
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
            that_d.ui.buildGenericErrorModal();
        }
    });
}

Devices.prototype.bind = function(tid, did) {
    var uri = "/api/familydevice/bind";
    var body = "{\"typeId\": \"" + tid + "\", \"deviceId\": \"" + did + "\"}";
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            that_d.ui.buildGenericErrorModal();
            return;
        }
        // console.log("request %s:%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (responseData['result']['ownedStatus'] == "HB_binded")
                    ui.setButtonClickable(did, false);
                return;
            }
            that_r.ui.buildGenericErrorModal(
                    responseData["errors"]["code"],
                    responseData["errors"]["message"]);
        } catch (err) {
            console.log(err);
            that_d.ui.buildGenericErrorModal();
        }
    });
}

Devices.prototype.unbind = function(did) {
    var uri = "/api/familydevice/unbind";
    var body = "{\"deviceId\": \"" + did + "\"}";
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            that_d.ui.buildGenericErrorModal();
            return;
        }
        // console.log("request %s:%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                that_d.initUI();
                return;
            }
            that_r.ui.buildGenericErrorModal(
                    responseData["errors"]["code"],
                    responseData["errors"]["message"]);
        } catch (err) {
            console.log(err);
            that_d.ui.buildGenericErrorModal();
        }
    });
}

Devices.prototype.delete = function(did) {
    var uri = "/api/familydevice/delete";
    var body = "{\"deviceId\": \"" + did + "\"}";
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            that_d.ui.buildGenericErrorModal();
            return;
        }
        // console.log("request %s:%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                that_d.initUI();
                return;
            }
            that_d.ui.buildGenericErrorModal("device delete", "status return not ok");
        } catch (err) {
            console.log(err);
            that_d.ui.buildGenericErrorModal();
        }
    });
}

Devices.prototype.profile = function(tid, finishCallback) {
    var uri = "/api/familydevice/profile";
    var body = "{\"typeId\": \"" + tid + "\"}";
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            that_d.ui.buildGenericErrorModal();
            return;
        }
        // console.log("request %s:%s", uri, responseText);
        try {
            responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                that_d.prfs[tid] = responseData["result"];
                if (finishCallback)
                    finishCallback();
                return;
            }
            that_r.ui.buildGenericErrorModal(
                    responseData["errors"]["code"],
                    responseData["errors"]["message"]);
        } catch (err) {
            console.log(err);
            that_d.ui.buildGenericErrorModal();
        }
    });
}

Devices.prototype.status = function(did, finishCallback) {
    var uri = "/api/familydevice/status";
    var body = "{\"deviceId\": \"" + did + "\"}";
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            that_d.ui.buildGenericErrorModal();
            return;
        }
        // console.log("request %s:%s", uri, responseText);
        try {
            responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData['result']);
                return;
            }
            that_r.ui.buildGenericErrorModal(
                    responseData["errors"]["code"],
                    responseData["errors"]["message"]);
        } catch (err) { console.log(err); }
    });
}

Devices.prototype.operate = function(did, name, value) {
    var uri = "/api/familydevice/operate";
    var body = '{"deviceId":"' + did + '","propSet":[{"name":"' + name + '", "value":"' + value + '"}]}';
    that_d.uglyRetry = true;
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            that_d.ui.buildGenericErrorModal();
            return;
        }
        // console.log("request %s:%s", uri, responseText);
        try {
            responseData = JSON.parse(responseText);
            if (responseData["status"] != global.RES_STATUS_OK) {
                that_r.ui.buildGenericErrorModal(
                        responseData["errors"]["code"],
                        responseData["errors"]["message"]);
            }
        } catch (err) { console.log(err); }
    });
}

Devices.prototype.rename = function(did, value, finishCallback) {
    var uri = "/api/familydevice/rename";
    var body = '{"deviceId":"' + did + '","newName":"' + value + '"}';
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            that_d.ui.buildGenericErrorModal();
            return;
        }
        // console.log("request %s:%s", uri, responseText);
        try {
            responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData['result']);
                return;
            }
            that_r.ui.buildGenericErrorModal(
                    responseData["errors"]["code"],
                    responseData["errors"]["message"]);
        } catch (err) { console.log(err); }
    });
}
