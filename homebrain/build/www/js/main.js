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

    NAV_GATEWAYS: "nav-gateways",
    NAV_DEVICES: "nav-devices",
    NAV_RULES: "nav-rules",

    RES_STATUS_OK: 1,
}

var events = {
    LOAD_INIT: 1,
    LOGIN_UI: 2,
    LOGIN_OK: 10,
    SHOW_GATEWAYS: 20,
    SHOW_DEVICES: 21,
    SHOW_RULES: 22,
}

var ui = new UI();
var auth = new Auth(ui);
var gateways = new Gateways(ui);
var devices = new Devices(ui);
var rules = new Rules(ui, devices);

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
        case events.SHOW_DEVICES:
        case events.LOGIN_OK:
            devices.initUI();
            break;
        case events.SHOW_RULES:
            rules.initUI();
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
    gateways.onAdd(type);
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

function onChangeProperty(did, propName, value) {
    if (ui.activeNavId == global.NAV_DEVICES)
        devices.operate(did, propName, Number(value));
    else
        rules.changeProperty(did, propName, Number(value));
}

function showRules() {
    sendMessage(events.SHOW_RULES);
}

function openRuleCreationModal() {
    rules.onCreate();
}

function onRuleEditModalClicked(eId) {
    rules.onEditModal(eId);
}

function onRuleModify(idx) {
    rules.onEdit(idx);
}

function onRuleDelete(rId) {
    rules.onDelete(rId);
}

function onRuleExecute(rId) {
    rules.onExecute(rId);
}

function onSaveRuleChanges() {
    rules.saveChanges();
}

function onToggleTriggerEnable(key) {
    rules.toggleEnable(key);
}

function onChangeRuleInfo(what, value) {
    rules.changeInfo(what, value);
}

function onDeviceSelected(did, check) {
    rules.deviceSelected(did, check);
}

function onPropertySelected(did, propName, check) {
    rules.propertySelected(did, propName, check);
}

function onManualRuleSelected(rid, check) {
    rules.manualRuleSelected(rid, check);
}

function onSymbolSelected(did, propName, symbol) {
    rules.symbolSelected(did, propName, symbol);
}

function onToggleCondition(divId) {
    rules.toggleCondition(divId);
}

function onConditonLogic(type, logic) {
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
