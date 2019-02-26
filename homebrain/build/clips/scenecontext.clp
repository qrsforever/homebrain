;=================================================================
; date: 2018-08-13 21:33:39
; title: scenecontext
;=================================================================

; Scene Context
(defclass SceneContext (is-a Context) (role concrete)
    (slot stime  (type INTEGER))
    (slot where  (type SYMBOL) (default none))
    (slot target (type SYMBOL) (default none))
    (slot follow (type SYMBOL) (default none))
    (slot action (type SYMBOL) (allowed-symbols enter story exit none) (default none))
    (multislot services (type SYMBOL))
)

; micro service
(deftemplate service
    (slot master (type INSTANCE-ADDRESS) (allowed-classes SceneContext))
    (slot name   (type SYMBOL))
    (slot skey   (type NUMBER SYMBOL) (default none))
    (slot state  (type SYMBOL) (allowed-symbols start stop running none) (default none))
    (slot ntime  (type INTEGER) (default 0))
    (multislot saves (type NUMBER SYMBOL))
    (multislot args  (type NUMBER SYMBOL))
)

(defmessage-handler SceneContext init after ()
    (logd "SceneContext init")
)

(defmessage-handler SceneContext delete before ()
    (logd "SceneContext delete")
)

(defmessage-handler SceneContext switch-scene-exit (?where ?target)
    (logd "SceneContext switch-scene-exit: " ?where ", " ?target)
    (dynamic-put where ?where)
    (dynamic-put follow ?target)
    (dynamic-put action exit)
    (return TRUE)
)

(defmessage-handler SceneContext switch-scene-enter (?where ?target)
    (logd "SceneContext switch-scene-enter: " ?where ", " ?target)
    (dynamic-put stime (nth$ 1 (now)))
    (dynamic-put where ?where)
    (dynamic-put target ?target)
    (dynamic-put follow none)
    (dynamic-put action enter)
    (return TRUE)
)

(defmessage-handler SceneContext add-service (?name)
    (logd "SceneContext add-service: " ?name)
    (bind ?svs (dynamic-get services))
    (if (neq ?svs FALSE)
     then
        (bind ?idx (member$ ?name ?svs))
        (if (eq ?idx FALSE)
         then
            (dynamic-put services (insert$ ?svs 1 ?name))
        )
    )
    (return TRUE)
)

(defmessage-handler SceneContext del-service (?name)
    (logd "SceneContext del-service: " ?name)
    (bind ?svs (dynamic-get services))
    (if (neq ?svs FALSE)
     then
        (bind ?idx (member$ ?name ?svs))
        (if (neq ?idx FALSE)
         then
            (dynamic-put services (delete$ ?svs ?idx ?idx))
        )
    )
    (return TRUE)
)

; trigger scene enter
(defrule _rul-switch-scene
    (declare (salience ?*SALIENCE-HIGH*))
    ?f <- (switch-scene ?where ?target)
  =>
    (retract ?f)
    (logi "switch-scene(where: " ?where ", target: " ?target)
    (bind ?sct (nth$ 1 (find-instance ((?sn SceneContext)) (eq ?sn:where ?where))))
    (if (eq ?sct nil)
     then
        (logi "make-instance of SceneContext")
        (make-instance of SceneContext (stime (nth$ 1 (now))) (where ?where) (target ?target) (action enter))
        (return)
    )
    (send ?sct switch-scene-exit ?where ?target)
)

; low salience for enter follow scene after prev scene exit
(defrule _rul-exit-and-enter-scene
    (declare (salience ?*SALIENCE-LOWEST*))
    ?sct <- (object (is-a SceneContext)
        (action exit) (where ?where) (target ?target)(follow ?follow)
    )
  =>
    (logi "scene from " ?target " to " ?follow)
    (if (neq ?follow none)
     then
        (send ?sct switch-scene-enter ?where ?follow)
    )
)

(defrule _rul-story-scene
    (declare (salience ?*SALIENCE-LOWEST*))
    ?sct <- (object (is-a SceneContext) (action enter))
  =>
    (send ?sct put-action story)
)

; trigger micro service
(defrule _rul-start-service
    (declare (salience ?*SALIENCE-HIGH*))
    ?f <- (start-service ?master ?name $?args)
  =>
    (retract ?f)
    (logd "start-service: " ?name ", master: " ?master)
    (bind ?pos (str-index : ?name))
    (if (and (neq ?pos FALSE) (> ?pos 1))
     then
        (bind ?nn (string-to-field (sub-string 1 (- ?pos 1) ?name)))
        (bind ?ss (string-to-field (sub-string (+ ?pos 1) (str-length ?name) ?name)))
     else
        (bind ?nn ?name)
        (bind ?ss none)
    )
    (bind ?s (nth$ 1 (find-fact ((?ms service))
                            (and (eq ?ms:name ?nn)(eq ?ms:skey ?ss)))))
    (if (eq ?s nil)
     then
        (logd "new service: " ?name)
        (assert (service (master ?master) (name ?nn) (skey ?ss) (state start) (args $?args)))
     else
        (bind ?curr (fact-slot-value ?s state))
        (if (eq ?curr stop)
         then
            (logd "start service: " ?name)
            (modify ?s (master ?master) (state start) (args $?args))
         else
            (logd "already start service: " ?name)
            (return)
        )
    )
    (if (instancep ?master)
     then
        (send ?master add-service ?name)
    )
    ; time factor may be necessary for service, so refresh time.
    (assert (datetime (now)))
)

(defrule _rul-stop-service
    (declare (salience ?*SALIENCE-HIGH*))
    ?f <- (stop-service ?name)
  =>
    (retract ?f)
    (logd "stop-service: " ?name)
    (bind ?pos (str-index : ?name))
    (if (and (neq ?pos FALSE) (> ?pos 1))
     then
        (bind ?nn (string-to-field (sub-string 1 (- ?pos 1) ?name)))
        (bind ?ss (string-to-field (sub-string (+ ?pos 1) (str-length ?name) ?name)))
     else
        (bind ?nn ?name)
        (bind ?ss none)
    )
    (bind ?s (nth$ 1 (find-fact ((?ms service))
                            (and (eq ?ms:name ?nn)(eq ?ms:skey ?ss)))))
    (if (eq ?s nil)
     then
        (return)
    )
    (bind ?curr (fact-slot-value ?s state))
    (if (eq ?curr stop)
     then
        (return)
    )
    (modify ?s (state stop))
    ; time factor may be necessary for service, so refresh time.
    (assert (datetime (now)))
)

(defrule _rul-service-state-stop
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (service (state stop) (master ?master) (name ?name) (skey ?key))
  =>
    (retract ?f)
    (logi "retract service: " ?name ":" ?key ", master: " ?master)
    (if (instancep ?master)
     then
        (if (neq ?key none)
         then
            (send ?master del-service (sym-cat ?name : ?key))
         else
            (send ?master del-service ?name)
        )
    )
)
