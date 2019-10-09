/// @file main.js
/// @Brief
/// @author QRS
/// @version 0.0.9
/// @date 2019-01-29

var global = {
    COOKIE_NAME_TOKEN: "lerong_token",
    COOKIE_PATH: "/",

    DIV_LOADING: "page-loading",
    DIV_LOGIN: "page-login",
    DIV_GATEWAYS: "gateways",
    DIV_DEVICES: "devices",
    DIV_RULES: "rules",
    DIV_SCENES: "scenes",

    NAV_GATEWAYS: "nav-gateways",
    NAV_DEVICES: "nav-devices",
    NAV_RULES: "nav-rules",
    NAV_SCENES: "nav-scenes",

    RES_STATUS_OK: 1,
}

var events = {
    LOAD_INIT: 1,
    LOGIN_UI: 2,
    LOGIN_OK: 10,
    SHOW_GATEWAYS: 20,
    SHOW_DEVICES: 21,
    SHOW_RULES: 22,
    SHOW_SCENES: 23,
}

var sysinfo = null;

var testscenes = [
    {"sceneId": "0", "sceneName": "none", "icon": "ic_default_scene"},
    {"sceneId": "1", "sceneName": "comehome", "icon": "ic_icons_home"},
    {"sceneId": "2", "sceneName": "leavehome", "icon": "ic_icons_leave"},
    {"sceneId": "3", "sceneName": "wakeup", "icon": "ic_icons_getup"},
    {"sceneId": "4", "sceneName": "gotosleep", "icon": "ic_icons_sleep"},
    {"sceneId": "5", "sceneName": "reading", "icon": "ic_icons_reading"},
    {"sceneId": "6", "sceneName": "seemovie", "icon": "ic_icons_movie"},
    {"sceneId": "7", "sceneName": "soshelp", "icon": "ic_icons_soshelp"},
];

var ui = new UI();
var auth = new Auth(ui);
var gateways = new Gateways(ui);
var devices = new Devices(ui);
var rules = new Rules(ui, devices, testscenes);
var scenes = new Scenes(ui, devices, rules, testscenes);

function getSysinfo(force) {
    if (sysinfo && !force)
        return sysinfo;

    httpRequest("GET", "/api/sysinfo", null, function(responseText, error){
        if (error && error.status != 200) {
            console.log(error);
            return;
        }
        try {
            var responseData = JSON.parse(responseText);
            if (responseData["status"] == global.RES_STATUS_OK) {
                sysinfo = responseData["data"];
            }
        } catch (err) {
            console.log(err);
        }
    });
    return null;
}

function handleMessage(msg) {
    switch(msg.what) {
        case events.LOAD_INIT:
            auth.tokenCheck();
            break;
        case events.LOGIN_UI:
            auth.initUI();
            break;
        case events.SHOW_GATEWAYS:
            gateways.initUI();
            break;
        case events.LOGIN_OK:
            getSysinfo(true);
        case events.SHOW_DEVICES:
            devices.initUI();
            break;
        case events.SHOW_RULES:
            rules.initUI();
            break;
        case events.SHOW_SCENES:
            scenes.initUI();
            break;
        default:
            ;
    }
}

var queue = new Queue(handleMessage);
var intervalId = setInterval( function() {
    queue.processMessage();
}, 300);

function sendMessage(what, arg1, arg2, cb) {
    queue.sendMessage({
        what: what,
        arg1: arg1,
        arg2: arg2,
        callback: cb
    });
}

function init() {
    console.log("init");
    sendMessage(events.LOAD_INIT);
}

function submitLogin() {
    auth.submitLogin();
}

function logout() {
    auth.logout();
}

function showGateways() {
    sendMessage(events.SHOW_GATEWAYS);
}

function openGatewayBindingModal() {
    gateways.onSelect();
}

function onGatewayDelete(bridgeId) {
    gateways.onDelete(bridgeId);
}

function onSelectGateway(type) {
    gateways.onAddOrEdit(type, null);
}

function onEditGateway(type, gatewayId) {
    gateways.onAddOrEdit(type, gatewayId);
}

function onGatewayAddSubmit(type) {
    gateways.addSubmit(type);
}

function onChangeGatewayInfo() {
    gateways.displayErrorMessage("");
}

function onGatewayNet(bridgeId) {
    gateways.onNet(bridgeId);
}

function onToggleBridgeSwitch(bridgeId) {
    gateways.toggleSwitchBinary(bridgeId);
}

function onRefreshBridgeSubDevices(bridgeId) {
    gateways.onRefresh(bridgeId);
}

function showDevices() {
    sendMessage(events.SHOW_DEVICES);
}

function openOnboardingModal() {
    devices.discovery();
}

function onBindDevice(tid, did) {
    devices.bind(tid, did);
}

function onUnbindDevice(did) {
    devices.unbind(did);
}

function onDeleteDevice(did) {
    devices.delete(did);
}

function onChangeDeviceName(did, value) {
    devices.changeName(did, value);
}

function onToggleContainer(id) {
    if (ui.activeNavId == global.NAV_GATEWAYS)
        gateways.toggleExpandBody(id);
    else if (ui.activeNavId == global.NAV_DEVICES)
        devices.toggleExpandBody(id);
    else
        rules.toggleExpandBody(id);
}

function onToggleDeviceSwitch(did, propName, keyId) {
    devices.toggleSwitchBinary(did, propName, keyId);
}

function onToggleRuleSwitch(rid, keyId) {
    rules.toggleSwitchBinary(rid, keyId);
}

function updateSaturationSliderBackground(did, hueValue, hueMax) {
    devices.updateSaturationSliderBG(did, hueValue, hueMax);
}

function onChangeName(did, name) {
    devices.changeName(did, name);
}

function onChangeProperty(did, propName, value) {
    if (ui.activeNavId == global.NAV_DEVICES)
        devices.operate(did, propName, Number(value));
    else if (ui.activeNavId == global.NAV_RULES)
        rules.changeProperty(did, propName, Number(value));
    else if (ui.activeNavId == global.NAV_SCENES)
        scenes.changeProperty(did, propName, Number(value));
    else
        console.log("error onchange property");
}

function showRules() {
    sendMessage(events.SHOW_RULES);
}

function openRuleCreationModal() {
    rules.onCreate();
}

function onRuleEditModalClicked(eId) {
    if (ui.activeNavId == global.NAV_RULES)
        rules.onEditModal(eId);
    else
        scenes.onEditModal(eId);
}

function onRuleModify(idx) {
    if (ui.activeNavId == global.NAV_RULES)
        rules.onEdit(idx);
    else
        scenes.onEdit(idx);
}

function onRuleDelete(rId) {
    if (ui.activeNavId == global.NAV_RULES)
        rules.onDelete(rId);
    else
        scenes.onDelete(rId);
}

function onRuleExecute(rId, key) {
    if (ui.activeNavId == global.NAV_RULES)
        rules.onExecute(rId);
    else
        scenes.onExecute(rId, key);
}

function onSaveRuleChanges() {
    if (ui.activeNavId == global.NAV_RULES)
        rules.saveChanges();
    else
        scenes.saveChanges();
}

function onToggleTriggerEnable(key) {
    rules.toggleEnable(key);
}

function onChangeRuleInfo(what, value) {
    rules.changeInfo(what, value);
}

function onDeviceSelected(did, check) {
    if (ui.activeNavId == global.NAV_RULES)
        rules.deviceSelected(did, check);
    else
        scenes.deviceSelected(did, check);
}

function onSceneSelected(scene) {
    rules.sceneSelected(scene);
}

function onPropertySelected(did, propName, check) {
    if (ui.activeNavId == global.NAV_RULES)
        rules.propertySelected(did, propName, check);
    else
        scenes.propertySelected(did, propName, check);
}

function onManualRuleSelected(rid, rname, check) {
    if (ui.activeNavId == global.NAV_RULES)
        rules.manualRuleSelected(rid, check);
    else
        scenes.ruleSelected(rid, rname, check);
}

function onSymbolSelected(did, propName, symbol) {
    if (ui.activeNavId == global.NAV_RULES)
        rules.symbolSelected(did, propName, symbol);
    else
        scenes.symbolSelected(did, propName, symbol);
}

function onToggleCondition(divId) {
    rules.toggleCondition(divId);
}

function onToggleSceneEvent(divId) {
    scenes.toggleSceneEvent(divId);
}

function onSceneEnableRule(rId, check) {
    scenes.enableRule(rId, check);
}

function onConditionLogic(type, logic) {
    rules.conditionLogicSelected(type, logic);
}

function onRuleTime(idx, what, value) {
    rules.timeSetting(idx, what, value);
}

function onTimeConditionAdd() {
    rules.timeConditionAdd();
}

function onTimeConditionDel(idx) {
    rules.timeConditionDel(idx);
}

function showScenes() {
    sendMessage(events.SHOW_SCENES);
}

function onMicroServiceAdd(sid) {
    scenes.microServiceAdd(sid);
}

function onMicroServiceDel(sid) {
    scenes.microServiceDel(sid);
}

function onMicroServiceParams(idx, divId, value) {
    scenes.microServiceParams(idx, divId, value);
}

function onMicroServiceDepends(idx, divId, value) {
    scenes.microServiceDepends(idx, divId, value);
}
