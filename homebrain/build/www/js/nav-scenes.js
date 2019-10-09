/// @file nav-scense.js
/// @Brief
/// @author QRS
/// @version 0.0.9
/// @date 2019-06-05

var Scenes = function(ui, ds, rs, testscenes) {
    this.ui = ui;
    this.d = ds;
    this.r = rs;
    this.expandStates = [];
    this.expandArrows = [];
    this.editItem = {
        ONENTER: 0,
        ONEXIT: 1,
        ONSTORY: 2,
    };
    this.editItemStr = [
        "onEnter",
        "onExit",
        "onStory",
    ];
    this.currentActive = null;
    this.currentModal = 0;
    this.editTitles = ["{{scene_enter}}", "{{scene_exit}}", "{{scene_story}}"];
    this.microServices = [
        "autolight",
        "gradlight",
        "sosalarm",
    ];
    this.data = null;
    this.rules = [];
    this.scenes = testscenes;
    that_s = this;
}

Scenes.prototype.initUI = function() {
    // console.log("Scenes initUI");
    that_s.ui.showLoadingIndicator("Scenes LOADING...");
    that_s.rules = [];
    that_s.list(function(resData) {
        if (!that_s.currentActive) {
            sysinfo = getSysinfo();
            if (sysinfo) {
                try {
                    that_s.currentActive = sysinfo["scene_mode"];
                } catch (err) {
                    console.log("err: scene_mode");
                }
            }
        }
        that_s.ui.displayNav(global.NAV_SCENES, function() {
            that_s.currentModal = 0;
        });

        var body = document.getElementById("body-scenes");
        var intro = document.getElementById("introduction-scenes");
        var html = "";
        //TODO I'am too lazy
        for (let i = 0; i < that_s.scenes.length; ++i) {
            var label = that_s.scenes[i]["sceneName"];
            var rid = that_s.scenes[i]["sceneId"];
            var icon = that_s.scenes[i]["icon"];
            var flag = false;
            if (label == "none")
                continue;
            for (let j = 0, l = resData.length; j < l;  ++j) {
                if (rid == resData[j]["sceneId"])
                    flag = true;
            }
            html += '<div';
            html += '    id="' +  rid + '"';
            html += '    class="device-list-item">';
            html += '    <div class="device-list-item-header">';
            html += '        <button';
            html += '            id="' + rid + '_iconbutton"';
            html += '            class="iconselection-button"';
            html += '            onclick="onRuleModify(' + i + ')";>';
            html += '            <img';
            html += '                id="' + rid + '_icon"';
            html += '                class="device-list-item-header-icon"';
            html += '                src="' + getIconUrl(icon) + '"';
            html += '                alt=""/>';
            html += '        </button>';
            html += '        <div class="device-list-item-header-rest">';
            html += '           <div class="device-list-item-header-title">';
            html += '               <label id="' + rid + '"> <span>' + label + '</span></label>';
            html += '           </div>';
            if (flag) {
                html += '       <button';
                html += '           class="default-icon-button list-icon-button reset-icon"';
                html += '           onclick="onRuleDelete(\'' + rid + '\')"';
                html += '       ></button>';
            }
            html += '           <button id="' + label + '"';
            if (label == that_s.currentActive)
                html += '           class="default-icon-button list-icon-button activate2-icon"';
            else
                html += '           class="default-icon-button list-icon-button activate-icon"';
            html += '               onclick="onRuleExecute(\'' + rid + '\', \'' + label + '\')"';
            html += '           ></button>';
            html += '        </div>';
            html += '    </div>';
            html += '</div>';
        }
        that_s.r.list(function(resData) {
            var length = resData.length;
            for (let i = 0; i < length; ++i) {
                var label = resData[i]["sceneName"];
                var rid = resData[i]["sceneId"];
                if ("1" != resData[i]["manualEnabled"])
                    that_s.rules.push(JSON.parse('{"sceneId":"' + rid + '", "sceneName":"' + label + '"}'));
            }
            intro.style.display = "none";
            body.style.display = "";
            body.innerHTML = html;
        });
    });
}

Scenes.prototype.toggleExpandBody = function(rid) {
    var toggleExpandId = "toggleExpand" + rid;
    var descrBodyId = "description" + rid;
    var descrElem = document.getElementById(descrBodyId);

    if (that_s.expandStates[rid] == true) {
        removeClass(descrBodyId, "device-list-item-open");
        if (descrElem)
            descrElem.style.display = "none";
        removeClass(toggleExpandId , "open-icon");
        addClass(toggleExpandId , "close-icon");
        that_s.expandStates[rid] = false;
    } else {
        addClass(descrBodyId, "device-list-item-open");
        if (descrElem)
            descrElem.style.display = "block";
        removeClass(toggleExpandId, "close-icon");
        addClass(toggleExpandId, "open-icon");
        that_s.expandStates[rid] = true;
    }
}

Scenes.prototype.displayErrorMessage = function (msg) {
	var errDiv = document.getElementById("rule_editmodal_errormsg");
    if (errDiv) {
        if (msg)
            errDiv.textContent = msg;
        else
            errDiv.innerHTML = "&nbsp;";
    }
}

Scenes.prototype.getDeviceObj = function (did, propName) {
    // console.log("getDeviceObj %s %s modal[%d]", did, propName, that_s.currentModal);
    var tagEvent = that_s.editItemStr[that_s.currentModal];
    var devControlObjs = that_s.data["scene"][tagEvent]["deviceControl"];
    for (let i = 0, l = devControlObjs.length; i < l; ++i) {
        if (devControlObjs[i]["deviceId"] == did) {
            if (propName) {
                if (devControlObjs[i]["propName"] == propName)
                    return {'index': i, 'obj': devControlObjs[i]};
            } else {
                return {'index': i, 'obj': devControlObjs[i]};
            }
        }
    }
    return null;
}

Scenes.prototype.getNotifyObj = function (rid) {
    // console.log("getNotifyObj %s modal[%d]", rid, that_s.currentModal);
    var tagEvent = that_s.editItemStr[that_s.currentModal];
    var ntfContentObjs = that_s.data["scene"][tagEvent]["notifyContent"];
    if (ntfContentObjs) {
        for (let i = 0, l = ntfContentObjs.length; i < l; ++i) {
            if (ntfContentObjs[i]["sceneId"] == rid)
                return {'index': i, 'obj': ntfContentObjs[i]};
        }
    }
    return null;
}

Scenes.prototype.getArrowClass = function(divId) {
    return !that_s.expandArrows[divId] ? "close2-icon" : "open-icon";
}

Scenes.prototype.onEdit = function(idx) {
    if (idx < -1 || idx >= that_s.scenes.length)
        return;
    that_s.query(that_s.scenes[idx]["sceneId"], function(resData) {
        if (JSON.stringify(resData)=="{}") {
            that_s.data = {
                "sceneId": that_s.scenes[idx]["sceneId"],
                "sceneName": that_s.scenes[idx]["sceneName"],
                "scene": {
                    "onEnter": {
                        "deviceControl": [],
                        "notifyContent": [],
                    },
                    "onExit": {
                        "deviceControl": [],
                        "notifyContent": [],
                    },
                    "onStory": {
                        "microService": []
                    }
                },
            };
        } else {
            that_s.data = resData;
        }
        that_s.onEditModal(0);
    })
}

Scenes.prototype.onDelete = function(rId) {
    that_s.delete(rId, function(resData){
        that_s.initUI();
    });
}

Scenes.prototype.onExecute = function(scene, key) {
    that_s.execute(scene, function(resData) {
        var divId = that_s.currentActive;
        if (hasClass(divId, "activate2-icon")) {
            removeClass(divId, "activate2-icon");
            addClass(divId, "activate-icon");
        }
        if (hasClass(key, "activate-icon")) {
            removeClass(key, "activate-icon");
            addClass(key, "activate2-icon");
        }
        that_s.currentActive = key;
    });
}

Scenes.prototype.onEditModal = function(eId) {
    that_s.currentModal = eId;
	var html = "";
	html += '<h3 class="modal-title">';
	html +=      "{{tab_title_scenes}}";
	html += '</h3>';

	html += '<div class="modal-rule-header">';
	html += '    <div class="device-list-item">';
	html += '        <div class="device-list-item-header">';
    html += '            <img class="rule-icon ' + (eId == that_s.editItem.ONENTER ? 'hl_bg"' : '"');
    html += '                alt="" src="' + getIconUrl("ic_icons_enter") + '" ';
    html += '                onclick="javascript:onRuleEditModalClicked(' + that_s.editItem.ONENTER + ');"/>';
    html += '            <img class="rule-icon ' + (eId == that_s.editItem.ONEXIT ? 'hl_bg"' : '"');
    html += '                alt="" src="' + getIconUrl("ic_icons_exit") + '" ';
    html += '                onclick="javascript:onRuleEditModalClicked(' + that_s.editItem.ONEXIT + ');"/>';
    html += '            <img class="rule-icon ' + (eId == that_s.editItem.ONSTORY ? 'hl_bg"' : '"');
    html += '                alt="" src="' + getIconUrl("ic_icons_story") + '" ';
    html += '                onclick="javascript:onRuleEditModalClicked(' + that_s.editItem.ONSTORY + ');"/>';
    html += '        </div>';
    html += '    </div>';
	html += '</div>';
    html += '<div class="page-header">';
    html += '    <h1 class="page-title">' + that_s.editTitles[eId] + '(' + that_s.data['sceneName'] + ')</h1>';
    html += '</div>';
    html += '<div class="modal-container">';
    html += '    <div id="devices-selection-modal-list" class="modal-scrollarea">';
    html += '        <div class="spinning-wheel-container">';
    html += '            <div class="spinning-wheel"></div>';
    html += '        </div>';
    html += '    </div>';

    html += '    <div id="rule_editmodal_errormsg" class="form-errormsg" style="">&nbsp;</div>';
    html += '    <div class="modal-buttons">';
    html += '        <button class="btn-default" onclick="ui.closeModal()" type="button">';
    html +=              "{{cancel}}";
    html += '        </button>';
    html += '        <button disable="" ';
    html += '            id="devices-selection-modal-submit"';
    html += '            class="btn-primary" onclick="onSaveRuleChanges()" type="button">';
    html +=              "{{save}}";
    html += '        </button>';
    html += '    </div>';
    html += '</div>';
    that_s.ui.openModal(html);
    that_s.buildRuleList(eId);
}

Scenes.prototype.buildRuleList = function (eId) {
    var devs = that_s.d.devs
    var prfs = that_s.d.prfs;

    function leftHtmlCallback(did, propName, propConf) {
        if (propConf['write'] == 'F')
            return '<span id="' + did + '_' + propName + '_ro" class="property-control-span">ro</span>';
        var result = that_s.getDeviceObj(did, propName);
        var html = '';
        html += '<input';
        html += '    id="devbody_' + did + '_' + propName + '"';
        html += '    class="property-control-checkbox" ';
        html += '    type="checkbox" ';
        html +=      result ? "checked" : "";
        html += '    onclick="onPropertySelected(\'' + did + '\', \'' + propName + '\', this.checked)"';
        html += '/>';
        return html;
    }

    function rightHtmlCallback(did, propName, propConf) {
        if (propConf['type'] != 'int')
            return '';
        return '<span id="' + did + '_' + propName + '_text" class="property-control-span"></span>';
    }

    var tagEvent = that_s.editItemStr[eId];
    var html = "";
    if (eId != that_s.editItem.ONSTORY) {
        // device control
        var devControlObjs = that_s.data["scene"][tagEvent]["deviceControl"];
        var tag = tagEvent + 'DeviceControl';
        html += '<div id="' + tag + '" class="device-list-item ">';
        html += '   <div class="device-list-item-header">';
        html += '        <button id="' + tag + '_btn" ';
        html += '              class="default-icon-button list-icon-button ' + that_s.getArrowClass(tag) + '"';
        html += '              onclick="onToggleSceneEvent(\'' + tag + '\');"></button>';
        html += '        <div class="device-list-item-open device-list-item-header-rest">';
        html += '            <div class="device-list-item-header-title">';
        html += '                <label><span>Device Control</span></label>';
        html += '            </div>';
        html += '        </div>';
        html += '    </div>';
        html += '    <div id="' + tag + '_body" style="display:' + (!that_s.expandArrows[tag] ? "none" : "block") + '">';
        for(var did in devs) {
            var tid = devs[did].typeId;
            var iconSrc = "ic_default_device";
            if (prfs[tid])
                iconSrc = prfs[tid]['iconid'];
            var checked = false;
            for (var i = 0, l = devControlObjs.length; i < l; ++i) {
                if (devControlObjs[i]['deviceId'] == did) {
                    checked = true;
                    break;
                }
            }
            html += '    <div class="rule-device-selection-border rule-condition-body">';
            html += '        <label class="rule-device-selection-item">';
            html += '             <img';
            html += '                 id="' + did + '_icon" ';
            html += '                 class="device-selection-item-icon"';
            html += '                 src="' + getIconUrl(iconSrc) + '"';
            html += '                 alt=""';
            html += '             />';
            html += '            <div class="device-selection-item-rest">';
            html += '                <span class="device-selection-item-name">';
            html +=                      did + " " + devs[did].alias;
            html += '                </span>';
            html += '                <input id="ruleEntry"' + did + '" type="checkbox"';
            html +=                      checked ? "checked" : "";
            html += '                    onclick="onDeviceSelected(\'' + did + '\', this.checked)"';
            html += '                />';
            html += '            </div>';
            html += '        </label>';
            html += '       <div id="devbody_' + did + '"';
            html += '           class="device-list-item-body"';
            html +=             checked ? ' style="display:block">' : ' style="display:none">';
            html +=             that_s.d.buildDeviceBody(did, tid, leftHtmlCallback, rightHtmlCallback);
            html += '       </div>';
            html += '    </div>';
        }
        html += '    </div>';
        html += '</div>';

        // notify content for rule
        var tag = tagEvent + 'NotifyContent'
        html += '<div id="' + tag + '" class="device-list-item ">';
        html += '   <div class="device-list-item-header">';
        html += '        <button id="' + tag + '_btn" ';
        html += '              class="default-icon-button list-icon-button ' + that_s.getArrowClass(tag) + '"';
        html += '              onclick="onToggleSceneEvent(\'' + tag + '\');"></button>';
        html += '        <div class="device-list-item-open device-list-item-header-rest">';
        html += '            <div class="device-list-item-header-title">';
        html += '                <label><span>Enable/Disable Rules</span></label>';
        html += '            </div>';
        html += '        </div>';
        html += '    </div>';
        html += '    <div id="' + tag + '_body" style="display:' + (!that_s.expandArrows[tag] ? "none" : "block") + '">';
        for (var i = 0, l = that_s.rules.length; i < l; ++i) {
            var rid = that_s.rules[i]["sceneId"];
            var rname = that_s.rules[i]["sceneName"]
            var ntfContentObjs = that_s.getNotifyObj(rid);
            var checked = false;
            if (ntfContentObjs && ntfContentObjs['obj']['enable'] == "1")
                checked = true;
            html += '    <div class="rule-device-selection-border rule-condition-body">';
            html += '        <label class="rule-device-selection-item">';
            html += '             <img';
            html += '                 id="' + rid + '_icon" ';
            html += '                 class="device-selection-item-icon"';
            html += '                 src="' + getIconUrl("ic_default_scene") + '"';
            html += '                 alt=""';
            html += '             />';
            html += '            <div class="device-selection-item-rest">';
            html += '                <span class="device-selection-item-name">';
            html +=                      rname;
            html += '                </span>';
            html += '                <input id="ruleEntry"' + rid + '" type="checkbox"';
            html +=                      ntfContentObjs ? "checked" : "";
            html += '                    onclick="onManualRuleSelected(\'' + rid + '\',\'' + rname + '\', this.checked)"';
            html += '                />';
            html += '            </div>';
            html += '        </label>';
            html += '        <div id="rulbody_' + rid + '"';
            html += '            class="device-list-item-body"';
            html +=              ntfContentObjs ? ' style="display:block">' : ' style="display:none">';
            html += '            <div class="device-list-item-resource">';
            html += '                 <div class="device-list-item-property">';
            html += '                     <div class="device-list-item-property-title">';
            html +=                            "Enable";
            html += '                     </div>';
            html += '                     <div class="device-list-item-property-value">';
            html += '                          <input id="enable_' + rid + '"';
            html += '                              type="checkbox"';
            html +=                                checked ? "checked" : "";
            html += '                              class="property-control property-control-checkbox"';
            html += '                              onchange="onSceneEnableRule(\'' + rid + '\', this.checked)"';
            html += '                          />';
            html += '                     </div>';
            html += '                 </div>';
            html += '            </div>';
            html += '        </div>';
            html += '    </div>';
        }
        html += '    </div>';
        html += '</div>';
    } else {
        // micro service
        var miscrServrObjs = that_s.data["scene"][tagEvent]["microService"];
        var tag = tagEvent + 'MicroService';
        html += '<div id="sceneMicroService" class="device-list-item ">';
        html += '    <div class="device-list-item-header">';
        html += '        <button id="' + tag + '_btn" ';
        html += '              class="default-icon-button list-icon-button ' + that_s.getArrowClass(tag) + '"';
        html += '              onclick="onToggleSceneEvent(\'' + tag + '\');"></button>';
        html += '        <div class="device-list-item-open device-list-item-header-rest">';
        html += '            <div class="device-list-item-header-title">';
        html += '                <label><span>Micro Service</span></label>';
        html += '            </div>';
        html += '        </div>';
        html += '    </div>';
        html += '    <div id="' + tag + '_body" style="display:' + (!that_s.expandArrows[tag] ? "none" : "block") + '">';
        html += '        <div id="' + tag + '_enable" class="device-list-item-body">';
        for (var i = 0, l =  miscrServrObjs.length; i < l; ++i) {
            var it = miscrServrObjs[i];
            html += '        <div class="modal-buttons">';
            html += '            <button id="' + tag + '_del' + i + '"';
            html += '                class="default-icon-button del-icon"';
            html += '                onclick="onMicroServiceDel(' + i + ');">';
            html += '            </button>';
            html += '            <label><span>' + it['name'] + ':' + it['msid'] + '</span></label>';
            html += '        </div>';
            // deps
            var deps = it['deps'];
            html += '        <div class="device-list-item-property">';
            html += '            <div class="device-list-item-property-title">Depends MS</div>';
            html += '            <div class="device-list-item-property-value">';
            html += '                <input id = "deps' + i + '"';
            html += '                    type="text" class="rule-input"';
            html += '                    placeholder="ms1,ms2,ms3"';
            html += '                    value="' + deps + '"';
            html += '                    onchange="onMicroServiceDepends(' + i + ',\'deps\', this.value)"/>';
            html += '            </div>';
            html += '        </div>';
            if (it['name'] == "autolight") {
                var lightUUID = it['params']['lightUUID'];
                var lightStep = it['params']['lightStep'];
                var sensorUUID = it['params']['sensorUUID'];
                var sensorIll = it['params']['sensorIll'];
                // light
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Light UUID</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <select id = "lightUUID' + i + '"';
                html += '                onchange="onMicroServiceParams(' + i + ',\'lightUUID\', this.value)">';
                for (var did in devs) {
                    var tid = devs[did].typeId;
                    var supertype = 'unkown';
                    if (prfs[tid]) {
                        supertype = prfs[tid]['supertype'];
                        if (supertype == "oic.d.light") {
                            if (lightUUID != did)
                                html += '<option value="' + did + '">' + did + '</option>';
                        }
                    }
                }
                html += '                <option value="' + lightUUID + '" selected="select">' + lightUUID + '</option>';
                html += '            </select>';
                html += '        </div>';
                html += '    </div>';

                // light step
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Light Step</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <input id = "lightStep' + i + '"';
                html += '                type="text" class="rule-input"';
                html += '                value=' + lightStep;
                html += '                onchange="onMicroServiceParams(' + i + ',\'lightStep\', this.value)"/>';
                html += '        </div>';
                html += '    </div>';

                // sensor
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Sensor UUID</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <select id = "sensorUUID' + i + '"';
                html += '                onchange="onMicroServiceParams(' + i + ',\'sensorUUID\', this.value)">';
                for (var did in devs) {
                    var tid = devs[did].typeId;
                    var supertype = 'unkown';
                    if (prfs[tid]) {
                        supertype = prfs[tid]['supertype'];
                        if (supertype == "oic.d.envsensor") {
                            if (sensorUUID != did)
                                html += ' <option value="' + did + '">' + did + '</option>';
                        }
                    }
                }
                html += '                 <option value="' + sensorUUID + '" selected="select">' + sensorUUID + '</option>';
                html += '            </select>';
                html += '        </div>';
                html += '    </div>';

                // ill
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Sensor Ill</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <input id = "sensorIll' + i + '"';
                html += '                type="text" class="rule-input"';
                html += '                value=' + sensorIll;
                html += '                onchange="onMicroServiceParams(' + i + ',\'sensorIll\', this.value)"/>';
                html += '        </div>';
                html += '    </div>';
            } else if (it['name'] == "gradlight") {
                var lightUUID = it['params']['lightUUID'];
                var direction = it['params']['direction'];
                var stepCount = it['params']['stepCount'];
                var timeSeconds = it ['params']['timeSeconds'];
                // light
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Light UUID</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <select id = "lightUUID' + i + '"';
                html += '                onchange="onMicroServiceParams(' + i + ',\'lightUUID\', this.value)">';
                for (var did in devs) {
                    var tid = devs[did].typeId;
                    var supertype = 'unkown';
                    if (prfs[tid]) {
                        supertype = prfs[tid]['supertype'];
                        if (supertype == "oic.d.light") {
                            if (lightUUID != did)
                                html += '<option value="' + did + '">' + did + '</option>';
                        }
                    }
                }
                html += '                <option value="' + lightUUID + '" selected="select">' + lightUUID + '</option>';
                html += '            </select>';
                html += '        </div>';
                html += '    </div>';

                // direction
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Direction</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <select id = "direction' + i + '"';
                html += '                onchange="onMicroServiceParams(' + i + ',\'direction\', this.value)">';
                if (direction == "up") {
                    html += '            <option value="up" selected="select">up</option>';
                    html += '            <option value="down">down</option>';
                } else {
                    html += '            <option value="up">up</option>';
                    html += '            <option value="down" selected="select">down</option>';
                }
                html += '            </select>';
                html += '        </div>';
                html += '    </div>';

                // step count
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Step Count</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <input id = "stepCount' + i + '"';
                html += '                type="text" class="rule-input"';
                html += '                value=' + stepCount;
                html += '                onchange="onMicroServiceParams(' + i + ',\'stepCount\', this.value)"/>';
                html += '        </div>';
                html += '    </div>';
                // time seconds
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Time (s)</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <input id = "timeSeconds' + i + '"';
                html += '                type="text" class="rule-input"';
                html += '                value=' + timeSeconds;
                html += '                onchange="onMicroServiceParams(' + i + ',\'timeSeconds\', this.value)"/>';
                html += '        </div>';
                html += '    </div>';
            } else if (it['name'] == "sosalarm") {
                var sosUUID = it['params']['sosUUID'];
                var alarmUUID = it['params']['alarmUUID'];
                var lightUUID = it['params']['lightUUID'];
                var timeSeconds = it ['params']['timeSeconds'];
                // sosUUID
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Sos UUID</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <select id = "sosUUID' + i + '"';
                html += '                onchange="onMicroServiceParams(' + i + ',\'sosUUID\', this.value)">';
                for (var did in devs) {
                    var tid = devs[did].typeId;
                    var supertype = 'unkown';
                    if (prfs[tid]) {
                        supertype = prfs[tid]['supertype'];
                        if (supertype == "oic.d.sosalarm" || supertype == "oic.d.scenectrl") {
                            if (sosUUID != did)
                                html += '<option value="' + did + '">' + did + '</option>';
                        }
                    }
                }
                html += '                <option value="' + sosUUID + '" selected="select">' + sosUUID + '</option>';
                html += '            </select>';
                html += '        </div>';
                html += '    </div>';

                // alarmUUID 
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Alarm UUID</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <select id = "alarmUUID' + i + '"';
                html += '                onchange="onMicroServiceParams(' + i + ',\'alarmUUID\', this.value)">';
                for (var did in devs) {
                    var tid = devs[did].typeId;
                    var supertype = 'unkown';
                    if (prfs[tid]) {
                        supertype = prfs[tid]['supertype'];
                        if (supertype == "oic.d.alarmer") {
                            if (sosUUID != did)
                                html += '<option value="' + did + '">' + did + '</option>';
                        }
                    }
                }
                html += '                <option value="' + alarmUUID + '" selected="select">' + alarmUUID + '</option>';
                html += '            </select>';
                html += '        </div>';
                html += '    </div>';

                // lightUUID 
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Light UUID</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <select id = "lightUUID' + i + '"';
                html += '                onchange="onMicroServiceParams(' + i + ',\'lightUUID\', this.value)">';
                for (var did in devs) {
                    var tid = devs[did].typeId;
                    var supertype = 'unkown';
                    if (prfs[tid]) {
                        supertype = prfs[tid]['supertype'];
                        if (supertype == "oic.d.light") {
                            if (sosUUID != did)
                                html += '<option value="' + did + '">' + did + '</option>';
                        }
                    }
                }
                html += '                <option value="' + lightUUID + '" selected="select">' + lightUUID + '</option>';
                html += '            </select>';
                html += '        </div>';
                html += '    </div>';

                // time seconds
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">Time (s)</div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <input id = "timeSeconds' + i + '"';
                html += '                type="text" class="rule-input"';
                html += '                value=' + timeSeconds;
                html += '                onchange="onMicroServiceParams(' + i + ',\'timeSeconds\', this.value)"/>';
                html += '        </div>';
                html += '    </div>';
            }
            html += '        <div class="hline"></div>';
        }
        html += '            <div class="modal-buttons">';
        for (var i = 0, l = that_s.microServices.length; i < l; ++i) {
            html += '            <center>';
            html += '                <button id="' + tag + '_add' + i + '"';
            html += '                    class="default-icon-button add-icon"';
            html += '                    onclick="onMicroServiceAdd(' + i + ');">';
            html += '                </button>';
            html += '                <label><span>' + that_s.microServices[i] + '</span></label>';
            html += '            </center>';
        }
        html += '            </div>';
        html += '        </div>';
        html += '    </div>';
        html += '</div>';
    }

    var listRule = document.getElementById("devices-selection-modal-list");
    listRule.innerHTML = html;

    if (eId != that_s.editItem.ONSTORY) {
        for (var i = 0, l = devControlObjs.length; i < l; ++i) {
            let did = devControlObjs[i]['deviceId'];
            let name = devControlObjs[i]['propName'];
            let value = devControlObjs[i]['propValue'];
            that_s.d.updateValue(did, name, value);
            updateElementText(did + '_' + name + '_text', value);
        }
    }
}

Scenes.prototype.deviceSelected = function (did, check) {
    // console.log("did = %s %d", did, check);
    var key = "devbody_" + did;
    var elem = document.getElementById(key);
    if (elem)
        elem.style.display = check ? "block" : "none";

    var tagEvent = that_s.editItemStr[that_s.currentModal];
    var devControlObjs = that_s.data["scene"][tagEvent]["deviceControl"];
    var result = null;
    if (check) {
        if (!devControlObjs)
            that_s.data["scene"][tagEvent]["deviceControl"] = [];
    } else {
        while (true) {
            result = that_s.getDeviceObj(did, null);
            if (!result)
                break;
            devControlObjs.splice(result['index'], 1);
        }
    }
}

Scenes.prototype.propertySelected = function (did, propName, check) {
    // console.log("propertySelected(%s, %s, %d, %d)", did, propName, check, that_s.currentModal);
    var tagEvent = that_s.editItemStr[that_s.currentModal];
    var devControlObjs = that_s.data["scene"][tagEvent]["deviceControl"];
    var result = that_s.getDeviceObj(did, propName);
    if (check) {
        if (!result)
            devControlObjs.push(JSON.parse('{"deviceId":"' + did + '","propName":"' + propName + '","propValue":"0"}'));
    } else {
        if (result)
            devControlObjs.splice(result['index'], 1);
        updateElementText(did + '_' + propName + '_text', "");
    }
}

Scenes.prototype.changeProperty = function (did, propName, value) {
    // console.log("changeProperty(%s, %s, %d)", did, propName, value);
    var result = that_s.getDeviceObj(did, propName);
    if (result) {
        result["obj"]["propValue"] = value.toString();
        updateElementText(did + '_' + propName + '_text', value);
    }
}

Scenes.prototype.ruleSelected = function (rid, rname, check) {
    // console.log("rid = %s %d", rid, check);
    var key = "rulbody_" + rid;
    var elem = document.getElementById(key);
    if (elem)
        elem.style.display = check ? "block" : "none";

    var tagEvent = that_s.editItemStr[that_s.currentModal];
    if (!that_s.data["scene"][tagEvent]["notifyContent"])
        that_s.data["scene"][tagEvent]["notifyContent"] = [];
    var ntfContentObjs = that_s.data["scene"][tagEvent]["notifyContent"];
    var result = that_s.getNotifyObj(rid);
    if (check) {
        if (!result)
            ntfContentObjs.push(JSON.parse('{"sceneId":"' + rid + '","sceneName":"' + rname + '","enable":"0"}'));
    } else {
        if (result)
            ntfContentObjs.splice(result['index'], 1);
    }
}

Scenes.prototype.enableRule = function (rid, check) {
    // console.log("rid = %s %d", rid, check);
    var tagEvent = that_s.editItemStr[that_s.currentModal];
    var ntfContentObjs = that_s.data["scene"][tagEvent]["notifyContent"];
    var result = that_s.getNotifyObj(rid);
    if (!result)
        return;
    if (check)
        ntfContentObjs[result['index']]["enable"] = "1";
    else
        ntfContentObjs[result['index']]["enable"] = "0";
}

Scenes.prototype.toggleSceneEvent = function(divId) {
    that_s.displayErrorMessage(null);
    switch(divId) {
        case 'onEnterDeviceControl':
        case 'onEnterNotifyContent':
        case 'onEnterMicroService':
        case 'onExitDeviceControl':
        case 'onExitNotifyContent':
        case 'onStoryMicroService':
            var btnId = divId + "_btn";
            var bodyElem = document.getElementById(divId + "_body");
            if (hasClass(btnId, "close2-icon")) {
                addClass(btnId, "open-icon");
                removeClass(btnId, "close2-icon");
                bodyElem.style.display = "block";
                addClass(divId, "device-list-item-open");
                that_s.expandArrows[divId] = true;
            } else {
                addClass(btnId, "close2-icon");
                removeClass(btnId, "open-icon");
                bodyElem.style.display = "none";
                removeClass(divId, "device-list-item-open");
                that_s.expandArrows[divId] = false;
            }
            break;
        default:
            return;
    }
}

Scenes.prototype.microServiceAdd = function(idx) {
    var tagEvent = that_s.editItemStr[that_s.currentModal];
    var miscrServrObjs = that_s.data["scene"][tagEvent]["microService"];
    if (that_s.microServices[idx] == "autolight") {
        miscrServrObjs.push({
                            "name": "autolight",
                            "msid": "" + genRandom(100, 999),
                            "deps": "",
                            "params": {
                                "lightUUID": "None",
                                "lightStep": "5",
                                "sensorUUID": "None",
                                "sensorIll": "35",
                            }});
    } else if (that_s.microServices[idx] == "gradlight") {
        miscrServrObjs.push({
                            "name": "gradlight",
                            "msid": "" + genRandom(100, 999),
                            "deps": "",
                            "params": {
                                "lightUUID": "None",
                                "direction": "up",
                                "stepCount": "8",
                                "timeSeconds": "8",
                            }});
    } else if (that_s.microServices[idx] == "sosalarm") {
        miscrServrObjs.push({
                            "name": "sosalarm",
                            "msid": "" + genRandom(100, 999),
                            "deps": "",
                            "params": {
                                "sosUUID": "None",
                                "alarmUUID": "None",
                                "lightUUID": "None",
                                "timeSeconds": "5",
                            }});
    }
    that_s.onEditModal(that_s.currentModal);
}

Scenes.prototype.microServiceDel = function(idx) {
    var tagEvent = that_s.editItemStr[that_s.currentModal];
    var miscrServrObjs = that_s.data["scene"][tagEvent]["microService"];
    if (miscrServrObjs.length == 0)
        return;
    miscrServrObjs.splice(idx, 1);
    that_s.onEditModal(that_s.currentModal);
}

Scenes.prototype.microServiceParams = function (idx, divId, value) {
    var tagEvent = that_s.editItemStr[that_s.currentModal];
    var miscrServrObjs = that_s.data["scene"][tagEvent]["microService"];
    miscrServrObjs[idx]["params"][divId] = document.getElementById(divId + idx).value;
}

Scenes.prototype.microServiceDepends = function (idx, divId, value) {
    var tagEvent = that_s.editItemStr[that_s.currentModal];
    var miscrServrObjs = that_s.data["scene"][tagEvent]["microService"];
    miscrServrObjs[idx]['deps'] = document.getElementById(divId + idx).value;
}

Scenes.prototype.saveChanges = function () {
    that_s.displayErrorMessage(null);

    var deviceCtrlObjs1 = that_s.data["scene"]["onEnter"]["deviceControl"];
    var deviceCtrlObjs2  = that_s.data["scene"]["onExit"]["deviceControl"];
    var notifyContObjs1 = that_s.data["scene"]["onEnter"]["notifyContent"];
    var notifyContObjs2  = that_s.data["scene"]["onExit"]["notifyContent"];
    var miscrServrObjs = that_s.data["scene"]["onStory"]["microService"];
    if (deviceCtrlObjs1.length == 0 && deviceCtrlObjs2.length == 0 &&
        notifyContObjs1.length == 0 && notifyContObjs2.length == 0 && miscrServrObjs.length == 0) {
        that_s.displayErrorMessage("no config!");
        return;
    }

    // console.log(that_s.data);

    for (var i = 0, l = miscrServrObjs.length; i < l; ++i) {
        micro = miscrServrObjs[i];
       if (micro['name'] == "autolight") {
           if (micro['params']['lightUUID'] == "None" ||
               micro['params']['sensorUUID'] == "None") {
               that_s.displayErrorMessage("ONSTORY: autolight config error!");
               return;
           }
       } else if (micro['name'] == "gradlight") {
           if (micro['params']['lightUUID'] == "None" ||
               parseInt(micro['params']['stepCount']) <= 0  ||
               parseInt(micro['params']['timeSeconds']) <= 0) {
               that_s.displayErrorMessage("ONSTORY: gradlight config error!");
               return;
           }
       }
    }

    that_s.modify(function(result, resData){
        if (result != global.RES_STATUS_OK)
            that_s.displayErrorMessage("save scenes error!");
        else
            that_s.initUI();
    });
    return;
}

Scenes.prototype.list = function(finishCallback) {
    var uri = "/api/familyscene/listscene";
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            return;
        }
        // console.log("request %s :%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData["result"]["data"]);
                return;
            }
            that_r.ui.buildGenericErrorModal(
                    responseData["result"]["resInfo"],
                    responseData["result"]["details"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Scenes.prototype.delete = function(rId, finishCallback) {
    var uri = "/api/familyscene/deletescene";
    var body = '{"sceneId":"' + rId + '"}';
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            return;
        }
        // console.log("request %s :%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData["result"]);
                return;
            }
            that_r.ui.buildGenericErrorModal(
                    responseData["result"]["resInfo"],
                    responseData["result"]["details"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Scenes.prototype.modify = function(finishCallback) {
    var uri = "/api/familyscene/modifyscene";
    var body = JSON.stringify(that_s.data);
    // console.log(body);
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            return;
        }
        // console.log("request %s :%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData["status"], responseData["result"]);
                return;
            }
            that_s.ui.buildGenericErrorModal(
                    responseData["result"]["resInfo"],
                    responseData["result"]["details"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Scenes.prototype.query = function(rId, finishCallback) {
    var uri = "/api/familyscene/queryscene";
    var body = '{"sceneId":"' + rId + '"}';
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            return;
        }
        // console.log("request %s :%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData["result"]["scene"]);
                return;
            }
            that_s.ui.buildGenericErrorModal(
                    responseData["result"]["resInfo"],
                    responseData["result"]["details"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Scenes.prototype.execute = function(scene, finishCallback) {
    var uri = "/api/familyscene/executescene";
    var body = '{"sceneId":"' + scene + '", "room": "default"}';
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            return;
        }
        // console.log("request %s :%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback)
                    finishCallback(responseData["result"]);
                return;
            }
            that_s.ui.buildGenericErrorModal(
                    responseData["result"]["resInfo"],
                    responseData["result"]["details"]);
        } catch (err) {
            console.log(err);
        }
    });
}
