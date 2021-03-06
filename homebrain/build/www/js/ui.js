/// @file ui.js
/// @Brief
/// @author QRS
/// @version 0.0.9
/// @date 2019-01-29

var UI = function() {
    this.targetPage = "";
    this.lastBasePage = "";
    this.activePage = "";
    this.activeNavId = "";
    this.activeDivId = "";
    this.onModalClosedCallback = null;
    this.onModalConfirmationCallback = null;
    this.argsForConfirmation = null;
    this.modalContainer  = null;
    this.onPageChangedCallback = null;
}

UI.prototype.displayDiv = function (divId) {
    if (divId !== this.activeDivId) {
        if (this.activeDivId) {
            var previous = document.getElementById(this.activeDivId);
            if (previous)
                previous.style.display = "none";
        }
        if (divId) {
            var next = document.getElementById(divId);
            if (next)
                next.style.display = "";
            this.activeDivId = divId;
        }
    }
}

UI.prototype.displayNav = function(navId, leaveCallback) {
    this.closeModal();
    if (navId !== this.activeNavId) {
        if (this.activeNavId) {
            removeClass(this.activeNavId, "nav-links-active");
            addClass(this.activeNavId, "nav-links");
            if (this.onPageChangedCallback)
                this.onPageChangedCallback();
        }
        removeClass(navId, "nav-links");
        addClass(navId, "nav-links-active");
        this.activeNavId = navId;
        this.onPageChangedCallback = leaveCallback;
    }
    switch (navId) {
        case global.NAV_GATEWAYS:
            this.displayDiv(global.DIV_GATEWAYS);
            break;
        case global.NAV_DEVICES:
            this.displayDiv(global.DIV_DEVICES);
            break;
        case global.NAV_RULES:
            this.displayDiv(global.DIV_RULES);
            break;
        case global.NAV_SCENES:
            this.displayDiv(global.DIV_SCENES);
            break;
        default:
            ;
    }
}

UI.prototype.showLoadingIndicator = function(text) {
    text = text || "";
    document.getElementById("page-loading-text").textContent = text;
    this.displayDiv(global.DIV_LOADING);
}

UI.prototype.enableNavigation = function (user, token)  {
    var navRoot = document.getElementById("nav-root");
    if (token) {
        navRoot.style.visibility = "visible";
        var dropdownUsernameElement = document.getElementById("dropdown-username");
        if (dropdownUsernameElement) {
            dropdownUsernameElement.textContent = user;
        }
        return
    }
    navRoot.style.visibility = "hidden";
}

UI.prototype.openModal = function (bodyHtml, closedCallback, unclosable) {
    this.closeModal();
    this.modalContainer = document.createElement("div");
    this.modalContainer.id = "modal-wall";
    this.modalContainer.tabIndex = -1;

    if (closedCallback)
        this.onModalClosedCallback = closedCallback;

    var html = "";
    html += '<div class="modal">';

    if (!unclosable) {
        html += '<button type="button" class="modal-close-button" onclick="ui.closeModal()">';
        html += '    &times';
        html += '</button>';
    }

    html += '    <div class="modal-content">';
    html +=          bodyHtml;
    html += '    </div>';
    html += '</div>';
    this.modalContainer.innerHTML = html;
    document.body.style.overflow = "hidden";
    document.body.appendChild(this.modalContainer);
    this.modalContainer.focus();
}

UI.prototype.closeModal = function() {
    if (this.modalContainer) {
        document.body.style.overflow = "";
        document.body.removeChild(this.modalContainer);
        if (this.onModalClosedCallback) {
            this.onModalClosedCallback();
        }
        this.modalContainer = null;
    }
    this.onModalClosedCallback = null;
}

UI.prototype.openConfirmationModal = function (title, message, callbackFunction) {
    if (callbackFunction) {
        this.onModalConfirmationCallback = callbackFunction
        this.argsForConfirmation = Array.prototype.slice.call(arguments, 3);
    }
    var html = "";
    html += '<h3 class="modal-title">';
    html +=      title;
    html += '</h3>';

    if (message) {
        html += '<p class="modal-info-text">';
        html +=      message;
        html += '</p>';
    }

    html += '<div class="modal-buttons">';
    html += '    <button class="btn-default" onclick="ui.closeModal()" type="button">';
    html +=          "{{no}}";
    html += '    </button>';
    html += '    <button class="btn-primary" onclick="ui.onConfirmationModalConfirmed()" type="button">';
    html +=          "{{yes}}";
    html += '    </button>';
    html += '</div>';
    this.openModal(html);
}

UI.prototype.onConfirmationModalConfirmed = function () {
    this.closeModal();
    this.onModalConfirmationCallback.apply(undefined, this.argsForConfirmation);
}

UI.prototype.buildGenericErrorModal = function (title, info) {
    var html = "";
    html += '<h3 class="modal-title">';
    if (!title)
        html +=  "{{onboarding_title}}";
    else
        html +=  title;
    html += '</h3>';
    html += '<p class="modal-info-text">';
    if (!info)
        html +=  "{{internal_error}}";
    else
        html +=      info;
    html += '</p>';
    this.openModal(html);
}

UI.prototype.setButtonClickable = function (id, value) {
    var obj = document.getElementById(id);
    if (obj) {
        if (value)
            obj.disabled = false;
        else
            obj.disabled = true;
    }
}

UI.prototype.notImplAlert = function (str) {
    if (str)
        alert(str);
    else
        alert("TODO I'll implement later!");
}
