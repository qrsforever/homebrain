;=================================================================
; date: 2018-06-14 19:12:27
; title: super type device
; author: QRS
;=================================================================

(defclass oic.d.envsensor (is-a DEVICE) (role abstract))

(defclass oic.d.light (is-a DEVICE) (role abstract))

(defclass oic.d.tv (is-a DEVICE) (role abstract))

(defclass oic.d.lightctrl (is-a DEVICE) (role abstract))

(defclass oic.d.scenectrl (is-a DEVICE) (role abstract))

(defclass oic.d.curtainctrl (is-a DEVICE) (role abstract))

(defclass oic.d.curtainmotor (is-a DEVICE) (role abstract))

(defclass oic.d.alarmer (is-a DEVICE) (role abstract))

(defclass oic.d.sosalarm (is-a DEVICE) (role abstract))

(defclass oic.d.boolsensor (is-a DEVICE) (role abstract))
