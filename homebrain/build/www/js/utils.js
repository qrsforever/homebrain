/// @file utils.js
/// @Brief
/// @author QRS
/// @version 0.0.9
/// @date 2019-01-29

function trimString(val) {
    return val.replace(/^\s+|\s+$/g, '');
}

function Queue(handle) {
    var elems = [];
    var handle = handle;

    this.sendMessage = function(msg) {
        if (msg == null)
            return;
        elems.push(msg);
    }

    this.processMessage = function() {
        while (elems.length != 0) {
            var msg = elems.shift();
            if (msg.callback)
                msg.callback(msg);
            else
                handle(msg);
        }
    }
}

function urlencode(propertyValueMap) {
    var args = [];

    var keys = Object.keys(propertyValueMap);
    for (var i = 0; i < keys.length; i++) {
        var key = keys[i];
        var value = propertyValueMap[key];
        if (Array.isArray(value))
            value = value.join(",");
        var str = encodeURIComponent(key) + "=" + encodeURIComponent(value);
        args.push(str);
    }
    return args.join("&");
}

function httpRequest(method, uri, body, responseCallback, async) {
    var xhr;
    try {
        xhr = new XMLHttpRequest();
    } catch (e3) {
        responseCallback(undefined, {
            status: 0,
            message: "XMLHttpRequest not supported",
            body: "",
        });
        return;
    }

    xhr.onreadystatechange = function() {
        if(xhr.readyState == 4) {
            if(xhr.status == 200) {
                if (responseCallback != null) {
                    responseCallback(xhr.responseText, undefined);
                }
            } else {
                if (xhr.status == 401) {
                    try {
                        console.log("TODO here!");
                    } catch (ignore) {}
                }
                if (responseCallback != null) {
                    responseCallback(undefined, {
                        status: xhr.status,
                        message: xhr.statusText,
                        body: xhr.responseText,
                    });
                }
            }
        }
    };

    xhr.open(method, uri, true);
    xhr.responseType = "text";

    if (global.token) {
        xhr.setRequestHeader("X-App-Token", global.token);
    }

    xhr.send(body);
}

function addClass(idName, className) {
    if (!idName) {
        return;
    }
    try {
        var elem = document.getElementById(idName);
        if (elem == null) return;
        var elemClass = elem.className || elem.getAttribute('class') || '';
        var separator = elemClass && elemClass.length > 0 ? ' ' : '';

        if (!elemClass || elemClass == '' || !elemClass.match(className)) {
            if (elem.className) {
                elem.className += separator + className;
            }
            else {
                elem.setAttribute('class', elemClass + separator + className);
            }
        }
    } catch (e) {
    }
}

function removeClass(idName, className) {
    if (!idName) {
        return;
    }
    try {
        var elem = document.getElementById(idName);
        if (elem == null) return;
        var elemClass = elem.className || elem.getAttribute('class') || '';
        var classList = elemClass.split(' ');
        var newClass = '';

        for (var i = 0; i < classList.length; i++) {
            if (!(classList[i] == className)) {
                newClass += classList[i] + ' ';
            }
        }

        newClass = trimString(newClass);

        if (elem.className) {
            elem.className = newClass;
        } else {
            elem.setAttribute('class', newClass);
        }
    } catch (e) {
    }
}

function hasClass(idName, className) {
    if (!idName) {
        return;
    }
    try {
        var elem = document.getElementById(idName);
        if (elem == null) return;

        var classes = elem.className || elem.getAttribute('class') || '';
        var classList = classes.split(' ');

        if (classList.indexOf(className) >= 0) {
            return true;
        } else {
            return false;
        }
    } catch (ignore) {}
}

function loadSessionCookie() {
    var cookies = document.cookie.split(";");
    for (var i=0; i<cookies.length; i++) {
        var cookie = trimString(cookies[i]);
        if (cookie.indexOf(global.COOKIE_NAME_TOKEN + "=") !== 0) {
            continue;
        }
        var cookieValueStr = cookie.substr(global.COOKIE_NAME_TOKEN.length + 1);
        try {
            var info = JSON.parse(decodeURIComponent(cookieValueStr));
            if (info["username"] && info["token"]) {
                return {
                    username: "" + info["username"],
                    token: "" + info["token"],
                };
            }
        } catch (err) {
        }
    }
    return null;
}

function saveSessionCookie(username, token) {
    var info = JSON.stringify({"username": username, "token": token});

    var expirationDate = new Date();
    expirationDate.setMonth(expirationDate.getMonth() + 1);

    document.cookie = global.COOKIE_NAME_TOKEN + "=" + encodeURIComponent(info)
        + "; expires=" + expirationDate.toUTCString()
        + "; path=" + global.COOKIE_PATH
        + (location.protocol === "https:" ? "; secure" : "");
}

function deleteSessionCookie() {
    document.cookie = global.COOKIE_NAME_TOKEN + "="
        + "; expires=Thu, 01 Jan 1970 00:00:00 GMT"
        + "; path=" + global.COOKIE_PATH;
}

function updateElementText(key, value) {
    var textElem = document.getElementById(key);
    if (textElem)
        textElem.textContent = value;
}

function updateElementValue(key, value) {
    var inputElem = document.getElementById(key);
    if (inputElem)
        inputElem.value = value;
}

function getIconUrl(iconId) {
    return "assets/icons/" + iconId + ".png";
}

function parseRange(str) {
    var intReg = /v\s*>[=]\s*(\d+)\s*and\s*v\s*<[=]\s*(\d+)/g;
    var result = intReg.exec(str);
    if (result)
        return [result[1], result[2]];
    return [0, 0];
}
