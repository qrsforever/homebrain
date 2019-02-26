var Auth = function(ui) {
    this.token = "";
    this.user = "";
    this.ui = ui;
    that_a = this;
}

Auth.prototype.tokenCheck = function() {
    this.userinfo(function(responseText, error) {
        if (error && error.status != 200) {
            sendMessage(events.LOGIN_UI);
            return;
        }
        console.log("userinfo: %s", responseText);
        try {
            var responseData = JSON.parse(responseText);
            if (!responseData["logged_in"])
                registrationLink.style.visibility = "visible";
            that_a.user = responseData["user"];
        } catch (ignore) {}

        var tokenInfo = loadSessionCookie();
        if (tokenInfo && tokenInfo.token) {
            that_a.token = tokenInfo.token;
            that_a.ui.enableNavigation(that_a.user, that_a.token);
            sendMessage(events.LOGIN_OK);
            return;
        }
        sendMessage(events.LOGIN_UI);
    });
}

Auth.prototype.initUI = function() {
    console.log("Auth initUI");

    var form = document.getElementById("login-form");
    var usernameInput = document.getElementById("login-username");
    var passwordInput = document.getElementById("login-password");
    var messagebox = document.getElementById("login-errormsg");
    var registrationLink = document.getElementById("login-registration-link");

    form.reset();
    messagebox.style.display = "none";
    registrationLink.style.visibility = "hidden";

    this.ui.displayDiv(global.DIV_LOGIN);
}

Auth.prototype.submitLogin = function() {
    console.log("Auth submitLogin");
    var submitButton = document.getElementById("login-submit");
    var usernameInput = document.getElementById("login-username");
    var passwordInput = document.getElementById("login-password");
    var messagebox = document.getElementById("login-errormsg");

    submitButton.disabled = true;
    usernameInput.disabled = true;
    passwordInput.disabled = true;
    messagebox.style.display = "none";

    this.login(usernameInput.value, passwordInput.value, function(responseText, error) {
        try {
            submitButton.disabled = false;
            usernameInput.disabled = false;
            passwordInput.disabled = false;
            if (error) {
                switch (error.status) {
                    case 401:
                        messagebox.textContent = "{{login_invalid}}"
                        break;
                    case 412:
                        messagebox.textContent = "{{login_wrong_user}}"
                        break;
                    default:
                        messagebox.textContent = "{{login_internal_failure}}"
                }
            } else {
                var responseData = JSON.parse(responseText);
                that_a.token = responseData["token"];
                that_a.user = usernameInput.value;
                that_a.ui.enableNavigation(that_a.user, that_a.token);
                saveSessionCookie(that_a.user, that_a.token);
                sendMessage(events.LOGIN_OK);
            }
        } catch (err) {
            messagebox.textContent = "{{login_internal_failure}}"
        }
    });
}

Auth.prototype.userinfo = function(callback) {
    httpRequest("GET", "/api/authentication/userinfo", null, callback);
}

Auth.prototype.login = function(username, passwd, callback) {
    var body = urlencode({"user": username, "pwd": passwd})
    httpRequest("POST", "/api/authentication/login", body, callback);
}

Auth.prototype.logout = function(callback) {
    httpRequest("POST", "/api/authentication/logout", "", null);
    this.token = "";
    this.user = "";
    this.ui.enableNavigation(null, null);
    deleteSessionCookie();
    sendMessage(events.LOGIN_UI);
}
