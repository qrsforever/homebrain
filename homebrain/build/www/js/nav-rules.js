/// @file nav-rules.js
/// @Brief
/// @author QRS
/// @version 0.0.9
/// @date 2019-01-30

var Rules = function(ui, ds) {
    this.ui = ui;
    this.d = ds;
    this.expandStates = [];
    this.expandArrows = [];
    this.editItem = {
        TRIGGER: 0,
        CONDITIONS: 1,
        ACTIONS: 2,
    };
    this.currentModal = 0,
    this.editTitles = ["{{rule_trigger}}", "{{rule_conditions}}", "{{rule_actions}}"];
    this.triggerEnableItems = {
        enabled: "{{trigger_enable_trigger}}",
        timeCondition: "{{trigger_enable_time_condition}}",
        deviceCondition: "{{trigger_enable_device_condition}}",
        environmentCondition: "{{trigger_enable_environment_condition}}",
        notify: "{{trigger_enable_notify_action}}",
        manual: "{{trigger_enable_trigger_manual}}",
    };
    this.timeItems = {
        year: "{{time_year}}",
        month: "{{time_month}}",
        day: "{{time_day}}",
        week: "{{time_week}}",
        hour: "{{time_hour}}",
        minute: "{{time_minute}}",
        second: "{{time_second}}",
    }
    this.rules = [];
    this.data = null;
    this.symbols = ["==", ">=", "<=", ">", "<", "!="];
    that_r = this;
}

Rules.prototype.initUI = function() {
    console.log("Rules initUI");
    that_r.rules = [];
    that_r.ui.showLoadingIndicator("RULES LOADING...");
    that_r.list(function(resData) {
        that_r.ui.displayNav(global.NAV_RULES, function() {
        that_r.currentModal = 0;
        });
        var length = resData.length;
        var body = document.getElementById("body-rules");
        var intro = document.getElementById("introduction-rules");
        if (length == 0) {
            body.style.display = "none";
            intro.style.display = "";
            return;
        }
        var html = "";
        for (let i = 0; i < length; ++i) {
            var label = resData[i]["sceneName"];
            var descr = resData[i]["description"];
            var rid = resData[i]["sceneId"];
            var enable = resData[i]["triggerEnabled"];
            var manual = resData[i]["manualEnabled"];
            var friends = resData[i]["friends"];
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
            html += '                src="' + getIconUrl("ic_default_rule") + '"';
            html += '                alt=""/>';
            html += '        </button>';
            html += '        <div class="device-list-item-header-rest">';
            html += '           <div class="device-list-item-header-title">';
            html += '               <label id="' + rid + '"> <span>' + label + '</span></label>';
            html += '           </div>';
            if (enable == "1" && manual == "1") {
                if (friends.length > 0) {
                    html += '  <button';
                    html += '      class="default-icon-button list-icon-button lock-rule-icon"';
                    html += '      onclick="onRuleModify(' + i + ')"';
                    html += '  ></button>';
                }
                html += '      <button';
                html += '          class="default-icon-button list-icon-button activate-icon"';
                html += '          onclick="onRuleExecute(\'' + rid + '\')"';
                html += '      ></button>';
            }
            html += '           <button';
            html += '               id="' + 'toggleSwitch' + rid + '"';
            html += '               class="default-icon-button list-icon-button ' + (enable == "1" ? "switch-on-icon" : "switch-off-icon") + '"';
            html += '               onclick="onToggleRuleSwitch(\'' + rid + '\', \'' + "toggleSwitch" + rid + '\')"';
            html += '           ></button>';
            html += '           <button';
            html += '               class="default-icon-button list-icon-button trash-icon"';
            html += '               onclick="onRuleDelete(\'' + rid + '\')"';
            html += '           ></button>';
            html += '           <button';
            html += '               id="' + 'toggleExpand' + rid + '"';
            html += '               class="default-icon-button list-icon-button close-icon"';
            html += '               onclick="onToggleContainer(\'' + rid + '\')"';
            html += '           ></button>';
            html += '        </div>';
            html += '    </div>';
            html += '    <div id="description' + rid + '"';
            html += '        style="display:' + (that_r.expandStates[rid] == true ? 'block' : 'none') + ';"';
            html += '        class="device-list-item-body">';
            html += '        <div class="device-list-item-resource">';
            html += '            <div class="device-list-item-property">';
            html += '               <div class="device-list-item-property-title">{{id}}</div>';
            html += '               <div class="device-list-item-property-value">';
            html += '                  <label id="' + rid + '_label1"> <span>' + rid + '</span></label>';
            html += '               </div>';
            html += '            </div>';
            html += '            <div class="device-list-item-property">';
            html += '               <div class="device-list-item-property-title">{{rule_description}}</div>';
            html += '               <div class="device-list-item-property-value">';
            html += '                  <label id="' + rid + '_label2"> <span>' + descr + '</span></label>';
            html += '               </div>';
            html += '            </div>';
            if (friends.length > 0) {
                html += '        <div class="device-list-item-property">';
                html += '           <div class="device-list-item-property-title">{{rule_friends}}</div>';
                html += '           <div class="device-list-item-property-value">';
                html += '              <label id="' + rid + '_label3"> <span>' + friends  + '</span></label>';
                html += '           </div>';
                html += '        </div>';
            }
            html += '        </div>';
            html += '    </div>';
            html += '</div>';
            that_r.rules.push(resData[i]);
        }
        intro.style.display = "none";
        body.style.display = "";
        body.innerHTML = html;
    });
}

Rules.prototype.toggleExpandBody = function(rid) {
    var toggleExpandId = "toggleExpand" + rid;
    var descrBodyId = "description" + rid;
    var descrElem = document.getElementById(descrBodyId);

    if (that_r.expandStates[rid] == true) {
        removeClass(descrBodyId, "device-list-item-open");
        if (descrElem)
            descrElem.style.display = "none";
        removeClass(toggleExpandId , "open-icon");
        addClass(toggleExpandId , "close-icon");
        that_r.expandStates[rid] = false;
    } else {
        addClass(descrBodyId, "device-list-item-open");
        if (descrElem)
            descrElem.style.display = "block";
        removeClass(toggleExpandId, "close-icon");
        addClass(toggleExpandId, "open-icon");
        that_r.expandStates[rid] = true;
    }
}

Rules.prototype.toggleSwitchBinary = function(rId, key) {
    if (hasClass(key, "switch-off-icon")) {
        that_r.query(rId, function(resData){
            that_r.data = resData;
            that_r.data["trigger"]["switch"]["enabled"] = "on";
            that_r.saveChanges();
        });
    } else {
        that_r.query(rId, function(resData){
            that_r.data = resData;
            that_r.data["trigger"]["switch"]["enabled"] = "off";
            that_r.saveChanges();
        });
    }
}

Rules.prototype.displayErrorMessage = function (msg) {
	var errDiv = document.getElementById("rule_editmodal_errormsg");
    if (errDiv) {
        if (msg)
            errDiv.textContent = msg;
        else
            errDiv.innerHTML = "&nbsp;";
    }
}

Rules.prototype.getDeviceObj = function (did, propName) {
    console.log("getDeviceObj %s %s %d", did, propName, that_r.currentModal);
    if (that_r.currentModal == that_r.editItem.CONDITIONS) {
        let devStatusObjs = that_r.data["conditions"]["deviceCondition"]["deviceStatus"];
        for (let i = 0, l = devStatusObjs.length; i < l; ++i) {
            if (devStatusObjs[i]["deviceId"] == did &&
                devStatusObjs[i]["propName"] == propName) {
                return {'index': i, 'obj': devStatusObjs[i]};
            }
        }
    } else if (that_r.currentModal == that_r.editItem.ACTIONS) {
        let devControlObjs = that_r.data["actions"]["deviceControl"];
        for (let i = 0, l = devControlObjs.length; i < l; ++i) {
            if (devControlObjs[i]["deviceId"] == did &&
                devControlObjs[i]["propName"] == propName) {
                return {'index': i, 'obj': devControlObjs[i]};
            }
        }
    }
    return null;
}

Rules.prototype.getArrowClass = function(divId) {
    return !that_r.expandArrows[divId] ? "close2-icon" : "open-icon";
}

Rules.prototype.onCreate = function() {
    that_r.data = {
        "sceneId": "",
        "sceneName": "",
        "description":"",
        "trigger": {
            "switch": {
                "enabled": "on",
                "notify": "off",
                "timeCondition": "off",
                "deviceCondition": "on",
                "environmentCondition": "off",
                "manual": "off"
            },
            "triggerType": "auto|manual"
        },
        "conditions": {
            "conditionLogic": "and",
            "timeCondition": [],
            "deviceCondition": {
                "deviceLogic": "and",
                "deviceStatus": []
            },
            "environmentCondition": {
                "environmentLogic": "and",
                "environmentStatus":[]
            },
        },
        "actions": {
            "deviceControl": [],
            "manualRuleId": [],
            "notify":{}
        }
    };
    that_r.onEditModal(0);
}

Rules.prototype.onEdit = function(idx) {
    if (idx < -1 || idx >= that_r.rules.length)
        return;
    var rule = that_r.rules[idx];
    if (rule.manualEnabled == "1" && rule.friends.length > 0) {
        var friends = rule.friends.split("|");
        console.log(rule.friends);
        var html = "";
        html += '<h3 class="modal-title">';
        html +=      "{{rule_associated_title}}";
        html += '</h3>';
        html += '<p class="modal-info-text">';
        html +=      "{{rule_associated_info}}";
        html += '</p>';
        html += '<div class="modal-scrollarea">';
        for (var i = 0, l = friends.length; i < l; i++) {
            for (var j = 0, s = that_r.rules.length; j < s; ++j) {
                if (that_r.rules[j].sceneId == friends[i]) {
                    html += '<label class="device-selection-item">';
                    html += '    <img class="device-selection-item-icon"';
                    html += '        src="' + getIconUrl("ic_default_rule") + '" />';
                    html += '    <div class="device-selection-item-rest">';
                    html += '        <span class="device-selection-item-name">';
                    html +=              that_r.rules[j].sceneName;
                    html += '        </span>';
                    html += '        <button id="' + rule.sceneId + '"';
                    html += '            class="btn-primary"';
                    html += '            onclick="onRuleModify(' + j + ');">';
                    html +=             "{{rule_edit}}";
                    html += '        </button>';
                    html += '    </div>'
                    html += '</label>';
                }
            }
        }
        html += '</div>';
        that_d.ui.openModal(html);
    } else {
        that_r.query(rule.sceneId, function(resData){
            that_r.data = resData;
            that_r.onEditModal(0);
        });
    }
}

Rules.prototype.onDelete = function(rId) {
    that_r.delete(rId, function(resData){
        that_r.initUI();
    });
}

Rules.prototype.onExecute = function(rId) {
    that_r.execute(rId, function(resData){
    });
}

Rules.prototype.onEditModal = function(eId) {
    that_r.currentModal = eId;
	var html = "";
	html += '<h3 class="modal-title">';
	html +=      "{{tab_title_rules}}";
	html += '</h3>';

	html += '<div class="modal-rule-header">';
	html += '    <div class="device-list-item">';
	html += '        <div class="device-list-item-header">';
    html += '            <img class="rule-icon ' + (eId == that_r.editItem.TRIGGER ? 'hl_bg"' : '"');
    html += '                alt="" src="' + getIconUrl("rule_trigger") + '" ';
    html += '                onclick="javascript:onRuleEditModalClicked(' + that_r.editItem.TRIGGER + ');"/>';
    html += '            <img class="rule-icon ' + (eId == that_r.editItem.CONDITIONS ? 'hl_bg"' : '"');
    html += '                alt="" src="' + getIconUrl("rule_conditions") + '" ';
    html += '                onclick="javascript:onRuleEditModalClicked(' + that_r.editItem.CONDITIONS + ');"/>';
    html += '            <img class="rule-icon ' + (eId == that_r.editItem.ACTIONS ? 'hl_bg"' : '"');
    html += '                alt="" src="' + getIconUrl("rule_actions") + '" ';
    html += '                onclick="javascript:onRuleEditModalClicked(' + that_r.editItem.ACTIONS + ');"/>';
    html += '        </div>';
    html += '    </div>';
	html += '</div>';
    html += '<div class="page-header">';
    html += '    <h1 class="page-title">' + that_r.editTitles[eId] + '</h1>';
    if (eId == that_r.editItem.CONDITIONS) {
        var orLogic = (that_r.data["conditions"]["conditionLogic"] == "or") ? true : false;
        html += '<div style="margin-left:20%">';
        html += '    <label><span>or</span></label>';
        html += '    <input id="deviceConditionsOR" type="radio" name="conditionLogic" ' + (orLogic ? 'checked' : '');
        html += '          class="scene-checkbox" onclick="onConditonLogic(\'conditionLogic\', \'or\');"/>';
        html += '    <label><span>and</span></label>';
        html += '    <input id="deviceConditionsAND" type="radio" name="conditionLogic" '+ (orLogic ? '' : 'checked');
        html += '          class="scene-checkbox" onclick="onConditonLogic(\'conditionLogic\', \'and\');"/>';
        html += '</div>';
    }
    html += '</div>';

    html += '<div class="modal-container">';
    if (eId == that_r.editItem.TRIGGER) {
        html += '<div id="trigger-enable" class="device-list-item-body" style="display:block;">';
        html += '    <div class="device-list-item-property">';
        html += '        <div class="device-list-item-property-title">';
        html +=              "{{rule_name}}";
        html += '        </div>';
        html += '        <div class="device-list-item-property-value">';
        html += '            <input id = "trigger_changerulename"';
        html += '                   type="text" placeholder="{{rule_name_hint}}" class="rule-input"';
        html += '                   value="' + that_r.data["sceneName"] + '"';
        html += '                   onchange="onChangeRuleInfo(1, this.value)">';
        html += '        </div>';
        html += '    </div>';
        html += '    <div class="device-list-item-property">';
        html += '        <div class="device-list-item-property-title">';
        html +=              "{{rule_description}}";
        html += '        </div>';
        html += '        <div class="device-list-item-property-value">';
        html += '            <input id = "trigger_changeruledescription"';
        html += '                   type="text" placeholder="{{rule_description_hint}}" class="rule-input"';
        html += '                   value="' + that_r.data["description"] + '"';
        html += '                   onchange="onChangeRuleInfo(2, this.value)">';
        html += '        </div>';
        html += '    </div>';
        html += '    <div class="device-list-item-resource">';
        for (var key in that_r.triggerEnableItems) {
            let iconCls = that_r.data["trigger"]["switch"][key] === "on" ? "enable-icon" : "disable-icon";
            html += '    <div class="device-list-item-property">';
            html += '        <div class="trigger-list-item-property-title">' + that_r.triggerEnableItems[key] + '</div>';
            html += '        <div class="trigger-list-item-property-value">';
            html += '            <button id="' + 'trigger_enable_' +  key + '"';
            html += '                    class="property-control default-icon-button ' + iconCls + '"';
            html += '                    onclick="onToggleTriggerEnable(\'' + key + '\')">';
            html += '            </button>';
            html += '        </div>';
            html += '    </div>';
        }
        html += '    </div>';
        html += '</div>';
    } else {
        html += '<div id="devices-selection-modal-list" class="modal-scrollarea">';
        html += '        <div class="spinning-wheel-container">';
        html += '            <div class="spinning-wheel"></div>';
        html += '        </div>';
        html += '</div>';
    }
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
    that_r.ui.openModal(html);
    if (eId != that_r.editItem.TRIGGER)
        that_r.buildRuleList(eId);
}

Rules.prototype.buildRuleList = function (eId) {
    var devs = that_r.d.devs
    var prfs = that_r.d.prfs;

    function leftHtmlCallback(did, propName) {
        var result = that_r.getDeviceObj(did, propName);
        var html = "";
        html += '<input';
        html += '    id="devbody_' + did + '_' + propName + '"';
        html += '    class="property-control-checkbox" ';
        html += '    type="checkbox" ';
        html +=      result ? "checked" : "";
        html += '    onclick="onPropertySelected(\'' + did + '\', \'' + propName + '\', this.checked)"';
        html += '/>';
        return html;
    }

    function rightHtmlCallback(did, propName, type) {
        if  (type != "int")
            return "";
        var symbIdx = -1;
        var symbols = that_r.symbols;
        var result = that_r.getDeviceObj(did, propName);
        if (result) {
            for (var i = 0, l = symbols.length; i < l; ++i) {
                if (result["obj"]["propValue"].indexOf(symbols[i]) > 0) {
                    symbIdx = i;
                    break;
                }
            }
        }
        var html = "";
        html += '<div class="device-combobox-container">';
        html += '    <select';
        html += '        id="symbolSelected_' + did + '_' + propName + '"';
        html += '        class="device-combobox"';
        html += '        onchange="onSymbolSelected(\'' + did + '\', \'' + propName + '\', this.value)"';
        html +=          result ? 'style="display:block">' : 'style="display:none">';
        for (var i = 0, l = symbols.length; i < l; ++i) {
            if (i == symbIdx)
                html += '<option class="device-combobox-item" value="" selected="">' + symbols[i] + '</option>';
            else
                html += '<option class="device-combobox-item" value="' + symbols[i] + '">' + symbols[i] + '</option>';
        }
        html += '    </select>';
        html += '</div>';
        html += '<span id="' + did + '_' + propName + '_text" class="property-control-span"></span>';
        return html;
    }

    var deviceLogic = that_r.data["conditions"]["deviceCondition"]["deviceLogic"];
    var devStatusObjs = that_r.data["conditions"]["deviceCondition"]["deviceStatus"];
    var envLogic = that_r.data["conditions"]["environmentCondition"]["environmentLogic"];
    var envStatusObjs = that_r.data["conditions"]["environmentCondition"]["environmentStatus"];
    var timeCond = that_r.data["conditions"]["timeCondition"];
    var devControlObjs = that_r.data["actions"]["deviceControl"];
    var devOrLogic = (deviceLogic == "or") ? true : false;
    var envOrLogic = (envLogic == "or") ? true : false;
    var manualRuleObjs = that_r.data["actions"]["manualRuleId"];

    var html = "";
    html += '<div id="deviceCondition" class="device-list-item ">';
    html += '    <div class="device-list-item-header">';
    html += '        <button id="deviceCondition_btn" ';
    html += '              class="default-icon-button list-icon-button ' + that_r.getArrowClass("deviceCondition") + '"';
    html += '              onclick="onToggleCondition(\'deviceCondition\');"></button>';
    html += '        <div class="device-list-item-open device-list-item-header-rest">';
    html += '            <div class="device-list-item-header-title">';
    html += '                <label><span>{{device_condition}}</span></label>';
    html += '            </div>';
    if (eId == that_r.editItem.CONDITIONS) {
        html += '        <label><span>or</span></label>';
        html += '        <input id="deviceConditionsOR" type="radio" name="deviceLogic" ' + (devOrLogic ? 'checked' : '');
        html += '              class="scene-checkbox" onclick="onConditonLogic(\'deviceLogic\', \'or\');"/>';
        html += '        <label><span>and</span></label>';
        html += '        <input id="deviceConditionsAND" type="radio" name="deviceLogic" '+ (devOrLogic ? '' : 'checked');
        html += '              class="scene-checkbox" onclick="onConditonLogic(\'deviceLogic\', \'and\');"/>';
    }
    html += '        </div>';
    html += '    </div>';
    html += '    <div id="deviceCondition_body" style="display:' + (!that_r.expandArrows["deviceCondition"] ? "none" : "block") + '">';
    for(var did in devs) {
        var tid = devs[did].typeId;
        var iconSrc = "ic_default_device";
        if (prfs[tid])
            iconSrc = prfs[tid]['iconid'];
        var checked = false;
        if (eId == that_r.editItem.CONDITIONS) {
            for (var i = 0, l = devStatusObjs.length; i < l; ++i) {
                if (devStatusObjs[i]['deviceId'] == did) {
                    checked = true;
                    break;
                }
            }
        } else if (eId == that_r.editItem.ACTIONS) {
            for (var i = 0, l = devControlObjs.length; i < l; ++i) {
                if (devControlObjs[i]['deviceId'] == did) {
                    checked = true;
                    break;
                }
            }
        }
        html += '<div class="rule-device-selection-border rule-condition-body">';
        html += '    <label class="rule-device-selection-item">';
        html += '         <img';
        html += '             id="' + did + '_icon" ';
        html += '             class="device-selection-item-icon"';
        html += '             src="' + getIconUrl(iconSrc) + '"';
        html += '             alt=""';
        html += '         />';
        html += '        <div class="device-selection-item-rest">';
        html += '            <span class="device-selection-item-name">';
        html +=                  did + " " + devs[did].label;
        html += '            </span>';
        html += '            <input id="ruleEntry" type="checkbox"';
        html +=                  checked ? "checked" : "";
        html += '                onclick="onDeviceSelected(\'' + did + '\', this.checked)"';
        html += '            />';
        html += '        </div>';
        html += '    </label>';
        html += '   <div id="devbody_' + did + '"';
        html += '       class="device-list-item-body"';
        html +=         checked ? ' style="display:block">' : ' style="display:none">';
        html +=         that_r.d.buildDeviceBody(did, tid, leftHtmlCallback, eId == that_r.editItem.CONDITIONS ? rightHtmlCallback : null);
        html += '   </div>';
        html += '</div>';
    }
    html += '   </div>';
    html += '</div>';
    if (eId == that_r.editItem.CONDITIONS) {
        html += '<div id="environmentCondition" class="device-list-item ">';
        html += '    <div class="device-list-item-header">';
        html += '        <button id="environmentCondition_btn" ';
        html += '              class="default-icon-button list-icon-button ' + that_r.getArrowClass("environmentCondition") + '"';
        html += '              onclick="onToggleCondition(\'environmentCondition\');"></button>';
        html += '        <div class="device-list-item-open device-list-item-header-rest">';
        html += '            <div class="device-list-item-header-title">';
        html += '                <label><span>{{env_condition}}</span></label>';
        html += '            </div>';
        html += '            <label><span>or</span></label>';
        html += '            <input id="deviceConditionsOR" type="radio" name="envLogic" ' + (envOrLogic ? 'checked' : '');
        html += '                  class="scene-checkbox" onclick="onConditonLogic(\'environmentLogic\', \'or\');"/>';
        html += '            <label><span>and</span></label>';
        html += '            <input id="deviceConditionsAND" type="radio" name="envLogic" '+ (envOrLogic ? '' : 'checked');
        html += '                  class="scene-checkbox" onclick="onConditonLogic(\'environmentLogic\', \'and\');"/>';
        html += '        </div>';
        html += '    </div>';
        html += '    <div id="" style="display:none"><center>Not implement yet</center></div>';
        html += '    <div id="environmentCondition_body" style="display:' + (!that_r.expandArrows["environmentCondition"] ? "none" : "block") + '">';
        html += '        <center>Not implement yet</center>';
        html += '    </div>';
        html += '</div>';

        html += '<div id="timeCondition" class="device-list-item ">';
        html += '    <div class="device-list-item-header">';
        html += '        <button id="timeCondition_btn" ';
        html += '              class="default-icon-button list-icon-button ' + that_r.getArrowClass("timeCondition") + '"';
        html += '              onclick="onToggleCondition(\'timeCondition\');"></button>';
        html += '        <div class="device-list-item-open device-list-item-header-rest">';
        html += '            <div class="device-list-item-header-title">';
        html += '                <label><span>{{time_condition}}</span></label>';
        html += '            </div>';
        html += '        </div>';
        html += '    </div>';
        html += '    <div id="timeCondition_body" style="display:' + (!that_r.expandArrows["timeCondition"] ? "none" : "block") + '">';
        html += '        <div id="timeCondition_enable" class="device-list-item-body">';
        for (var i = 0, l = timeCond.length; i < l; ++i) {
            html += '        <center><button id="timeCondition_del' + i + '"';
            html += '            class="default-icon-button del-icon"';
            html += '            onclick="onTimeConditionDel(' + i + ');">';
            html += '        </button></center>';
            for (var it in that_r.timeItems) {
                html += '    <div class="device-list-item-property">';
                html += '        <div class="device-list-item-property-title">';
                html +=              that_r.timeItems[it];
                html += '        </div>';
                html += '        <div class="device-list-item-property-value">';
                html += '            <input id = "time_"' + it;
                html += '                   type="text" class="rule-input"';
                html += '                   placeholder="every"';
                if (timeCond[i][it])
                    html += '               value=' + timeCond[i][it];
                html += '                   onchange="onRuleTime(' + i + ',\'' + it + '\', this.value)">';
                html += '        </div>';
                html += '    </div>';
            }
        }
        html += '        <center><button id="timeCondition_add' + i + '"';
        html += '            class="default-icon-button add-icon"';
        html += '            onclick="onTimeConditionAdd();">';
        html += '        </button></center>';
        html += '        </div>';
        html += '    </div>';
        html += '</div>';
    } else if (eId == that_r.editItem.ACTIONS) {
        html += '<div id="notify" class="device-list-item ">';
        html += '    <div class="device-list-item-header">';
        html += '        <button id="notify_btn" ';
        html += '              class="default-icon-button list-icon-button ' + that_r.getArrowClass("notify") + '"';
        html += '              onclick="onToggleCondition(\'notify\');"></button>';
        html += '        <div class="device-list-item-open device-list-item-header-rest">';
        html += '            <div class="device-list-item-header-title">';
        html += '                <label><span>{{notify_action}}</span></label>';
        html += '            </div>';
        html += '        </div>';
        html += '    </div>';
        html += '    <div id="notify_body" style="' + (!that_r.expandArrows["notify"] ? "display:none" : "display:block") + '">';
        html += '        <center>Not implement yet</center>';
        html += '    </div>';
        html += '</div>';

        if (that_r.data["trigger"]["switch"]["manual"] != "on") {
            html += '<div id="manualRuleId" class="device-list-item ">';
            html += '    <div class="device-list-item-header">';
            html += '        <button id="manualRuleId_btn" ';
            html += '              class="default-icon-button list-icon-button ' + that_r.getArrowClass("manualRuleId") + '"';
            html += '              onclick="onToggleCondition(\'manualRuleId\');"></button>';
            html += '        <div class="device-list-item-open device-list-item-header-rest">';
            html += '            <div class="device-list-item-header-title">';
            html += '                <label><span>{{manual_action}}</span></label>';
            html += '            </div>';
            html += '        </div>';
            html += '    </div>';
            html += '    <div id="manualRuleId_body" style="display:' + (!that_r.expandArrows["manualRuleId"] ? "none" : "block") + '">';
            html += '        <div class="rule-device-selection-border rule-condition-body">';
            for (var i = 0, l = that_r.rules.length; i < l; ++i) {
                var rid = that_r.rules[i]["sceneId"];
                if (that_r.rules[i]["manualEnabled"] != "1" || rid == that_r.data["sceneId"])
                    continue;
                var rname = that_r.rules[i]["sceneName"]
                var checked = false;
                if (manualRuleObjs.length > 0 && manualRuleObjs.indexOf(rid) > -1)
                    checked = true;
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
                html += '                <input id="manualRuleEntry" type="checkbox"';
                html +=                      checked ? "checked" : "";
                html += '                    onclick="onManualRuleSelected(\'' + rid + '\', this.checked)"';
                html += '                />';
                html += '            </div>';
                html += '        </label>';
            }
            html += '        </div>';
            html += '    </div>';
            html += '</div>';
        }
    }

    var listRule = document.getElementById("devices-selection-modal-list");
    listRule.innerHTML = html;

    if (eId == that_r.editItem.CONDITIONS) {
        for (var i = 0, l = devStatusObjs.length; i < l; ++i) {
            let symbols = that_r.symbols;
            let did = devStatusObjs[i]['deviceId'];
            let name = devStatusObjs[i]['propName'];
            let value = devStatusObjs[i]['propValue'];
            for (var j = 0, s = symbols.length; j < s; ++j) {
                if (value.indexOf(symbols[j]) > 0) {
                    value = value.split(symbols[j])[1].toString();
                    that_r.d.updateValue(did, name, value);
                    updateElementText(did + '_' + name + '_text', value);
                    break;
                }
            }
        }
    } else if (eId == that_r.editItem.ACTIONS) {
        for (var i = 0, l = devControlObjs.length; i < l; ++i) {
            let did = devControlObjs[i]['deviceId'];
            let name = devControlObjs[i]['propName'];
            let value = devControlObjs[i]['propValue'];
            that_r.d.updateValue(did, name, value);
            updateElementText(did + '_' + name + '_text', value);
        }
    }
}

Rules.prototype.deviceSelected = function (did, check) {
    console.log("did = %s %d", did, check);
    var key = "devbody_" + did;
    var elem = document.getElementById(key);
    if (elem)
        elem.style.display = check ? "block" : "none";

    if (that_r.currentModal == that_r.editItem.CONDITIONS) {
        var devStatusObjs = that_r.data["conditions"]["deviceCondition"]["deviceStatus"];
        if (check) {
            if (!devStatusObjs)
                that_r.data["conditions"]["deviceCondition"]["deviceStatus"] = [];
        } else {
            if (devStatusObjs)
                that_r.data["conditions"]["deviceCondition"]["deviceStatus"] = [];
        }
    } else if (that_r.currentModal == that_r.editItem.ACTIONS) {
        var devControlObjs = that_r.data["actions"]["deviceControl"];
        if (check) {
            if (!devControlObjs)
                that_r.data["actions"]["deviceControl"] = [];
        } else {
            if (devControlObjs)
                that_r.data["actions"]["deviceControl"] = [];
        }
    }
}

Rules.prototype.propertySelected = function (did, propName, check) {
    console.log("propertySelected(%s, %s, %d, %d)", did, propName, check, that_r.currentModal);
    if (that_r.currentModal == that_r.editItem.CONDITIONS) {
        var elem = document.getElementById('symbolSelected_' + did + '_' + propName);
        if (elem)
            elem.style.display = check ? "block" : "none";

        var devStatusObjs = that_r.data["conditions"]["deviceCondition"]["deviceStatus"];
        var result = that_r.getDeviceObj(did, propName);
        if (check) {
            if (!result)
                devStatusObjs.push(JSON.parse('{"deviceId":"' + did + '","propName":"' + propName + '","propValue":"v==0"}'));
        } else {
            if (result)
                devStatusObjs.splice(result['index'], 1);
            updateElementText(did + '_' + propName + '_text', "");
        }
    } else if (that_r.currentModal == that_r.editItem.ACTIONS) {
        var devControlObjs = that_r.data["actions"]["deviceControl"];
        var result = that_r.getDeviceObj(did, propName);
        if (check) {
            if (!result)
                devControlObjs.push(JSON.parse('{"deviceId":"' + did + '","propName":"' + propName + '","propValue":"0"}'));
        } else {
            if (result)
                devControlObjs.splice(result['index'], 1);
            updateElementText(did + '_' + propName + '_text', "");
        }
    }
}

Rules.prototype.symbolSelected = function (did, propName, symbol) {
    console.log("symbolSelected(%s, %s, %s)", did, propName, symbol);
    var result = that_r.getDeviceObj(did, propName);
    if (result) {
        var propValue = result["obj"]["propValue"];
        for (var i = 0, l = that_r.symbols.length; i < l; ++i) {
            if (propValue.indexOf(that_r.symbols[i]) > 0) {
                result["obj"]["propValue"] = propValue.replace(that_r.symbols[i], symbol);
                break;
            }
        }
    }
}

Rules.prototype.manualRuleSelected = function (rid, check) {
    manualRuleObjs = that_r.data["actions"]["manualRuleId"];
    if (check)
        manualRuleObjs.push(rid.toString());
    else {
        var idx = manualRuleObjs.indexOf(rid.toString())
        if (idx > -1)
            manualRuleObjs.splice(idx, 1);
    }
}

Rules.prototype.timeConditionAdd = function() {
    var timeConds = that_r.data["conditions"]["timeCondition"];
    timeConds.push({"year":"", "month":"", "day":"", "week":"", "hour":"", "minute":"", "second":""});
    that_r.onEditModal(that_r.editItem.CONDITIONS);
}

Rules.prototype.timeConditionDel = function(idx) {
    var timeConds = that_r.data["conditions"]["timeCondition"];
    if (timeConds.length == 0)
        return;
    timeConds.splice(idx, 1);
    that_r.onEditModal(that_r.editItem.CONDITIONS);
}

Rules.prototype.timeSetting = function(idx, what, value) {
    var timeConds = that_r.data["conditions"]["timeCondition"];
    if (idx < 0 || idx >= timeConds.length)
        return;
    timeConds[idx][what] = value;
}

Rules.prototype.changeProperty = function (did, propName, value) {
    console.log("changeProperty(%s, %s, %d)", did, propName, value);
    var result = that_r.getDeviceObj(did, propName);
    if (result) {
        value = value.toString();
        if (that_r.currentModal == that_r.editItem.CONDITIONS) {
            var propValue = result["obj"]["propValue"];
            var symb = "==";
            for (var i = 0, l = that_r.symbols.length; i < l; ++i) {
                if (propValue.indexOf(that_r.symbols[i]) > 0) {
                    symb = that_r.symbols[i];
                    break;
                }
            }
            result["obj"]["propValue"] = 'v' + symb + value;
        } else if (that_r.currentModal == that_r.editItem.ACTIONS) {
            result["obj"]["propValue"] = value;
        }
        updateElementText(did + '_' + propName + '_text', value);
    }
}

Rules.prototype.changeInfo = function (what, value) {
    console.log("changeInfo(%d, %s)\n", what, value);
    if (what == 1)
        that_r.data["sceneName"] = value.replace(/\"/g, "\\\"");
    else if (what == 2)
        that_r.data["description"] = value.replace(/\"/g, "\\\"");
}

Rules.prototype.toggleEnable = function (key) {
    var divId = "trigger_enable_" + key;
    if (hasClass(divId, "disable-icon")) {
        addClass(divId, "enable-icon");
        removeClass(divId, "disable-icon");
        that_r.data["trigger"]["switch"][key] = "on";
    } else {
        addClass(divId, "disable-icon");
        removeClass(divId, "enable-icon");
        that_r.data["trigger"]["switch"][key] = "off";
    }
}


Rules.prototype.toggleCondition = function(divId) {
    switch(divId) {
        case 'deviceCondition':
        case 'timeCondition':
        case 'environmentCondition':
        case 'notify':
        case 'manualRuleId':
            var btnId = divId + "_btn";
            var bodyElem = document.getElementById(divId + "_body");
            if (hasClass(btnId, "close2-icon")) {
                addClass(btnId, "open-icon");
                removeClass(btnId, "close2-icon");
                bodyElem.style.display = "block";
                addClass(divId, "device-list-item-open");
                that_r.expandArrows[divId] = true;
            } else {
                addClass(btnId, "close2-icon");
                removeClass(btnId, "open-icon");
                bodyElem.style.display = "none";
                removeClass(divId, "device-list-item-open");
                that_r.expandArrows[divId] = false;
            }
            break;
        default:
            return;
    }
}

Rules.prototype.conditionLogicSelected = function(type, logic) {
    switch(type) {
        case 'conditionLogic':
            that_r.data["conditions"]["conditionLogic"] = logic;
            break;
        case 'deviceLogic':
            that_r.data["conditions"]["deviceCondition"]["deviceLogic"] = logic;
            break;
        case 'deviceLogic':
            that_r.data["conditions"]["environmentCondition"]["environmentLogic"] = logic;
            break;
        default:
            ;
    }
}

Rules.prototype.saveChanges = function () {
    if (that_r.data["sceneName"] == "") {
        that_r.displayErrorMessage("{{rule_errmsg_rule_name_not_set}}");
        return;
    }

    if (!that_r.data["sceneId"]) {
        for(var i = 0, l = that_r.rules.length; i < l; ++i) {
            if (that_r.rules[i]["sceneName"] == that_r.data["sceneName"]) {
                that_r.displayErrorMessage("{{rule_errmsg_rule_name_already_exist}}");
                return;
            }
        }
    }

    if (that_r.data["trigger"]["switch"]["timeCondition"] != "on" &&
        that_r.data["trigger"]["switch"]["deviceCondition"] != "on" &&
        that_r.data["trigger"]["switch"]["environmentCondition"] != "on" &&
        that_r.data["trigger"]["switch"]["manual"] != "on") {
        that_r.displayErrorMessage("{{rule_errmsg_one_condition_at_least}}");
        return;
    }

    if (that_r.data["trigger"]["switch"]["timeCondition"] == "on"){
        var timeObj = that_r.data["conditions"]["timeCondition"];
        if (!timeObj || timeObj.length == 0) {
            that_r.displayErrorMessage("{{rule_errmsg_time_condition_not_set}}");
            return;
        }
    }

    if (that_r.data["trigger"]["switch"]["deviceCondition"] == "on"){
        var devStatusObj = that_r.data["conditions"]["deviceCondition"]["deviceStatus"];
        if (!devStatusObj || devStatusObj.length == 0) {
            that_r.displayErrorMessage("{{rule_errmsg_device_condition_not_set}}");
            return;
        }
    }

    if (that_r.data["trigger"]["switch"]["environmentCondition"] == "on"){
        var envStatusObj = that_r.data["conditions"]["environmentCondition"];
        if (!envStatusObj || envStatusObj.length == 0) {
            that_r.displayErrorMessage("{{rule_errmsg_environment_condition_not_set}}");
            return;
        }
    }

    var actions = that_r.data["actions"];
    if (!actions) {
        that_r.displayErrorMessage("{{rule_errmsg_actions_not_set}}");
        return;
    }
    if (!((actions["deviceControl"]
              && actions["deviceControl"].length) ||
          (actions["manualRuleId"]
              && actions["manualRuleId"].length) ||
          (actions["notify"]
              && actions["notify"].length))) {
        that_r.displayErrorMessage("{{rule_errmsg_actions_not_set}}");
        return;
    }

    if (that_r.data["trigger"]["switch"]["manual"] == "on")
        actions["manualRuleId"] = [];

    that_r.displayErrorMessage(null);

    // console.log(that_r.data);

    if (!that_r.data["sceneId"]) {
        that_r.add(function(result, resData){
            if (result != global.RES_STATUS_OK)
                that_r.displayErrorMessage("{{rule_errmsg_rule_add_error}}");
            else
                that_r.initUI();
        });
    } else {
        that_r.modify(function(result, resData){
            if (result != global.RES_STATUS_OK)
                that_r.displayErrorMessage("{{rule_errmsg_rule_mod_error}}");
            else
                that_r.initUI();
        });
    }
}

Rules.prototype.add = function(finishCallback) {
    var uri = "/api/familyscene/add";
    var body = JSON.stringify(that_r.data);
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
            that_r.ui.buildGenericErrorModal(
                    responseData["result"]["resInfo"],
                    responseData["result"]["details"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Rules.prototype.delete = function(rId, finishCallback) {
    var uri = "/api/familyscene/delete";
    var body = '{"sceneId":"' + rId + '"}';
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
                    responseData["result"]["resInfo"],
                    responseData["result"]["details"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Rules.prototype.modify = function(finishCallback) {
    var uri = "/api/familyscene/modify";
    var body = JSON.stringify(that_r.data);
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
            that_r.ui.buildGenericErrorModal(
                    responseData["result"]["resInfo"],
                    responseData["result"]["details"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Rules.prototype.query = function(rId, finishCallback) {
    var uri = "/api/familyscene/query";
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
            that_r.ui.buildGenericErrorModal(
                    responseData["result"]["resInfo"],
                    responseData["result"]["details"]);
        } catch (err) {
            console.log(err);
        }
    });
}

Rules.prototype.list = function(finishCallback) {
    var uri = "/api/familyscene/listall";
    httpRequest("POST", uri, body, function(responseText, error) {
        if (error && error.status != 200) {
            console.log(error);
            return;
        }
        // console.log("request %s :%s", uri, responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                if (finishCallback) {
                    var rules = responseData["result"]["data"];
                    finishCallback(rules.sort(function(a, b){
                        return a.sceneName.localeCompare(b.sceneName);
                    }));
                }
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

Rules.prototype.execute = function(rId, finishCallback) {
    var uri = "/api/familyscene/execute";
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

Rules.prototype.switch = function(rId, enabled, finishCallback) {
    var uri = "/api/familyscene/switch";
    var body = '{"sceneId":"' + rId + '", ' + '"triggerEnabled":"' + enabled + '"}';
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
