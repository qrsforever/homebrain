;=================================================================
; date: 2018-06-14 19:11:27
; title: All Global Construct
;=================================================================

;-----------------------------------------------------------------
;    Global varible
;       1. Use ?*var* to access the value of global varible
;       2. Use bind function to set the value of global varible.
;-----------------------------------------------------------------

; Salience Level Define
(defglobal
    ?*SALIENCE-HIGHEST*  =  10000
    ?*SALIENCE-HIGHER*   =   5000
    ?*SALIENCE-HIGH*     =   1000
    ?*SALIENCE-NORMAL*   =      0
    ?*SALIENCE-LOW*      =  -1000
    ?*SALIENCE-LOWER*    =  -5000
    ?*SALIENCE-LOWEST*   = -10000
)

; Log Level Define: must be consisence with defined in Log.h
(defglobal
    ?*LOG-LEVEL-ERROR*      = 1
    ?*LOG-LEVEL-WARNING*    = 2
    ?*LOG-LEVEL-DEBUG*      = 3
    ?*LOG-LEVEL-INFO*       = 4
    ?*LOG-LEVEL-TRACE*      = 5
)

; varibles: rule engine
(defglobal
    ?*RULE-TIMEOUT-MS*      = 3000
    ?*RULE-RETRY-COUNT*     = 1
)

;-----------------------------------------------------------------
;   Global Template
;-----------------------------------------------------------------

; scene
(deftemplate scene
    (slot where  (type SYMBOL) (default node))
    (slot target (type SYMBOL) (default none))
    (slot follow (type SYMBOL) (default node))
    (slot action (type SYMBOL) (allowed-symbols enter exit none) (default none))
)

; micro service
(deftemplate service
    (slot name  (type SYMBOL))
    (slot state (type SYMBOL) (allowed-symbols start stop running) (default stop))
    (multislot saves (type NUMBER SYMBOL))
    (multislot args  (type NUMBER SYMBOL))
    (slot ntime (type INTEGER) (default 0))
)

; timer event
(deftemplate timer-event
    (slot id)
)

;-----------------------------------------------------------------
;    Global Class
;-----------------------------------------------------------------

(defclass Context (is-a USER))

; Base Device Abstract
(defclass DEVICE (is-a USER) (role abstract)
    (slot ID     (visibility public) (type SYMBOL))
    (slot Class  (visibility public) (type SYMBOL) (access initialize-only))
    (slot UUID   (visibility public) (type STRING))
)

(defmessage-handler DEVICE init after ()
    (bind ?self:ID (instance-name-to-symbol (instance-name ?self)))
    (bind ?self:Class (class ?self))
)

; Sub Device define slot must specify facet: (visibility public)
(defmessage-handler DEVICE putData (?slot ?value)
    (bind ?old (nth$ 1 (dynamic-get ?slot)))
    (dynamic-put ?slot ?value ?old)
)

(defmessage-handler DEVICE getData (?slot)
    (return (nth$ 1 (dynamic-get ?slot)))
)

;-----------------------------------------------------------------
;    Global Rule
;-----------------------------------------------------------------

; show facts, ruels, instances and so on debug info
(defrule show-elem
    ?f <- (show ?elem)
  =>
    (retract ?f)
    (switch ?elem
        (case instances then    (instances))
        (case facts     then    (facts))
        (case templates then    (list-deftemplates))
        (case rules     then    (rules))
        (case agenda    then    (agenda))
        (case classes   then    (list-defclasses))
        (case globals   then    (show-defglobals))
        (case memory    then    (printout info "Memory Used:"(/ (mem-used) 131072)" MB" crlf))
        (default (printout warn "Unkown elem: " ?elem crlf))
    )
)

; (datetime (now)) from program
(defrule retract-datetime
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (datetime $?)
  =>
    (if (>= ?*LOG-LEVEL* ?*LOG-LEVEL-TRACE*)
      then
        (facts)
    )
    (retract ?f)
)

; retract fact: (action-reponse id value)
(defrule retract-rule-response
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (rule-response ?id $?)
  =>
    (retract ?f)
)

; low salience for enter follow scene after prev scene exit
(defrule _rul-enter-scene
    (declare (salience ?*SALIENCE-LOW*))
    ?f <- (scene (action ?action &:(eq ?action exit)) (follow ?follow &:(neq ?follow none)))
  =>
    (modify ?f (target ?follow) (action enter) (follow none))
)

; trigger scene enter
(defrule _rul-switch-scene
    (declare (salience ?*SALIENCE-HIGH*))
    ?f <- (switch-scene ?where ?target)
  =>
    (retract ?f)
    (bind ?fscene (nth$ 1 (find-fact ((?sn scene)) (eq ?sn:where ?where))))
    (if (eq ?fscene nil)
     then
        (assert (scene (where ?where) (target ?target) (action enter)))
        (return)
    )
    (modify ?fscene (follow ?target) (action exit))
)

; trigger micro service
(defrule _rul-start-service
    (declare (salience ?*SALIENCE-HIGH*))
    ?f <- (start-service ?name $?args)
  =>
    (retract ?f)
    (bind ?s (nth$ 1 (find-fact ((?ms service))
                            (eq ?ms:name ?name))))

    (if (eq ?s nil)
     then
        (assert (service (name ?name) (state start) (args $?args)))
        (return)
    )
    (bind ?curr (fact-slot-value ?s state))
    (if (eq ?curr stop)
     then
        (modify ?s (state start) (args $?args))
     else
        (printout t "already start service: " ?name crlf)
    )
)

(defrule _rul-stop-service
    (declare (salience ?*SALIENCE-HIGH*))
    ?f <- (stop-service ?name)
  =>
    (retract ?f)
    (bind ?s (nth$ 1 (find-fact ((?ms service))
                            (eq ?ms:name ?name))))
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
)

; remove the timer-event
(defrule remove-timer-event
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (remove-timer-event ?id)
  =>
    (retract ?f)
    (bind ?fact (nth$ 1 (find-fact ((?t timer-event)) (eq ?t:id ?id))))
    (if (neq ?fact nil)
     then
        (retract ?fact)
    )
)
